#ifndef DocumentClassManagerDialog_h
#define DocumentClassManagerDialog_h

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
  \file DocumentClassManagerDialog.h
  \brief list document classes
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QPushButton>

#include "CustomDialog.h" 
#include "TreeWidget.h"

class DocumentClass;
class DocumentClassManager;

//! list document classes
class DocumentClassManagerDialog: public CustomDialog
{

  //! Qt meta object declaration
  Q_OBJECT;
    
  public:
  
  //! constructor
  DocumentClassManagerDialog( QWidget* parent, DocumentClassManager *manager );

  signals:
  
  //! emited when document class are modified
  void updateNeeded( void );
  
  private slots:
  
  //! update button enability
  void _updateButtons( void );
    
  //! edit file associated to class
  void _edit( void );
  
  //! remove a class from the list
  void _remove( void );

  //! load classes from a file
  void _loadFile( void );

  //! save classes to a file
  void _save( void );
  
  //! reload document classes
  void _reload( void );
  
  private:
  
  //! display all classes to listview
  void _loadClasses( void );
  
  //! add document class to listview
  void _addClass( DocumentClass& );
  
  //! number of columns
  enum { n_columns_ = 2 };

  //! column type enumeration
  enum ColumnTypes {
    NAME,
    FILE
  }; 
  
  //! column titles
  static const char* column_titles_[ n_columns_ ];

  //! list item
  class Item: public TreeWidget::Item
  {
    
    public: 
    
    //! constructor
    Item( DocumentClass& document_class ):
      document_class_( &document_class )
    { update(); }
    
    //! update from document class
    void update( void );
    
    //! document class
    DocumentClass& documentClass( void )
    { 
      assert( document_class_, "invalid document class" );
      return *document_class_;
    }
    
    private:
    
    //! document class
    DocumentClass* document_class_;
    
  };
  
  //! document class manager
  DocumentClassManager* document_class_manager_;
  
  //! document classes list
  TreeWidget* list_;
  
  //! edit
  QPushButton* edit_button_;
  
  //! remove
  QPushButton* remove_button_;
  
  //! save
  QPushButton* save_button_;
  
};

#endif
