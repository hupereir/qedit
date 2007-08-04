
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
* Place, Suite 330, Boston, MA  02111-1307 USA                           
*                         
*                         
*******************************************************************************/

/*!
  \file AutoSaveThread.cc
  \brief independent thread used to make regular backups of files
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <fstream>
#include <QApplication>
#include <QFile>

#include "AutoSaveThread.h"
#include "XmlOptions.h"
#include "Str.h"
#include "Util.h"

using namespace std;

//________________________________________________________________
File AutoSaveThread::autoSaveName( const File& file )
{
  
  Debug::Throw( "AutoSaveThread::MakeAutoSaveName.\n" );
  
  Str file_path( file.path().replace( "/", "_" ) );
  
  // retrieve backup path
  string path( XmlOptions::get().get<string>( "AUTOSAVE_PATH" ) );
  
  // retrieve user name
  string user( Util::user() );
  
  return File( user + file_path + file.localName() ).addPath( path );
}

//_______________________________________________________________
void AutoSaveThread::run( void )
{
  
  if( contents_changed_ || file_changed_ )
  {
    QFile out( file().c_str() );
    if( !out.open( QIODevice::WriteOnly ) )
    {
      qApp->postEvent( reciever_, new AutoSaveEvent( file_, false ) ); 
      return;
    }
  
    out.write( contents_.toAscii() );
    out.close();
    qApp->postEvent( reciever_, new AutoSaveEvent( file_, true ) ); 
  }
}
