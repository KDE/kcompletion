/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompletion.h"
#include "kcompletion_p.h"
#include <kcompletion_debug.h>

#include <QCollator>

void KCompletionPrivate::init()
{
    completionMode = KCompletion::CompletionPopup;
    treeNodeAllocator = KCompTreeNode::allocator(); // keep strong-ref to allocator instance
    treeRoot = new KCompTreeNode;
    beep = true;
    ignoreCase = false;
    hasMultipleMatches = false;
    shouldAutoSuggest = true;
    rotationIndex = 0;
}

void KCompletionPrivate::addWeightedItem(const QString &item)
{
    Q_Q(KCompletion);
    if (order != KCompletion::Weighted) {
        q->addItem(item, 0);
        return;
    }

    int len = item.length();
    uint weight = 0;

    // find out the weighting of this item (appended to the string as ":num")
    int index = item.lastIndexOf(QLatin1Char(':'));
    if (index > 0) {
        bool ok;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        weight = QStringView(item).mid(index + 1).toUInt(&ok);
#else
        weight = item.midRef(index + 1).toUInt(&ok);
#endif
        if (!ok) {
            weight = 0;
        }

        len = index; // only insert until the ':'
    }

    q->addItem(item.left(len), weight);
    return;
}

// tries to complete "string" from the tree-root
QString KCompletionPrivate::findCompletion(const QString &string)
{
    QChar ch;
    QString completion;
    const KCompTreeNode *node = treeRoot;

    // start at the tree-root and try to find the search-string
    for (int i = 0; i < string.length(); i++) {
        ch = string.at(i);
        node = node->find(ch);

        if (node) {
            completion += ch;
        } else {
            return QString(); // no completion
        }
    }

    // Now we have the last node of the to be completed string.
    // Follow it as long as it has exactly one child (= longest possible
    // completion)

    while (node->childrenCount() == 1) {
        node = node->firstChild();
        if (!node->isNull()) {
            completion += *node;
        }
    }
    // if multiple matches and auto-completion mode
    // -> find the first complete match
    if (node && node->childrenCount() > 1) {
        hasMultipleMatches = true;

        if (completionMode == KCompletion::CompletionAuto) {
            rotationIndex = 1;
            if (order != KCompletion::Weighted) {
                while ((node = node->firstChild())) {
                    if (!node->isNull()) {
                        completion += *node;
                    } else {
                        break;
                    }
                }
            } else {
                // don't just find the "first" match, but the one with the
                // highest priority

                const KCompTreeNode *temp_node = nullptr;
                while (1) {
                    int count = node->childrenCount();
                    temp_node = node->firstChild();
                    uint weight = temp_node->weight();
                    const KCompTreeNode *hit = temp_node;
                    for (int i = 1; i < count; i++) {
                        temp_node = node->childAt(i);
                        if (temp_node->weight() > weight) {
                            hit = temp_node;
                            weight = hit->weight();
                        }
                    }
                    // 0x0 has the highest priority -> we have the best match
                    if (hit->isNull()) {
                        break;
                    }

                    node = hit;
                    completion += *node;
                }
            }
        }
    }

    return completion;
}

void KCompletionPrivate::defaultSort(QStringList &stringList)
{
    QCollator c;
    c.setCaseSensitivity(Qt::CaseSensitive);
    std::stable_sort(stringList.begin(), stringList.end(), c);
}

KCompletion::KCompletion()
    : d_ptr(new KCompletionPrivate(this))
{
    Q_D(KCompletion);
    d->init();
    setOrder(Insertion);
}

KCompletion::~KCompletion()
{
}

void KCompletion::setOrder(CompOrder order)
{
    Q_D(KCompletion);
    d->order = order;
    d->matches.setSorting(order);
}

KCompletion::CompOrder KCompletion::order() const
{
    Q_D(const KCompletion);
    return d->order;
}

void KCompletion::setIgnoreCase(bool ignoreCase)
{
    Q_D(KCompletion);
    d->ignoreCase = ignoreCase;
}

bool KCompletion::ignoreCase() const
{
    Q_D(const KCompletion);
    return d->ignoreCase;
}

void KCompletion::setItems(const QStringList &itemList)
{
    clear();
    insertItems(itemList);
}

void KCompletion::insertItems(const QStringList &items)
{
    Q_D(KCompletion);
    bool weighted = (d->order == Weighted);
    QStringList::ConstIterator it;
    if (weighted) { // determine weight
        for (it = items.begin(); it != items.end(); ++it) {
            d->addWeightedItem(*it);
        }
    } else {
        for (it = items.begin(); it != items.end(); ++it) {
            addItem(*it, 0);
        }
    }
}

QStringList KCompletion::items() const
{
    Q_D(const KCompletion);
    KCompletionMatchesWrapper list(d->sorterFunction); // unsorted
    bool addWeight = (d->order == Weighted);
    list.extractStringsFromNode(d->treeRoot, QString(), addWeight);

    return list.list();
}

bool KCompletion::isEmpty() const
{
    Q_D(const KCompletion);
    return (d->treeRoot->childrenCount() == 0);
}

void KCompletion::postProcessMatch(QString *) const
{
}

void KCompletion::postProcessMatches(QStringList *) const
{
}

void KCompletion::postProcessMatches(KCompletionMatches *) const
{
}

void KCompletion::addItem(const QString &item)
{
    Q_D(KCompletion);
    d->matches.clear();
    d->rotationIndex = 0;
    d->lastString.clear();

    addItem(item, 0);
}

void KCompletion::addItem(const QString &item, uint weight)
{
    Q_D(KCompletion);
    if (item.isEmpty()) {
        return;
    }

    KCompTreeNode *node = d->treeRoot;
    int len = item.length();

    bool sorted = (d->order == Sorted);
    bool weighted = ((d->order == Weighted) && weight > 1);

    // knowing the weight of an item, we simply add this weight to all of its
    // nodes.

    for (int i = 0; i < len; i++) {
        node = node->insert(item.at(i), sorted);
        if (weighted) {
            node->confirm(weight - 1); // node->insert() sets weighting to 1
        }
    }

    // add 0x0-item as delimiter with evtl. weight
    node = node->insert(0x0, true);
    if (weighted) {
        node->confirm(weight - 1);
    }
    //     qDebug("*** added: %s (%i)", item.toLatin1().constData(), node->weight());
}

void KCompletion::removeItem(const QString &item)
{
    Q_D(KCompletion);
    d->matches.clear();
    d->rotationIndex = 0;
    d->lastString.clear();

    d->treeRoot->remove(item);
}

void KCompletion::clear()
{
    Q_D(KCompletion);
    d->matches.clear();
    d->rotationIndex = 0;
    d->lastString.clear();

    delete d->treeRoot;
    d->treeRoot = new KCompTreeNode;
}

QString KCompletion::makeCompletion(const QString &string)
{
    Q_D(KCompletion);
    if (d->completionMode == CompletionNone) {
        return QString();
    }

    // qDebug() << "KCompletion: completing: " << string;

    d->matches.clear();
    d->rotationIndex = 0;
    d->hasMultipleMatches = false;
    d->lastMatch = d->currentMatch;

    // in Shell-completion-mode, emit all matches when we get the same
    // complete-string twice
    if (d->completionMode == CompletionShell && string == d->lastString) {
        // Don't use d->matches since calling postProcessMatches()
        // on d->matches here would interfere with call to
        // postProcessMatch() during rotation

        d->matches.findAllCompletions(d->treeRoot, string, d->ignoreCase, d->hasMultipleMatches);
        QStringList l = d->matches.list();
        postProcessMatches(&l);
        Q_EMIT matches(l);

        return QString();
    }

    QString completion;
    // in case-insensitive popup mode, we search all completions at once
    if (d->completionMode == CompletionPopup || d->completionMode == CompletionPopupAuto) {
        d->matches.findAllCompletions(d->treeRoot, string, d->ignoreCase, d->hasMultipleMatches);
        if (!d->matches.isEmpty()) {
            completion = d->matches.first();
        }
    } else {
        completion = d->findCompletion(string);
    }

    if (d->hasMultipleMatches) {
        Q_EMIT multipleMatches();
    }

    d->lastString = string;
    d->currentMatch = completion;

    postProcessMatch(&completion);

    if (!string.isEmpty()) { // only emit match when string is not empty
        // qDebug() << "KCompletion: Match: " << completion;
        Q_EMIT match(completion);
    }

    return completion;
}

QStringList KCompletion::substringCompletion(const QString &string) const
{
    Q_D(const KCompletion);
    // get all items in the tree, eventually in sorted order
    KCompletionMatchesWrapper allItems(d->sorterFunction, d->order);
    allItems.extractStringsFromNode(d->treeRoot, QString(), false);

    QStringList list = allItems.list();

    // subStringMatches is invoked manually, via a shortcut
    if (list.isEmpty()) {
        return list;
    }

    if (string.isEmpty()) { // shortcut
        postProcessMatches(&list);
        return list;
    }

    QStringList matches;
    for (QStringList::ConstIterator it = list.constBegin(), total = list.constEnd(); it != total; ++it) {
        QString item = *it;
        if (item.indexOf(string, 0, Qt::CaseInsensitive) != -1) { // always case insensitive
            postProcessMatch(&item);
            matches.append(item);
        }
    }

    return matches;
}

void KCompletion::setCompletionMode(CompletionMode mode)
{
    Q_D(KCompletion);
    d->completionMode = mode;
}

KCompletion::CompletionMode KCompletion::completionMode() const
{
    Q_D(const KCompletion);
    return d->completionMode;
}

void KCompletion::setShouldAutoSuggest(const bool shouldAutoSuggest)
{
    Q_D(KCompletion);
    d->shouldAutoSuggest = shouldAutoSuggest;
}

bool KCompletion::shouldAutoSuggest() const
{
    Q_D(const KCompletion);
    return d->shouldAutoSuggest;
}

void KCompletion::setSorterFunction(SorterFunction sortFunc)
{
    Q_D(KCompletion);
    d->sorterFunction = sortFunc ? sortFunc : KCompletionPrivate::defaultSort;
}

QStringList KCompletion::allMatches()
{
    Q_D(KCompletion);
    // Don't use d->matches since calling postProcessMatches()
    // on d->matches here would interfere with call to
    // postProcessMatch() during rotation
    KCompletionMatchesWrapper matches(d->sorterFunction, d->order);
    bool dummy;
    matches.findAllCompletions(d->treeRoot, d->lastString, d->ignoreCase, dummy);
    QStringList l = matches.list();
    postProcessMatches(&l);
    return l;
}

KCompletionMatches KCompletion::allWeightedMatches()
{
    Q_D(KCompletion);
    // Don't use d->matches since calling postProcessMatches()
    // on d->matches here would interfere with call to
    // postProcessMatch() during rotation
    KCompletionMatchesWrapper matches(d->sorterFunction, d->order);
    bool dummy;
    matches.findAllCompletions(d->treeRoot, d->lastString, d->ignoreCase, dummy);
    KCompletionMatches ret(matches);
    postProcessMatches(&ret);
    return ret;
}

QStringList KCompletion::allMatches(const QString &string)
{
    Q_D(KCompletion);
    KCompletionMatchesWrapper matches(d->sorterFunction, d->order);
    bool dummy;
    matches.findAllCompletions(d->treeRoot, string, d->ignoreCase, dummy);
    QStringList l = matches.list();
    postProcessMatches(&l);
    return l;
}

KCompletionMatches KCompletion::allWeightedMatches(const QString &string)
{
    Q_D(KCompletion);
    KCompletionMatchesWrapper matches(d->sorterFunction, d->order);
    bool dummy;
    matches.findAllCompletions(d->treeRoot, string, d->ignoreCase, dummy);
    KCompletionMatches ret(matches);
    postProcessMatches(&ret);
    return ret;
}

void KCompletion::setSoundsEnabled(bool enable)
{
    Q_D(KCompletion);
    d->beep = enable;
}

bool KCompletion::soundsEnabled() const
{
    Q_D(const KCompletion);
    return d->beep;
}

bool KCompletion::hasMultipleMatches() const
{
    Q_D(const KCompletion);
    return d->hasMultipleMatches;
}

/////////////////////////////////////////////////////
///////////////// tree operations ///////////////////

QString KCompletion::nextMatch()
{
    Q_D(KCompletion);
    QString completion;
    d->lastMatch = d->currentMatch;

    if (d->matches.isEmpty()) {
        d->matches.findAllCompletions(d->treeRoot, d->lastString, d->ignoreCase, d->hasMultipleMatches);
        if (!d->matches.isEmpty()) {
            completion = d->matches.first();
        }
        d->currentMatch = completion;
        d->rotationIndex = 0;
        postProcessMatch(&completion);
        Q_EMIT match(completion);
        return completion;
    }

    QStringList matches = d->matches.list();
    d->lastMatch = matches[d->rotationIndex++];

    if (d->rotationIndex == matches.count()) {
        d->rotationIndex = 0;
    }

    completion = matches[d->rotationIndex];
    d->currentMatch = completion;
    postProcessMatch(&completion);
    Q_EMIT match(completion);
    return completion;
}

const QString &KCompletion::lastMatch() const
{
    Q_D(const KCompletion);
    return d->lastMatch;
}

QString KCompletion::previousMatch()
{
    Q_D(KCompletion);
    QString completion;
    d->lastMatch = d->currentMatch;

    if (d->matches.isEmpty()) {
        d->matches.findAllCompletions(d->treeRoot, d->lastString, d->ignoreCase, d->hasMultipleMatches);
        if (!d->matches.isEmpty()) {
            completion = d->matches.last();
        }
        d->currentMatch = completion;
        d->rotationIndex = 0;
        postProcessMatch(&completion);
        Q_EMIT match(completion);
        return completion;
    }

    QStringList matches = d->matches.list();
    d->lastMatch = matches[d->rotationIndex];

    if (d->rotationIndex == 0) {
        d->rotationIndex = matches.count();
    }

    d->rotationIndex--;

    completion = matches[d->rotationIndex];
    d->currentMatch = completion;
    postProcessMatch(&completion);
    Q_EMIT match(completion);
    return completion;
}

/////////////////////////////////
/////////

// Implements the tree. Every node is a QChar and has a list of children, which
// are Nodes as well.
// QChar( 0x0 ) is used as the delimiter of a string; the last child of each
// inserted string is 0x0.

KCompTreeNode::~KCompTreeNode()
{
    // delete all children
    KCompTreeNode *cur = m_children.begin();
    while (cur) {
        KCompTreeNode *next = cur->m_next;
        delete m_children.remove(cur);
        cur = next;
    }
}

// Adds a child-node "ch" to this node. If such a node is already existent,
// it will not be created. Returns the new/existing node.
KCompTreeNode *KCompTreeNode::insert(const QChar &ch, bool sorted)
{
    KCompTreeNode *child = find(ch);
    if (!child) {
        child = new KCompTreeNode(ch);

        // FIXME, first (slow) sorted insertion implementation
        if (sorted) {
            KCompTreeNode *prev = nullptr;
            KCompTreeNode *cur = m_children.begin();
            while (cur) {
                if (ch > *cur) {
                    prev = cur;
                    cur = cur->m_next;
                } else {
                    break;
                }
            }
            if (prev) {
                m_children.insert(prev, child);
            } else {
                m_children.prepend(child);
            }
        }

        else {
            m_children.append(child);
        }
    }

    // implicit weighting: the more often an item is inserted, the higher
    // priority it gets.
    child->confirm();

    return child;
}

// Iteratively removes a string from the tree. The nicer recursive
// version apparently was a little memory hungry (see #56757)
void KCompTreeNode::remove(const QString &str)
{
    QString string = str;
    string += QChar(0x0);

    QVector<KCompTreeNode *> deletables(string.length() + 1);

    KCompTreeNode *child = nullptr;
    KCompTreeNode *parent = this;
    deletables.replace(0, parent);

    int i = 0;
    for (; i < string.length(); i++) {
        child = parent->find(string.at(i));
        if (child) {
            deletables.replace(i + 1, child);
        } else {
            break;
        }

        parent = child;
    }

    for (; i >= 1; i--) {
        parent = deletables.at(i - 1);
        child = deletables.at(i);
        if (child->m_children.count() == 0) {
            delete parent->m_children.remove(child);
        }
    }
}

QStringList KCompletionMatchesWrapper::list() const
{
    if (m_sortedList && m_dirty) {
        m_sortedList->sort();
        m_dirty = false;

        m_stringList.clear();

        // high weight == sorted last -> reverse the sorting here
        QList<KSortableItem<QString>>::const_iterator it;
        for (it = m_sortedList->constBegin(); it != m_sortedList->constEnd(); ++it) {
            m_stringList.prepend((*it).value());
        }
    } else if (m_compOrder == KCompletion::Sorted) {
        m_sorterFunction(m_stringList);
    }

    return m_stringList;
}

void KCompletionMatchesWrapper::findAllCompletions(const KCompTreeNode *treeRoot, const QString &string, bool ignoreCase, bool &hasMultipleMatches)
{
    // qDebug() << "*** finding all completions for " << string;

    if (string.isEmpty()) {
        return;
    }

    if (ignoreCase) { // case insensitive completion
        extractStringsFromNodeCI(treeRoot, QString(), string);
        hasMultipleMatches = (count() > 1);
        return;
    }

    QChar ch;
    QString completion;
    const KCompTreeNode *node = treeRoot;

    // start at the tree-root and try to find the search-string
    for (int i = 0; i < string.length(); i++) {
        ch = string.at(i);
        node = node->find(ch);

        if (node) {
            completion += ch;
        } else {
            return; // no completion -> return empty list
        }
    }

    // Now we have the last node of the to be completed string.
    // Follow it as long as it has exactly one child (= longest possible
    // completion)

    while (node->childrenCount() == 1) {
        node = node->firstChild();
        if (!node->isNull()) {
            completion += *node;
        }
        // qDebug() << completion << node->latin1();
    }

    // there is just one single match)
    if (node->childrenCount() == 0) {
        append(node->weight(), completion);
    }

    else {
        // node has more than one child
        // -> recursively find all remaining completions
        hasMultipleMatches = true;
        extractStringsFromNode(node, completion);
    }
}

void KCompletionMatchesWrapper::extractStringsFromNode(const KCompTreeNode *node, const QString &beginning, bool addWeight)
{
    if (!node) {
        return;
    }

    // qDebug() << "Beginning: " << beginning;
    const KCompTreeChildren *list = node->children();
    QString string;
    QString w;

    // loop thru all children
    for (KCompTreeNode *cur = list->begin(); cur; cur = cur->m_next) {
        string = beginning;
        node = cur;
        if (!node->isNull()) {
            string += *node;
        }

        while (node && node->childrenCount() == 1) {
            node = node->firstChild();
            if (node->isNull()) {
                break;
            }
            string += *node;
        }

        if (node && node->isNull()) { // we found a leaf
            if (addWeight) {
                // add ":num" to the string to store the weighting
                string += QLatin1Char(':');
                w.setNum(node->weight());
                string.append(w);
            }
            append(node->weight(), string);
        }

        // recursively find all other strings.
        if (node && node->childrenCount() > 1) {
            extractStringsFromNode(node, string, addWeight);
        }
    }
}

void KCompletionMatchesWrapper::extractStringsFromNodeCI(const KCompTreeNode *node, const QString &beginning, const QString &restString)
{
    if (restString.isEmpty()) {
        extractStringsFromNode(node, beginning, false /*noweight*/);
        return;
    }

    QChar ch1 = restString.at(0);
    QString newRest = restString.mid(1);
    KCompTreeNode *child1;
    KCompTreeNode *child2;

    child1 = node->find(ch1); // the correct match
    if (child1) {
        extractStringsFromNodeCI(child1, beginning + QChar(*child1), newRest);
    }

    // append the case insensitive matches, if available
    if (ch1.isLetter()) {
        // find out if we have to lower or upper it. Is there a better way?
        QChar ch2 = ch1.toLower();
        if (ch1 == ch2) {
            ch2 = ch1.toUpper();
        }
        if (ch1 != ch2) {
            child2 = node->find(ch2);
            if (child2) {
                extractStringsFromNodeCI(child2, beginning + QChar(*child2), newRest);
            }
        }
    }
}

void KCompTreeNodeList::append(KCompTreeNode *item)
{
    m_count++;
    if (!m_last) {
        m_last = item;
        m_last->m_next = nullptr;
        m_first = item;
        return;
    }
    m_last->m_next = item;
    item->m_next = nullptr;
    m_last = item;
}

void KCompTreeNodeList::prepend(KCompTreeNode *item)
{
    m_count++;
    if (!m_last) {
        m_last = item;
        m_last->m_next = nullptr;
        m_first = item;
        return;
    }
    item->m_next = m_first;
    m_first = item;
}

void KCompTreeNodeList::insert(KCompTreeNode *after, KCompTreeNode *item)
{
    if (!after) {
        append(item);
        return;
    }

    m_count++;

    item->m_next = after->m_next;
    after->m_next = item;

    if (after == m_last) {
        m_last = item;
    }
}

KCompTreeNode *KCompTreeNodeList::remove(KCompTreeNode *item)
{
    if (!m_first || !item) {
        return nullptr;
    }
    KCompTreeNode *cur = nullptr;

    if (item == m_first) {
        m_first = m_first->m_next;
    } else {
        cur = m_first;
        while (cur && cur->m_next != item) {
            cur = cur->m_next;
        }
        if (!cur) {
            return nullptr;
        }
        cur->m_next = item->m_next;
    }
    if (item == m_last) {
        m_last = cur;
    }
    m_count--;
    return item;
}

KCompTreeNode *KCompTreeNodeList::at(uint index) const
{
    KCompTreeNode *cur = m_first;
    while (index-- && cur) {
        cur = cur->m_next;
    }
    return cur;
}

QSharedPointer<KZoneAllocator> KCompTreeNode::m_alloc(new KZoneAllocator(8 * 1024));
