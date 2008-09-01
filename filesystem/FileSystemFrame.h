#ifndef FileSystemFrame_h
#define FileSystemFrame_h

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
   \file FileSystemFrame.h
   \brief customized ListView for file/directory navigation
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <map>
#include <QIcon>
#include <QTimer>
#include <QWidget>
#include <QFileSystemWatcher>

#include "File.h"
#include "FileSystemModel.h"
#include "FileSystemHistory.h"
#include "FileSystemThread.h"

class CustomComboBox;
class TreeView;

//! customized ListView for file/directory navigation
class FileSystemFrame: public QWidget, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:
    
  //! constructor
  FileSystemFrame( QWidget* parent );
    
  //! destructor
  ~FileSystemFrame( void )
  { Debug::Throw( "FileSystemFrame::~FileSystemFrame.\n" ); }

  //! base directory
  const File& path( void ) const
  { return path_; }
  
  //! home path
  const File& home( void ) const
  { return home_path_; }
  
  //! home path
  void setHome( const File& path );
  
  //! clear list, add navigator button
  void clear();
      
  signals:
    
  //! file selected
  void fileActivated( FileRecord );
   
  public slots:
  
  //! base directory
  void setPath( File );
   
  protected:
   
  //! custom event, used to retrieve file validity check event
  void customEvent( QEvent* );
 
  //! show event
  virtual void showEvent( QShowEvent* );

  private slots:
  
  //! item activated
  void _itemActivated( const QModelIndex& );
  
  //! show hidden files
  void _toggleShowHiddenFiles( bool state );
  
  //! update configuration
  void _updateConfiguration( void );
  
  //! save configuration
  void _saveConfiguration( void );
     
  //! update navigation actions based on current location and history
  void _updateNavigationActions( void );

  //! update path
  void _updatePath( void );
  
  //! update path
  void _updatePath( const QString& );
  
  //! update directory
  void _update( const QString& );
   
  //! update directory
  void _update( void );
 
  //! update actions
  void _updateActions( void );
  
  //! previous directory
  void _previousDirectory( void );
  
  //! next directory
  void _nextDirectory( void );
  
  //! parent directory
  void _parentDirectory( void );
  
  //! home directory
  void _homeDirectory( void );
   
  //! open
  void _open( void );
  
  //! store selected jobs in model
  void _storeSelection( void );

  //! restore selected jobs from model
  void _restoreSelection( void );
  
  private:
  
  //! path comboBox
  CustomComboBox& _comboBox( void ) const 
  { 
    assert( path_combobox_ );
    return *path_combobox_;
  }
  
  //! model
  FileSystemModel& _model( void )
  { return model_; }
  
  //! list
  TreeView& _list( void ) const
  {
    assert( list_ );
    return *list_; 
  }
  
  //! install actions
  void _installActions( void );

  //!@name actions
  //@{
  
  //! hidden files
  QAction& _hiddenFilesAction( void ) const
  { return *hidden_files_action_; }
    
  //! previous directory
  QAction& _previousDirectoryAction( void ) const
  { return *previous_directory_action_; }
  
  //! next directory
  QAction& _nextDirectoryAction( void ) const
  { return *next_directory_action_; }
  
  // directory above 
  QAction& _parentDirectoryAction( void ) const
  { return *parent_directory_action_; }
 
  // home directory 
  QAction& _homeDirectoryAction( void ) const
  { return *home_directory_action_; } 
  
  //! open action
  QAction& _openAction( void ) const
  { return *open_action_; }

  //@}

  //! path comboBox
  CustomComboBox* path_combobox_;
  
  //! model
  FileSystemModel model_;
  
  //! list of files
  TreeView* list_;

  //! current path
  File path_;
        
  //!@name actions
  //@{
  
  //! show/hide hidden files
  QAction* hidden_files_action_;
  
  //! previous directory action
  QAction* previous_directory_action_;
  
  //! next directory action
  QAction* next_directory_action_;
  
  //! parent directory action
  QAction* parent_directory_action_;
  
  //! home directory action
  QAction* home_directory_action_;
    
  //! open
  QAction* open_action_;
  
  //@}

  //! home directory
  File home_path_;
  
  //! path naviagtion history
  FileSystemHistory history_;
  
  //! file system watcher
  QFileSystemWatcher file_system_watcher_;
  
  //! thread to list files
  FileSystemThread thread_;

};

#endif
