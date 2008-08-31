
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
#include "QuestionDialog.h"
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
  recent_files_( &files ),
  enabled_( true )
{
  
  Debug::Throw( "RecentFilesFrame:RecentFilesFrame.\n" );

  // layout
  setLayout( new QVBoxLayout() );
  layout()->setMargin(0);
  layout()->setSpacing(2);
  
  // list
  layout()->addWidget( list_ = new TreeView( this ) );
  list().setModel( &_model() );  
  list().setSelectionMode( QAbstractItemView::ContiguousSelection ); 
  list().setMaskOptionName( "RECENT_FILES_MASK" );
  list().header()->hide();
  
  // actions
  _installActions();
  
  // add actions to list
  list().menu().addMenu( new ColumnSortingMenu( &list().menu(), &list() ) );
  list().menu().addAction( &_openAction() );
  list().menu().addSeparator();
  list().menu().addAction( &_cleanAction() );
  connect( &list(), SIGNAL( customContextMenuRequested( const QPoint& ) ), SLOT( _updateActions( void ) ) );
  
  // connections
  connect( &_model(), SIGNAL( layoutChanged() ), &list(), SLOT( updateMask() ) );

  connect( list().header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSortMethod( int, Qt::SortOrder ) ) );
  connect( list().selectionModel(), SIGNAL( currentRowChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _itemSelected( const QModelIndex& ) ) );
  connect( &list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemActivated( const QModelIndex& ) ) );

  connect( &_recentFiles(), SIGNAL( validFilesChecked( void ) ), SLOT( update( void ) ) );
  
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  _updateConfiguration();
  
}

//______________________________________________________________________
RecentFilesFrame::~RecentFilesFrame( void )
{ Debug::Throw( "RecentFilesFrame::~RecentFilesFrame.\n" ); }

//____________________________________________
void RecentFilesFrame::select( const File& file )
{
  
  if( !_enabled() ) return;
  
  Debug::Throw() << "RecentFilesFrame::select - file: " << file << ".\n";
   
  // find model index that match the file
  QModelIndex index( _model().index( FileRecord( file ) ) );
  
  // check if index is valid and not selected
  if( ( !index.isValid() ) || (index == list().selectionModel()->currentIndex() ) ) return;

  // select found index but disable the selection changed callback
  _setEnabled( false );
  list().selectionModel()->select( index,  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
  list().selectionModel()->setCurrentIndex( index,  QItemSelectionModel::Current|QItemSelectionModel::Rows );
  _setEnabled( true );
  
}

//______________________________________________________________________
void RecentFilesFrame::update( void )
{ 
  Debug::Throw( "RecentFilesFrame:update.\n" ); 
 
  // update records
  _model().update( _recentFiles().records() );
  list().resizeColumns();
  
  // clean action enability
  _cleanAction().setEnabled( _recentFiles().cleanEnabled() );

  Debug::Throw( "RecentFilesFrame:_update - done.\n" ); 
  
}

//____________________________________________
void RecentFilesFrame::enterEvent( QEvent* e )
{

  Debug::Throw( "RecentFilesFrame::enterEvent.\n" );
  QWidget::enterEvent( e );

  // check recent files validity
  _recentFiles().checkValidFiles();
  
}

//______________________________________________________________________
void RecentFilesFrame::_updateConfiguration( void )
{ 
  Debug::Throw( "RecentFilesFrame::_updateConfiguration.\n" ); 
  
  // session files list sorting
  if( XmlOptions::get().find( "RECENT_FILES_SORT_COLUMN" ) && XmlOptions::get().find( "RECENT_FILES_SORT_ORDER" ) )
  { 
   
    list().sortByColumn( 
      XmlOptions::get().get<int>( "RECENT_FILES_SORT_COLUMN" ), 
      (Qt::SortOrder)(XmlOptions::get().get<int>( "RECENT_FILES_SORT_ORDER" ) ) ); 

  } else if( XmlOptions::get().find( "SORT_FILES_BY_DATE" ) ) {
    
    list().sortByColumn( 
      FileRecordModel::TIME, 
      Qt::DescendingOrder ); 
  
  } else {
    
    list().sortByColumn( 
      FileRecordModel::FILE, 
      Qt::DescendingOrder ); 
    
  }

}

//______________________________________________________________________
void RecentFilesFrame::_updateActions( void )
{ 
  Debug::Throw( "RecentFilesFrame:_updateActions.\n" );
  FileRecordModel::List selection( _model().get( list().selectionModel()->selectedRows() ) );
  
  bool has_valid_selection( find_if( selection.begin(), selection.end(), FileRecord::ValidFTor() ) != selection.end() );
  _openAction().setEnabled( has_valid_selection );
  
}

//______________________________________________________________________
void RecentFilesFrame::_clean( void )
{ 
  
  Debug::Throw( "RecentFilesFrame:_clean.\n" ); 
  if( !QuestionDialog( this,"Remove invalid or duplicated files from list ?" ).exec() ) return;
  _recentFiles().clean();
  update();
  
}

//______________________________________________________________________
void RecentFilesFrame::_open( void )
{ 
  
  Debug::Throw( "RecentFilesFrame:_open.\n" ); 
  FileRecordModel::List selection( _model().get( list().selectionModel()->selectedRows() ) );
  FileRecordModel::List valid_selection;
  for( FileRecordModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { if( iter->isValid() ) valid_selection.push_back( *iter ); }
  
  // one should check the number of files to be edited
  for( FileRecordModel::List::const_iterator iter = valid_selection.begin(); iter != valid_selection.end(); iter++ )
  { emit fileActivated( *iter ); }
  
}

//______________________________________________________________________
void RecentFilesFrame::_storeSortMethod( int column, Qt::SortOrder order )
{
  
  Debug::Throw( "RecentFilesFrame::_storeSortMethod.\n" );
  XmlOptions::get().set<int>( "RECENT_FILES_SORT_COLUMN", column );
  XmlOptions::get().set<int>( "RECENT_FILES_SORT_ORDER", order );
  
}

//______________________________________________________________________
void RecentFilesFrame::_itemSelected( const QModelIndex& index )
{ 
  
  Debug::Throw( "RecentFilesFrame::_itemSelected.\n" );
  if( !index.isValid() ) return;  
  
  _setEnabled( false );
  emit fileSelected( model_.get( index ) );
  _setEnabled( true );
  
}

//______________________________________________________________________
void RecentFilesFrame::_itemActivated( const QModelIndex& index )
{ 
  Debug::Throw( "RecentFilesFrame::_itemActivated.\n" );
  if( !( index.isValid() && _enabled() ) ) return;
  
  _setEnabled( false );
  emit fileActivated( _model().get( index ) );
  _setEnabled( true );
  
}

//______________________________________________________________________
void RecentFilesFrame::_installActions( void )
{
  
  Debug::Throw( "RecentFilesFrame::_installActions.\n" );

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
