#ifndef DocumentClassDialog_h
#define DocumentClassDialog_h

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
* Place, Suite 330, Boston, MA  02111-1307 USA                           
*                         
*                         
*******************************************************************************/
 
/*!
  \file DocumentClassDialog.h
  \brief list document classes
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDialog>

#include "Counter.h"
#include "CustomListView.h"

class DocumentClass;

//! list document classes
class DocumentClassDialog: public QDialog, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT;
    
  public:
  
  //! constructor
  DocumentClassDialog( QWidget* parent );

  signals:
  
  //! emitted when a document class is selected
  void classSelected( std::string name );
  
  private slots:
  
  //! select document class
  void _select( QTreeWidgetItem* item )
  { emit classSelected( qPrintable( item->text( NAME ) ) ); }
  
  //! select document class
  void _select( void );
  
  //! remove a class from the list
  void _remove( void );
  
  //! edit file associated to class
  void _edit( void );
  
  //! load classes from a file
  void _loadFile( void );

  //! save classes to a file
  void _save( void );
  
  //! reload document classes
  void _reload( void );
  
  private:
  
  //! display all classes to listview
  void _load( void );
  
  //! add document class to listview
  void _add( const DocumentClass& );
  
  //! number of columns
  enum { n_columns = 3 };

  //! column type enumeration
  enum ColumnTypes {
    NAME,
    FILE,
    EMPTY
  }; 
  
  //! column titles
  static const char* column_titles_[ n_columns ];
  
  //! document classes list
  CustomListView* list_;
  
};

#endif
