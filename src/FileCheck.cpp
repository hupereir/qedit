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
*******************************************************************************/

#include "Debug.h"
#include "File.h"
#include "FileCheck.h"
#include "TextDisplay.h"
#include "TextView.h"

#include <QtGui/QApplication>
#include <algorithm>
#include <cassert>

//____________________________________________________
FileCheck::FileCheck( QObject* parent ):
    QObject( parent ),
    Counter( "FileCheck" )
{
    Debug::Throw( "FileCheck::FileCheck.\n" );
    connect( &fileSystemWatcher_, SIGNAL( fileChanged( const QString& ) ), SLOT( _fileChanged( const QString& ) ) );
}

//______________________________________________________
FileCheck::~FileCheck( void )
{ Debug::Throw( "FileCheck::~FileCheck.\n" ); }

//______________________________________________________
void FileCheck::registerDisplay( TextDisplay* display )
{
    Debug::Throw( "FileCheck::registerDisplay.\n" );
    if( !isAssociated( display ) ) { BASE::Key::associate( this, display ); }
}

//______________________________________________________
void FileCheck::addFile( const QString& file )
{

    Debug::Throw() << "FileCheck::addFile: " << file << endl;
    if( files_.find( file ) == files_.end() )
    {
        files_.insert( file );
        fileSystemWatcher_.addPath( file );
    }

}

//______________________________________________________
void FileCheck::removeFile( const QString& file )
{

    Debug::Throw() << "FileCheck::removeFile: " << file << endl;
    files_.erase( file );
    fileSystemWatcher_.removePath( file );

    return;

}

//______________________________________________________
void FileCheck::_fileChanged( const QString& file )
{

    Debug::Throw() << "FileCheck::_fileChanged: " << file << endl;

    // filecheck data
    Data data( file );
    File local( file );

    if( !local.exists() )
    {

        data.setFlag( Data::REMOVED );
        data.setTimeStamp( TimeStamp::now() );
        removeFile( file );

    } else {

        data.setFlag( Data::MODIFIED );
        data.setTimeStamp( local.lastModified() );

    }

    data_.insert( data );
    timer_.start( 200, this );

}

//______________________________________________________
void FileCheck::timerEvent( QTimerEvent* event )
{
    if( event->timerId() == timer_.timerId() )
    {

        // stop timer
        timer_.stop();
        if( data_.empty() ) return;

        BASE::KeySet<TextDisplay> displays( this );
        for( DataSet::const_iterator iter = data_.begin(); iter != data_.end(); ++iter )
        {

            BASE::KeySet<TextDisplay>::iterator displayIter( find_if( displays.begin(), displays.end(), TextDisplay::SameFileFTor( iter->file() ) ) );
            if( displayIter != displays.end() )
            {

                // assign to this display and others
                BASE::KeySet<TextDisplay> associates( *displayIter );
                associates.insert( *displayIter );
                for( BASE::KeySet<TextDisplay>::iterator displayIter = associates.begin(); displayIter != associates.end(); ++displayIter )
                {

                    // check whether data are still relevant for this display
                    if( !( iter->flag() == Data::REMOVED || ((*displayIter)->lastSaved().isValid() && (*displayIter)->lastSaved() < iter->timeStamp()) ) )
                    { continue; }

                    (*displayIter)->setFileCheckData( *iter );
                    if( !( (*displayIter)->isActive() && (*displayIter)->QTextEdit::hasFocus() ) )
                    { continue; }

                    // retrieve associated TextView
                    BASE::KeySet<TextView> views( *displayIter );
                    if( !views.empty() ) (*views.begin())->checkDisplayModifications( *displayIter );

                }

            } else {

                // permanently remove file from list
                removeFile( iter->file() );

            }

        }

        // clear
        data_.clear();

    } else return QObject::timerEvent( event );


}
