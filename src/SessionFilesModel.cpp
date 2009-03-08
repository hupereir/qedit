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
#include <assert.h>
#include <QIcon>
#include <QPalette>

#include "Icons.h"
#include "CustomPixmap.h"
#include "FileRecordProperties.h"
#include "SessionFilesModel.h"
#include "Singleton.h"
#include "XmlOptions.h"

using namespace std;

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
    
  } else assert(0);

  // store in map and return
  _icons().insert( make_pair( type, icon ) );
  return icon;
   
}
