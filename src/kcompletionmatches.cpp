/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompletionmatches.h"

#include <kcompletion.h>
#include <kcompletion_p.h> // for KCompletionMatchesWrapper

class KCompletionMatchesPrivate
{
public:
    KCompletionMatchesPrivate(bool sort, KCompletionMatches *qq)
        : q(qq)
        , sorting(sort)
    {
    }

    KCompletionMatches *const q;
    bool sorting = false;
    CompletionItemsVec m_matches;
};

KCompletionMatches::KCompletionMatches(bool sort_P)
    : d(new KCompletionMatchesPrivate(sort_P, this))
{
}

KCompletionMatches::KCompletionMatches(const KCompletionMatches &o)
    : d(new KCompletionMatchesPrivate(o.sorting(), this))
{
    *this = KCompletionMatches::operator=(o);
}

KCompletionMatches &KCompletionMatches::operator=(const KCompletionMatches &other)
{
    if (this == &other) {
        return *this;
    }
    d->m_matches = other.d->m_matches;
    d->sorting = other.sorting();

    return *this;
}

KCompletionMatches::KCompletionMatches(KCompletionMatchesWrapper &matches)
    : d(new KCompletionMatchesPrivate(matches.sorting(), this))
{
    if (matches.m_sortedListPtr) {
        d->m_matches = *matches.m_sortedListPtr;
    } else {
        const QStringList list = matches.list();
        d->m_matches.reserve(list.size());
        std::transform(list.crbegin(), list.crend(), std::back_inserter(d->m_matches), [](const QString &str) {
            return Item{1, str};
        });
    }
}

KCompletionMatches::~KCompletionMatches() = default;

QStringList KCompletionMatches::list(bool sort_P)
{
    if (d->sorting && sort_P) {
        std::sort(d->m_matches.begin(), d->m_matches.end());
    }
    QStringList stringList;
    stringList.reserve(d->m_matches.size());
    // high weight == sorted last -> reverse the sorting here
    std::transform(d->m_matches.crbegin(), d->m_matches.crend(), std::back_inserter(stringList), [](const Item &item) {
        return item.text;
    });
    return stringList;
}

bool KCompletionMatches::sorting() const
{
    return d->sorting;
}

void KCompletionMatches::removeDuplicates()
{
    for (auto it1 = d->m_matches.begin(); it1 != d->m_matches.end(); ++it1) {
        auto it2 = it1;
        for (++it2; it2 != d->m_matches.end();) {
            if (it1->text == it2->text) {
                // Use the max weight
                it1->key = std::max(it1->key, it2->key);
                it2 = d->m_matches.erase(it2);
                continue;
            }
            ++it2;
        }
    }
}
