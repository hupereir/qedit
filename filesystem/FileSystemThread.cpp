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
#include <QtGui/QApplication>

//______________________________________________________
QEvent::Type FileSystemEvent::eventType( void )
{

    #if QT_VERSION >= 0x040400
    static QEvent::Type event_type = (QEvent::Type) QEvent::registerEventType();
    #else
    static QEvent::Type event_type = QEvent::User;
    #endif

    return event_type;

}

//______________________________________________________
FileSystemThread::FileSystemThread( QObject* reciever ):
    Counter( "FileSystemThread" ),
    reciever_( reciever ),
    sizePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::SIZE ) ),
    showHiddenFiles_( false )
{}

//______________________________________________________
void FileSystemThread::run( void )
{

    // loop over directory contents
    FileSystemModel::List newFiles;

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
        newFiles << record;

    }

    qApp->postEvent( reciever_, new FileSystemEvent( path_, newFiles ) );

    return;

}
