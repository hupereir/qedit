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

#include "AutoSave.h"
#include "AutoSave.moc"
#include "Debug.h"
#include "MainWindow.h"
#include "Options.h"
#include "Singleton.h"
#include "TextDisplay.h"

//______________________________________________________
AutoSave::AutoSave( QObject* parent ):
    QObject( parent ),
    Counter( "AutoSave" ),
    enabled_( false ),
    interval_( 0 )
{
    Debug::Throw( "AutoSave::AutoSave.\n" );
    connect( Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
}

//______________________________________________________
AutoSave::~AutoSave( void )
{

    Debug::Throw( "AutoSave::~AutoSave.\n" );

    // loop over threads
    for( ThreadList::iterator iter = threads_.begin(); iter != threads_.end(); ++iter )
    {

        // remove file
        File autosaved( (*iter)->file() );
        if( autosaved.exists() && autosaved.isWritable() ) autosaved.remove();
        delete *iter;

    }

}

//______________________________________________________
void AutoSave::newThread( TextDisplay* display )
{

    Debug::Throw( "AutoSave::newThread.\n" );

    // create new Thread
    AutoSaveThread *thread = new AutoSaveThread( this );

    // associate to MainWindow
    BASE::Key::associate( display, thread );

    // add to list
    threads_.append( thread );

    // save file immediatly
    if( _enabled() ) saveFiles( display );

}

//______________________________________________________
void AutoSave::saveFiles( const TextDisplay* display )
{

    Debug::Throw( "AutoSave::saveFiles.\n" );

    // do nothing if interval is 0
    if( !( _enabled() ) || threads_.empty() ) return;

    // needed to see if display was found.
    bool found( false );

    // loop over threads and restart
    for( ThreadList::iterator iter = threads_.begin(); iter != threads_.end(); ++iter )
    {

        // check if argument display, if valid, is associated to this thread
        if( display && !(*iter)->isAssociated( display ) ) continue;
        found = true;

        // if thread is running, skipp
        if( (*iter)->isRunning() ) continue;

        // retrieve associated displays
        BASE::KeySet<TextDisplay> displays( *iter );

        // remove thread if none is found
        if( displays.empty() )
        {

            // remove file
            File autosaved( (*iter)->file() );
            if( autosaved.exists() && autosaved.isWritable() ) autosaved.remove();

            // delete thread
            delete *iter;

            // remove from list
            // advance iterator and check for end of list
            iter = threads_.erase( iter );

            // check if at end
            if( iter == threads_.end() ) break;
            else continue;

        }

        // update file and content
        TextDisplay& display( **displays.begin() );
        if( !( display.file().isEmpty() || display.isNewDocument() ) )
        {
            (*iter)->setFile( display.file() );
            (*iter)->setContents( display.toPlainText() );
            (*iter)->setTextEncoding( display.textEncoding() );
            (*iter)->start();
        }

    }

    // check if thread matching display was found
    Q_ASSERT( found || !display );

    // restart timer
    if( !threads_.empty() )  timer_.start( interval_, this );

}

//______________________________________________________
void AutoSave::timerEvent( QTimerEvent* event )
{

    if( event->timerId() == timer_.timerId() )
    {

        timer_.stop();
        saveFiles();

    } else return QObject::timerEvent( event );

}

//______________________________________________________
void AutoSave::_updateConfiguration( void )
{

    Debug::Throw( "AutoSave::_updateConfiguration.\n" );

    // save AutoSave interval and start timer
    enabled_ = XmlOptions::get().get<bool>( "AUTOSAVE" );
    interval_ = 1000*XmlOptions::get().get<int>("AUTOSAVE_INTERVAL");

    if( interval_ > 0 ) timer_.start( interval_, this );
    else timer_.stop();

}
