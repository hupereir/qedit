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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "SessionFilesWidget.h"

#include "Application.h"
#include "ColumnSortingMenu.h"
#include "ColumnSelectionMenu.h"
#include "ContextMenu.h"
#include "Debug.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "FileRecordToolTipWidget.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "MimeTypeIconProvider.h"
#include "SessionFilesView.h"
#include "Singleton.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlOptions.h"

#include <QButtonGroup>
#include <QHeaderView>
#include <QLayout>

//_______________________________________________________________
SessionFilesWidget::SessionFilesWidget( QWidget* parent ):
QWidget( parent ),
Counter( "SessionFilesWidget" )
{

    Debug::Throw( "SessionFilesWidget:SessionFilesWidget.\n" );

    // layout
    setLayout( new QVBoxLayout );
    layout()->setMargin(0);
    layout()->setSpacing(2);

    // tooltip widget and icon provider
    toolTipWidget_ = new FileRecordToolTipWidget( this );
    mimeTypeIconProvider_ = new MimeTypeIconProvider( this );

    // list
    layout()->addWidget( list_ = new SessionFilesView( this ) );
    list_->setMouseTracking( true );
    list_->setModel( &model_ );
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
    auto menu = new ContextMenu( list_ );
    menu->addMenu( new ColumnSortingMenu( menu, list_ ) );
    menu->addMenu( new ColumnSelectionMenu( menu, list_ ) );
    menu->addSeparator();
    menu->addAction( openAction_ );
    menu->addAction( saveAction_ );
    menu->addAction( &Base::Singleton::get().application<Application>()->windowServer().saveAllAction() );
    menu->addAction( closeAction_ );
    menu->addSeparator();
    menu->addAction( previousFileAction_ );
    menu->addAction( nextFileAction_ );

    // connections
    connect( &model_, SIGNAL(layoutChanged()), list_, SLOT(updateMask()) );
    connect( &model_, SIGNAL(requestOpen(FileRecord)), SIGNAL(fileActivated(FileRecord)) );
    connect( list_, SIGNAL(customContextMenuRequested(QPoint)), SLOT(_updateActions()) );
    connect( list_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(_itemSelected(QModelIndex)) );
    connect( list_, SIGNAL(activated(QModelIndex)), SLOT(_itemActivated(QModelIndex)) );
    connect( list_, SIGNAL(hovered(QModelIndex)), SLOT(_showToolTip(QModelIndex)) );

}

//____________________________________________
void SessionFilesWidget::select( const File& file )
{

    Debug::Throw() << "SessionFilesWidget::select - file: " << file << ".\n";

    // find model index that match the file
    auto index( model_.index( FileRecord( file ) ) );

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
void SessionFilesWidget::update()
{
    Debug::Throw( "SessionFilesWidget:update.\n" );

    // store in model
    auto records( Base::Singleton::get().application<Application>()->windowServer().records( WindowServer::None, window() ) );
    model_.update( records );

    list_->updateMask();
    list_->resizeColumns();

    // make sure selected record appear selected in list
    auto iter = std::find_if( records.begin(), records.end(), FileRecord::HasFlagFTor( FileRecordProperties::Selected ) );
    if( iter != records.end() ) select( iter->file() );

    Debug::Throw( "SessionFilesWidget:update - done.\n" );

}

//______________________________________________________________________
void SessionFilesWidget::_updateActions()
{

    Debug::Throw( "SessionFilesWidget:_updateActions.\n" );

    // get number of entries in model
    int counts( model_.rowCount() );

    // get selected files
    const auto selection( model_.get( list_->selectionModel()->selectedRows() ) );
    bool hasSelection( !selection.empty() );

    openAction_->setEnabled( hasSelection );
    closeAction_->setEnabled( hasSelection );
    saveAction_->setEnabled( std::any_of( selection.begin(), selection.end(), FileRecord::HasFlagFTor( FileRecordProperties::Modified ) ) );

    previousFileAction_->setEnabled( counts >= 2 && hasSelection );
    nextFileAction_->setEnabled( counts >= 2 && hasSelection );

}

//______________________________________________________
void SessionFilesWidget::_showToolTip( const QModelIndex& index )
{

    if( !( index.isValid() && window()->isActiveWindow() ) ) toolTipWidget_->hide();
    else {

        // fileInfo
        const auto record( model_.get( index ) );

        // icon
        auto icon = mimeTypeIconProvider_->icon( record.file().extension() );
        if( icon.isNull() )
        {
            const int iconProperty = FileRecord::PropertyId::get( FileRecordProperties::Icon );
            if( record.hasProperty( iconProperty ) )
            { icon = IconEngine::get( record.property( iconProperty ) ); }
        }

        // rect
        auto rect = list_->visualRect( index );
        rect.translate( list_->viewport()->mapToGlobal( QPoint( 0, 0 ) ) );
        toolTipWidget_->setIndexRect( rect );

        // assign to tooltip widget
        toolTipWidget_->setRecord( record, icon );

        // show
        toolTipWidget_->showDelayed();

    }

}

//______________________________________________________________________
void SessionFilesWidget::_selectPreviousFile()
{

    Debug::Throw( "SessionFilesWidget:_selectPreviousFile.\n" );

    // check counts
    int counts( model_.rowCount() );
    if( counts < 2 ) return;

    // get current index
    const auto currentIndex( list_->selectionModel()->currentIndex() );
    if( !currentIndex.isValid() ) return;

    // get previous
    auto index( currentIndex );
    while( (index = model_.index( index.row()-1, currentIndex.column())).isValid() )
    {
        const auto record( model_.get( index ) );
        if( record.hasFlag( FileRecordProperties::Active ) ) break;
    }

    if( !index.isValid() ) index = model_.index( counts-1, currentIndex.column() );
    if( (!index.isValid()) || index == currentIndex ) return;

    // select new index
    list_->selectionModel()->setCurrentIndex( index,  QItemSelectionModel::Current|QItemSelectionModel::Rows );

}


//______________________________________________________________________
void SessionFilesWidget::_selectNextFile()
{

    Debug::Throw( "SessionFilesWidget:_selectNextFile.\n" );

    // check counts
    int counts( model_.rowCount() );
    if( counts < 2 ) return;

    // get current index
    const auto currentIndex( list_->selectionModel()->currentIndex() );
    if( !currentIndex.isValid() ) return;

    // get next
    auto index( currentIndex );
    while( (index = model_.index( index.row()+1, currentIndex.column())).isValid() )
    {
        const auto record( model_.get( index ) );
        if( record.hasFlag( FileRecordProperties::Active ) ) break;
    }

    if( !index.isValid() ) index = model_.index( 0, currentIndex.column() );
    if( (!index.isValid()) || index == currentIndex ) return;

    // select new index
    list_->selectionModel()->setCurrentIndex( index,  QItemSelectionModel::Current|QItemSelectionModel::Rows );

}

//______________________________________________________________________
void SessionFilesWidget::_open()
{

    Debug::Throw( "SessionFilesWidget:_open.\n" );
    for( const auto& record:model_.get( list_->selectionModel()->selectedRows() ) )
    { emit fileActivated( record ); }

}

//______________________________________________________________________
void SessionFilesWidget::_save()
{

    Debug::Throw( "SessionFilesWidget:_save.\n" );

    FileRecord::List modifiedRecords;
    for( const auto& record:model_.get( list_->selectionModel()->selectedRows() ) )
    { if( record.hasFlag( FileRecordProperties::Modified ) ) modifiedRecords.append( record ); }

    if( !modifiedRecords.empty() ) emit filesSaved( modifiedRecords );

}


//______________________________________________________________________
void SessionFilesWidget::_close()
{
    const auto selection( model_.get( list_->selectionModel()->selectedRows() ) );
    if( !selection.empty() ) emit filesClosed( selection );
}

//______________________________________________________________________
void SessionFilesWidget::_itemSelected( const QModelIndex& index )
{ if( index.isValid() ) emit fileSelected( model_.get( index ) ); }

//______________________________________________________________________
void SessionFilesWidget::_itemActivated( const QModelIndex& index )
{

    Debug::Throw( "SessionFilesWidget::_itemActivated.\n" );
    if( !index.isValid() ) return;
    emit fileActivated( model_.get( index ) );

}

//______________________________________________________________________
void SessionFilesWidget::_installActions()
{

    Debug::Throw( "SessionFilesWidget::_installActions.\n" );

    // next file
    addAction( nextFileAction_ = new QAction( IconEngine::get(  IconNames::Down ), tr( "Select next File" ), this ) );
    connect( nextFileAction_, SIGNAL(triggered()), SLOT(_selectNextFile()) );
    nextFileAction_->setShortcut( Qt::CTRL + Qt::Key_Tab );

    // previous file
    addAction( previousFileAction_ = new QAction( IconEngine::get(  IconNames::Up ), tr( "Select Previous File" ), this ) );
    connect( previousFileAction_, SIGNAL(triggered()), SLOT(_selectPreviousFile()) );
    previousFileAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_Tab );

    // open
    addAction( openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open" ), this ) );
    connect( openAction_, SIGNAL(triggered()), SLOT(_open()) );
    openAction_->setToolTip( tr( "Open selected files" ) );

    // save
    addAction( saveAction_ = new QAction( IconEngine::get( IconNames::Save ), tr( "Save" ), this ) );
    connect( saveAction_, SIGNAL(triggered()), SLOT(_save()) );
    saveAction_->setToolTip( tr( "Save selected files" ) );

    // close
    addAction( closeAction_ = new QAction( IconEngine::get( IconNames::DialogClose ), tr( "Close" ), this ) );
    connect( closeAction_, SIGNAL(triggered()), SLOT(_close()) );
    closeAction_->setShortcut( QKeySequence::Delete );
    closeAction_->setToolTip( tr( "Close selected files" ) );

}