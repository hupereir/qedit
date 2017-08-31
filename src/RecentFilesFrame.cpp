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

#include "RecentFilesFrame.h"

#include "ColumnSortingMenu.h"
#include "ColumnSelectionMenu.h"
#include "ContextMenu.h"
#include "Debug.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "FileList.h"
#include "FileRecordToolTipWidget.h"
#include "QuestionDialog.h"
#include "QtUtil.h"
#include "TreeView.h"
#include "Util.h"
#include "XmlOptions.h"

#include <QButtonGroup>
#include <QHeaderView>
#include <QLayout>

//_______________________________________________________________
RecentFilesFrame::RecentFilesFrame( QWidget* parent, FileList& files ):
    QWidget( parent ),
    Counter( "RecentFilesFrame" ),
    recentFiles_( &files )
{

    Debug::Throw( "RecentFilesFrame:RecentFilesFrame.\n" );

    // layout
    setLayout( new QVBoxLayout );
    layout()->setMargin(0);
    layout()->setSpacing(2);

    // tooltip widget
    toolTipWidget_ = new FileRecordToolTipWidget( this );

    // list
    layout()->addWidget( list_ = new TreeView( this ) );
    list_->setMouseTracking( true );
    list_->setDragEnabled( true );
    list_->setModel( &model_ );
    list_->setSelectionMode( QAbstractItemView::ContiguousSelection );
    list_->setOptionName( "RECENT_FILES" );
    list_->header()->hide();

    // actions
    _installActions();

    // add actions to list
    auto menu = new ContextMenu( list_ );
    menu->addMenu( new ColumnSortingMenu( menu, list_ ) );
    menu->addMenu( new ColumnSelectionMenu( menu, list_ ) );
    menu->addAction( openAction_ );
    menu->addSeparator();
    menu->addAction( cleanAction_ );

    // connections
    connect( &model_, SIGNAL(layoutChanged()), list_, SLOT(updateMask()) );
    connect( list_, SIGNAL(customContextMenuRequested(QPoint)), SLOT(_updateActions()) );
    connect( list_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(_itemSelected(QModelIndex)) );
    connect( list_, SIGNAL(activated(QModelIndex)), SLOT(_itemActivated(QModelIndex)) );
    connect( list_, SIGNAL(hovered(QModelIndex)), SLOT(_showToolTip(QModelIndex)) );

    connect( recentFiles_, SIGNAL(validFilesChecked()), SLOT(update()) );
    connect( recentFiles_, SIGNAL(contentsChanged()), SLOT(update()) );

}

//____________________________________________
void RecentFilesFrame::select( const File& file )
{

    Debug::Throw() << "RecentFilesFrame::select - file: " << file << ".\n";

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
void RecentFilesFrame::update()
{

    Debug::Throw( "RecentFilesFrame:update.\n" );

    // lock signal emission
    actionsLocked_ = true;

    model_.update( recentFiles_->records() );
    list_->updateMask();
    list_->resizeColumns();
    list_->update();

    // clean action enability
    cleanAction_->setEnabled( recentFiles_->cleanEnabled() );

    // unlock
    actionsLocked_ = false;

}

//____________________________________________
void RecentFilesFrame::enterEvent( QEvent* e )
{

    Debug::Throw( "RecentFilesFrame::enterEvent.\n" );
    QWidget::enterEvent( e );

    // check recent files validity
    recentFiles_->checkValidFiles();

}

//______________________________________________________________________
void RecentFilesFrame::_updateActions()
{
    Debug::Throw( "RecentFilesFrame:_updateActions.\n" );
    FileRecordModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );

    bool hasValidSelection( std::any_of( selection.begin(), selection.end(), FileRecord::ValidFTor() ) );
    openAction_->setEnabled( hasValidSelection );

}

//______________________________________________________
void RecentFilesFrame::_showToolTip( const QModelIndex& index )
{

    if( !index.isValid() ) toolTipWidget_->hide();
    else {

        // fileInfo
        const auto record( model_.get( index ) );

        // icon
        QIcon icon;
        QVariant iconVariant( model_.data( index, Qt::DecorationRole ) );
        if( iconVariant.canConvert( QVariant::Icon ) ) icon = iconVariant.value<QIcon>();

        // rect
        const auto rect = list_->visualRect( index )
            .translated( list_->viewport()->mapToGlobal( QPoint( 0, 0 ) ) );
        toolTipWidget_->setIndexRect( rect );

        // assign to tooltip widget
        toolTipWidget_->setRecord( record, icon );

        // show
        toolTipWidget_->showDelayed();

    }

}

//______________________________________________________________________
void RecentFilesFrame::_clean()
{

    Debug::Throw( "RecentFilesFrame:_clean.\n" );
    if( !QuestionDialog( window(), tr( "Remove invalid or duplicated files from list ?" ) ).exec() ) return;
    recentFiles_->clean();
    update();

}

//______________________________________________________________________
void RecentFilesFrame::_open()
{

    Debug::Throw( "RecentFilesFrame:_open.\n" );
    FileRecordModel::List validSelection;

    for( const auto& record:model_.get( list_->selectionModel()->selectedRows() ) )
    { if( record.isValid() ) emit fileActivated( record ); }

}

//______________________________________________________________________
void RecentFilesFrame::_itemSelected( const QModelIndex& index )
{
    Debug::Throw( "RecentFilesFrame::_itemSelected.\n" );
    if( !actionsLocked_ && index.isValid() )
    {  emit fileSelected( model_.get( index ) ); }
}

//______________________________________________________________________
void RecentFilesFrame::_itemActivated( const QModelIndex& index )
{
    Debug::Throw( "RecentFilesFrame::_itemActivated.\n" );
    if( !actionsLocked_ && index.isValid() )
    { emit fileActivated( model_.get( index ) ); }
}

//______________________________________________________________________
void RecentFilesFrame::_installActions()
{

    Debug::Throw( "RecentFilesFrame::_installActions.\n" );

    // clean
    addAction( cleanAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Clean" ), this ) );
    connect( cleanAction_, SIGNAL(triggered()), SLOT(_clean()) );
    cleanAction_->setEnabled( false );
    cleanAction_->setToolTip( tr( "Clean invalid files" ) );

    // open
    addAction( openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open Selected Files" ), this ) );
    connect( openAction_, SIGNAL(triggered()), SLOT(_open()) );

}
