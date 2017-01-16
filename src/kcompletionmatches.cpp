/* This file is part of the KDE libraries
   Copyright (C) 1999,2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcompletionmatches.h"

#include <kcompletion.h>
#include <kcompletion_p.h> // for KCompletionMatchesWrapper

class KCompletionMatchesPrivate
{
public:
    KCompletionMatchesPrivate(bool sort, KCompletionMatches *parent)
        : sorting(sort),
          q_ptr(parent) {}

    bool sorting;
    KCompletionMatches * const q_ptr;

    Q_DECLARE_PUBLIC(KCompletionMatches)
};

KCompletionMatches::KCompletionMatches(const KCompletionMatches &o)
    : KSortableList<QString, int>(),
      d_ptr(new KCompletionMatchesPrivate(o.sorting(), this))
{
    *this = KCompletionMatches::operator=(o);
}

KCompletionMatches &KCompletionMatches::operator=(const KCompletionMatches &o)
{
    Q_D(KCompletionMatches);
    if (*this == o) {
        return *this;
    }
    KCompletionMatchesList::operator=(o);
    d->sorting = o.sorting();

    return *this;
}

KCompletionMatches::KCompletionMatches(bool sort_P)
    : d_ptr(new KCompletionMatchesPrivate(sort_P, this))
{
}

KCompletionMatches::KCompletionMatches(const KCompletionMatchesWrapper &matches)
    : d_ptr(new KCompletionMatchesPrivate(matches.sorting(), this))
{
    if (matches.m_sortedList != nullptr) {
        KCompletionMatchesList::operator=(*matches.m_sortedList);
    } else {
        const QStringList l = matches.list();
        for (QStringList::ConstIterator it = l.begin();
                it != l.end();
                ++it) {
            prepend(KSortableItem<QString, int>(1, *it));
        }
    }
}

KCompletionMatches::~KCompletionMatches()
{
}

QStringList KCompletionMatches::list(bool sort_P) const
{
    Q_D(const KCompletionMatches);
    if (d->sorting && sort_P) {
        const_cast< KCompletionMatches * >(this)->sort();
    }
    QStringList stringList;
    // high weight == sorted last -> reverse the sorting here
    for (ConstIterator it = begin(); it != end(); ++it) {
        stringList.prepend((*it).value());
    }
    return stringList;
}

bool KCompletionMatches::sorting() const
{
    Q_D(const KCompletionMatches);
    return d->sorting;
}

void KCompletionMatches::removeDuplicates()
{
    Iterator it1, it2;
    for (it1 = begin(); it1 != end(); ++it1) {
        for ((it2 = it1), ++it2; it2 != end();) {
            if ((*it1).value() == (*it2).value()) {
                // use the max height
                (*it1).first = qMax((*it1).key(), (*it2).key());
                it2 = erase(it2);
                continue;
            }
            ++it2;
        }
    }
}
