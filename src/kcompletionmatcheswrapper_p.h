/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPLETIONMATCHESWRAPPER_P_H
#define KCOMPLETIONMATCHESWRAPPER_P_H

#include "kcompletion.h"

#include <kcompletionmatches.h>

#include <QSharedPointer>
#include <kzoneallocator_p.h>

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

    uint size() const
    {
        if (m_sortedList) {
            return m_sortedList->size();
        }
        return m_stringList.size();
    }

    bool isEmpty() const
    {
        return size() == 0;
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

#endif // KCOMPLETIONMATCHESWRAPPER_P_H
