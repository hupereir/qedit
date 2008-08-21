
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
   \file SessionFilesFrame.cpp
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>
#include <QHeaderView>
#include <QLayout>

#include "Application.h"
#include "ColumnSortingMenu.h"
#include "Debug.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "Icons.h"
#include "IconEngine.h"
#include "SessionFilesFrame.h"
#include "TreeView.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlOptions.h"

using namespace std;

//_______________________________________________________________
SessionFilesFrame::SessionFilesFrame( QWidget* parent ):
  QWidget( parent ),
  Counter( "SessionFilesFrame" )
{
  
  Debug::Throw( "SessionFilesFrame:SessionFilesFrame.\n" );

  // layout
  setLayout( new QVBoxLayout() );
  layout()->setMargin(0);
  layout()->setSpacing(2);
  
  // list
  layout()->addWidget( list_ = new TreeView( this ) );
  list().setModel( &_model() );
  list().setMaskOptionName( "SESSION_FILES_MASK" );
  list().setSelectionMode( QAbstractItemView::ContiguousSelection ); 
  list().header()->hide();
  
  // actions
  _installActions();
  
  // add actions to menu
  list().menu().addMenu( new ColumnSortingMenu( &list().menu(), &list() ) );
  list().menu().addSeparator();
  list().menu().addAction( &_openAction() );
  list().menu().addAction( &_saveAction() );
  list().menu().addAction( &static_cast< Application*>( qApp )->windowServer().saveAllAction() );
  list().menu().addAction( &_closeAction() );
  connect( &list(), SIGNAL( customContextMenuRequested( const QPoint& ) ), SLOT( _updateActions() ) );
  
  // connections
  connect( &_model(), SIGNAL( layoutChanged() ), &list(), SLOT( updateMask() ) );
  connect( list().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _checkSelection() ) );

  connect( list().header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSortMethod( int, Qt::SortOrder ) ) );
  connect( &list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemActivated( const QModelIndex& ) ) );
 
  //! configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  _updateConfiguration();
  
}

//______________________________________________________________________
SessionFilesFrame::~SessionFilesFrame( void )
{ Debug::Throw( "SessionFilesFrame::~SessionFilesFrame.\n" ); }

//____________________________________________
void SessionFilesFrame::selectFile( const File& file )
{
  Debug::Throw() << "SessionFilesFrame::selectFile - file: " << file << ".\n";
   
  // find model index that match the file
  QModelIndex index( _model().index( FileRecord( file ) ) );
  
  // check if index is valid and not selected
  if( ( !index.isValid() ) || list().selectionModel()->isSelected( index ) ) return;
  
  // select found index but disable the selection changed callback
  list().selectionModel()->select( index,  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
  
}

//______________________________________________________________________
void SessionFilesFrame::_updateConfiguration( void )
{ 
  Debug::Throw( "SessionFilesFrame::_updateConfiguration.\n" ); 
  
  // session files list sorting
  if( XmlOptions::get().find( "SESSION_FILES_SORT_COLUMN" ) && XmlOptions::get().find( "SESSION_FILES_SORT_ORDER" ) )
  { 
    list().sortByColumn( 
      XmlOptions::get().get<int>( "SESSION_FILES_SORT_COLUMN" ), 
      (Qt::SortOrder)(XmlOptions::get().get<int>( "SESSION_FILES_SORT_ORDER" ) ) ); 
  }

}

//______________________________________________________________________
void SessionFilesFrame::_update( void )
{ 
  Debug::Throw( "SessionFilesFrame:_update.\n" ); 
 
  // store in model
  FileRecord::List files( static_cast< Application*>( qApp )->windowServer().files( false, window() ) );
  _model().update( files );

  // resize columns
  list().resizeColumns();

  // make sure selected record appear selected in list
  FileRecord::List::const_iterator iter = find_if( files.begin(), files.end(), FileRecord::HasFlagFTor( FileRecordProperties::SELECTED ) );
  if( iter != files.end() ) selectFile( iter->file() );  
  
  Debug::Throw( "SessionFilesFrame:_update - done.\n" ); 

}

//______________________________________________________________________
void SessionFilesFrame::_updateActions( void )
{ 
  
  Debug::Throw( "SessionFilesFrame:_updateActions.\n" );

  // check selected files
  SessionFilesModel::List selection( model_.get( list().selectionModel()->selectedRows() ) );
  
  _openAction().setEnabled( !selection.empty() );
  _closeAction().setEnabled( !selection.empty() );
  _saveAction().setEnabled( find_if( selection.begin(), selection.end(), FileRecord::HasFlagFTor( FileRecordProperties::MODIFIED ) ) != selection.end() );

}

//______________________________________________________________________
void SessionFilesFrame::_checkSelection( void )
{ 
  Debug::Throw( "SessionFilesFrame:_checkSelection.\n" );
    
  QList<QModelIndex> selection( list().selectionModel()->selectedRows() );
  if( selection.isEmpty() ) return;
  emit fileSelected( model_.get( selection.back() ) );
  
}

//______________________________________________________________________
void SessionFilesFrame::_open( void )
{ 
  
  Debug::Throw( "SessionFilesFrame:_open.\n" ); 
  SessionFilesModel::List selection( model_.get( list().selectionModel()->selectedRows() ) );
  
  for( SessionFilesModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { emit fileActivated( *iter ); }
  
}

//______________________________________________________________________
void SessionFilesFrame::_save( void )
{ 
  
  Debug::Throw( "SessionFilesFrame:_save.\n" ); 
  SessionFilesModel::List selection( model_.get( list().selectionModel()->selectedRows() ) );

  SessionFilesModel::List modified_records;
  for( SessionFilesModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { if( iter->hasFlag( FileRecordProperties::MODIFIED ) ) modified_records.push_back( *iter ); }
  
  if( !modified_records.empty() ) emit filesSaved( modified_records );
  
}


//______________________________________________________________________
void SessionFilesFrame::_close( void )
{ 
  
  Debug::Throw( "SessionFilesFrame:_close.\n" ); 
  SessionFilesModel::List selection( model_.get( list().selectionModel()->selectedRows() ) );
  if( !selection.empty() ) emit filesClosed( selection );
  
}

//______________________________________________________________________
void SessionFilesFrame::_storeSortMethod( int column, Qt::SortOrder order )
{
  
  Debug::Throw( "SessionFilesFrame::_storeSortMethod.\n" );
  XmlOptions::get().set<int>( "SESSION_FILES_SORT_COLUMN", column );
  XmlOptions::get().set<int>( "SESSION_FILES_SORT_ORDER", order );
  
}

//______________________________________________________________________
void SessionFilesFrame::_installActions( void )
{
  
  Debug::Throw( "SessionFilesFrame::_installActions.\n" );

  // update
  addAction( update_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Update &recent files", this ) );
  connect( update_action_, SIGNAL( triggered() ), SLOT( _update() ) );

  // open
  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open selected files", this ) );
  connect( &_openAction(), SIGNAL( triggered() ), SLOT( _open() ) );
  _openAction().setToolTip( "Open selected files" );
  
  // save
  addAction( save_action_ = new QAction( IconEngine::get( ICONS::SAVE ), "&Save selected files", this ) );
  connect( &_saveAction(), SIGNAL( triggered() ), SLOT( _save() ) );
  _saveAction().setToolTip( "Save selected files" );

  // save
  addAction( close_action_ = new QAction( IconEngine::get( ICONS::DIALOG_CLOSE ), "&Close selected files", this ) );
  connect( &_closeAction(), SIGNAL( triggered() ), SLOT( _close() ) );
  _closeAction().setToolTip( "Close selected files" );

}

//______________________________________________________________________
void SessionFilesFrame::_itemActivated( const QModelIndex& index )
{ 
  Debug::Throw( "SessionFilesFrame::_itemActivated.\n" );
  if( !index.isValid() ) return;
  emit fileActivated( _model().get( index ) );
}
