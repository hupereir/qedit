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
  \file FileCheck.cpp
  \brief keep track of external file modifications
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <algorithm>
#include <assert.h>

#include "Debug.h"
#include "File.h"
#include "FileCheck.h"
#include "TextDisplay.h"
#include "TextView.h"

using namespace std;

//____________________________________________________
FileCheck::FileCheck( QObject* parent ):
  QObject( parent ),
  Counter( "FileCheck" )
{ 
  Debug::Throw( "FileCheck::FileCheck.\n" );
  connect( &_fileSystemWatcher(), SIGNAL( fileChanged( const QString& ) ), SLOT( _fileChanged( const QString& ) ) );
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
    _fileSystemWatcher().addPath( file );
  } 

}

//______________________________________________________
void FileCheck::removeFile( const QString& file )
{
  
  Debug::Throw() << "FileCheck::removeFile: " << file << endl;
  files_.erase( file );
  _fileSystemWatcher().removePath( file );
  
  return;
  
}

//______________________________________________________
void FileCheck::printMonitoredFiles( void )
{
  
  Debug::Throw( 0, "FileCheck::printMonitoredFiles.\n" );
  QStringList files( _fileSystemWatcher().files());  
  
  for( QStringList::const_iterator iter = files.begin(); iter != files.end(); iter++ )
  { Debug::Throw(0) << "FileCheck::printMonitoredFiles - " << *iter << endl; }
  
  Debug::Throw(0) << endl;

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
    
    Debug::Throw(0) << "FileCheck::_fileChanged - removed" << endl;
    data.setFlag( Data::REMOVED );
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
    for( DataSet::const_iterator iter = data_.begin(); iter != data_.end(); iter++ )
    {
      
      BASE::KeySet<TextDisplay>::iterator display_iter( find_if( displays.begin(), displays.end(), TextDisplay::SameFileFTor( iter->file() ) ) );
      if( display_iter != displays.end() )
      {
        
        // assign to this display and others
        BASE::KeySet<TextDisplay> associates( *display_iter );
        associates.insert( *display_iter );
        for( BASE::KeySet<TextDisplay>::iterator display_iter = associates.begin(); display_iter != associates.end(); display_iter++ )
        { 
          
          // check whether data are still relevant for this display
          if( !( iter->flag() == Data::REMOVED || ((*display_iter)->lastSaved().isValid() && (*display_iter)->lastSaved() < iter->timeStamp()) ) ) continue;
          
          (*display_iter)->setFileCheckData( *iter ); 
          if( !(*display_iter)->isActive() ) continue;

          // retrieve associated TextView
          BASE::KeySet<TextView> views( *display_iter );
          if( !views.empty() ) (*views.begin())->checkModifiedDisplays();
      
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
