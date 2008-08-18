
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
   \file RecentFilesFrame.cpp
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
#include "Icons.h"
#include "IconEngine.h"
#include "FileList.h"
#include "QtUtil.h"
#include "RecentFilesFrame.h"
#include "TreeView.h"
#include "Util.h"
#include "XmlOptions.h"

using namespace std;

//_______________________________________________________________
RecentFilesFrame::RecentFilesFrame( QWidget* parent, FileList& files ):
  QWidget( parent ),
  Counter( "RecentFilesFrame" ),
  recent_files_( &files )
{
  
  Debug::Throw( "RecentFilesFrame:RecentFilesFrame.\n" );

  // layout
  setLayout( new QVBoxLayout() );
  layout()->setMargin(0);
  layout()->setSpacing(2);
  
  // list
  layout()->addWidget( list_ = new TreeView( this ) );
  _list().setModel( &_model() );  
  _list().setSelectionMode( QAbstractItemView::ContiguousSelection ); 
  _list().setMaskOptionName( "RECENT_FILES_MASK" );
  _list().header()->hide();
  
  // actions
  _installActions();
  
  // add actions to list
  _list().menu().addMenu( new ColumnSortingMenu( &_list().menu(), &_list() ) );
  _list().menu().addAction( &_openAction() );
  _list().menu().addSeparator();
  _list().menu().addAction( &_cleanAction() );
  
  // connections
  connect( &_model(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), SLOT( _restoreSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), &_list(), SLOT( updateMask() ) );

  connect( _list().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateActions() ) );
  connect( _list().header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSortMethod( int, Qt::SortOrder ) ) );
  connect( &_list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemSelected( const QModelIndex& ) ) );

  connect( &_recentFiles(), SIGNAL( validFilesChecked( void ) ), SLOT( _update( void ) ) );
  
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  _updateConfiguration();
  _updateActions();
  
}

//______________________________________________________________________
RecentFilesFrame::~RecentFilesFrame( void )
{ Debug::Throw( "RecentFilesFrame::~RecentFilesFrame.\n" ); }

//____________________________________________
void RecentFilesFrame::enterEvent( QEvent* e )
{

  Debug::Throw( "RecentFilesFrame::enterEvent.\n" );
  QWidget::enterEvent( e );

  // check recent files validity
  _recentFiles().checkValidFiles();
  
}

//____________________________________________
void RecentFilesFrame::showEvent( QShowEvent* )
{
  Debug::Throw( "RecentFilesFrame::showEvent.\n" );
  _update();
}

//______________________________________________________________________
void RecentFilesFrame::_updateConfiguration( void )
{ 
  Debug::Throw( "RecentFilesFrame::_updateConfiguration.\n" ); 
  
  // session files list sorting
  if( XmlOptions::get().find( "RECENT_FILES_SORT_COLUMN" ) && XmlOptions::get().find( "RECENT_FILES_SORT_ORDER" ) )
  { 
    _list().sortByColumn( 
      XmlOptions::get().get<int>( "RECENT_FILES_SORT_COLUMN" ), 
      (Qt::SortOrder)(XmlOptions::get().get<int>( "RECENT_FILES_SORT_ORDER" ) ) ); 
  }

}

//______________________________________________________________________
void RecentFilesFrame::_update( void )
{ 
  Debug::Throw( "RecentFilesFrame:_update.\n" ); 
 
  // check visibility
  if( !isVisible() ) return;
 
  // update records
  _model().update( _recentFiles().records() );
  _list().resizeColumns();
  
  // clean action enability
  _cleanAction().setEnabled( _recentFiles().hasInvalidFiles() || _recentFiles().hasDuplicatedFiles() );
  
}

//______________________________________________________________________
void RecentFilesFrame::_updateActions( void )
{ 
  Debug::Throw( "RecentFilesFrame:_updateActions.\n" );
  FileRecordModel::List selection( model_.get( _list().selectionModel()->selectedRows() ) );
  
  bool has_valid_selection( find_if( selection.begin(), selection.end(), FileRecord::ValidFTor() ) != selection.end() );
  _openAction().setEnabled( has_valid_selection );
  
}

//______________________________________________________________________
void RecentFilesFrame::_clean( void )
{ 
  
  Debug::Throw( "RecentFilesFrame:_clean.\n" ); 
  if( !_recentFiles().check() && !QtUtil::questionDialog( this,"clear list ?" ) ) return;
  else if( _recentFiles().check() && !QtUtil::questionDialog( this,"Remove invalid/duplicated files from list ?" ) ) return;
  _recentFiles().clean();
  _recentFiles().clearDuplicates();
  _update();
  
}

//______________________________________________________________________
void RecentFilesFrame::_open( void )
{ 
  
  Debug::Throw( "RecentFilesFrame:_open.\n" ); 
  FileRecordModel::List selection( model_.get( _list().selectionModel()->selectedRows() ) );
  FileRecordModel::List valid_selection;
  for( FileRecordModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { if( iter->isValid() ) valid_selection.push_back( *iter ); }
  
  // one should check the number of files to be edited
  for( FileRecordModel::List::const_iterator iter = valid_selection.begin(); iter != valid_selection.end(); iter++ )
  { emit fileSelected( *iter ); }
  
}

//______________________________________________________________________
void RecentFilesFrame::_storeSortMethod( int column, Qt::SortOrder order )
{
  
  Debug::Throw( "RecentFilesFrame::_storeSortMethod.\n" );
  XmlOptions::get().set<int>( "RECENT_FILES_SORT_COLUMN", column );
  XmlOptions::get().set<int>( "RECENT_FILES_SORT_ORDER", order );
  
}

//______________________________________________________________________
void RecentFilesFrame::_installActions( void )
{
  
  Debug::Throw( "RecentFilesFrame::_installActions.\n" );

  // update
  addAction( update_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Update &recent files", this ) );
  connect( &updateAction(), SIGNAL( triggered() ), SLOT( _update() ) );

  // clean
  addAction( clean_action_ = new QAction( IconEngine::get( ICONS::DELETE ), "&Clean", this ) );
  connect( &_cleanAction(), SIGNAL( triggered() ), SLOT( _clean() ) );
  _cleanAction().setEnabled( false );
  _cleanAction().setToolTip( "Clean invalid files" );

  // open
  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open selected files", this ) );
  connect( &_openAction(), SIGNAL( triggered() ), SLOT( _open() ) );
  _openAction().setToolTip( "Show selected files" );
  
}

//______________________________________________________________________
void RecentFilesFrame::_itemSelected( const QModelIndex& index )
{ 
  Debug::Throw( "RecentFilesFrame::_itemSelected.\n" );
  if( !index.isValid() ) return;
  emit fileSelected( _model().get( index ) );
}

//______________________________________________________________________
void RecentFilesFrame::_storeSelection( void )
{ 
  Debug::Throw( "RecentFilesFrame::_storeSelection.\n" ); 

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
void RecentFilesFrame::_restoreSelection( void )
{ 
  
  Debug::Throw( "RecentFilesFrame::_restoreSelection.\n" ); 

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
