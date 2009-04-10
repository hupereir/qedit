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
  \file FileSystemThread.cpp
  \brief check validity of a set of files
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QApplication>
#include <QDateTime>
#include <QDir>

#include "Debug.h"
#include "FileSystemThread.h"
#include "FileRecordProperties.h"

using namespace std;

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
  size_property_id_( FileRecord::PropertyId::get( FileRecordProperties::SIZE ) ),
  show_hidden_files_( false )
{}
  
//______________________________________________________
void FileSystemThread::run( void )
{

  // loop over directory contents
  QDir dir( path_ );
  QDir::Filters filter = QDir::AllEntries | QDir::NoDotAndDotDot;
  if( show_hidden_files_ ) filter |= QDir::Hidden;
  dir.setFilter( filter );
  QFileInfoList entries( dir.entryInfoList() );
  FileSystemModel::List new_files;
  
  // add navigator
  FileRecord record( File("..") );
  record.setFlags( FileSystemModel::NAVIGATOR );
  new_files.push_back( record );
  
  // loop over entries and add
  for( QFileInfoList::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    
    if( iter->fileName() == ".." || iter->fileName() == "." ) continue;
    
    // create file record
    FileRecord record( iter->fileName(), TimeStamp( iter->lastModified().toTime_t() ) );
    
    // assign size
    record.addProperty( size_property_id_, QString().setNum(iter->size()) );
    
    // assign type
    record.setFlag( iter->isDir() ? FileSystemModel::FOLDER : FileSystemModel::DOCUMENT );
    if( iter->isSymLink() ) record.setFlag( FileSystemModel::LINK );
        
    // add to model
    new_files.push_back( record );
    
  }

  qApp->postEvent( reciever_, new FileSystemEvent( path_, new_files ) );  

  return;
  
}
