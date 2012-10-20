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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA 02111-1307 USA
*
*
*******************************************************************************/

#include "Debug.h"
#include "FileSystemThread.h"
#include "FileRecordProperties.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>

//______________________________________________________
FileSystemThread::FileSystemThread( QObject* parent ):
    QThread( parent ),
    Counter( "FileSystemThread" ),
    sizePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::SIZE ) ),
    showHiddenFiles_( false )
{
    qRegisterMetaType<File>( "File" );
    qRegisterMetaType<File::List>( "FileRecord::List" );
}

//______________________________________________________
void FileSystemThread::run( void )
{

    // clear files
    records_.clear();

    // loop over entries and add
    unsigned int flags = File::None;
    if( showHiddenFiles_ ) flags |= File::ShowHiddenFiles;
    foreach( const File& file, path_.listFiles( flags ) )
    {

        // check if hidden
        if( showHiddenFiles_ && file.isHidden() ) continue;

        // create file record
        FileRecord record( file, file.lastModified() );

        // assign size
        record.addProperty( sizePropertyId_, QString().setNum(file.fileSize()) );

        // assign type
        record.setFlag( file.isDirectory() ? FileSystemModel::Folder : FileSystemModel::Document );
        if( file.isLink() ) record.setFlag( FileSystemModel::Link );

        // add to model
        records_ << record;

    }

    emit filesAvailable( path_, records_ );
    return;

}
