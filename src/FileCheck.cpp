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
{ BASE::Key::associate( this, display ); }

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
void FileCheck::removeFile( const QString& file, bool forced )
{
  
  Debug::Throw() << "FileCheck::removeFile: " << file << endl;
  FileSet::iterator iter( files_.find( file ) );
  assert( iter != files_.end() );
  files_.erase( file );
  
  return;
  
}

//______________________________________________________
void FileCheck::_fileChanged( const QString& file )
{
  
  Debug::Throw() << "FileCheck::_fileChanged: " << file << endl;  
  
  // filecheck data
  Data data;
  
  // find associated display with matching file
  BASE::KeySet<TextDisplay> displays( this );
  BASE::KeySet<TextDisplay>::iterator iter( find_if( displays.begin(), displays.end(), TextDisplay::SameFileFTor( file ) ) );
  if( iter != displays.end() )
  {
    
    File local( file );
    if( !local.exists() ) data.setFlag( Data::REMOVED );
    else {
      data.setFlag( Data::MODIFIED );
      data.setTimeStamp( local.lastModified() );
    }      
  
    // assign to this display and others
    BASE::KeySet<TextDisplay> associates( *iter );
    associates.insert( *iter );
    for( BASE::KeySet<TextDisplay>::iterator iter = associates.begin(); iter != associates.end(); iter++ )
    { (*iter)->setFileCheckData( data ); }
    
  } else { 
    
    // remove file from list otherwise
    removeFile( file, true );
    
  }
      
}
