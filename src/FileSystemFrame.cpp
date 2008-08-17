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
#include <QDateTime>
#include <QDir>

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
  Counter( "FileSystemFrame" )
{

  Debug::Throw( "FileSystemFrame::FileSystemFrame.\n" );
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(0);
  setLayout( layout );

  // install actions
  _installActions();

  // file list
  layout->addWidget( list_ = new TreeView( this ), 1);
  list().setModel( &model_ );
  list().setSelectionMode( QAbstractItemView::ContiguousSelection ); 
  list().setMaskOptionName( "FILE_SYSTEM_LIST_MASK" );
    
  // list menu  
  list().menu().addAction( &previousDirectoryAction() );
  list().menu().addAction( &nextDirectoryAction() );
  list().menu().addAction( &parentDirectoryAction() );
  list().menu().addAction( &homeDirectoryAction() );

  list().menu().addSeparator();
  list().menu().addAction( &hiddenFilesAction() );
  
  _updateNavigationActions();

  connect( &model_, SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelection() ) );
  connect( &model_, SIGNAL( layoutChanged() ), SLOT( _restoreSelection() ) );
  connect( &model_, SIGNAL( layoutChanged() ), &list(), SLOT( updateMask() ) );

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
  _reload();
  
}
  
//_________________________________________________________
void FileSystemFrame::clear()
{

  Debug::Throw( "FileSystemFrame::Clear.\n" );
  model_.clear();

}

//______________________________________________________
void FileSystemFrame::enterEvent( QEvent* event )
{
  
  Debug::Throw( "FileSystemFrame::enterEvent.\n" );
  reload();
  QWidget::enterEvent( event );
  return;
  
}

//______________________________________________________
void FileSystemFrame::_updateConfiguration( void )
{
  Debug::Throw( "FileSystemFrame::_updateConfiguration.\n" );
  hiddenFilesAction().setChecked( XmlOptions::get().get<bool>( "SHOW_HIDDEN_FILES" ) );
}

//______________________________________________________
void FileSystemFrame::_saveConfiguration( void )
{
  Debug::Throw( "FileSystemFrame::_saveConfiguration.\n" );
  XmlOptions::get().set<bool>( "SHOW_HIDDEN_FILES", hiddenFilesAction().isChecked() );
}
    
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
void FileSystemFrame::_reload( void )
{

  Debug::Throw( "FileSystemFrame::_reload.\n" );
  if( path().empty() ) 
  {
    clear();
    return;
  }

  // loop over directory contents
  QDir dir( path().c_str() );
  QDir::Filters filter = QDir::AllEntries | QDir::NoDotAndDotDot;
  if( hiddenFilesAction().isChecked() ) filter |= QDir::Hidden;
  dir.setFilter( filter );
  QFileInfoList entries( dir.entryInfoList() );
  FileSystemModel::List new_files;
  
  // add navigator
  FileRecord record( File("..") );
  record.addProperty( FileRecordProperties::TYPE, Str().assign<unsigned int>( FileSystemModel::NAVIGATOR ) );
  new_files.push_back( record );
  
  // loop over entries and add
  for( QFileInfoList::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    
    if( iter->fileName() == ".." || iter->fileName() == "." ) continue;

    // create file record
    FileRecord record( File( qPrintable( iter->fileName() ) ), TimeStamp( iter->lastModified().toTime_t() ) );
    
    // assign size
    record.addProperty( FileRecordProperties::SIZE, Str().assign<long int>( iter->size() ) );
    
    // assign type
    unsigned int type = 0;
    if( iter->isDir() ) type |= FileSystemModel::FOLDER;
    else type |= FileSystemModel::DOCUMENT;
    
    if( iter->isSymLink() ) type |= FileSystemModel::LINK;
    
    record.addProperty( FileRecordProperties::TYPE, Str().assign<unsigned int>( type ) );
    
    // add to model
    new_files.push_back( record );
    
  }

  // update model and list
  model_.update( new_files );
  list().resizeColumnToContents( FileSystemModel::FILE );
  
}

//______________________________________________________
void FileSystemFrame::_previousDirectory( void )
{ 
  Debug::Throw( "FileSystemFrame::_previousDirectory.\n" );
  if( !history_.previousAvailable() ) return;
  setPath( history_.previous() );
  emit fileSelected( path() );
}

//______________________________________________________
void FileSystemFrame::_nextDirectory( void )
{ 
  Debug::Throw( "FileSystemFrame::_nextDirectory.\n" );
  if( !history_.nextAvailable() ) return;
  setPath( history_.next() );
  emit fileSelected( path() );
}

//______________________________________________________
void FileSystemFrame::_parentDirectory( void )
{

  Debug::Throw( "FileSystemFrame::_parentDirectory.\n" );
  QDir dir( path().c_str() );
  dir.cdUp();
  setPath( File( qPrintable( dir.absolutePath() ) ) );
  emit fileSelected( path() );

}

//______________________________________________________
void FileSystemFrame::_homeDirectory( void )
{
  Debug::Throw( "FileSystemFrame::_homeDirectory.\n" );
  setPath( Util::home() );
  emit fileSelected( path() );
}

//________________________________________
void FileSystemFrame::_storeSelection( void )
{
    
  // clear
  model_.clearSelectedIndexes();
  
  // retrieve selected indexes in list
  QModelIndexList selected_indexes( list().selectionModel()->selectedRows() );
  for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
  { 
    // check column
    if( !iter->column() == 0 ) continue;
    model_.setIndexSelected( *iter, true ); 
  }
  
  return;
    
}

//________________________________________
void FileSystemFrame::_restoreSelection( void )
{

  // retrieve indexes
  QModelIndexList selected_indexes( model_.selectedIndexes() );
  if( selected_indexes.empty() ) list().selectionModel()->clear();
  else {
    
    list().selectionModel()->select( selected_indexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
    { list().selectionModel()->select( *iter, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }
  
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
