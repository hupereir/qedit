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
  \file    DocumentClassModel.h
  \brief   Job model. Stores job information for display in lists
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include "DocumentClassModel.h"
using namespace std;

//__________________________________________________________________
const char* DocumentClassModel::column_titles_[DocumentClassModel::n_columns] = 
{
  "name",
  "file",
};

//__________________________________________________________________
QVariant DocumentClassModel::data( const QModelIndex& index, int role ) const
{
  Debug::Throw( "DocumentClassModel::data.\n" );
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  
  // retrieve associated file info
  const DocumentClass& document_class( get()[index.row()] );
  
  // return text associated to file and column
  if( role == Qt::DisplayRole )
  {
    if( index.column() == NAME ) return document_class.name();
    if( index.column() == FILE ) return document_class.file().c_str();
  }
  
  return QVariant();
  
}

//__________________________________________________________________
QVariant DocumentClassModel::headerData(int section, Qt::Orientation orientation, int role) const
{

  if( 
    orientation == Qt::Horizontal && 
    role == Qt::DisplayRole && 
    section >= 0 && 
    section < n_columns )
  { return QString( column_titles_[section] ); }
  
  // return empty
  return QVariant(); 

}


//____________________________________________________________
void DocumentClassModel::_sort( int column, Qt::SortOrder order )
{ 

  Debug::Throw() << "DocumentClassModel::sort - column: " << column << " order: " << order << endl;
  std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) );
      
}

//________________________________________________________
bool DocumentClassModel::SortFTor::operator () ( const DocumentClass& first, const DocumentClass& second ) const
{
  
  switch( type_ )
  {
    
    case NAME: return ( order_ == Qt::AscendingOrder ) ? second.name() < first.name() : first.name() < second.name();
    case FILE: return ( order_ == Qt::AscendingOrder ) ? second.file() < first.file() : first.file() < second.file();
    default:
    throw runtime_error( DESCRIPTION( "invalid column" ) );
    return true;
  }

}
