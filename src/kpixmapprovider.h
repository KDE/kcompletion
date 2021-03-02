/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPIXMAPPROVIDER_H
#define KPIXMAPPROVIDER_H

#include <QPixmap>
#include <kcompletion_export.h>

/**
 * @class KPixmapProvider kpixmapprovider.h KPixmapProvider
 * @short An abstract interface for looking up icons
 *
 * It will be called whenever an icon is searched for @p text.
 *
 * Used e.g. by KHistoryComboBox
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * @deprecated Since 5.66, use a std::function that takes a QString and returns a QIcon/QPixmap
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
     * @return the pixmap for the arguments, or null if there is none
     * @deprecated Since 5.66, use a std::function that takes a QString and returns a QIcon/QPixmap.
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
