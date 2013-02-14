
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

#include "SessionFilesFrame.h"

#include "Application.h"
#include "AnimatedTreeView.h"
#include "ColumnSortingMenu.h"
#include "ColumnSelectionMenu.h"
#include "ContextMenu.h"
#include "Debug.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "FileRecordToolTipWidget.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Singleton.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlOptions.h"

#include <QButtonGroup>
#include <QHeaderView>
#include <QLayout>

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

    // tooltip widget
    toolTipWidget_ = new FileRecordToolTipWidget( this );

    // list
    model_.setDragEnabled( true );
    layout()->addWidget( list_ = new AnimatedTreeView( this ) );
    list_->setMouseTracking( true );
    list_->setModel( &model_ );
    list_->setItemMargin( 2 );
    list_->setOptionName( "SESSION_FILES" );
    list_->header()->hide();
    list_->setDragEnabled( true );
    list_->setAcceptDrops( true );
    list_->setDropIndicatorShown(true);

    // selection mode
    list_->setSelectionMode( QAbstractItemView::ContiguousSelection );

    // actions
    _installActions();

    // add actions to menu
    ContextMenu* menu( new ContextMenu( list_ ) );
    menu->addMenu( new ColumnSortingMenu( menu, list_ ) );
    menu->addMenu( new ColumnSelectionMenu( menu, list_ ) );
    menu->addSeparator();
    menu->addAction( openAction_ );
    menu->addAction( saveAction_ );
    menu->addAction( &Singleton::get().application<Application>()->windowServer().saveAllAction() );
    menu->addAction( closeAction_ );
    menu->addSeparator();
    menu->addAction( previousFileAction_ );
    menu->addAction( nextFileAction_ );

    // connections
    connect( &model_, SIGNAL( layoutChanged() ), list_, SLOT( updateMask() ) );
    connect( list_, SIGNAL( customContextMenuRequested( const QPoint& ) ), SLOT( _updateActions() ) );
    connect( list_->selectionModel(), SIGNAL( currentRowChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _itemSelected( const QModelIndex& ) ) );
    connect( list_, SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemActivated( const QModelIndex& ) ) );
    connect( list_, SIGNAL( hovered( const QModelIndex& ) ), SLOT( _showToolTip( const QModelIndex& ) ) );

}

//______________________________________________________________________
SessionFilesFrame::~SessionFilesFrame( void )
{ Debug::Throw( "SessionFilesFrame::~SessionFilesFrame.\n" ); }

//____________________________________________
void SessionFilesFrame::select( const File& file )
{

    Debug::Throw() << "SessionFilesFrame::select - file: " << file << ".\n";

    // find model index that match the file
    QModelIndex index( model_.index( FileRecord( file ) ) );

    // check if index is valid and not selected
    if( !index.isValid() ) return;

    // ensure index is selected
    if( !list_->selectionModel()->isSelected( index ) )
    { list_->selectionModel()->select( index, QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    // ensure index is current
    if( index != list_->selectionModel()->currentIndex() )
    { list_->selectionModel()->setCurrentIndex( index,  QItemSelectionModel::Current|QItemSelectionModel::Rows ); }

}

//______________________________________________________________________
void SessionFilesFrame::update( void )
{
    Debug::Throw( "SessionFilesFrame:update.\n" );

    // store in model
    const FileRecord::List records( Singleton::get().application<Application>()->windowServer().records( false, window() ) );
    model_.update( records );

    list_->updateMask();
    list_->resizeColumns();

    // make sure selected record appear selected in list
    FileRecord::List::const_iterator iter = std::find_if( records.begin(), records.end(), FileRecord::HasFlagFTor( FileRecordProperties::SELECTED ) );
    if( iter != records.end() ) select( iter->file() );

    Debug::Throw( "SessionFilesFrame:update - done.\n" );

}

//______________________________________________________________________
void SessionFilesFrame::_updateActions( void )
{

    Debug::Throw( "SessionFilesFrame:_updateActions.\n" );

    // get number of entries in model
    int counts( model_.rowCount() );

    // get selected files
    SessionFilesModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    bool hasSelection( !selection.empty() );

    openAction_->setEnabled( hasSelection );
    closeAction_->setEnabled( hasSelection );
    saveAction_->setEnabled( std::find_if( selection.begin(), selection.end(), FileRecord::HasFlagFTor( FileRecordProperties::MODIFIED ) ) != selection.end() );

    previousFileAction_->setEnabled( counts >= 2 && hasSelection );
    nextFileAction_->setEnabled( counts >= 2 && hasSelection );

}

//______________________________________________________
void SessionFilesFrame::_showToolTip( const QModelIndex& index )
{

    if( !index.isValid() ) toolTipWidget_->hide();
    else {

        // fileInfo
        const FileRecord record( model_.get( index ) );

        // icon
        QIcon icon;
        // QVariant iconVariant( model_.data( index, Qt::DecorationRole ) );
        // if( iconVariant.canConvert( QVariant::Icon ) ) icon = iconVariant.value<QIcon>();
        const int iconProperty = FileRecord::PropertyId::get( FileRecordProperties::ICON );
        if( record.hasProperty( iconProperty ) )
        { icon = IconEngine::get( record.property( iconProperty ) ); }

        // rect
        QRect rect( list_->visualRect( index ) );
        rect.translate( list_->viewport()->mapToGlobal( QPoint( 0, 0 ) ) );
        toolTipWidget_->setIndexRect( rect );

        // assign to tooltip widget
        toolTipWidget_->setRecord( record, icon );

        // show
        toolTipWidget_->showDelayed();

    }

}

//______________________________________________________________________
void SessionFilesFrame::_selectPreviousFile( void )
{

    Debug::Throw( "SessionFilesFrame:_selectPreviousFile.\n" );

    // check counts
    int counts( model_.rowCount() );
    if( counts < 2 ) return;

    // get current index
    QModelIndex current_index( list_->selectionModel()->currentIndex() );
    if( !current_index.isValid() ) return;

    // get previous
    QModelIndex index( current_index );
    while( (index = model_.index( index.row()-1, current_index.column())).isValid() )
    {
        FileRecord record( model_.get( index ) );
        if( record.hasFlag( FileRecordProperties::ACTIVE ) ) break;
    }

    if( !index.isValid() ) index = model_.index( counts-1, current_index.column() );
    if( (!index.isValid()) || index == current_index ) return;

    // select new index
    list_->selectionModel()->setCurrentIndex( index,  QItemSelectionModel::Current|QItemSelectionModel::Rows );

}


//______________________________________________________________________
void SessionFilesFrame::_selectNextFile( void )
{

    Debug::Throw( "SessionFilesFrame:_selectNextFile.\n" );

    // check counts
    int counts( model_.rowCount() );
    if( counts < 2 ) return;

    // get current index
    QModelIndex current_index( list_->selectionModel()->currentIndex() );
    if( !current_index.isValid() ) return;

    // get previous
    QModelIndex index( current_index );
    while( (index = model_.index( index.row()+1, current_index.column())).isValid() )
    {
        FileRecord record( model_.get( index ) );
        if( record.hasFlag( FileRecordProperties::ACTIVE ) ) break;
    }

    if( !index.isValid() ) index = model_.index( 0, current_index.column() );
    if( (!index.isValid()) || index == current_index ) return;

    // select new index
    list_->selectionModel()->setCurrentIndex( index,  QItemSelectionModel::Current|QItemSelectionModel::Rows );

}

//______________________________________________________________________
void SessionFilesFrame::_open( void )
{

    Debug::Throw( "SessionFilesFrame:_open.\n" );
    foreach( const FileRecord& record, model_.get( list_->selectionModel()->selectedRows() ) )
    { emit fileActivated( record ); }

}

//______________________________________________________________________
void SessionFilesFrame::_save( void )
{

    Debug::Throw( "SessionFilesFrame:_save.\n" );

    FileRecord::List modifiedRecords;
    foreach( const FileRecord& record, model_.get( list_->selectionModel()->selectedRows() ) )
    { if( record.hasFlag( FileRecordProperties::MODIFIED ) ) modifiedRecords << record; }

    if( !modifiedRecords.empty() ) emit filesSaved( modifiedRecords );

}


//______________________________________________________________________
void SessionFilesFrame::_close( void )
{

    Debug::Throw( "SessionFilesFrame:_close.\n" );
    SessionFilesModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    if( !selection.empty() )
    {
      FileRecord::List records;
      foreach( const FileRecord& record, selection )
      { records << record; }
      emit filesClosed( records );
    }
}

//______________________________________________________________________
void SessionFilesFrame::_itemSelected( const QModelIndex& index )
{ if( index.isValid() ) emit fileSelected( model_.get( index ) ); }

//______________________________________________________________________
void SessionFilesFrame::_itemActivated( const QModelIndex& index )
{

    Debug::Throw( "SessionFilesFrame::_itemActivated.\n" );
    if( !index.isValid() ) return;
    emit fileActivated( model_.get( index ) );

}

//______________________________________________________________________
void SessionFilesFrame::_installActions( void )
{

    Debug::Throw( "SessionFilesFrame::_installActions.\n" );

    // next file
    addAction( nextFileAction_ = new QAction( IconEngine::get(  ICONS::DOWN ), "Select next File", this ) );
    connect( nextFileAction_, SIGNAL( triggered() ), SLOT( _selectNextFile() ) );
    nextFileAction_->setShortcut( Qt::CTRL + Qt::Key_Tab );

    // previous file
    addAction( previousFileAction_ = new QAction( IconEngine::get(  ICONS::UP ), "Select Previous File", this ) );
    connect( previousFileAction_, SIGNAL( triggered() ), SLOT( _selectPreviousFile() ) );
    previousFileAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_Tab );

    // open
    addAction( openAction_ = new QAction( IconEngine::get( ICONS::OPEN ), "Open", this ) );
    connect( openAction_, SIGNAL( triggered() ), SLOT( _open() ) );
    openAction_->setToolTip( "Open selected files" );

    // save
    addAction( saveAction_ = new QAction( IconEngine::get( ICONS::SAVE ), "Save", this ) );
    connect( saveAction_, SIGNAL( triggered() ), SLOT( _save() ) );
    saveAction_->setToolTip( "Save selected files" );

    // close
    addAction( closeAction_ = new QAction( IconEngine::get( ICONS::DIALOG_CLOSE ), "Close", this ) );
    connect( closeAction_, SIGNAL( triggered() ), SLOT( _close() ) );
    closeAction_->setShortcut( QKeySequence::Delete );
    closeAction_->setToolTip( "Close selected files" );

}
