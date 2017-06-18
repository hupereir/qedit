#ifndef FileIconProvider_h
#define FileIconProvider_h

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

#include "BaseFileIconProvider.h"
#include "Counter.h"
#include "FileRecord.h"
#include "MimeTypeIconProvider.h"

#include <QHash>
#include <QIcon>

//* icon provider for file records
class FileIconProvider : public BaseFileIconProvider
{

    public:

    //* constructor
    explicit FileIconProvider( QObject* parent = nullptr ):
        BaseFileIconProvider( parent )
    {}

    //* icon matching given model index
    using BaseFileIconProvider::icon;
    const QIcon& icon( const FileRecord& );

    private:

    //* mime type icon provider
    MimeTypeIconProvider mimeTypeIconProvider_;

};

#endif
