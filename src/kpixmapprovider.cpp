/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpixmapprovider.h"

#if KCOMPLETION_BUILD_DEPRECATED_SINCE(5, 66)

KPixmapProvider::~KPixmapProvider()
{
}

void KPixmapProvider::virtual_hook(int, void *)
{
    /*BASE::virtual_hook( id, data );*/
}
#endif
