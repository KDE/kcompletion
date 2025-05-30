/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPLETIONMATCHES_H
#define KCOMPLETIONMATCHES_H

#include <kcompletion_export.h>
#include <ksortablelist.h>

#include <QStringList>
#include <memory>

class KCompletionMatchesWrapper;
class KCompletionMatchesPrivate;

/*!
 * \typedef KCompletionMatchesList
 * \relates KCompletionMatches
 */
typedef KSortableList<QString> KCompletionMatchesList;

/*!
 * \class KCompletionMatches
 * \inmodule KCompletion
 *
 * \brief List for keeping matches returned from KCompletion.
 *
 * This structure is returned by KCompletion::allWeightedMatches().
 * It also keeps the weight of the matches, allowing
 * you to modify some matches or merge them with matches
 * from another call to allWeightedMatches(), and sort the matches
 * after that in order to have the matches ordered correctly.
 *
 * Example (a simplified example of what Konqueror's completion does):
 * \code
 * KCompletionMatches matches = completion->allWeightedMatches(location);
 * if(!location.startsWith("www."))
 *     matches += completion->allWeightedmatches("www." + location");
 * matches.removeDuplicates();
 * QStringList list = matches.list();
 * \endcode
 */
class KCOMPLETION_EXPORT KCompletionMatches : public KCompletionMatchesList
{
public:
    Q_DECLARE_PRIVATE(KCompletionMatches)
    /*!
     * Default constructor.
     *
     * \a sort if false, the matches won't be sorted before the conversion,
     *             use only if you're sure the sorting is not needed
     */
    KCompletionMatches(bool sort);

    KCompletionMatches(const KCompletionMatches &);

    KCompletionMatches &operator=(const KCompletionMatches &);

    /*!
     * \internal
     */
    KCompletionMatches(const KCompletionMatchesWrapper &matches);

    ~KCompletionMatches();
    /*!
     * Removes duplicate matches. Needed only when you merged several matches
     * results and there's a possibility of duplicates.
     */
    void removeDuplicates();
    /*!
     * Returns the matches as a QStringList.
     *
     * \a sort if false, the matches won't be sorted before the conversion,
     *             use only if you're sure the sorting is not needed
     *
     * Returns the list of matches
     */
    QStringList list(bool sort = true) const;
    /*!
     * If sorting() returns \c false, the matches aren't sorted by their weight,
     * even if \c true is passed to list().
     *
     * \c true if the matches won't be sorted
     */
    bool sorting() const;

private:
    std::unique_ptr<KCompletionMatchesPrivate> const d_ptr;
};

#endif // KCOMPLETIONMATCHES_H
