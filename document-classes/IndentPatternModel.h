#ifndef IndentPatternModel_h
#define IndentPatternModel_h

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
  \brief   Stores file information for display in lists
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include "Counter.h"
#include "ListModel.h"
#include "IndentPattern.h"

//! IndentPattern model. Stores file information for display in lists
class IndentPatternModel : public ListModel<IndentPattern>, public Counter
{
  
  public:
    
  //! constructor
  IndentPatternModel(QObject *parent = 0):
    ListModel<IndentPattern>(parent),
    Counter( "IndentPatternModel" )
  {}
  
  //! destructor
  virtual ~IndentPatternModel()
  {}
  
  //! number of columns
  enum { n_columns = 4 };

 //! column type enumeration
  enum ColumnType { 
    ID,
    NAME,
    TYPE,
    SCALE
  };

  //!@name methods reimplemented from base class
  //@{
  
  // return data for a given index
  virtual QVariant data(const QModelIndex &index, int role) const;
   
  //! header data
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  
  //! number of columns for a given index
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
  { return n_columns; }

  //@}
  
  protected:
  
  //! sort
  virtual void _sort( int column, Qt::SortOrder order = Qt::AscendingOrder )
  { return; }
   
  private:
  
  //! list column names
  static const QString column_titles_[n_columns];

};

#endif
