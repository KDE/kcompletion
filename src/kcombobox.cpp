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

#include "kcombobox.h"

#include <kcompletionbox.h>
#include <klineedit.h>

#include <QUrl>
#include <QPointer>
#include <QMenu>

class KComboBoxPrivate
{
public:
    KComboBoxPrivate(KComboBox *parent)
        : q_ptr(parent)
    {
    }
    ~KComboBoxPrivate()
    {
    }

    /**
     * Initializes the variables upon construction.
     */
    void init();

    void _k_lineEditDeleted();

    KLineEdit *klineEdit = nullptr;
    bool trapReturnKey = false;
    KComboBox * const q_ptr;
    Q_DECLARE_PUBLIC(KComboBox)
};

void KComboBoxPrivate::init()
{
    Q_Q(KComboBox);
}

void KComboBoxPrivate::_k_lineEditDeleted()
{
    Q_Q(KComboBox);
    // yes, we need those ugly casts due to the multiple inheritance
    // sender() is guaranteed to be a KLineEdit (see the connect() to the
    // destroyed() signal
    const KCompletionBase *base = static_cast<const KCompletionBase *>(static_cast<const KLineEdit *>(q->sender()));

    // is it our delegate, that is destroyed?
    if (base == q->delegate()) {
        q->setDelegate(nullptr);
    }
}


KComboBox::KComboBox(QWidget *parent)
    : QComboBox(parent),
      d_ptr(new KComboBoxPrivate(this))
{
    Q_D(KComboBox);
    d->init();
}

KComboBox::KComboBox(bool rw, QWidget *parent)
    : QComboBox(parent),
      d_ptr(new KComboBoxPrivate(this))
{
    Q_D(KComboBox);
    d->init();
    setEditable(rw);
}

KComboBox::~KComboBox()
{
}

bool KComboBox::contains(const QString &text) const
{
    if (text.isEmpty()) {
        return false;
    }

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i) {
        if (itemText(i) == text) {
            return true;
        }
    }
    return false;
}

int KComboBox::cursorPosition() const
{
    return (isEditable()) ? lineEdit()->cursorPosition() : -1;
}

void KComboBox::setAutoCompletion(bool autocomplete)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        if (autocomplete) {
            d->klineEdit->setCompletionMode(KCompletion::CompletionAuto);
            setCompletionMode(KCompletion::CompletionAuto);
        } else {
            d->klineEdit->setCompletionMode(KCompletion::CompletionPopup);
            setCompletionMode(KCompletion::CompletionPopup);
        }
    }
}

bool KComboBox::autoCompletion() const
{
    return completionMode() == KCompletion::CompletionAuto;
}

#ifndef KCOMPLETION_NO_DEPRECATED
void KComboBox::setContextMenuEnabled(bool showMenu)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->setContextMenuPolicy(showMenu ? Qt::DefaultContextMenu : Qt::NoContextMenu);
    }
}

void KComboBox::setUrlDropsEnabled(bool enable)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->setUrlDropsEnabled(enable);
    }
}
#endif

bool KComboBox::urlDropsEnabled() const
{
    Q_D(const KComboBox);
    return d->klineEdit && d->klineEdit->urlDropsEnabled();
}

void KComboBox::setCompletedText(const QString &text, bool marked)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->setCompletedText(text, marked);
    }
}

void KComboBox::setCompletedText(const QString &text)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->setCompletedText(text);
    }
}

void KComboBox::makeCompletion(const QString &text)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->makeCompletion(text);
    }

    else { // read-only combo completion
        if (text.isNull() || !view()) {
            return;
        }

        view()->keyboardSearch(text);
    }
}

void KComboBox::rotateText(KCompletionBase::KeyBindingType type)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->rotateText(type);
    }
}

void KComboBox::setTrapReturnKey(bool trap)
{
    Q_D(KComboBox);
    d->trapReturnKey = trap;

    if (d->klineEdit) {
        d->klineEdit->setTrapReturnKey(trap);
    } else {
        qWarning("KComboBox::setTrapReturnKey not supported with a non-KLineEdit.");
    }
}

bool KComboBox::trapReturnKey() const
{
    Q_D(const KComboBox);
    return d->trapReturnKey;
}

void KComboBox::setEditUrl(const QUrl &url)
{
    QComboBox::setEditText(url.toDisplayString());
}

void KComboBox::addUrl(const QUrl &url)
{
    QComboBox::addItem(url.toDisplayString());
}

void KComboBox::addUrl(const QIcon &icon, const QUrl &url)
{
    QComboBox::addItem(icon, url.toDisplayString());
}

void KComboBox::insertUrl(int index, const QUrl &url)
{
    QComboBox::insertItem(index, url.toDisplayString());
}

void KComboBox::insertUrl(int index, const QIcon &icon, const QUrl &url)
{
    QComboBox::insertItem(index, icon, url.toDisplayString());
}

void KComboBox::changeUrl(int index, const QUrl &url)
{
    QComboBox::setItemText(index, url.toDisplayString());
}

void KComboBox::changeUrl(int index, const QIcon &icon, const QUrl &url)
{
    QComboBox::setItemIcon(index, icon);
    QComboBox::setItemText(index, url.toDisplayString());
}

void KComboBox::setCompletedItems(const QStringList &items, bool autosubject)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        d->klineEdit->setCompletedItems(items, autosubject);
    }
}

KCompletionBox *KComboBox::completionBox(bool create)
{
    Q_D(KComboBox);
    if (d->klineEdit) {
        return d->klineEdit->completionBox(create);
    }
    return nullptr;
}

QSize KComboBox::minimumSizeHint() const
{
    Q_D(const KComboBox);
    QSize size = QComboBox::minimumSizeHint();
    if (isEditable() && d->klineEdit) {
        // if it's a KLineEdit and it's editable add the clear button size
        // to the minimum size hint, otherwise looks ugly because the
        // clear button will cover the last 2/3 letters of the biggest entry
        QSize bs = d->klineEdit->clearButtonUsedSize();
        if (bs.isValid()) {
            size.rwidth() += bs.width();
            size.rheight() = qMax(size.height(), bs.height());
        }
    }
    return size;
}

void KComboBox::setLineEdit(QLineEdit *edit)
{
    Q_D(KComboBox);
    if (!isEditable() && edit &&
            !qstrcmp(edit->metaObject()->className(), "QLineEdit")) {
        // uic generates code that creates a read-only KComboBox and then
        // calls combo->setEditable(true), which causes QComboBox to set up
        // a dumb QLineEdit instead of our nice KLineEdit.
        // As some KComboBox features rely on the KLineEdit, we reject
        // this order here.
        delete edit;
        KLineEdit *kedit = new KLineEdit(this);

        if (isEditable()) {
            kedit->setClearButtonEnabled(true);
        }

        edit = kedit;
    }

    // reuse an existing completion object, if it does not belong to the previous
    // line edit and gets destroyed with it
    QPointer<KCompletion> completion = compObj();

    QComboBox::setLineEdit(edit);
    edit->setCompleter(nullptr); // remove Qt's builtin completer (set by setLineEdit), we have our own
    d->klineEdit = qobject_cast<KLineEdit *>(edit);
    setDelegate(d->klineEdit);

    if (completion && d->klineEdit) {
        d->klineEdit->setCompletionObject(completion);
    }

    // Connect the returnPressed signal for both Q[K]LineEdits'
    if (edit) {
        connect(edit, QOverload<>::of(&QLineEdit::returnPressed),
                this, QOverload<>::of(&KComboBox::returnPressed));
    }

    if (d->klineEdit) {
        // someone calling KComboBox::setEditable(false) destroys our
        // line edit without us noticing. And KCompletionBase::delegate would
        // be a dangling pointer then, so prevent that. Note: only do this
        // when it is a KLineEdit!
        connect(edit, SIGNAL(destroyed()), SLOT(_k_lineEditDeleted()));

        connect(d->klineEdit, QOverload<const QString&>::of(&KLineEdit::returnPressed),
                this, QOverload<const QString&>::of(&KComboBox::returnPressed));

        connect(d->klineEdit, &KLineEdit::completion,
                this, &KComboBox::completion);

        connect(d->klineEdit, &KLineEdit::substringCompletion,
                this, &KComboBox::substringCompletion);

        connect(d->klineEdit, &KLineEdit::textRotation,
                this, &KComboBox::textRotation);

        connect(d->klineEdit, &KLineEdit::completionModeChanged,
                this, &KComboBox::completionModeChanged);

        connect(d->klineEdit, &KLineEdit::aboutToShowContextMenu,
                this, &KComboBox::aboutToShowContextMenu);

        // match the declaration of the deprecated signal
#if QT_DEPRECATED_SINCE(5, 15) || QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        connect(d->klineEdit, &KLineEdit::completionBoxActivated,
                this, QOverload<const QString&>::of(&QComboBox::activated));
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        connect(d->klineEdit, &KLineEdit::completionBoxActivated,
                this, QOverload<const QString&>::of(&QComboBox::textActivated));
#endif

        d->klineEdit->setTrapReturnKey(d->trapReturnKey);
    }
}

void KComboBox::setCurrentItem(const QString &item, bool insert, int index)
{
    int sel = -1;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i) {
        if (itemText(i) == item) {
            sel = i;
            break;
        }
    }

    if (sel == -1 && insert) {
        if (index >= 0) {
            insertItem(index, item);
            sel = index;
        } else {
            addItem(item);
            sel = count() - 1;
        }
    }
    setCurrentIndex(sel);
}

void KComboBox::setEditable(bool editable)
{
    if (editable == isEditable()) {
        return;
    }

    if (editable) {
        // Create a KLineEdit instead of a QLineEdit
        // Compared to QComboBox::setEditable, we might be missing the SH_ComboBox_Popup code though...
        // If a style needs this, then we'll need to call QComboBox::setEditable and then setLineEdit again
        KLineEdit *edit = new KLineEdit(this);
        edit->setClearButtonEnabled(true);
        setLineEdit(edit);
    } else {
        QComboBox::setEditable(editable);
    }
}

#include "moc_kcombobox.cpp"
