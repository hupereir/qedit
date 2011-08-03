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

/*!
  \file    Icons.h
  \brief   Icon filenames
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QString>
#include "BaseIcons.h"
#include "DocumentClassIcons.h"
#include "FileSystemIcons.h"

//! namespace for icon static name wrappers
namespace ICONS
{

    static const QString CLOSE = "fileclose.png";

    static const QString DOCUMENTS = "documents.png";
    static const QString HISTORY = "history.png";
    static const QString FILESYSTEM = "user-home.png";

    static const QString HTML="html.png";
    static const QString PDF = "pdf.png";
    static const QString INDENT="indent.png";
    static const QString SPELLCHECK = "fonts.png";
    static const QString VIEW_LEFTRIGHT = "view_left_right.png";
    static const QString VIEW_TOPBOTTOM = "view_top_bottom.png";
    static const QString VIEW_RIGHT = "view_right.png";
    static const QString VIEW_BOTTOM = "view_bottom.png";
    static const QString VIEW_REMOVE = "view_remove.png";
    static const QString VIEW_DETACH = "window_fullscreen.png";

    static const QString TAG = "bookmark.png";

}

#endif
