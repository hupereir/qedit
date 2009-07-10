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
#include <QAction>
#include <QPushButton>

#include "BaseMainWindow.h" 
#include "DocumentClassModel.h"
#include "DocumentClassManager.h"

class TreeView;

//! list document classes
class DocumentClassManagerDialog: public BaseMainWindow, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT;
    
  public:
  
  //! constructor
  DocumentClassManagerDialog( QWidget*, const DocumentClassManager& );
  
  protected:
  
  //! close event
  virtual void closeEvent( QCloseEvent* );
  
  //!@name actions
  //@{
  QAction& _newAction( void ) const
  { return *new_action_; }
  
  QAction& _openAction( void ) const
  { return *open_action_; }
  
  QAction& _saveAction( void ) const
  { return *save_action_; }
    
  QAction& _editAction( void ) const
  { return *edit_action_; }
  
  QAction& _reloadAction( void ) const
  { return *reload_action_; }
  
  QAction& _removeAction( void ) const
  { return *remove_action_; }

  //@}
  
  private slots:
    
  //! update button enability
  void _updateActions( void );
    
  //! add new document class
  void _add( void );
  
  //! edit file associated to class
  void _edit( void );
  
  //! remove a class from the list
  void _remove( void );

  //! load classes from a file
  void _loadFile( void );

  //! save all classes to given path
  void _save( void );
  
  //! reload document classes
  void _reload( void );
  
  private:
  
  //! actions
  void _installActions( void );

  //! display all classes to listview
  void _loadClasses( const DocumentClassManager& );
  
  //! modification state
  bool _modified( void ) const
  { return modified_; }
  
  //! check modification state
  void _checkModified( void );
  
  //! set modification state
  void _setModified( bool value );
  
  //! window title
  void _updateWindowTitle( void );
  
  //! save document classes, return possible warnings
  QStringList _saveDocumentClasses( void );
  
  //! model
  const DocumentClassModel& _model( void ) const
  { return model_; }
  
  //! model
  DocumentClassModel& _model( void )
  { return model_; }

  //! list
  TreeView& _list( void ) const
  { return *list_; }
  
  //! document class manager backup
  DocumentClassManager backup_;
  
  //! model
  DocumentClassModel model_;
  
  //! document classes list
  TreeView* list_;
 
  //! check if modified
  bool modified_;
  
  //!@name actions
  //#{
  
  QAction *new_action_;
  
  QAction *open_action_;
  
  QAction *save_action_;
  
  QAction *edit_action_;
  
  QAction *reload_action_;
  
  QAction *remove_action_;
  
  //@}
  
};

#endif
