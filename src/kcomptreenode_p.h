/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPTREENODE_P_H
#define KCOMPTREENODE_P_H

#include "kcompletion_export.h"

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

#endif // KCOMPTREENODE_P_H
