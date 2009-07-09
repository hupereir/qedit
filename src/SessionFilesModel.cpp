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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        
* for more details.                     
*                          
* You should have received a copy of the GNU General Public License along with 
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     
* Place, Suite 330, Boston, MA  02111-1307 USA                           
*                         
*                         
*******************************************************************************/
 
/*!
  \file SessionFilesModel.cpp
  \brief model for object records
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <algorithm>
#include <cassert>
#include <QIcon>
#include <QMimeData>
#include <QPalette>
#include <QSet>

#include "Icons.h"
#include "CustomPixmap.h"
#include "FileRecordProperties.h"
#include "SessionFilesModel.h"
#include "Singleton.h"
#include "XmlFileList.h"
#include "XmlFileRecord.h"
#include "XmlOptions.h"

using namespace std;

//______________________________________________________________
const QString SessionFilesModel::DRAG = "base/sessionfilesmodel/drag";

//__________________________________________________________________
SessionFilesModel::IconCache& SessionFilesModel::_icons( void )
{ 
  static IconCache cache;
  return cache;
}

//__________________________________________________________________
SessionFilesModel::SessionFilesModel( QObject* parent ):
  FileRecordModel( parent )
{
  
  Debug::Throw("SessionFilesModel::SessionFilesModel.\n" );
  setShowIcons( false );
  connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );

}
  
//__________________________________________________________________
Qt::ItemFlags SessionFilesModel::flags(const QModelIndex &index) const
{
  
  // get flags from parent class
  Qt::ItemFlags flags( FileRecordModel::flags( index ) );
  return flags | Qt::ItemIsDropEnabled;
  
}

//__________________________________________________________________
QVariant SessionFilesModel::data( const QModelIndex& index, int role ) const
{
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  
  // retrieve associated file info
  const FileRecord& record( get(index) );
  
  if( role == Qt::DecorationRole && index.column() == ICON ) 
  {
    
    if( record.hasFlag( FileRecordProperties::MODIFIED ) ) return _icon( FileRecordProperties::MODIFIED );
    else if( record.hasFlag( FileRecordProperties::ALTERED ) ) return _icon( FileRecordProperties::ALTERED );
    else return _icon( FileRecordProperties::NONE );

  } else if( role == Qt::ForegroundRole ) {
    
    return record.hasFlag( FileRecordProperties::ACTIVE ) ? 
      QPalette().color( QPalette::Text ):
      QPalette().color( QPalette::Disabled, QPalette::Text );
    
  } else return FileRecordModel::data( index, role );
 
  return QVariant();
  
}

//____________________________________________________________
void SessionFilesModel::_updateConfiguration( void )
{
  Debug::Throw( "SessionFilesModel::_updateConfiguration.\n" );
  _icons().clear();
}

//________________________________________________________
QIcon SessionFilesModel::_icon( unsigned int type )
{

  //Debug::Throw( "SessionFilesModel::_icon.\n" );
   
  IconCache::const_iterator iter( _icons().find( type ) );
  if( iter != _icons().end() ) return iter->second;

  // pixmap size
  unsigned int pixmap_size = XmlOptions::get().get<unsigned int>( "LIST_ICON_SIZE" );
  QSize size( pixmap_size, pixmap_size );
  QSize scale(size*0.9);
 
  QIcon icon;
  if( type == FileRecordProperties::MODIFIED ) 
  {
  
    icon = CustomPixmap()
      .empty( size )
      .merge( CustomPixmap().find( ICONS::SAVE )
      .scaled( scale, Qt::KeepAspectRatio, Qt::SmoothTransformation ), CustomPixmap::CENTER );
  
  } else if( type == FileRecordProperties::ALTERED ) {
    
    icon = CustomPixmap()
      .empty( size )
      .merge( CustomPixmap().find( ICONS::WARNING )
      .scaled( scale, Qt::KeepAspectRatio, Qt::SmoothTransformation ), CustomPixmap::CENTER );
    
  } else if( type == FileRecordProperties::NONE ) {
    
    icon = CustomPixmap().empty( size );
    
  } else assert( false );

  // store in map and return
  _icons().insert( make_pair( type, icon ) );
  return icon;
   
}

//______________________________________________________________________
QStringList SessionFilesModel::mimeTypes( void ) const
{
  QStringList types;
  types << DRAG;
  return types;
}

//______________________________________________________________________
QMimeData* SessionFilesModel::mimeData(const QModelIndexList &indexes) const
{

  // return FileRecordModel::mimeData( indexes );
  std::set<QString> filenames;
  std::set<FileRecord> records;
  for( QModelIndexList::const_iterator iter = indexes.begin(); iter != indexes.end(); iter++ )
  { 
  
    if( iter->isValid() ) 
    {
      FileRecord record( get(*iter ) );
      records.insert( record );
      filenames.insert( record.file() ); 
    }
    
  }
  
  if( filenames.empty() ) return 0;
  else {
    
    QMimeData *mime = new QMimeData();
    
    // fill text data
    QString full_text;
    QTextStream buffer( &full_text );
    for( std::set<QString>::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++ )
    { buffer << *iter << endl; }
    mime->setText( full_text );
    
    // fill DRAG data. Use XML
    QDomDocument document;
    QDomElement top = document.appendChild( document.createElement( XmlFileRecord::XML_FILE_LIST ) ).toElement();
    for( std::set<FileRecord>::const_iterator iter = records.begin(); iter != records.end(); iter++ )
    { 
      
      if( iter->file().isEmpty() ) continue;
      top.appendChild( XmlFileRecord( *iter ).domElement( document ) ); 
      
    }
    mime->setData( DRAG, document.toByteArray() ); 
    return mime;
    
  }

}

//__________________________________________________________________
bool SessionFilesModel::dropMimeData(const QMimeData* data , Qt::DropAction action, int row, int column, const QModelIndex& parent)
{ 
    
  // check action
  if( action == Qt::IgnoreAction) return true;

  // Drag from Keyword model
  if( !data->hasFormat( DRAG ) ) return false;
  
  FileRecordModel::List records;
  
  // get dropped file record (use XML)
  // dom document
  QDomDocument document;
  if( !document.setContent( data->data( DRAG ), false ) ) return false;
  
  QDomElement doc_element = document.documentElement();
  QDomNode node = doc_element.firstChild();
  for(QDomNode node = doc_element.firstChild(); !node.isNull(); node = node.nextSibling() )
  {
    QDomElement element = node.toElement();
    if( element.isNull() ) continue;

    // special options
    if( element.tagName() == XmlFileRecord::XML_RECORD ) 
    {
    
      XmlFileRecord record( element );
      if( !record.file().isEmpty() ) records.push_back( record );
      
    }
  }
  
  // get current record
  if( parent.isValid() )
  {
    
    FileRecord target( get( parent ) );
  
    // loop over sources and emit proper signal
    for( FileRecordModel::List::const_iterator iter = records.begin(); iter != records.end(); iter++ )
    { emit reparentFiles( iter->file(), target.file() ); }
    return true;
    
  } else {
    
    // look for first active file in this window
    FileRecord target;
    QModelIndex target_index;
    for( int row = 0; row < rowCount(); row++ )
    { 
      
      QModelIndex index( SessionFilesModel::index( row, 0 ) );
      if( flags( index ) & Qt::ItemIsEnabled )
      { 
        FileRecord record( get( index ) );
        if( record.hasFlag( FileRecordProperties::ACTIVE ) )
        {
          target_index = index;
          target = record;
          break;
        }
        
      }
    }
    
    // check that target_index is valid
    if( !target_index.isValid() ) return false;
  
    // emit relevant reparent signal
    for( FileRecordModel::List::const_iterator iter = records.begin(); iter != records.end(); iter++ )
    { emit reparentFilesToMain( iter->file(), target.file() ); }
    
    return true;

  }
    
}
