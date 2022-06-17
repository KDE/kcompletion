/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPLETION_PRIVATE_H
#define KCOMPLETION_PRIVATE_H

#include "kcompletion.h"
#include "kcompletionmatcheswrapper_p.h"
#include "kcomptreenode_p.h"

#include <kcompletionmatches.h>

#include <QSharedPointer>
#include <kzoneallocator_p.h>

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
