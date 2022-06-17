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
 * Every node is a QChar and has a list of children, which  are Nodes as well.
 *
 * QChar( 0x0 ) is used as the delimiter of a string; the last child of each
 * inserted string is 0x0.
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

    ~KCompTreeNode()
    {
        // delete all children
        KCompTreeNode *cur = m_children.begin();
        while (cur) {
            KCompTreeNode *next = cur->m_next;
            delete m_children.remove(cur);
            cur = next;
        }
    }

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

    // Adds a child-node "ch" to this node. If such a node is already existent,
    // it will not be created. Returns the new/existing node.
    inline KCompTreeNode *insert(const QChar &ch, bool sorted);

    // Iteratively removes a string from the tree. The nicer recursive
    // version apparently was a little memory hungry (see #56757)
    inline void remove(const QString &str);

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

#endif // KCOMPTREENODE_P_H
