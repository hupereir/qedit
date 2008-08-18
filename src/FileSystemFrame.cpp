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
   \file FileSystemFrame.cpp
   \brief customized ListView for file/directory navigation
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLayout>
#include <QDir>

#include "CustomComboBox.h"
#include "CustomToolBar.h"
#include "FileRecordProperties.h"
#include "FileSystemFrame.h"
#include "Icons.h"
#include "IconEngine.h"
#include "QtUtil.h"
#include "TextEditor.h"
#include "TreeView.h"
#include "Util.h"
#include "XmlOptions.h"

using namespace std;

//_____________________________________________
FileSystemFrame::FileSystemFrame( QWidget *parent ):
  QWidget( parent ),
  Counter( "FileSystemFrame" ),
  file_system_watcher_( this ),
  thread_( this )
{

  Debug::Throw( "FileSystemFrame::FileSystemFrame.\n" );
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(0);
  setLayout( layout );

  // install actions
  _installActions();

  // toolbar
  CustomToolBar* toolbar = new CustomToolBar( "navigation toolbar", this, "NAVIGATION_TOOLBAR" );
  toolbar->addAction( &parentDirectoryAction() );
  toolbar->addAction( &previousDirectoryAction() );
  toolbar->addAction( &nextDirectoryAction() );
  toolbar->addAction( &homeDirectoryAction() );
  layout->addWidget( toolbar );
  
  // combo box
  layout->addWidget( path_combobox_ = new CustomComboBox( this ) );
  _comboBox().setEditable( true );
  
  connect( &_comboBox(), SIGNAL( currentIndexChanged( const QString& ) ), SLOT( _updatePath( const QString& ) ) );
  connect( &_comboBox(), SIGNAL( currentIndexChanged( const QString& ) ), SLOT( _updatePath( const QString& ) ) );
  connect( _comboBox().lineEdit(), SIGNAL(returnPressed()), SLOT( _updatePath( void ) ) );
  
  // file list
  layout->addWidget( list_ = new TreeView( this ), 1);
  _list().setModel( &_model() );
  _list().setSelectionMode( QAbstractItemView::ContiguousSelection ); 
  _list().setMaskOptionName( "FILE_SYSTEM_LIST_MASK" );
    
  // list menu  
  _list().menu().addAction( &previousDirectoryAction() );
  _list().menu().addAction( &nextDirectoryAction() );
  _list().menu().addAction( &parentDirectoryAction() );
  _list().menu().addAction( &homeDirectoryAction() );

  _list().menu().addSeparator();
  _list().menu().addAction( &hiddenFilesAction() );

  connect( &_list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemActivated( const QModelIndex& ) ) );
  
  _updateNavigationActions();

  connect( &_model(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), SLOT( _restoreSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), &_list(), SLOT( updateMask() ) );

  connect( &file_system_watcher_, SIGNAL( directoryChanged( const QString& ) ), SLOT( _reload( const QString& ) ) );
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();

}

//_________________________________________________________
void FileSystemFrame::setPath( File path )
{

  Debug::Throw() << "FileSystemFrame::setPath - path: " << path << endl;

  assert( path.isDirectory() );
  path_ = path;
  history_.add( path );
    
  _reload();
  _updateNavigationActions();
  
  // update combobox
  if( _comboBox().findText( path.c_str() ) < 0 )
  { _comboBox().addItem( path.c_str() ); }
  
  _comboBox().setEditText( path.c_str() );

  // reset file system watcher
  QStringList directories( file_system_watcher_.directories() );
  if( !directories.isEmpty() ) file_system_watcher_.removePaths( directories );
  file_system_watcher_.addPath( path.c_str() );
  
}
  
//_________________________________________________________
void FileSystemFrame::clear()
{

  Debug::Throw( "FileSystemFrame::Clear.\n" );
  _model().clear();

}

//______________________________________________________
void FileSystemFrame::customEvent( QEvent* event )
{
      
  if( event->type() != QEvent::User ) return;
  
  FileSystemEvent* file_system_event( dynamic_cast<FileSystemEvent*>(event) );
  if( !file_system_event ) return;
  
  // check path
  if( file_system_event->path() != path() ) 
  {
    reloadAction().trigger();
    return;
  }
  
  // update model and list
  _model().update( file_system_event->files() );
  _list().resizeColumnToContents( FileSystemModel::FILE );

  unsetCursor();
  
}

//______________________________________________________
void FileSystemFrame::_itemActivated( const QModelIndex& index )
{
  Debug::Throw( "FileSystemFrame::_itemActivated.\n" );

  if( !index.isValid() ) return;

  // retrieve file
  FileRecord record( model_.get( index ) );
  unsigned int type( record.property<unsigned int>( FileRecordProperties::TYPE ) );
  if( type & FileSystemModel::FOLDER )
  {

    // make full name
    File path( record.file() );
    path = path.addPath( FileSystemFrame::path() );
    setPath( path );

  } else if( type & FileSystemModel::NAVIGATOR ) { 
    
    parentDirectoryAction().trigger();
    
  } else {
    
    record.setFile( record.file().addPath( path() ) );
    Debug::Throw() << "FileSystemFrame::_itemActivated - file: " << record.file() << endl;
    emit fileSelected( record );
  
  }
  
}

//______________________________________________________
void FileSystemFrame::_updateConfiguration( void )
{
  Debug::Throw( "FileSystemFrame::_updateConfiguration.\n" );
  hiddenFilesAction().setChecked( XmlOptions::get().get<bool>( "SHOW_HIDDEN_FILES" ) );
}

//______________________________________________________
void FileSystemFrame::_saveConfiguration( void )
{ Debug::Throw( "FileSystemFrame::_saveConfiguration.\n" ); }
    
//______________________________________________________
void FileSystemFrame::_updateNavigationActions( void )
{
  Debug::Throw( "FileSystemFrame::_updateNavigationActions.\n" );
  previousDirectoryAction().setEnabled( history_.previousAvailable() );
  nextDirectoryAction().setEnabled( history_.nextAvailable() );
  parentDirectoryAction().setEnabled( !QDir( path().c_str() ).isRoot() );
  return;
}
     
//______________________________________________________
void FileSystemFrame::_updatePath( void )
{
  Debug::Throw( "FileSystemFrame::_updatePath.\n" );
  _updatePath( _comboBox().lineEdit()->text() );
}

//______________________________________________________
void FileSystemFrame::_updatePath( const QString& value )
{
  Debug::Throw( "FileSystemFrame::_updatePath.\n" );
  
  // check if path has changed
  if( value == path().c_str() ) return;
  
  // if path is empty set path to home directory
  if( value.isEmpty() )
  {
    homeDirectoryAction().trigger();
    return;
  }
  
  // check if path exists and is a directory
  File path( qPrintable( value ) );
  if( !( path.exists() && path.isDirectory() ) ) { setPath( path_ ); }
  else setPath( path );
  
}
  
//______________________________________________________
void FileSystemFrame::_reload( const QString& value )
{
  Debug::Throw( "FileSystemFrame::_reload.\n" );
  if( !isVisible() ) return;
  if( value != path().c_str() ) return;
  _reload();
  
}
  
//______________________________________________________
void FileSystemFrame::_reload( void )
{
  Debug::Throw( "FileSystemFrame::_reload.\n" );

  if( path().empty() || !( path().exists() && path().isDirectory() ) ) return;
  
  if( thread_.isRunning() ) return;
  thread_.setPath( path(), hiddenFilesAction().isChecked() );
  thread_.start();
  
  setCursor( Qt::WaitCursor ); 

}

//______________________________________________________
void FileSystemFrame::_toggleShowHiddenFiles( bool state )
{ 
  Debug::Throw( "FileSystemFrame::_toggleShowHiddenFiles.\n" );
  XmlOptions::get().set( "SHOW_HIDDEN_FILES", state );
}

//______________________________________________________
void FileSystemFrame::_previousDirectory( void )
{ 
  Debug::Throw( "FileSystemFrame::_previousDirectory.\n" );
  if( !history_.previousAvailable() ) return;
  setPath( history_.previous() );
}

//______________________________________________________
void FileSystemFrame::_nextDirectory( void )
{ 
  Debug::Throw( "FileSystemFrame::_nextDirectory.\n" );
  if( !history_.nextAvailable() ) return;
  setPath( history_.next() );
}

//______________________________________________________
void FileSystemFrame::_parentDirectory( void )
{

  Debug::Throw( "FileSystemFrame::_parentDirectory.\n" );
  QDir dir( path().c_str() );
  dir.cdUp();
  setPath( File( qPrintable( dir.absolutePath() ) ) );

}

//______________________________________________________
void FileSystemFrame::_homeDirectory( void )
{
  Debug::Throw( "FileSystemFrame::_homeDirectory.\n" );
  setPath( Util::home() );
}

//________________________________________
void FileSystemFrame::_storeSelection( void )
{
    
  // clear
  _model().clearSelectedIndexes();
  
  // retrieve selected indexes in list
  QModelIndexList selected_indexes( _list().selectionModel()->selectedRows() );
  for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
  { 
    // check column
    if( !iter->column() == 0 ) continue;
    _model().setIndexSelected( *iter, true ); 
  }
  
  return;
    
}

//________________________________________
void FileSystemFrame::_restoreSelection( void )
{

  // retrieve indexes
  QModelIndexList selected_indexes( _model().selectedIndexes() );
  if( selected_indexes.empty() ) _list().selectionModel()->clear();
  else {
    
    _list().selectionModel()->select( selected_indexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
    { _list().selectionModel()->select( *iter, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }
  
  }
  
  return;
}

//_____________________________________________
void FileSystemFrame::_installActions( void )
{

  Debug::Throw( "FileSystemFrame::_installActions.\n" );

  // hidden files
  addAction( hidden_files_action_ = new QAction( "&Show hidden files", this ) );
  hidden_files_action_->setCheckable( true );
  connect( hidden_files_action_, SIGNAL( toggled( bool ) ), SLOT( _reload() ) );
  connect( hidden_files_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleShowHiddenFiles( bool ) ) );
    
  // reload
  addAction( reload_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "&Reload", this ) );
  connect( hidden_files_action_, SIGNAL( triggered() ), SLOT( _reload() ) );
  
  // previous directory
  addAction( previous_directory_action_ = new QAction( IconEngine::get( ICONS::PREVIOUS_DIRECTORY ), "&Previous", this ) );
  connect( previous_directory_action_, SIGNAL( triggered() ), SLOT( _previousDirectory() ) );
  previous_directory_action_->setToolTip( "Change path to previous directory (from history)" );
  
  // next directory (from history)
  addAction( next_directory_action_ = new QAction( IconEngine::get( ICONS::NEXT_DIRECTORY ), "&Next", this ) );
  connect( next_directory_action_, SIGNAL( triggered() ), SLOT( _nextDirectory() ) );
  next_directory_action_->setToolTip( "Change path to next directory (from history)" );
  
  // parent directory in tree
  addAction( parent_directory_action_ = new QAction( IconEngine::get( ICONS::PARENT_DIRECTORY ), "&Parent directory", this ) );
  connect( parent_directory_action_, SIGNAL( triggered() ), SLOT( _parentDirectory() ) );
  parent_directory_action_->setToolTip( "Change path to parent directory" );
  
  // home directory
  addAction( home_directory_action_ = new QAction( IconEngine::get( ICONS::HOME_DIRECTORY ), "&Home", this ) );
  connect( home_directory_action_, SIGNAL( triggered() ), SLOT( _homeDirectory() ) );
  home_directory_action_->setToolTip( "Change path to user home" );

}
