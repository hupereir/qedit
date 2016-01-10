#ifndef IconNames_h
#define IconNames_h

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

//* namespace for icon static name wrappers
namespace IconNames
{

    static const QString Close = "document-close";

    static const QString Save = "document-save";
    static const QString SaveAs = "document-save-as";
    static const QString SaveAll = "document-save-all";
    static const QString New = "document-new";

    static const QString Up = "arrow-up";
    static const QString Down = "arrow-down";

    static const QString Documents = "document-multiple";
    static const QString History = "view-history";
    static const QString FileSystem = "folder";

    static const QString Pdf = "pdf";
    static const QString Indent = "format-indent-more";
    static const QString SpellCheck = "tools-check-spelling";
    static const QString ViewLeftRight = "view-split-left-right";
    static const QString ViewTopBottom = "view-split-top-bottom";
    static const QString ViewRight = "view-right-new";
    static const QString ViewBottom = "view-bottom-new";
    static const QString ViewRemove = "view-close";
    static const QString ViewDetach = "view-fullscreen";

    static const QString Tag = "bookmarks";

}

#endif
