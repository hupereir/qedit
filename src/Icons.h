#ifndef Icons_h
#define Icons_h

// $Id$

/******************************************************************************
 *
 * Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
 *
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * software; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *
 *******************************************************************************/

#include "FileSystemIcons.h"
#include "BaseIcons.h"

#include <QtCore/QString>

//! namespace for icon static name wrappers
namespace ICONS
{

    static const QString CLOSE = "document-close.png";

    static const QString SAVE = "document-save.png";
    static const QString SAVE_AS = "document-save-as.png";
    static const QString SAVE_ALL = "document-save-all.png";
    static const QString NEW = "document-new.png";

    static const QString UP = "arrow-up.png";
    static const QString DOWN = "arrow-down.png";

    static const QString DOCUMENTS = "document-multiple.png";
    static const QString HISTORY = "view-history.png";
    static const QString FILESYSTEM = "folder.png";

    static const QString PDF = "pdf.png";
    static const QString INDENT="format-indent-more.png";
    static const QString SPELLCHECK = "tools-check-spelling.png";
    static const QString VIEW_LEFTRIGHT = "view-split-left-right.png";
    static const QString VIEW_TOPBOTTOM = "view-split-top-bottom.png";
    static const QString VIEW_RIGHT = "view-right-new.png";
    static const QString VIEW_BOTTOM = "view-bottom-new.png";
    static const QString VIEW_REMOVE = "view-close.png";
    static const QString VIEW_DETACH = "view-fullscreen.png";

    static const QString TAG = "bookmarks.png";

}

#endif
