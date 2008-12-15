#ifndef _FileSystemModel_h_
#define _FileSystemModel_h_

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
  \file FileSystemModel.h
  \brief model for object counters
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <string.h>
#include <sstream>

#include "Counter.h"
#include "Debug.h"
#include "FileRecord.h"
#include "ListModel.h"
  
//! qlistview for object counters
class FileSystemModel: public ListModel<FileRecord>, public Counter
{

  Q_OBJECT
    
  public:

  //! file types
  enum FileType
  {
    DOCUMENT = 1<<0,
    FOLDER = 1<<1,
    LINK = 1<<2,
    NAVIGATOR = 1<<3,
    ANY = DOCUMENT | FOLDER | LINK | NAVIGATOR
  };
  
  //! number of columns
  enum { n_columns = 3 };

  //! column type enumeration
  enum ColumnType {
    FILE, 
    SIZE,
    TIME
  };

  //! constructor
  FileSystemModel( QObject* parent = 0 );
    
  //!@name methods reimplemented from base class
  //@{

  //! flags
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  
  // return data for a given index
  virtual QVariant data(const QModelIndex &, int ) const;
   
  //! header data
  virtual QVariant headerData(int, Qt::Orientation, int role = Qt::DisplayRole) const;
   
  //! number of columns for a given index
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
  { return column_titles_.size(); }

  //@}
  
  protected:
  
  //! sort
  virtual void _sort( int column, Qt::SortOrder order = Qt::AscendingOrder );
      
  private slots:
  
  //! configuration
  void _updateConfiguration( void );
  
  private:
  
  //! used to sort Counters
  class SortFTor: public ItemModel::SortFTor
  {
    
    public:
    
    //! constructor
    SortFTor( const int&, Qt::SortOrder, const std::vector<QString>& );
      
    //! prediction
    bool operator() ( FileRecord, FileRecord ) const;
    
    private:

    //! size property id 
    FileRecord::PropertyId::Id size_property_id_;
    
    //! column titles
    std::vector<QString> column_titles_;
    
  };
   
  //! install pixmaps
  void _installIcons( void ) const;

  //! icon cache
  typedef std::map<unsigned int, QIcon> IconCache;
       
  //! type icon cache
  static IconCache& _icons( void ); 
 
  //! column titles
  std::vector<QString> column_titles_;
  
  //! size property id
  FileRecord::PropertyId::Id size_property_id_;
  
};

#endif
