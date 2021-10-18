/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPLETION_PRIVATE_H
#define KCOMPLETION_PRIVATE_H

#include "kcompletion.h"

#include <kcompletionmatches.h>

#include <QSharedPointer>
#include <kzoneallocator_p.h>

class KCompTreeNode;

/**
 * @internal
 */
class KCOMPLETION_EXPORT KCompTreeNodeList
{
public:
    KCompTreeNodeList()
        : m_first(nullptr)
        , m_last(nullptr)
        , m_count(0)
    {
    }

    KCompTreeNode *begin() const
    {
        return m_first;
    }

    KCompTreeNode *end() const
    {
        return m_last;
    }

    KCompTreeNode *at(uint index) const;
    void append(KCompTreeNode *item);
    void prepend(KCompTreeNode *item);
    void insert(KCompTreeNode *after, KCompTreeNode *item);
    KCompTreeNode *remove(KCompTreeNode *item);

    uint count() const
    {
        return m_count;
    }

private:
    KCompTreeNode *m_first;
    KCompTreeNode *m_last;
    uint m_count;
};

typedef KCompTreeNodeList KCompTreeChildren;

/**
 * A helper class for KCompletion. Implements a tree of QChar.
 *
 * The tree looks like this (containing the items "kde", "kde-ui",
 * "kde-core" and "pfeiffer". Every item is delimited with QChar( 0x0 )
 *
 *              some_root_node
 *                  /     \
 *                 k       p
 *                 |       |
 *                 d       f
 *                 |       |
 *                 e       e
 *                /|       |
 *             0x0 -       i
 *                / \      |
 *               u   c     f
 *               |   |     |
 *               i   o     f
 *               |   |     |
 *              0x0  r     e
 *                   |     |
 *                   e     r
 *                   |     |
 *                  0x0   0x0
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @internal
 */
class KCOMPLETION_EXPORT KCompTreeNode : public QChar
{
public:
    KCompTreeNode()
        : QChar()
        , m_next(nullptr)
        , m_weight(0)
    {
    }

    explicit KCompTreeNode(const QChar &ch, uint weight = 0)
        : QChar(ch)
        , m_next(nullptr)
        , m_weight(weight)
    {
    }

    ~KCompTreeNode();

    KCompTreeNode(const KCompTreeNode &) = delete;
    KCompTreeNode &operator=(const KCompTreeNode &) = delete;

    void *operator new(size_t s)
    {
        Q_ASSERT(m_alloc);
        return m_alloc->allocate(s);
    }

    void operator delete(void *s)
    {
        Q_ASSERT(m_alloc);
        m_alloc->deallocate(s);
    }

    // Returns a child of this node matching ch, if available.
    // Otherwise, returns 0L
    inline KCompTreeNode *find(const QChar &ch) const
    {
        KCompTreeNode *cur = m_children.begin();
        while (cur && (*cur != ch)) {
            cur = cur->m_next;
        }
        return cur;
    }

    KCompTreeNode *insert(const QChar &, bool sorted);
    void remove(const QString &);

    inline int childrenCount() const
    {
        return m_children.count();
    }

    inline void confirm()
    {
        m_weight++;
    }

    inline void confirm(uint w)
    {
        m_weight += w;
    }

    inline void decline()
    {
        m_weight--;
    }

    inline uint weight() const
    {
        return m_weight;
    }

    inline const KCompTreeChildren *children() const
    {
        return &m_children;
    }

    inline const KCompTreeNode *childAt(int index) const
    {
        return m_children.at(index);
    }

    inline const KCompTreeNode *firstChild() const
    {
        return m_children.begin();
    }

    inline const KCompTreeNode *lastChild() const
    {
        return m_children.end();
    }

    /* We want to handle a list of KCompTreeNodes on our own, to not
       need to use QValueList<>. And to make it even faster we don't
       use an accessor, but just a public member. */
    KCompTreeNode *m_next;

    /**
     * Custom allocator used for all KCompTreeNode instances
     */
    static QSharedPointer<KZoneAllocator> allocator()
    {
        return m_alloc;
    }

private:
    uint m_weight;
    KCompTreeNodeList m_children;
    static QSharedPointer<KZoneAllocator> m_alloc;
};

/**
 * @internal
 */
class KCOMPLETION_EXPORT KCompletionMatchesWrapper
{
public:
    explicit KCompletionMatchesWrapper(KCompletion::SorterFunction const &sorterFunction, KCompletion::CompOrder compOrder = KCompletion::Insertion)
        : m_sortedList(compOrder == KCompletion::Weighted ? new KCompletionMatchesList : nullptr)
        , m_dirty(false)
        , m_compOrder(compOrder)
        , m_sorterFunction(sorterFunction)
    {
    }

    ~KCompletionMatchesWrapper()
    {
        delete m_sortedList;
    }

    KCompletionMatchesWrapper(const KCompletionMatchesWrapper &) = delete;
    KCompletionMatchesWrapper &operator=(const KCompletionMatchesWrapper &) = delete;

    void setSorting(KCompletion::CompOrder compOrder)
    {
        if (compOrder == KCompletion::Weighted && !m_sortedList) {
            m_sortedList = new KCompletionMatchesList;
        } else if (compOrder != KCompletion::Weighted) {
            delete m_sortedList;
            m_sortedList = nullptr;
        }
        m_compOrder = compOrder;
        m_stringList.clear();
        m_dirty = false;
    }

    KCompletion::CompOrder sorting() const
    {
        return m_compOrder;
    }

    void append(int i, const QString &string)
    {
        if (m_sortedList) {
            m_sortedList->insert(i, string);
        } else {
            m_stringList.append(string);
        }
        m_dirty = true;
    }

    void clear()
    {
        if (m_sortedList) {
            m_sortedList->clear();
        }
        m_stringList.clear();
        m_dirty = false;
    }

    uint count() const
    {
        if (m_sortedList) {
            return m_sortedList->count();
        }
        return m_stringList.count();
    }

    bool isEmpty() const
    {
        return count() == 0;
    }

    QString first() const
    {
        return list().constFirst();
    }

    QString last() const
    {
        return list().constLast();
    }

    QStringList list() const;

    void findAllCompletions(const KCompTreeNode *, const QString &, bool ignoreCase, bool &hasMultipleMatches);

    void extractStringsFromNode(const KCompTreeNode *, const QString &beginning, bool addWeight = false);

    void extractStringsFromNodeCI(const KCompTreeNode *, const QString &beginning, const QString &restString);

    mutable QStringList m_stringList;
    KCompletionMatchesList *m_sortedList;
    mutable bool m_dirty;
    KCompletion::CompOrder m_compOrder;
    KCompletion::SorterFunction const &m_sorterFunction;
};

class KCompletionPrivate
{
public:
    explicit KCompletionPrivate(KCompletion *parent)
        : q_ptr(parent)
    {
    }

    void init();

    ~KCompletionPrivate()
    {
        delete treeRoot;
    }

    void addWeightedItem(const QString &);
    QString findCompletion(const QString &string);

    // The default sorting function, sorts alphabetically
    static void defaultSort(QStringList &);

    // Pointer to sorter function
    KCompletion::SorterFunction sorterFunction{defaultSort};

    // list used for nextMatch() and previousMatch()
    KCompletionMatchesWrapper matches{sorterFunction};

    KCompletion::CompletionMode completionMode;

    QSharedPointer<KZoneAllocator> treeNodeAllocator;

    QString lastString;
    QString lastMatch;
    QString currentMatch;
    KCompTreeNode *treeRoot;
    KCompletion *const q_ptr;
    int rotationIndex;
    // TODO: Change hasMultipleMatches to bitfield after moving findAllCompletions()
    // to KCompletionMatchesPrivate
    KCompletion::CompOrder order : 3;
    bool hasMultipleMatches;
    bool beep : 1;
    bool ignoreCase : 1;
    bool shouldAutoSuggest : 1;
    Q_DECLARE_PUBLIC(KCompletion)
};

#endif // KCOMPLETION_PRIVATE_H
