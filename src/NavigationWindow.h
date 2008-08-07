// $Id$
#ifndef NavigationWindow_h
#define NavigationWindow_h

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
   \file NavigationWindow.h
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <assert.h>
#include <map>
#include <QAbstractButton>
#include <QAction>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QShowEvent>
#include <QStackedWidget>

#include "Counter.h"
#include "CustomMainWindow.h"
#include "FileRecordModel.h"

class TreeView;

//! editor windows navigator
/*!
  displays an up-to-date list of recent files
  as well as files opened in current session
*/
class NavigationWindow: public CustomMainWindow, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! creator
  NavigationWindow( QWidget* parent = 0 );

  //! destructor
  ~NavigationWindow( void );

  //!@name actions
  //@{
  
  //! visibility
  QAction& visibilityAction( void ) const
  { return *visibility_action_; }
  
  //@}
  
  signals:

  //! signal emited when a file is selected
  void fileSelected( FileRecord );  
  
  public slots:
  
  //! update session files
  void updateSessionFiles( void );

  //! update recent files
  void updateRecentFiles( void );
    
  //! update models
  void updateFiles( void )
  {
    updateSessionFiles();
    updateRecentFiles();
  }
  
  protected:
  
  //! close event
  void closeEvent( QCloseEvent* );
  
  //! show event
  virtual void showEvent( QShowEvent* );
    
  //! stack widget
  QStackedWidget& _stack( void ) const
  { 
    assert( stack_ );
    return *stack_;
  }
  
  //! session file list
  TreeView& _sessionFilesList( void ) const
  { 
    assert( session_files_list_ );  
    return *session_files_list_;
  }
  
  //! recent file list
  TreeView& _recentFilesList( void ) const
  {
    assert( recent_files_list_ );  
    return *recent_files_list_;
  }
  
  //! session files model
  FileRecordModel& _sessionFilesModel( void ) 
  { return session_files_model_; }
  
  //! recent files model
  FileRecordModel& _recentFilesModel( void ) 
  { return recent_files_model_; }
  
  private slots:
     
  //! update configuration
  void _updateConfiguration( void );
  
  //! update configuration
  void _saveConfiguration( void );
  
  //! sessionFilesItem selected
  void _sessionFilesItemSelected( const QModelIndex& index )
  { _itemSelected( _sessionFilesModel(), index ); }
  
  //! sessionFilesItem selected
  void _recentFilesItemSelected( const QModelIndex& index )
  { _itemSelected( _recentFilesModel(), index ); }

  //!@name selections
  //@{

  void _storeSessionFilesSelection( void )
  { _storeSelection( _sessionFilesList(), _sessionFilesModel() ); }
  
  void _restoreSessionFilesSelection( void )
  { _restoreSelection( _sessionFilesList(), _sessionFilesModel() ); }

  void _storeRecentFilesSelection( void )
  { _storeSelection( _recentFilesList(), _recentFilesModel() ); }
  
  void _restoreRecentFilesSelection( void )
  { _restoreSelection( _recentFilesList(), _recentFilesModel() ); }

  //@}
  
  //! display item page
  virtual void _display( QAbstractButton* );

  private:
  
  //! install actions
  void _installActions( void );
  
  //! item selected
  void _itemSelected( const FileRecordModel&, const QModelIndex& );
  
  //! store selection
  void _storeSelection( TreeView&, FileRecordModel& );
  
  //! store selection
  void _restoreSelection( TreeView&, FileRecordModel& );

  //! stack widget
  QStackedWidget* stack_;
  
  //! map widget to action
  typedef std::map<QAbstractButton*, QWidget* > ButtonMap;
  
  //! map widget to action in the toolbar
  ButtonMap buttons_;

  //! session files model
  FileRecordModel session_files_model_;
  
  //! session files list
  TreeView* session_files_list_;
  
  //! recent files model
  FileRecordModel recent_files_model_;

  //! recent files list
  TreeView* recent_files_list_;
      
  //!@name actions
  //@{
  
  //! visibility
  QAction* visibility_action_;
  
  //@}
  
};

#endif
