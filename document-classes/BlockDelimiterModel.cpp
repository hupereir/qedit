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
  \file    BlockDelimiterModel.h
  \brief   Job model. Stores job information for display in lists
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include "BlockDelimiterModel.h"
using namespace std;

//__________________________________________________________________
const QString BlockDelimiterModel::column_titles_[BlockDelimiterModel::n_columns] =
{
  "First",
  "Second",
  "Regular Expression"
};

//__________________________________________________________________
QVariant BlockDelimiterModel::data( const QModelIndex& index, int role ) const
{
  Debug::Throw( "BlockDelimiterModel::data.\n" );

  // check index, role and column
  if( !index.isValid() ) return QVariant();

  // retrieve associated file info
  const BlockDelimiter& delimiter( get()[index.row()] );

  // return text associated to file and column
  if( role == Qt::DisplayRole )
  {
    if( index.column() == FIRST ) return delimiter.first();
    if( index.column() == SECOND) return delimiter.second();
    if( index.column() == REGEXP && delimiter.regexp().isValid() ) return delimiter.regexp().pattern();
  }

  return QVariant();

}

//__________________________________________________________________
QVariant BlockDelimiterModel::headerData(int section, Qt::Orientation orientation, int role) const
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
