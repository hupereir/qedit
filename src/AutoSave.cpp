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
#include "Application.h"
#include "AutoSaveThread.h"
#include "Debug.h"
#include "MainWindow.h"
#include "Options.h"
#include "Singleton.h"
#include "TextDisplay.h"


#include <algorithm>

//______________________________________________________
AutoSave::AutoSave( QObject* parent ):
    QObject( parent ),
    Counter( QStringLiteral("AutoSave") )
{
    Debug::Throw( QStringLiteral("AutoSave::AutoSave.\n") );
    connect( Base::Singleton::get().application<Application>(), &Application::configurationChanged, this, &AutoSave::_updateConfiguration );
}

//______________________________________________________
void AutoSave::newThread( TextDisplay* display )
{

    Debug::Throw( QStringLiteral("AutoSave::newThread.\n") );

    // create thread with custom deleter
    ThreadPtr thread( new AutoSaveThread( this ), [](AutoSaveThread* thread)
    {
        if( thread->isRunning() )
        {
            thread->disconnect();
            QObject::connect( thread, &QThread::finished, thread, [thread]()
            {
                auto autosaved( thread->file() );
                if( autosaved.exists() && autosaved.isWritable() ) autosaved.remove();
                thread->deleteLater();
            } );
            thread->quit();
        } else {
            auto autosaved( thread->file() );
            if( autosaved.exists() && autosaved.isWritable() ) autosaved.remove();
            thread->deleteLater();
        }
    });

    // associate to MainWindow
    Base::Key::associate( display, thread.get() );

    // add to list
    threads_.append( thread );

    // save file immediatly
    if( _enabled() ) saveFiles( display );

}

//______________________________________________________
void AutoSave::saveFiles( const TextDisplay* display )
{

    Debug::Throw( QStringLiteral("AutoSave::saveFiles.\n") );

    // do nothing if interval is 0
    if( !( _enabled() ) || threads_.empty() ) return;

    // update thread from display
    auto updateThread = [] (AutoSaveThread* thread, const TextDisplay& display)
    {
        thread->setFile( display.file() );
        thread->setContent( display.toPlainText() );
        thread->setTextEncoding( display.textEncoding() );
        thread->setUseCompression( display.useCompression() );
        thread->start();
    };

    if( display )
    {
        if( !( display->file().isEmpty() || display->isNewDocument() ) )
        {
            // if a valid display is provided
            for( auto thread:Base::KeySet<AutoSaveThread>( display ) )
            { if( !thread->isRunning() ) updateThread( thread, *display ); }
        }
    } else {
        // first remove empty threads
        threads_.erase(
            std::remove_if( threads_.begin(), threads_.end(),
            [](ThreadPtr thread)
            { return !thread->isRunning() && Base::KeySet<TextDisplay>(thread.get()).empty(); }),
            threads_.end() );

        // loop over threads and restart
        for( auto&& iter = threads_.begin(); iter != threads_.end(); ++iter )
        {
            // check if argument display, if valid, is associated to this thread
            if( display && !(*iter)->isAssociated( display ) ) continue;

            // if thread is running, skipp
            if( (*iter)->isRunning() ) continue;

            // retrieve associated displays
            Base::KeySet<TextDisplay> displays( iter->get() );

            // update file and content
            auto&& display( **displays.begin() );
            if( !( display.file().isEmpty() || display.isNewDocument() ) )
            { updateThread( iter->get(), display ); }
        }
    }

    // restart timer
    if( !( threads_.empty() || timer_.isActive() ) )  timer_.start( interval_, this );
}

//______________________________________________________
void AutoSave::timerEvent( QTimerEvent* event )
{
    if( event->timerId() == timer_.timerId() )
    {
        timer_.stop();
        saveFiles();
    } else QObject::timerEvent( event );
}

//______________________________________________________
void AutoSave::_updateConfiguration()
{
    Debug::Throw( QStringLiteral("AutoSave::_updateConfiguration.\n") );
    // save AutoSave interval and start timer
    enabled_ = XmlOptions::get().get<bool>( QStringLiteral("AUTOSAVE") );
    interval_ = 1000*XmlOptions::get().get<int>(QStringLiteral("AUTOSAVE_INTERVAL"));
    if( interval_ > 0 ) timer_.start( interval_, this );
    else timer_.stop();
}
