/* This file is part of the KDE libraries

   Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2000 Stefan Schimanski <1Stein@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "khistorycombobox.h"

#include <kpixmapprovider.h>
#include <kstandardshortcut.h>

#include <QAbstractItemView>
#include <QApplication>
#include <QMenu>
#include <QWheelEvent>
#include <QComboBox>

class KHistoryComboBoxPrivate
{
public:
    KHistoryComboBoxPrivate(KHistoryComboBox *parent):
        q_ptr(parent) {}

    void init(bool useCompletion);
    void rotateUp();
    void rotateDown();

    /**
     * Resets the iterate index to -1
     */
    void reset();

    /**
     * Called from the popupmenu,
     * calls clearHistory() and emits cleared()
     */
    void clear();

    /**
     * Appends our own context menu entry.
     */
    void addContextMenuItems(QMenu *);

    /**
     * Used to emit the activated(QString) signal when enter is pressed
     */
    void simulateActivated(const QString &);

    /**
    * The text typed before Up or Down was pressed.
     */
    QString typedText;

    KPixmapProvider *pixmapProvider;
    KHistoryComboBox * const q_ptr;

    /**
     * The current index in the combobox, used for Up and Down
     */
    int currentIndex;

    /**
     * Indicates that the user at least once rotated Up through the entire list
     * Needed to allow going back after rotation.
     */
    bool rotated;

    Q_DECLARE_PUBLIC(KHistoryComboBox)
};

void KHistoryComboBoxPrivate::init(bool useCompletion)
{
    Q_Q(KHistoryComboBox);
    // Set a default history size to something reasonable, Qt sets it to INT_MAX by default
    q->setMaxCount(50);

    if (useCompletion) {
        q->completionObject()->setOrder(KCompletion::Weighted);
    }

    q->setInsertPolicy(KHistoryComboBox::NoInsert);
    currentIndex = -1;
    rotated = false;
    pixmapProvider = 0L;

    // obey HISTCONTROL setting
    QByteArray histControl = qgetenv("HISTCONTROL");
    if (histControl == "ignoredups" || histControl == "ignoreboth") {
        q->setDuplicatesEnabled(false);
    }

    q->connect(q, SIGNAL(aboutToShowContextMenu(QMenu*)), SLOT(addContextMenuItems(QMenu*)));
    q->connect(q, SIGNAL(activated(int)), SLOT(reset()));
    q->connect(q, SIGNAL(returnPressed(QString)), SLOT(reset()));
    // We want slotSimulateActivated to be called _after_ QComboBoxPrivate::_q_returnPressed
    // otherwise there's a risk of emitting activated twice (slotSimulateActivated will find
    // the item, after some app's slotActivated inserted the item into the combo).
    q->connect(q, SIGNAL(returnPressed(QString)), SLOT(simulateActivated(QString)), Qt::QueuedConnection);
}

// we are always read-write
KHistoryComboBox::KHistoryComboBox(QWidget *parent)
    : KComboBox(true, parent), d_ptr(new KHistoryComboBoxPrivate(this))
{
    Q_D(KHistoryComboBox);
    d->init(true);   // using completion
}

// we are always read-write
KHistoryComboBox::KHistoryComboBox(bool useCompletion,
                                   QWidget *parent)
    : KComboBox(true, parent), d_ptr(new KHistoryComboBoxPrivate(this))
{
    Q_D(KHistoryComboBox);
    d->init(useCompletion);
}

KHistoryComboBox::~KHistoryComboBox()
{
    Q_D(KHistoryComboBox);
    delete d->pixmapProvider;
}

void KHistoryComboBox::setHistoryItems(const QStringList &items)
{
    setHistoryItems(items, false);
}

void KHistoryComboBox::setHistoryItems(const QStringList &items,
                                       bool setCompletionList)
{
    QStringList insertingItems = items;
    KComboBox::clear();

    // limit to maxCount()
    const int itemCount = insertingItems.count();
    const int toRemove = itemCount - maxCount();

    if (toRemove >= itemCount) {
        insertingItems.clear();
    } else {
        for (int i = 0; i < toRemove; ++i) {
            insertingItems.pop_front();
        }
    }

    insertItems(insertingItems);

    if (setCompletionList && useCompletion()) {
        // we don't have any weighting information here ;(
        KCompletion *comp = completionObject();
        comp->setOrder(KCompletion::Insertion);
        comp->setItems(insertingItems);
        comp->setOrder(KCompletion::Weighted);
    }

    clearEditText();
}

QStringList KHistoryComboBox::historyItems() const
{
    QStringList list;
    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i) {
        list.append(itemText(i));
    }

    return list;
}

bool KHistoryComboBox::useCompletion() const
{
    return compObj();
}

void KHistoryComboBox::clearHistory()
{
    const QString temp = currentText();
    KComboBox::clear();
    if (useCompletion()) {
        completionObject()->clear();
    }
    setEditText(temp);
}

void KHistoryComboBoxPrivate::addContextMenuItems(QMenu *menu)
{
    Q_Q(KHistoryComboBox);
    if (menu) {
        menu->addSeparator();
        QAction *clearHistory = menu->addAction(QIcon::fromTheme("edit-clear-history"), q->tr("Clear &History"), q, SLOT(clear()));
        if (!q->count()) {
            clearHistory->setEnabled(false);
        }
    }
}

void KHistoryComboBox::addToHistory(const QString &item)
{
    Q_D(KHistoryComboBox);
    if (item.isEmpty() || (count() > 0 && item == itemText(0))) {
        return;
    }

    bool wasCurrent = false;
    // remove all existing items before adding
    if (!duplicatesEnabled()) {
        int i = 0;
        int itemCount = count();
        while (i < itemCount) {
            if (itemText(i) == item) {
                if (!wasCurrent) {
                    wasCurrent = (i == currentIndex());
                }
                removeItem(i);
                --itemCount;
            } else {
                ++i;
            }
        }
    }

    // now add the item
    if (d->pixmapProvider) {
        insertItem(0, d->pixmapProvider->pixmapFor(item, iconSize().height()), item);
    } else {
        insertItem(0, item);
    }

    if (wasCurrent) {
        setCurrentIndex(0);
    }

    const bool useComp = useCompletion();

    const int last = count() - 1; // last valid index
    const int mc = maxCount();
    const int stopAt = qMax(mc, 0);

    for (int rmIndex = last; rmIndex >= stopAt; --rmIndex) {
        // remove the last item, as long as we are longer than maxCount()
        // remove the removed item from the completionObject if it isn't
        // anymore available at all in the combobox.
        const QString rmItem = itemText(rmIndex);
        removeItem(rmIndex);
        if (useComp && !contains(rmItem)) {
            completionObject()->removeItem(rmItem);
        }
    }

    if (useComp) {
        completionObject()->addItem(item);
    }
}

bool KHistoryComboBox::removeFromHistory(const QString &item)
{
    if (item.isEmpty()) {
        return false;
    }

    bool removed = false;
    const QString temp = currentText();
    int i = 0;
    int itemCount = count();
    while (i < itemCount) {
        if (item == itemText(i)) {
            removed = true;
            removeItem(i);
            --itemCount;
        } else {
            ++i;
        }
    }

    if (removed && useCompletion()) {
        completionObject()->removeItem(item);
    }

    setEditText(temp);
    return removed;
}

// going up in the history, rotating when reaching QListBox::count()
//
// Note: this differs from QComboBox because "up" means ++index here,
// to simulate the way shell history works (up goes to the most
// recent item). In QComboBox "down" means ++index, to match the popup...
//
void KHistoryComboBoxPrivate::rotateUp()
{
    Q_Q(KHistoryComboBox);
    // save the current text in the lineedit
    // (This is also where this differs from standard up/down in QComboBox,
    // where a single keypress can make you lose your typed text)
    if (currentIndex == -1) {
        typedText = q->currentText();
    }

    ++currentIndex;

    // skip duplicates/empty items
    const int last = q->count() - 1; // last valid index
    const QString currText = q->currentText();

    while (currentIndex < last &&
            (currText == q->itemText(currentIndex) ||
             q->itemText(currentIndex).isEmpty())) {
        ++currentIndex;
    }

    if (currentIndex >= q->count()) {
        rotated = true;
        currentIndex = -1;

        // if the typed text is the same as the first item, skip the first
        if (q->count() > 0 && typedText == q->itemText(0)) {
            currentIndex = 0;
        }

        q->setEditText(typedText);
    } else {
        q->setCurrentIndex(currentIndex);
    }
}

// going down in the history, no rotation possible. Last item will be
// the text that was in the lineedit before Up was called.
void KHistoryComboBoxPrivate::rotateDown()
{
    Q_Q(KHistoryComboBox);
    // save the current text in the lineedit
    if (currentIndex == -1) {
        typedText = q->currentText();
    }

    --currentIndex;

    const QString currText = q->currentText();
    // skip duplicates/empty items
    while (currentIndex >= 0 &&
            (currText == q->itemText(currentIndex) ||
             q->itemText(currentIndex).isEmpty())) {
        --currentIndex;
    }

    if (currentIndex < 0) {
        if (rotated && currentIndex == -2) {
            rotated = false;
            currentIndex = q->count() - 1;
            q->setEditText(q->itemText(currentIndex));
        } else { // bottom of history
            currentIndex = -1;
            if (q->currentText() != typedText) {
                q->setEditText(typedText);
            }
        }
    } else {
        q->setCurrentIndex(currentIndex);
    }
}

void KHistoryComboBox::keyPressEvent(QKeyEvent *e)
{
    Q_D(KHistoryComboBox);
    int event_key = e->key() | e->modifiers();

    if (KStandardShortcut::rotateUp().contains(event_key)) {
        d->rotateUp();
    } else if (KStandardShortcut::rotateDown().contains(event_key)) {
        d->rotateDown();
    } else {
        KComboBox::keyPressEvent(e);
    }
}

void KHistoryComboBox::wheelEvent(QWheelEvent *ev)
{
    Q_D(KHistoryComboBox);
    // Pass to poppable listbox if it's up
    QAbstractItemView *const iv = view();
    if (iv && iv->isVisible()) {
        QApplication::sendEvent(iv, ev);
        return;
    }
    // Otherwise make it change the text without emitting activated
    if (ev->delta() > 0) {
        d->rotateUp();
    } else {
        d->rotateDown();
    }
    ev->accept();
}

void KHistoryComboBoxPrivate::reset()
{
    currentIndex = -1;
    rotated = false;
}

void KHistoryComboBox::setPixmapProvider(KPixmapProvider *prov)
{
    Q_D(KHistoryComboBox);
    if (d->pixmapProvider == prov) {
        return;
    }

    delete d->pixmapProvider;
    d->pixmapProvider = prov;

    // re-insert all the items with/without pixmap
    // I would prefer to use changeItem(), but that doesn't honor the pixmap
    // when using an editable combobox (what we do)
    if (count() > 0) {
        QStringList items(historyItems());
        clear();
        insertItems(items);
    }
}

void KHistoryComboBox::insertItems(const QStringList &items)
{
    Q_D(KHistoryComboBox);
    QStringList::ConstIterator it = items.constBegin();
    const QStringList::ConstIterator itEnd = items.constEnd();

    while (it != itEnd) {
        const QString item = *it;
        if (!item.isEmpty()) {   // only insert non-empty items
            if (d->pixmapProvider)
                addItem(d->pixmapProvider->pixmapFor(item, iconSize().height()),
                        item);
            else {
                addItem(item);
            }
        }
        ++it;
    }
}

void KHistoryComboBoxPrivate::clear()
{
    Q_Q(KHistoryComboBox);
    q->clearHistory();
    emit q->cleared();
}

void KHistoryComboBoxPrivate::simulateActivated(const QString &text)
{
    Q_Q(KHistoryComboBox);
    /* With the insertion policy NoInsert, which we use by default,
       Qt doesn't emit activated on typed text if the item is not already there,
       which is perhaps reasonable. Generate the signal ourselves if that's the case.
    */
    if ((q->insertPolicy() == q->NoInsert && q->findText(text, Qt::MatchFixedString | Qt::MatchCaseSensitive) == -1)) {
        emit q->activated(text);
    }

    /*
       Qt also doesn't emit it if the box is full, and policy is not
       InsertAtCurrent
    */
    else if (q->insertPolicy() != q->InsertAtCurrent && q->count() >= q->maxCount()) {
        emit q->activated(text);
    }
}

KPixmapProvider *KHistoryComboBox::pixmapProvider() const
{
    Q_D(const KHistoryComboBox);
    return d->pixmapProvider;
}

void KHistoryComboBox::reset()
{
    Q_D(KHistoryComboBox);
    d->reset();
}

#include "moc_khistorycombobox.cpp"
