
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
  \file FileSystemModel.cpp
  \brief model for object records
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QApplication>
#include <algorithm>
#include <assert.h>

#include "CustomPixmap.h"
#include "IconEngine.h"
#include "Icons.h"
#include "FileRecordProperties.h"
#include "FileSystemModel.h"
#include "XmlOptions.h"

using namespace std;

//_______________________________________________
FileSystemModel::IconCache FileSystemModel::icons_;

//__________________________________________________________________
FileSystemModel::FileSystemModel( QObject* parent ):
  ListModel<FileRecord>( parent )
{
  Debug::Throw("FileSystemModel::FileSystemModel.\n" );

  column_titles_.push_back( "file" );
  column_titles_.push_back( "size" );
  column_titles_.push_back( "time" );

  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );

}
  
//__________________________________________________________________
Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
  
  // default flags
  Qt::ItemFlags flags;
  if( index.isValid() )
  {
  
    // check associated record validity
    const FileRecord& record( get(index) );
    if( record.isValid() ) flags |=  Qt::ItemIsEnabled |  Qt::ItemIsSelectable;
  
  }
  
  return flags;
  
}

//__________________________________________________________________
QVariant FileSystemModel::data( const QModelIndex& index, int role ) const
{
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  
  // retrieve associated file info
  const FileRecord& record( get(index) );
  unsigned int type( record.property<unsigned int>( FileRecordProperties::TYPE ) );
  
  // return text associated to file and column
  if( role == Qt::DisplayRole ) {
          
    switch( index.column() )
    {
      
      case FILE:
      {
        // store local nmae
        string local_name( record.file().localName() );
        
        // loop over previous rows to find a match and increment version number
        unsigned int version( 0 );
        for( int row = 0; row < index.row(); row++ )
        { 
          if( get( FileSystemModel::index( row, FILE ) ).file().localName() == local_name ) version++; 
        }
        
        // form output string.
        ostringstream what;
        what << local_name;
        if( version ) what << " (" << version+1 << ")";
        return what.str().c_str();
      }
      
      case SIZE: 
      {
        if( type | DOCUMENT ) return QString( record.property( FileRecordProperties::SIZE ).c_str() );
        else return QVariant();
      }
      
      case TIME: 
      {
        if( type | DOCUMENT ) return QString( TimeStamp( record.time() ).string().c_str() );
        else return QVariant();
      }
      
      default:
      return QVariant();
   
    }
    
  } else if( role == Qt::DecorationRole && index.column() == FILE ) {
    
    return icons_[type];
    
  }
  
  return QVariant();
  
}

//__________________________________________________________________
QVariant FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{

  if( 
    orientation == Qt::Horizontal && 
    role == Qt::DisplayRole && 
    section >= 0 && 
    section < (int) column_titles_.size() )
  { return column_titles_[section]; }
  
  // return empty
  return QVariant(); 

}
  

//_________________________________________________________
void FileSystemModel::_updateConfiguration( void )
{

  Debug::Throw( "FileSystemModel::_updateConfiguration.\n" );

  // install pixmaps
  icons_.clear();
  _installIcons();
  reset();
  
}

//____________________________________________________________
void FileSystemModel::_sort( int column, Qt::SortOrder order )
{ std::sort( _get().begin(), _get().end(), SortFTor( column, order, column_titles_ ) ); }
  
//________________________________________________________
bool FileSystemModel::SortFTor::operator () ( FileRecord first, FileRecord second ) const
{
  
  if( order_ == Qt::AscendingOrder ) swap( first, second );
  unsigned int first_type( first.property<unsigned int>( FileRecordProperties::TYPE ) );
  unsigned int second_type( second.property<unsigned int>( FileRecordProperties::TYPE ) );
  
  if( first_type & NAVIGATOR ) return true;
  if( second_type & NAVIGATOR ) return false;
  if( (first_type & FOLDER) && (second_type & DOCUMENT ) ) return true;
  if( (second_type & FOLDER) && (first_type & DOCUMENT ) ) return false;
  
  switch( type_ )
  {
    
    case FILE: return first.file().localName() < second.file().localName();
    case TIME: return (first.time() != second.time() ) ? first.time() < second.time() : first.file().localName() < second.file().localName();
    case SIZE: 
    {
      long int first_size( first.property<long int>( FileRecordProperties::SIZE ) );
      long int second_size( second.property<long int>( FileRecordProperties::SIZE ) );
      return (first_size != second_size ) ? first_size < second_size : first.file().localName() < second.file().localName();
    }
    
    default: return true;
    
  }
  
}

//_____________________________________________________________________
void FileSystemModel::_installIcons( void ) const
{

  Debug::Throw( "FileSystemModel::_installIcons.\n" );

  if( !icons_.empty() ) return;

  // pixmap size
  unsigned int pixmap_size = XmlOptions::get().get<unsigned int>( "LIST_ICON_SIZE" );
  QSize size( pixmap_size, pixmap_size );
  QSize scaled(size*0.9);

  // type icons
  typedef std::map< int, string > IconNames;
  IconNames type_names;
  type_names[DOCUMENT] = ICONS::DOCUMENT;
  type_names[FOLDER] = ICONS::FOLDER;
  type_names[NAVIGATOR] = ICONS::PARENT_DIRECTORY;

  // load link pixmap
  CustomPixmap link = CustomPixmap().find( ICONS::LINK );

  for( IconNames::iterator iter = type_names.begin(); iter != type_names.end(); iter++ )
  {

    icons_[iter->first] = CustomPixmap()
      .empty( size )
      .merge( CustomPixmap().find( iter->second )
      .scale( scaled ), CustomPixmap::CENTER );

    icons_[iter->first | LINK] = CustomPixmap()
      .empty( size )
      .merge( CustomPixmap().find( iter->second )
      .merge( link, CustomPixmap::BOTTOM_LEFT )
      .scale( scaled ), CustomPixmap::CENTER );

  }

  return;
}
