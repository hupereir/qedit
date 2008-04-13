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
  \file    HighlightPatternModel.h
  \brief   Job model. Stores job information for display in lists
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include "HighlightPatternModel.h"
using namespace std;

//__________________________________________________________________
const char* HighlightPatternModel::column_titles_[HighlightPatternModel::n_columns] = 
{
  "name",
  "parent",
  "type"
};

//__________________________________________________________________
QVariant HighlightPatternModel::data( const QModelIndex& index, int role ) const
{
  Debug::Throw( "HighlightPatternModel::data.\n" );
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  
  // retrieve associated file info
  const HighlightPattern& pattern( get()[index.row()] );
  
  // return text associated to file and column
  if( role == Qt::DisplayRole )
  {
    if( index.column() == NAME ) return pattern.name().c_str();
    if( index.column() == PARENT && !pattern.parent().empty() ) return pattern.parent().c_str();
    if( index.column() == TYPE ) return pattern.typeName().c_str();
  }
  
  return QVariant();
  
}

//__________________________________________________________________
QVariant HighlightPatternModel::headerData(int section, Qt::Orientation orientation, int role) const
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
