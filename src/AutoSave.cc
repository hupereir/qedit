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
  \file AutoSave.cc
  \brief handles threads for file auto-save
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "AutoSave.h"
#include "Debug.h"
#include "EditFrame.h"
#include "Exception.h"
#include "Options.h"
#include "TextDisplay.h"

using namespace std;
using namespace BASE;

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
  
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() );

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
    if( autosaved.exist() && autosaved.isWritable() ) autosaved.remove();
    delete *iter;
  
  }
     
}

//______________________________________________________
void AutoSave::newThread( TextDisplay* display )
{
 
  Debug::Throw( "AutoSave::newThread.\n" );
 
  // create new Thread
  AutoSaveThread *thread = new AutoSaveThread( this );
  
  // associate to EditFrame
  BASE::Key::Associate( display, thread );
  
  // add to list
  threads_.push_back( thread );
  
  // save file immediatly
  if( interval_ ) saveFiles( display );
  
}

//______________________________________________________
void AutoSave::updateConfiguration( void )
{
  
  Debug::Throw( "AutoSave::updateConfiguration.\n" );

  // save AutoSave interval and start timer
  int interval = 1000*Options::Get<int>("AUTOSAVE_INTERVAL");
  if( interval > 0 ) {
    timer_.setInterval( interval );
    timer_.start();
  } else timer_.stop();
  
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
    
    // retrieve associated EditFrames
    KeySet<TextDisplay> displays( *iter );

    // remove thread if none is found
    if( displays.empty() )
    {
      
      // remove file
      File autosaved( (*iter)->file() );
      if( autosaved.Exist() && autosaved.IsWritable() ) autosaved.remove();
      
      // delete thread
      delete *iter;
      
      // remove from list
      iter = threads_.erase( iter );
      
      continue;
    }
    
    // update file and content
    if( !(*displays.begin())->file().empty() )
    {
      (*iter)->setFile( (*displays.begin())->file() );
      (*iter)->SetContents( (*displays.begin())->toPlainText() );
      (*iter)->start();
    }
    
  }
  
  // check if thread matching display was found
  if( display && !found )
  { 
    ostringstream what;
    what << "AutoSave::SaveFiles - unable to find thread matching TextDisplay " << display->GetKey();
    throw logic_error( DESCRIPTION( what.str() ) );
  }

  // restart timer 
  if( !threads_.empty() )  timer_.start();

}

//____________________________________________
void AutoSave::customEvent( QEvent* event )
{
  Debug::Throw( "AutoSave::customEvent.\n" );

  if( event->type() != QEvent::User ) {
    Debug::Throw() << "AutoSave::customEvent - unrecognized type " << event->type() << endl;
    return;
  }

  AutoSaveEvent* autosave_event( static_cast<AutoSaveEvent*>(event) );
  if( !autosave_event ) return;

}
