
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
#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "FileList.h"
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
  _list().setModel( &_model() );  
  _list().setMaskOptionName( "SESSION_FILES_MASK" );
  
  // actions
  _installActions();
  
  // add actions to menu
  _list().menu().addAction( &_openAction() );
  _list().menu().addSeparator();
  _list().menu().addAction( &static_cast< Application*>( qApp )->windowServer().saveAllAction() );
  
  // connections
  connect( &_model(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), SLOT( _restoreSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), &_list(), SLOT( updateMask() ) );

  connect( _list().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateActions() ) );
  connect( _list().header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSortMethod( int, Qt::SortOrder ) ) );
  connect( &_list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemSelected( const QModelIndex& ) ) );
 
  //! configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  _updateConfiguration();
  _updateActions();
  
}

//______________________________________________________________________
SessionFilesFrame::~SessionFilesFrame( void )
{ Debug::Throw( "SessionFilesFrame::~SessionFilesFrame.\n" ); }

//____________________________________________
void SessionFilesFrame::showEvent( QShowEvent* )
{
  Debug::Throw( "SessionFilesFrame::showEvent.\n" );
  _update();
}

//______________________________________________________________________
void SessionFilesFrame::_updateConfiguration( void )
{ 
  Debug::Throw( "SessionFilesFrame::_updateConfiguration.\n" ); 
  
  // session files list sorting
  if( XmlOptions::get().find( "SESSION_FILES_SORT_COLUMN" ) && XmlOptions::get().find( "SESSION_FILES_SORT_ORDER" ) )
  { 
    _list().sortByColumn( 
      XmlOptions::get().get<int>( "SESSION_FILES_SORT_COLUMN" ), 
      (Qt::SortOrder)(XmlOptions::get().get<int>( "SESSION_FILES_SORT_ORDER" ) ) ); 
  }

}

//______________________________________________________________________
void SessionFilesFrame::_update( void )
{ 
  Debug::Throw( "SessionFilesFrame:_update.\n" ); 
 
  // check visibility
  if( !isVisible() ) return;
 
  // retrieve file records
  SessionFilesModel::List files;
  WindowServer::FileRecordMap records( static_cast< Application*>( qApp )->windowServer().files() );
  for( WindowServer::FileRecordMap::const_iterator iter = records.begin(); iter != records.end(); iter++ )
  {
    
    FileRecord record( iter->first );
    record.setFlag( SessionFilesModel::MODIFIED, iter->second );
    // Debug::Throw( 0 ) << "SessionFilesFrame:_update - file: " << record.file() << " modified: " << iter->second << " " << record.hasFlag( SessionFilesModel::MODIFIED ) << endl; 
    files.push_back( record ); 
  }

  // update model
  _model().update( files );
  _list().resizeColumns();

}

//______________________________________________________________________
void SessionFilesFrame::_updateActions( void )
{ 
  Debug::Throw( "SessionFilesFrame:_updateActions.\n" );
  _openAction().setEnabled( !_list().selectionModel()->selectedRows().isEmpty() );
}

//______________________________________________________________________
void SessionFilesFrame::_open( void )
{ 
  
  Debug::Throw( "SessionFilesFrame:_open.\n" ); 
  SessionFilesModel::List selection( model_.get( _list().selectionModel()->selectedRows() ) );
  
  // one should check the number of files to be edited
  for( SessionFilesModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { emit fileSelected( *iter ); }
  
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
  
}

//______________________________________________________________________
void SessionFilesFrame::_itemSelected( const QModelIndex& index )
{ 
  Debug::Throw( "SessionFilesFrame::_itemSelected.\n" );
  if( !index.isValid() ) return;
  emit fileSelected( _model().get( index ) );
}

//______________________________________________________________________
void SessionFilesFrame::_storeSelection( void )
{ 
  Debug::Throw( "SessionFilesFrame::_storeSelection.\n" ); 

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

//______________________________________________________________________
void SessionFilesFrame::_restoreSelection( void )
{ 
  
  Debug::Throw( "SessionFilesFrame::_restoreSelection.\n" ); 

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
