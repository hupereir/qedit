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
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "FileIconProvider.h"
#include "FileSystemModel.h"

#include "CustomPixmap.h"
#include "FileSystemIcons.h"
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
    QIcon base;
    if( type & FileSystemModel::Navigator ) base = IconEngine::get( ICONS::PARENT );
    else if( type & FileSystemModel::Folder ) base = IconEngine::get( ICONS::FOLDER );
    else if( type & FileSystemModel::Document )
    {

        // try get icon from mimetypes
        base = mimeTypeIconProvider_.icon( fileRecord.file().extension() );
        if( base.isNull() ) base = IconEngine::get( ICONS::DOCUMENT );

    }

    {
        // normal icon. Only copy relevant pixmaps
        QIcon copy( base );
        base = QIcon();
        QIcon linkOverlay( IconEngine::get( ICONS::LINK ) );
        foreach( const QSize& size, copy.availableSizes() )
        { base.addPixmap( copy.pixmap( size ) ); }

    }

    // add link overlay
    if( type & FileSystemModel::Link )
    {

        QIcon copy( base );
        base = QIcon();
        QIcon linkOverlay( IconEngine::get( ICONS::LINK ) );
        foreach( const QSize& size, copy.availableSizes() )
        {

            QSize overlaySize;
            if( size.width() <= 16 ) overlaySize = QSize( 8, 8 );
            else if( size.width() <= 22 ) overlaySize = QSize( 8, 8 );
            else if( size.width() <= 32 ) overlaySize = QSize( 12, 12 );
            else if( size.width() <= 48 ) overlaySize = QSize( 16, 16 );
            else if( size.width() <= 64 ) overlaySize = QSize( 22, 22 );
            else if( size.width() <= 128 ) overlaySize = QSize( 48, 48 );
            else overlaySize = QSize( 64, 64 );

            base.addPixmap( CustomPixmap( copy.pixmap( size ) ).merge( linkOverlay.pixmap( overlaySize ), CustomPixmap::BOTTOM_RIGHT ) );

        }
    }

    // hidden file
    if( type & FileSystemModel::Hidden )
    {
        QIcon copy( base );
        base = QIcon();
        foreach( const QSize& size, copy.availableSizes() )
        { base.addPixmap( CustomPixmap( copy.pixmap( size ) ).transparent( 0.6 ) ); }

    }

    // insert in map and return
    return icons_.insert( key, base ).value();

}
