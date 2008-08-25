// $Id$
#ifndef RecentFilesFrame_h
#define RecentFilesFrame_h

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
****************************************************************************/

/*!
   \file RecentFilesFrame.h
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <assert.h>
#include <map>
#include <QAction>
#include <QPaintEvent>

#include "Counter.h"
#include "FileRecordModel.h"

class FileList;
class TreeView;

//! editor windows navigator
/*!
  displays an up-to-date list of recent files
  as well as files opened in current session
*/
class RecentFilesFrame: public QWidget, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! creator
  RecentFilesFrame( QWidget* parent, FileList&  );

  //! destructor
  ~RecentFilesFrame( void );

  //! list
  TreeView& list( void ) const
  { 
    assert( list_ );  
    return *list_;
  }

  //! select file in list
  void selectFile( const File& );

  //!@name actions
  //@{
  
  //! visibility
  QAction& updateAction( void ) const
  { return *update_action_; }
  
  //@}
  
  signals:

  //! signal emitted when a file is selected
  void fileSelected( FileRecord );  

  //! signal emited when a file is selected
  void fileActivated( FileRecord );  
      
  protected:
  
  //! enter event
  void enterEvent( QEvent* );
  
  //! recent files
  FileList& _recentFiles( void ) const
  { 
    assert( recent_files_ );
    return *recent_files_;
  }
      
  //! model
  FileRecordModel& _model( void ) 
  { return model_; }
        
  private slots:

  //! update configuration
  void _updateConfiguration( void );
     
  //! update session files
  void _update( void );

  //! update action
  void _updateActions( void );
  
  //! clean
  void _clean( void );
  
  //! open
  void _open( void );
  
  //! sessionFilesItem selected
  void _itemSelected( const QModelIndex& index );

  //! sessionFilesItem selected
  void _itemActivated( const QModelIndex& index );

  //! session files sort order
  void _storeSortMethod( int, Qt::SortOrder );
    
  private:
  
  //! install actions
  void _installActions( void );
   
  //! clean action
  QAction& _cleanAction( void ) const
  { return *clean_action_; }
  
  //! open action
  QAction& _openAction( void ) const
  { return *open_action_; }
  
  //! recent files
  FileList* recent_files_;
  
  //! model
  FileRecordModel model_;
  
  //! list
  TreeView* list_;
  
  //@}
  
  //!@name actions
  //@{
  
  //! update
  QAction* update_action_;
      
  //! clean action
  QAction* clean_action_;
  
  //! open action
  QAction* open_action_;
  
  //@}
  
};

#endif
