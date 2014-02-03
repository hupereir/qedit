#ifndef IconNames_h
#define IconNames_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "FileSystemIconNames.h"
#include "BaseIconNames.h"

#include <QString>

//! namespace for icon static name wrappers
namespace IconNames
{

    static const QString Close = "document-close.png";

    static const QString Save = "document-save.png";
    static const QString SaveAs = "document-save-as.png";
    static const QString SaveAll = "document-save-all.png";
    static const QString New = "document-new.png";

    static const QString Up = "arrow-up.png";
    static const QString Down = "arrow-down.png";

    static const QString Documents = "document-multiple.png";
    static const QString History = "view-history.png";
    static const QString FileSystem = "folder.png";

    static const QString Pdf = "pdf.png";
    static const QString Indent = "format-indent-more.png";
    static const QString SpellCheck = "tools-check-spelling.png";
    static const QString ViewLeftRight = "view-split-left-right.png";
    static const QString ViewTopBottom = "view-split-top-bottom.png";
    static const QString ViewRight = "view-right-new.png";
    static const QString ViewBottom = "view-bottom-new.png";
    static const QString ViewRemove = "view-close.png";
    static const QString ViewDetach = "view-fullscreen.png";

    static const QString Tag = "bookmarks.png";

}

#endif
