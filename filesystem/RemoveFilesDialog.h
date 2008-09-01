#ifndef _RemoveFilesDialog_h_
#define _RemoveFilesDialog_h_

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
  \file RemoveFilesDialog.h
  \brief Generic dialog with a FileInfo list
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <assert.h>
#include <QList>
#include <QPushButton>
#include <QCheckBox>

#include "BaseDialog.h"
#include "Counter.h"
#include "FileSystemModel.h"

class TreeView;
class TextEditor;

//! QDialog used to commit selected files
class RemoveFilesDialog: public BaseDialog, public Counter
{
  
  //! Qt meta object
  Q_OBJECT
  
  public:
          
  //! constructor
  RemoveFilesDialog( QWidget*, const FileSystemModel::List& files = FileSystemModel::List() );
    
  // return selected files
  FileSystemModel::List selectedFiles( void ) const;
  
  //! true if directories are to be removed recursively
  bool recursive( void ) const
  { return recursive_checkbox_->isChecked(); }

  protected:
  
  //! list
  TreeView& _list() const
  { 
    assert( list_ );
    return *list_; 
  }
  
  //! model
  const FileSystemModel& _model() const
  { return model_; }
  
  //! model
  FileSystemModel& _model()
  { return model_; }
 
  //! accept button
  QPushButton& _acceptButton( void ) const
  {
    assert( accept_button_ );
    return *accept_button_; 
  }
  
  //! cancel button
  QPushButton& _cancelButton( void ) const
  {
    assert( cancel_button_ );
    return *cancel_button_; 
  }
  
  protected slots:
  
  //! update button states
  virtual void _updateButtons( void );
  
  private:
  
  //! list of files
  TreeView* list_;

  //! model
  FileSystemModel model_;
  
  //! true to remove directories recursively
  QCheckBox* recursive_checkbox_;

  //! accept button
  QPushButton *accept_button_;
  
  //! cancel button
  QPushButton* cancel_button_;

  //! clear selection button
  QPushButton* clear_selection_button_;
  
};

#endif
