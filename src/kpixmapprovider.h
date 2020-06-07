/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPIXMAPPROVIDER_H
#define KPIXMAPPROVIDER_H

#include <kcompletion_export.h>
#include <QPixmap>

/**
 * @class KPixmapProvider kpixmapprovider.h KPixmapProvider
 *
 * A tiny abstract class with just one method:
 * pixmapFor()
 *
 * It will be called whenever an icon is searched for @p text.
 *
 * Used e.g. by KHistoryComboBox
 *
 * @deprecated since 5.66. Use an std::function that takes a QString and returns a QIcon/QPixmap
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @short an abstract interface for looking up icons
 */
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(5, 66)
class KCOMPLETION_EXPORT KPixmapProvider
{
public:
    virtual ~KPixmapProvider();
    /**
     * You may subclass this and return a pixmap of size @p size for @p text.
     * @param text the text that is associated with the pixmap
     * @param size the size of the icon in pixels, 0 for defaylt size.
     *             See KIconLoader::StdSize.
     * @deprecated since 5.66. Use an std::function that takes a QString and returns a QIcon/QPixmap.
     * @return the pixmap for the arguments, or null if there is none
     */
    KCOMPLETION_DEPRECATED_VERSION(5, 66, "Use an std::function that takes a QString and returns a QIcon/QPixmap")
    virtual QPixmap pixmapFor(const QString &text, int size = 0) = 0;
protected:
    /** Virtual hook, used to add new "virtual" functions while maintaining
    binary compatibility. Unused in this class.
    */
    virtual void virtual_hook(int id, void *data);
};
#endif

#endif // KPIXMAPPROVIDER_H
