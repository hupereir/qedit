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

#include "FileIconProvider.h"
#include "FileSystemModel.h"

#include "CustomPixmap.h"
#include "FileSystemIconNames.h"
#include "IconEngine.h"

//__________________________________________________________________
const QIcon& FileIconProvider::icon( const FileRecord& fileRecord )
{

    // get relevant file info type
    int type( fileRecord.flags() );
    if( type & FileSystemModel::Folder )
    {

        // copy to file info and get base class icon
        BaseFileInfo fileInfo( fileRecord.file() );
        fileInfo.setIsFolder();
        if( type & FileSystemModel::Link ) fileInfo.setIsLink();

        // try from base class
        const QIcon& icon( BaseFileIconProvider::icon( fileInfo ) );
        if( !icon.isNull() ) return icon;

    }

    if( type & FileSystemModel::Navigator ) type = FileSystemModel::Navigator;
    else type &= FileSystemModel::Any;

    // build key
    Key key( fileRecord.file().extension(), type );

    // check if already created
    IconCache::const_iterator iter( icons_.find( key) );
    if( iter != icons_.end() ) return iter.value();

    // create
    QIcon out;
    if( type & FileSystemModel::Navigator ) out = IconEngine::get( IconNames::Parent );
    else if( type & FileSystemModel::Folder ) out = IconEngine::get( IconNames::Folder );
    else if( type & FileSystemModel::Document )
    {

        // try get icon from mimetypes
        out = mimeTypeIconProvider_.icon( fileRecord.file().extension() );
        if( out.isNull() ) out = IconEngine::get( IconNames::Document );

    }

    out = IconEngine::copy( out );
    if( type & FileSystemModel::Link ) out = _linked( out );
    if( type & FileSystemModel::Hidden ) out = _hidden( out );

    // insert in map and return
    return icons_.insert( key, out ).value();

}
