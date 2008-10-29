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

/*!
  \file AutoSave.cpp
  \brief handles threads for file auto-save
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "AutoSave.h"
#include "Debug.h"
#include "MainWindow.h"
#include "Options.h"
#include "TextDisplay.h"

using namespace std;

//______________________________________________________
AutoSave::AutoSave( QObject* parent ):
  QObject( parent ),
  Counter( "AutoSave" ),
  interval_( 0 ),
  timer_( this )
{
  
  Debug::Throw( "AutoSave::AutoSave.\n" );
  timer_.setSingleShot( true );
  connect( &timer_, SIGNAL( timeout() ), this, SLOT( saveFiles() ) );
  
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );

}

//______________________________________________________
AutoSave::~AutoSave( void )
{
 
  Debug::Throw( "AutoSave::~AutoSave.\n" );
  
  // loop over threads
  for( ThreadList::iterator iter = threads_.begin(); iter != threads_.end(); iter++ )
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
  threads_.push_back( thread );
  
  // save file immediatly
  if( interval_ ) saveFiles( display );
  
}

//______________________________________________________
void AutoSave::saveFiles( const TextDisplay* display )
{

  Debug::Throw( "AutoSave::saveFiles.\n" );
  
  // do nothing if interval is 0
  if( !interval_ || threads_.empty() ) return;

  // needed to see if display was found.
  bool found( false );
  
  // loop over threads and restart
  for( ThreadList::iterator iter = threads_.begin(); iter != threads_.end(); iter++ )
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
      if( iter == threads_.end() ) break;
      else continue;
      
    }
    
    // update file and content
    TextDisplay& display( **displays.begin() );
    if( !( display.file().empty() || display.isNewDocument() ) )
    {
      (*iter)->setFile( display.file() );
      (*iter)->setContents( display.toPlainText() );
      (*iter)->start();
    }
    
  }
  
  // check if thread matching display was found
  assert( found || !display );
  
  // restart timer 
  if( !threads_.empty() )  timer_.start();
  Debug::Throw( "AutoSave::saveFiles - done. \n" );

}

//______________________________________________________
void AutoSave::_updateConfiguration( void )
{
  
  Debug::Throw( "AutoSave::_updateConfiguration.\n" );

  // save AutoSave interval and start timer
  interval_ = 1000*XmlOptions::get().get<unsigned int>("AUTOSAVE_INTERVAL");
  if( interval_ > 0 ) {
    timer_.setInterval( interval_ );
    timer_.start();
  } else timer_.stop();
  
}
