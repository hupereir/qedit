#ifndef FileRecordProperties_h
#define FileRecordProperties_h

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

#include <QString>
#include "FileRecordBaseProperties.h"

namespace FileRecordProperties
{

    static const QString ClassName = "class_name";
    static const QString Wrapped = "wrap";
    static const QString Dictionary = "dictionary";
    static const QString Filter = "filter";
    static const QString Size = "size";

    //* needed to tag session file records
    enum Flag
    {
        None = 0,
        Modified = 1<<0,
        Active = 1<<1,
        Selected = 1<<2,
        Altered = 1<<3
    };

    Q_DECLARE_FLAGS( Flags, Flag );

};

Q_DECLARE_OPERATORS_FOR_FLAGS( FileRecordProperties::Flags );

#endif
