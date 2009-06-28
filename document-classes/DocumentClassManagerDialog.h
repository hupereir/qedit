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

#include <cassert>
#include <QPushButton>

#include "CustomDialog.h" 
#include "DocumentClassModel.h"

class DocumentClassManager;
class TreeView;

//! list document classes
class DocumentClassManagerDialog: public CustomDialog
{

  //! Qt meta object declaration
  Q_OBJECT;
    
  public:
  
  //! constructor
  DocumentClassManagerDialog( QWidget*, DocumentClassManager* );

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

  //! save selected classes to a file
  void _save( void );
  
  //! save all classes to given path
  void _saveAll( void );
  
  //! reload document classes
  void _reload( void );
  
  private:
  
  //! display all classes to listview
  void _loadClasses( void );
  
  //! list
  TreeView& _list( void ) const
  { return *list_; }
  
  //! document class manager
  DocumentClassManager* document_class_manager_;
  
  //! model
  DocumentClassModel model_;
  
  //! document classes list
  TreeView* list_;
  
  //! edit
  QPushButton* edit_button_;
  
  //! remove
  QPushButton* remove_button_;
  
  //! save
  QPushButton* save_button_;
  
};

#endif
