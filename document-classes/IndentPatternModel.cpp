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
  \file    IndentPatternModel.h
  \brief   Job model. Stores job information for display in lists
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include "IndentPatternModel.h"


//__________________________________________________________________
const QString IndentPatternModel::column_titles_[IndentPatternModel::n_columns] =
{
  "Id",
  "Name",
  "Type",
  "Scale",
  "Comments"
};

//__________________________________________________________________
QVariant IndentPatternModel::data( const QModelIndex& index, int role ) const
{
  Debug::Throw( "IndentPatternModel::data.\n" );

  // check index, role and column
  if( !index.isValid() ) return QVariant();

  // retrieve associated file info
  const IndentPattern& pattern( get()[index.row()] );

  // return text associated to file and column
  if( role == Qt::DisplayRole )
  {
    if( index.column() == ID ) return pattern.id();
    if( index.column() == NAME ) return pattern.name();
    if( index.column() == TYPE ) return pattern.typeName();
    if( index.column() == SCALE ) return pattern.scale();
    if( index.column() == COMMENTS ) return pattern.comments();
  }

  return QVariant();

}

//__________________________________________________________________
QVariant IndentPatternModel::headerData(int section, Qt::Orientation orientation, int role) const
{

  if(
    orientation == Qt::Horizontal &&
    role == Qt::DisplayRole &&
    section >= 0 &&
    section < n_columns )
  { return column_titles_[section]; }

  // return empty
  return QVariant();

}
