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

#include <QHeaderView>
#include <QLayout>
#include <QDir>

#include "FileSystemIcons.h"
#include "ColumnSortingMenu.h"
#include "CustomComboBox.h"
#include "CustomToolBar.h"
#include "FileInformationDialog.h"
#include "FileRecordProperties.h"
#include "FileSystemFrame.h"
#include "IconEngine.h"
#include "QtUtil.h"
#include "RemoveFilesDialog.h"
#include "TextEditor.h"
#include "TreeView.h"
#include "Util.h"
#include "XmlOptions.h"

using namespace std;

//_____________________________________________
FileSystemFrame::FileSystemFrame( QWidget *parent ):
  QWidget( parent ),
  Counter( "FileSystemFrame" ),
  home_path_( Util::home() ),
  file_system_watcher_( this ),
  thread_( this )
{

  Debug::Throw( "FileSystemFrame::FileSystemFrame.\n" );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setSpacing(2);
  layout->setMargin(0);
  setLayout( layout );

  // install actions
  _installActions();
  
  // toolbar
  CustomToolBar* toolbar = new CustomToolBar( "navigation toolbar", this, "NAVIGATION_TOOLBAR" );
  toolbar->addAction( &_parentDirectoryAction() );
  toolbar->addAction( &_previousDirectoryAction() );
  toolbar->addAction( &_nextDirectoryAction() );
  toolbar->addAction( &_homeDirectoryAction() );
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
  _list().header()->hide();
  
  // list menu  
  _list().menu().addMenu( new ColumnSortingMenu( &_list().menu(), &_list() ) );
  _list().menu().addSeparator();
  _list().menu().addAction( &_previousDirectoryAction() );
  _list().menu().addAction( &_nextDirectoryAction() );
  _list().menu().addAction( &_parentDirectoryAction() );
  _list().menu().addAction( &_homeDirectoryAction() );
  
  _list().menu().addSeparator();
  _list().menu().addAction( &_hiddenFilesAction() );
  _list().menu().addAction( &_openAction() );
  _list().menu().addAction( &_removeAction() );
  
  _list().menu().addSeparator();
  _list().menu().addAction( &_filePropertiesAction() );

  connect( _list().selectionModel(), SIGNAL( currentRowChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _updateActions() ) );
  connect( _list().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateActions() ) );
  connect( &_list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemActivated( const QModelIndex& ) ) );
  
  _updateNavigationActions();

  connect( &_model(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), SLOT( _restoreSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), &_list(), SLOT( updateMask() ) );

  connect( &file_system_watcher_, SIGNAL( directoryChanged( const QString& ) ), SLOT( _update( const QString& ) ) );
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();
  _updateActions();

}

//_________________________________________________________
void FileSystemFrame::setPath( File path )
{

  Debug::Throw() << "FileSystemFrame::setPath - path: " << path << endl;

  assert( path.isDirectory() );
  path_ = path;
  history_.add( path );
    
  _update();
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
void FileSystemFrame::setHome( const File& path )
{ home_path_ = path; }

//_________________________________________________________
void FileSystemFrame::clear()
{

  Debug::Throw( "FileSystemFrame::Clear.\n" );
  _model().clear();

}

//____________________________________________
void FileSystemFrame::showEvent( QShowEvent* )
{
  Debug::Throw( "FileSystemFrame::showEvent.\n" );
  if( path().empty() ) setPath( Util::workingDirectory() );  
  else _update();
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
    _update();
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
  if( record.hasFlag( FileSystemModel::FOLDER ) )
  {

    // make full name
    File path( record.file() );
    path = path.addPath( FileSystemFrame::path() );
    setPath( path );

  } else if( record.hasFlag( FileSystemModel::NAVIGATOR ) ) { 
    
    _parentDirectoryAction().trigger();
    
  } else {
    
    emit fileActivated( record.setFile( record.file().addPath( path() ) ) );
  
  }
  
}

//______________________________________________________
void FileSystemFrame::_updateConfiguration( void )
{
  Debug::Throw( "FileSystemFrame::_updateConfiguration.\n" );
  _hiddenFilesAction().setChecked( XmlOptions::get().get<bool>( "SHOW_HIDDEN_FILES" ) );
}

//______________________________________________________
void FileSystemFrame::_saveConfiguration( void )
{ Debug::Throw( "FileSystemFrame::_saveConfiguration.\n" ); }
    
//______________________________________________________
void FileSystemFrame::_updateNavigationActions( void )
{
  Debug::Throw( "FileSystemFrame::_updateNavigationActions.\n" );
  _previousDirectoryAction().setEnabled( history_.previousAvailable() );
  _nextDirectoryAction().setEnabled( history_.nextAvailable() );
  _parentDirectoryAction().setEnabled( !QDir( path().c_str() ).isRoot() );
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
    _homeDirectoryAction().trigger();
    return;
  }
  
  // check if path exists and is a directory
  File path( qPrintable( value ) );
  if( !( path.exists() && path.isDirectory() ) ) { setPath( path_ ); }
  else setPath( path );
  
}
  
//______________________________________________________
void FileSystemFrame::_update( const QString& value )
{
  if( !isVisible() ) return;
  if( value != path().c_str() ) return;
  _update();
  
}
  
//______________________________________________________
void FileSystemFrame::_update( void )
{
  Debug::Throw( "FileSystemFrame::_update.\n" );

  if( path().empty() || !( path().exists() && path().isDirectory() ) ) return;
  if( thread_.isRunning() ) return;
  thread_.setPath( path(), _hiddenFilesAction().isChecked() );
  thread_.start();
  
  setCursor( Qt::WaitCursor ); 

}

//______________________________________________________________________
void FileSystemFrame::_updateActions( void )
{ 
  Debug::Throw( "FileSystemFrame:_updateActions.\n" );
  FileSystemModel::List selection( model_.get( _list().selectionModel()->selectedRows() ) );
  
  bool has_editable_selection( false );
  bool has_removable_selection( false );
  for( FileSystemModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
    if( !iter->hasFlag( FileSystemModel::NAVIGATOR ) ) has_removable_selection = true;
    if( iter->hasFlag( FileSystemModel::DOCUMENT ) )
    { 
      has_editable_selection = true; 
      break;
    }
  }

  _openAction().setEnabled( has_editable_selection );
  _removeAction().setEnabled( has_removable_selection );
  
  QModelIndex index( _list().selectionModel()->currentIndex() );
  _filePropertiesAction().setEnabled( index.isValid() && !_model().get( index ).hasFlag( FileSystemModel::NAVIGATOR ) );
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
  setPath( home() );
}

//______________________________________________________________________
void FileSystemFrame::_open( void )
{ 
  
  Debug::Throw( "FileSystemFrame:_open.\n" ); 
  FileSystemModel::List selection( model_.get( _list().selectionModel()->selectedRows() ) );
  FileSystemModel::List valid_selection;
  for( FileSystemModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { 
    if( iter->hasFlag( FileSystemModel::DOCUMENT ) )
    { valid_selection.push_back( *iter ); }
  }
  
  // one should check the number of files to be edited
  for( FileSystemModel::List::iterator iter = valid_selection.begin(); iter != valid_selection.end(); iter++ )
  { emit fileActivated( iter->setFile( iter->file().addPath( path() ) ) ); }
  
}

//______________________________________________________________________
void FileSystemFrame::_remove( void )
{ 
  
  Debug::Throw( "FileSystemFrame::_remove.\n" );

  // get selection
  FileSystemModel::List selection( model_.get( _list().selectionModel()->selectedRows() ) );
  FileSystemModel::List valid_selection;
  for( FileSystemModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { 
    if( !iter->hasFlag( FileSystemModel::NAVIGATOR ) )
    { valid_selection.push_back( *iter ); }
  }
  
  if( valid_selection.empty() ) return;
  
  RemoveFilesDialog dialog( this, valid_selection );
  if( !dialog.exec() ) return;
  
  valid_selection = dialog.selectedFiles();
  
  // loop over selected files and remove
    // retrieve selected items and remove corresponding files
  for( FileSystemModel::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { 
    File file( iter->file().addPath( path() ) );
    if( !file.exists() ) continue;
    if( file.isDirectory() && !dialog.recursive() ) continue;
    file.removeRecursive();
  }

}

//________________________________________
void FileSystemFrame::_fileProperties( void )
{
  
  Debug::Throw( "FileSystemFrame::_fileProperties.\n" );
  QModelIndex index( _list().selectionModel()->currentIndex() );
  if( !index.isValid() ) return;
  
  FileRecord record( _model().get( index ) );
  if( record.hasFlag( FileSystemModel::NAVIGATOR ) ) return;
  
  FileInformationDialog( this, record ).centerOnWidget( window() ).exec();
  
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
  _hiddenFilesAction().setCheckable( true );
  connect( &_hiddenFilesAction(), SIGNAL( toggled( bool ) ), SLOT( _update() ) );
  connect( &_hiddenFilesAction(), SIGNAL( toggled( bool ) ), SLOT( _toggleShowHiddenFiles( bool ) ) );
    
  // previous directory
  addAction( previous_directory_action_ = new QAction( IconEngine::get( ICONS::PREVIOUS_DIRECTORY ), "&Previous", this ) );
  connect( &_previousDirectoryAction(), SIGNAL( triggered() ), SLOT( _previousDirectory() ) );
  _previousDirectoryAction().setToolTip( "Change path to previous directory (from history)" );
  
  // next directory (from history)
  addAction( next_directory_action_ = new QAction( IconEngine::get( ICONS::NEXT_DIRECTORY ), "&Next", this ) );
  connect( &_nextDirectoryAction(), SIGNAL( triggered() ), SLOT( _nextDirectory() ) );
  _nextDirectoryAction().setToolTip( "Change path to next directory (from history)" );
  
  // parent directory in tree
  addAction( parent_directory_action_ = new QAction( IconEngine::get( ICONS::PARENT_DIRECTORY ), "&Parent directory", this ) );
  connect( &_parentDirectoryAction(), SIGNAL( triggered() ), SLOT( _parentDirectory() ) );
  _parentDirectoryAction().setToolTip( "Change path to parent directory" );
  
  // home directory
  addAction( home_directory_action_ = new QAction( IconEngine::get( ICONS::HOME_DIRECTORY ), "&Home", this ) );
  connect( &_homeDirectoryAction(), SIGNAL( triggered() ), SLOT( _homeDirectory() ) );
  _homeDirectoryAction().setToolTip( "Change path to current file working directory" );
  
  // open
  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open selected files", this ) );
  connect( &_openAction(), SIGNAL( triggered() ), SLOT( _open() ) );
  _openAction().setToolTip( "Edit selected files" );

  // remove
  addAction( remove_action_ = new QAction( IconEngine::get( ICONS::DELETE ), "Remo&ve", this ) );
  connect( remove_action_, SIGNAL( triggered() ), SLOT( _remove() ) );
  remove_action_->setShortcut( Qt::Key_Delete );
  remove_action_->setToolTip( "Remove selected files locally" );

  // file properties
  addAction( file_properties_action_ = new QAction( IconEngine::get( ICONS::INFO ), "&File properties", this ) );
  file_properties_action_->setShortcut( Qt::ALT + Qt::Key_Return );
  file_properties_action_->setToolTip( "Display current file properties" );
  connect( file_properties_action_, SIGNAL( triggered() ), SLOT( _fileProperties() ) );

}
