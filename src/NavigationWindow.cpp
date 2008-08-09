
/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later-
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
   \file NavigationWindow.cpp
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>
#include <QPainter>
#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <QLayout>

#include "Application.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "FileList.h"
#include "NavigationWindow.h"
#include "TreeView.h"
#include "WindowServer.h"

using namespace std;

//_______________________________________________________________
NavigationWindow::NavigationWindow( QWidget* parent, FileList& files ):
  CustomMainWindow( parent ),
  Counter( "MainWindow" ),
  recent_files_( &files )
{
  
  Debug::Throw( "NavigationWindow:NavigationWindow.\n" );
  _setSizeOptionName( "NAVIGATION_WINDOW" );
  
  QWidget* main( new QWidget( this ) );
  setCentralWidget( main );
  
  _installActions();
  
  // add horizontal layout for toolbar and stacked widget
  QHBoxLayout *h_layout = new QHBoxLayout();
  h_layout->setSpacing(2);
  h_layout->setMargin(2);
  main->setLayout( h_layout );
  
  // add vertical layout for toolbar buttons
  QVBoxLayout *v_layout = new QVBoxLayout();
  v_layout->setSpacing(2);
  v_layout->setMargin(2);
  h_layout->addLayout( v_layout, 0 );
    
  // create stack widget
  h_layout->addWidget( stack_ = new QStackedWidget( this ) );
  
  // create session files tree view
  session_files_list_ = new TreeView(0);  
  _sessionFilesList().setModel( &_sessionFilesModel() );  
  _sessionFilesList().setMask( (1<<FileRecordModel::FILE) );
  stack_->addWidget( &_sessionFilesList() );
  
  // create recent files tree view
  recent_files_list_ = new TreeView(0);  
  _recentFilesList().setModel( &_recentFilesModel() );  
  _recentFilesList().setMask( (1<<FileRecordModel::FILE) );
  stack_->addWidget( &_recentFilesList() );
  
  // create button group
  QButtonGroup* button_group = new QButtonGroup( this );
  connect( button_group, SIGNAL( buttonClicked( QAbstractButton* ) ), SLOT( _display( QAbstractButton* ) ) );
  button_group->setExclusive( true );

  // create matching buttons
  CustomToolButton* button;
  
  // session files
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->setChecked( true );
  button->setRotation( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&Session files" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_sessionFilesList() ) );
  
  // recent files
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->setRotation( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&Recent files" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_recentFilesList() ) );
  v_layout->addStretch( 1 );

  // connections  
  connect( &_sessionFilesModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSessionFilesSelection() ) );
  connect( &_sessionFilesModel(), SIGNAL( layoutChanged() ), SLOT( _restoreSessionFilesSelection() ) );
  connect( &_sessionFilesList(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _sessionFilesItemSelected( const QModelIndex& ) ) );

  connect( &_recentFilesModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeRecentFilesSelection() ) );
  connect( &_recentFilesModel(), SIGNAL( layoutChanged() ), SLOT( _restoreRecentFilesSelection() ) );
  connect( &_recentFilesList(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _recentFilesItemSelected( const QModelIndex& ) ) );
  
  //! configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( saveConfiguration() ), SLOT( _saveConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
 _updateConfiguration();
  
}

//______________________________________________________________________
NavigationWindow::~NavigationWindow( void )
{ Debug::Throw( "NavigationWindow::~NavigationWindow.\n" ); }


//____________________________________________
void NavigationWindow::enterEvent( QEvent* e )
{

  Debug::Throw( "NavigationWindow::enterEvent.\n" );
  CustomMainWindow::enterEvent( e );

  // check recent files validity
  _recentFiles().checkValidFiles();
  
}

//____________________________________________
void NavigationWindow::closeEvent( QCloseEvent* event )
{
  Debug::Throw( "NavigationWindow::closeEvent.\n" );

  // accept event
  visibilityAction().setChecked( false );
  event->accept();
}

//____________________________________________
void NavigationWindow::showEvent( QShowEvent* )
{
  Debug::Throw( "NavigationWindow::showEvent.\n" );
  _updateFiles();  
}

//______________________________________________________________________
void NavigationWindow::_updateConfiguration( void )
{ 
  Debug::Throw( "NavigationWindow::_updateConfiguration.\n" );
  visibilityAction().setChecked( XmlOptions::get().get<bool>( "SHOW_NAVIGATION_WINDOW" ) );
  
  // restore TreeView masks
  if( XmlOptions::get().find( "SESSION_FILES_MASK" ) ) _sessionFilesList().setMask( XmlOptions::get().get<unsigned int>( "SESSION_FILES_MASK" ) );
  if( XmlOptions::get().find( "RECENT_FILES_MASK" ) ) _recentFilesList().setMask( XmlOptions::get().get<unsigned int>( "RECENT_FILES_MASK" ) );
  
}

//______________________________________________________________________
void NavigationWindow::_saveConfiguration( void )
{ 

  Debug::Throw( "NavigationWindow::_saveConfiguration.\n" );
  XmlOptions::get().set<bool>( "SHOW_NAVIGATION_WINDOW" , visibilityAction().isChecked() );  
  XmlOptions::get().set<unsigned int>( "SESSION_FILES_MASK", _sessionFilesList().mask() );
  XmlOptions::get().set<unsigned int>( "RECENT_FILES_MASK", _recentFilesList().mask() );

}  

//______________________________________________________________________
void NavigationWindow::_updateSessionFiles( void )
{ 
  Debug::Throw( "NavigationWindow:_updateSessionFiles.\n" ); 

  // check visibility
  if( !( isVisible() && _stack().currentWidget() == &_sessionFilesList() ) ) return;
  
  // save mask
  unsigned int mask( _sessionFilesList().mask() );
  
  // retrieve file records
  FileRecordModel::List files;
  WindowServer::FileRecordMap records( static_cast< Application*>( qApp )->windowServer().files() );
  for( WindowServer::FileRecordMap::const_iterator iter = records.begin(); iter != records.end(); iter++ )
  { files.push_back( iter->first ); }

  // replace
  _sessionFilesModel().update( files );
 
  // restore mask and resize columns
  _sessionFilesList().setMask( mask );
  _sessionFilesList().resizeColumns();

}

//______________________________________________________________________
void NavigationWindow::_updateRecentFiles( void )
{ 
  Debug::Throw( "NavigationWindow:_updateRecentFiles.\n" ); 
 
  // check visibility
  if( !( isVisible() && _stack().currentWidget() == &_recentFilesList() ) ) return;
 
  // save mask
  unsigned int mask( _recentFilesList().mask() );
    
  // update records
  _recentFilesModel().update( _recentFiles().records() );
  
  // restore mask and resize columns
  _recentFilesList().setMask( mask );
  _recentFilesList().resizeColumns();

}

//______________________________________________________________________
void NavigationWindow::_display( QAbstractButton* button )
{  
  
  Debug::Throw( "NavigationWindow:_display.\n" ); 
  if( !button->isChecked() ) return;
  
  // retrieve item in map
  ButtonMap::const_iterator iter( buttons_.find( button ) );
  assert( iter != buttons_.end() );

  // display corresponding widget
  _stack().setCurrentWidget( iter->second );

  // update displays
  _updateFiles();
  
}

//______________________________________________________________________
void NavigationWindow::_installActions( void )
{
  
  Debug::Throw( "NavigationWindow::_installActions.\n" );
  addAction( visibility_action_ = new QAction( "&Navigation window", this ) );
  visibility_action_->setCheckable( true );
  visibility_action_->setChecked( false );
  connect( visibility_action_, SIGNAL( toggled( bool ) ), SLOT( setVisible( bool ) ) );
   
  // update session files
  addAction( session_files_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Update &session files", this ) );
  connect( session_files_action_, SIGNAL( triggered() ), SLOT( _updateSessionFiles() ) );
   
  // update recent files
  addAction( recent_files_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Update &recent files", this ) );
  connect( recent_files_action_, SIGNAL( triggered() ), SLOT( _updateRecentFiles() ) );
  
}

//______________________________________________________________________
void NavigationWindow::_itemSelected( const FileRecordModel& model, const QModelIndex& index )
{ 
  Debug::Throw( "NavigationWindow::_itemSelected.\n" );
  if( !index.isValid() ) return;
  emit fileSelected( model.get( index ) );
}

//______________________________________________________________________
void NavigationWindow::_storeSelection( TreeView& view, FileRecordModel& model )
{ 
  Debug::Throw( "NavigationWindow::_storeSelection.\n" ); 

  // clear
  model.clearSelectedIndexes();
  
  // retrieve selected indexes in list
  QModelIndexList selected_indexes( view.selectionModel()->selectedRows() );
  for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
  { 
    // check column
    if( !iter->column() == 0 ) continue;
    model.setIndexSelected( *iter, true ); 
  }

  return;
  
}

//______________________________________________________________________
void NavigationWindow::_restoreSelection( TreeView& view, FileRecordModel& model )
{ 
  
  Debug::Throw( "NavigationWindow::_restoreSelection.\n" ); 

  // retrieve indexes
  QModelIndexList selected_indexes( model.selectedIndexes() );
  if( selected_indexes.empty() ) view.selectionModel()->clear();
  else {
    
    view.selectionModel()->select( selected_indexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
    { view.selectionModel()->select( *iter, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }
  
  }
  
  return;
  
}
