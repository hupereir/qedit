
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
   \file NavigationFrame.cpp
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>
#include <QHeaderView>
#include <QLayout>

#include "Application.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "FileList.h"
#include "FileSystemFrame.h"
#include "NavigationFrame.h"
#include "TreeView.h"
#include "Util.h"
#include "WindowServer.h"

using namespace std;

//_______________________________________________________________
NavigationFrame::NavigationFrame( QWidget* parent, FileList& files ):
  QWidget( parent ),
  Counter( "NavigationFrame" ),
  default_width_( -1 ),
  recent_files_( &files )
{
  
  Debug::Throw( "NavigationFrame:NavigationFrame.\n" );

  // actions
  _installActions();
  
  // add horizontal layout for toolbar and stacked widget
  QHBoxLayout *h_layout = new QHBoxLayout();
  h_layout->setSpacing(2);
  h_layout->setMargin(0);
  setLayout( h_layout );
  
  // add vertical layout for toolbar buttons
  QVBoxLayout *v_layout = new QVBoxLayout();
  v_layout->setSpacing(2);
  v_layout->setMargin(0);
  h_layout->addLayout( v_layout, 0 );
    
  // stack widget
  h_layout->addWidget( stack_ = new QStackedWidget( this ) );
  
  // files tree view
  session_files_list_ = new TreeView(0);  
  _sessionFilesList().setModel( &_sessionFilesModel() );  
  _sessionFilesList().setMaskOptionName( "SESSION_FILES_MASK" );
  _stack().addWidget( &_sessionFilesList() );
  
  // recent files tree view
  recent_files_list_ = new TreeView(0);  
  _recentFilesList().setModel( &_recentFilesModel() );  
  _recentFilesList().setMaskOptionName( "RECENT_FILES_MASK" );
  _stack().addWidget( &_recentFilesList() );
  
  // file system list
  file_system_frame_ = new FileSystemFrame(0);
  _stack().addWidget( &fileSystemFrame() );
  
  // button group
  QButtonGroup* button_group = new QButtonGroup( this );
  connect( button_group, SIGNAL( buttonClicked( QAbstractButton* ) ), SLOT( _display( QAbstractButton* ) ) );
  button_group->setExclusive( true );

  // matching buttons
  CustomToolButton* button;
  
  // session files
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->setChecked( true );
  button->rotate( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&Session files" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_sessionFilesList() ) );
  
  // recent files
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->rotate( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&Recent files" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_recentFilesList() ) );

  // file system
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->rotate( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&File system" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &fileSystemFrame() ) );
  v_layout->addStretch( 1 );

  // connections
  connect( &_sessionFilesModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSessionFilesSelection() ) );
  connect( &_sessionFilesModel(), SIGNAL( layoutChanged() ), SLOT( _restoreSessionFilesSelection() ) );
  connect( &_sessionFilesModel(), SIGNAL( layoutChanged() ), &_sessionFilesList(), SLOT( updateMask() ) );
  connect( &_sessionFilesList(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _sessionFilesItemSelected( const QModelIndex& ) ) );
  connect( _sessionFilesList().header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSessionFilesSortMethod( int, Qt::SortOrder ) ) );

  connect( &_recentFilesModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeRecentFilesSelection() ) );
  connect( &_recentFilesModel(), SIGNAL( layoutChanged() ), SLOT( _restoreRecentFilesSelection() ) );
  connect( &_recentFilesModel(), SIGNAL( layoutChanged() ), &_recentFilesList(), SLOT( updateMask() ) );
  connect( &_recentFilesList(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _recentFilesItemSelected( const QModelIndex& ) ) );
  connect( _recentFilesList().header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeRecentFilesSortMethod( int, Qt::SortOrder ) ) );
  
  //! configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( saveConfiguration() ), SLOT( _saveConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();
  
}

//______________________________________________________________________
NavigationFrame::~NavigationFrame( void )
{ Debug::Throw( "NavigationFrame::~NavigationFrame.\n" ); }

//______________________________________________________________________
void NavigationFrame::setDefaultWidth( const int& value )
{ default_width_ = value; }


//____________________________________________
QSize NavigationFrame::sizeHint( void ) const
{ return (default_width_ ) >= 0 ? QSize( default_width_, 0 ):QWidget::sizeHint(); }

//____________________________________________
void NavigationFrame::enterEvent( QEvent* e )
{

  Debug::Throw( "NavigationFrame::enterEvent.\n" );
  QWidget::enterEvent( e );

  // check recent files validity
  _recentFiles().checkValidFiles();
  
}

//____________________________________________
void NavigationFrame::showEvent( QShowEvent* )
{
  Debug::Throw( "NavigationFrame::showEvent.\n" );
  _updateFiles();  
}

//______________________________________________________________________
void NavigationFrame::_updateConfiguration( void )
{ 
  Debug::Throw( "NavigationFrame::_updateConfiguration.\n" ); 
  
  // session files list sorting
  if( XmlOptions::get().find( "SESSION_FILES_SORT_COLUMN" ) && XmlOptions::get().find( "SESSION_FILES_SORT_ORDER" ) )
  { 
    _sessionFilesList().sortByColumn( 
      XmlOptions::get().get<int>( "RECENT_FILES_SORT_COLUMN" ), 
      (Qt::SortOrder)(XmlOptions::get().get<int>( "SESSION_FILES_SORT_ORDER" ) ) ); 
  }

  // recent files list sorting
  if( XmlOptions::get().find( "RECENT_FILES_SORT_COLUMN" ) && XmlOptions::get().find( "RECENT_FILES_SORT_ORDER" ) )
  { 
    _recentFilesList().sortByColumn( 
      XmlOptions::get().get<int>( "RECENT_FILES_SORT_COLUMN" ), 
      (Qt::SortOrder)(XmlOptions::get().get<int>( "RECENT_FILES_SORT_ORDER" ) ) ); 
  }

}

//______________________________________________________________________
void NavigationFrame::_saveConfiguration( void )
{ Debug::Throw( "NavigationFrame::_saveConfiguration.\n" ); }  

//______________________________________________________________________
void NavigationFrame::_updateSessionFiles( void )
{ 
  Debug::Throw( "NavigationFrame:_updateSessionFiles.\n" ); 

  // check visibility
  if( !( isVisible() && _stack().currentWidget() == &_sessionFilesList() ) ) return;
    
  // retrieve file records
  FileRecordModel::List files;
  WindowServer::FileRecordMap records( static_cast< Application*>( qApp )->windowServer().files() );
  for( WindowServer::FileRecordMap::const_iterator iter = records.begin(); iter != records.end(); iter++ )
  { files.push_back( iter->first ); }

  // replace
  _sessionFilesModel().update( files );
  
  // restore mask and resize columns
  _sessionFilesList().resizeColumns();

}

//______________________________________________________________________
void NavigationFrame::_updateRecentFiles( void )
{ 
  Debug::Throw( "NavigationFrame:_updateRecentFiles.\n" ); 
 
  // check visibility
  if( !( isVisible() && _stack().currentWidget() == &_recentFilesList() ) ) return;
 
  // update records
  _recentFilesModel().update( _recentFiles().records() );
  
  // restore mask and resize columns
  _recentFilesList().resizeColumns();

}

//______________________________________________________________________
void NavigationFrame::_updateFileSystemFiles( void )
{
   Debug::Throw( "NavigationFrame:_updateFileSystemFiles.\n" ); 
 
  // check visibility
  if( !( isVisible() && _stack().currentWidget() == &fileSystemFrame() ) ) return;
  if( fileSystemFrame().path().empty() ) fileSystemFrame().setPath( Util::workingDirectory() );
  else fileSystemFrame().reloadAction().trigger();
  
}

//______________________________________________________________________
void NavigationFrame::_storeSessionFilesSortMethod( int column, Qt::SortOrder order )
{
  
  Debug::Throw( "NavigationFrame::_storeSessionFilesSortMethod.\n" );
  XmlOptions::get().set<int>( "SESSION_FILES_SORT_COLUMN", column );
  XmlOptions::get().set<int>( "SESSION_FILES_SORT_ORDER", order );
  
}

//______________________________________________________________________
void NavigationFrame::_storeRecentFilesSortMethod( int column, Qt::SortOrder order )
{
  
  Debug::Throw( "NavigationFrame::_storeRecentFilesSortMethod.\n" );
  XmlOptions::get().set<int>( "RECENT_FILES_SORT_COLUMN", column );
  XmlOptions::get().set<int>( "RECENT_FILES_SORT_ORDER", order );
  
}

//______________________________________________________________________
void NavigationFrame::_display( QAbstractButton* button )
{  
  
  Debug::Throw( "NavigationFrame:_display.\n" ); 
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
void NavigationFrame::_installActions( void )
{
  
  Debug::Throw( "NavigationFrame::_installActions.\n" );
  addAction( visibility_action_ = new QAction( "Show &navigation panel", this ) );
  visibility_action_->setCheckable( true );
  visibility_action_->setChecked( true );
  visibility_action_->setShortcut( Qt::Key_F5 );
  connect( visibility_action_, SIGNAL( toggled( bool ) ), SLOT( setVisible( bool ) ) );
   
  // update session files
  addAction( session_files_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Update &session files", this ) );
  connect( session_files_action_, SIGNAL( triggered() ), SLOT( _updateSessionFiles() ) );
   
  // update recent files
  addAction( recent_files_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Update &recent files", this ) );
  connect( recent_files_action_, SIGNAL( triggered() ), SLOT( _updateRecentFiles() ) );
  
}

//______________________________________________________________________
void NavigationFrame::_itemSelected( const FileRecordModel& model, const QModelIndex& index )
{ 
  Debug::Throw( "NavigationFrame::_itemSelected.\n" );
  if( !index.isValid() ) return;
  emit fileSelected( model.get( index ) );
}

//______________________________________________________________________
void NavigationFrame::_storeSelection( TreeView& view, FileRecordModel& model )
{ 
  Debug::Throw( "NavigationFrame::_storeSelection.\n" ); 

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
void NavigationFrame::_restoreSelection( TreeView& view, FileRecordModel& model )
{ 
  
  Debug::Throw( "NavigationFrame::_restoreSelection.\n" ); 

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
