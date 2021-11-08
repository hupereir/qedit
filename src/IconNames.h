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

#include "BaseIconNames.h"
#include "FileSystemIconNames.h"

#include <QString>

//* namespace for icon static name wrappers
namespace IconNames
{

    static const QString Close = QStringLiteral("document-close");

    static const QString Save = QStringLiteral("document-save");
    static const QString SaveAs = QStringLiteral("document-save-as");
    static const QString SaveAll = QStringLiteral("document-save-all");
    static const QString New = QStringLiteral("document-new");

    static const QString Up = QStringLiteral("arrow-up");
    static const QString Down = QStringLiteral("arrow-down");

    static const QString Documents = QStringLiteral("document-multiple");
    static const QString History = QStringLiteral("view-history");

    static const QString Pdf = QStringLiteral("pdf");
    static const QString Indent = QStringLiteral("format-indent-more");
    static const QString SpellCheck = QStringLiteral("tools-check-spelling");
    static const QString ViewLeftRight = QStringLiteral("view-split-left-right");
    static const QString ViewTopBottom = QStringLiteral("view-split-top-bottom");
    static const QString ViewRight = QStringLiteral("view-right-new");
    static const QString ViewBottom = QStringLiteral("view-bottom-new");
    static const QString ViewRemove = QStringLiteral("view-close");
    static const QString ViewDetach = QStringLiteral("view-fullscreen");

    static const QString Tag = QStringLiteral("bookmarks");

}

#endif
