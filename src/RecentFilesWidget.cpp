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

#include "RecentFilesWidget.h"

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
RecentFilesWidget::RecentFilesWidget( QWidget* parent, FileList& files ):
    QWidget( parent ),
    Counter( "RecentFilesWidget" ),
    recentFiles_( &files )
{

    Debug::Throw( "RecentFilesWidget:RecentFilesWidget.\n" );

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
    connect( &model_, &QAbstractItemModel::layoutChanged, list_, &TreeView::updateMask );
    connect( list_, &QWidget::customContextMenuRequested, this, &RecentFilesWidget::_updateActions );
    connect( list_->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &RecentFilesWidget::_itemSelected );
    connect( list_, &QAbstractItemView::activated, this, &RecentFilesWidget::_itemActivated );
    connect( list_, &TreeView::hovered, this, &RecentFilesWidget::_showToolTip );

    connect( recentFiles_, SIGNAL(validFilesChecked()), SLOT(update()) );
    connect( recentFiles_, SIGNAL(contentsChanged()), SLOT(update()) );

}

//____________________________________________
void RecentFilesWidget::select( const File& file )
{

    Debug::Throw() << "RecentFilesWidget::select - file: " << file << ".\n";

    // find model index that match the file
    const auto index( model_.index( FileRecord( file ) ) );

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
void RecentFilesWidget::update()
{

    Debug::Throw( "RecentFilesWidget:update.\n" );

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
void RecentFilesWidget::enterEvent( QEvent* e )
{

    Debug::Throw( "RecentFilesWidget::enterEvent.\n" );
    QWidget::enterEvent( e );

    // check recent files validity
    recentFiles_->checkValidFiles();

}

//______________________________________________________________________
void RecentFilesWidget::_updateActions()
{
    Debug::Throw( "RecentFilesWidget:_updateActions.\n" );
    FileRecordModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );

    bool hasValidSelection( std::any_of( selection.begin(), selection.end(), FileRecord::ValidFTor() ) );
    openAction_->setEnabled( hasValidSelection );

}

//______________________________________________________
void RecentFilesWidget::_showToolTip( const QModelIndex& index )
{

    if( !index.isValid() ) toolTipWidget_->hide();
    else {

        // update tooltip content
        const auto record( model_.get( index ) );
        QIcon icon;
        const auto iconVariant( model_.data( index, Qt::DecorationRole ) );
        if( iconVariant.canConvert( QVariant::Icon ) ) icon = iconVariant.value<QIcon>();
        toolTipWidget_->setRecord( record, icon );

        // rect
        toolTipWidget_->setIndexRect(
            list_->visualRect( index ).
            translated( list_->viewport()->mapToGlobal( QPoint( 0, 0 ) ) ) );

        // show
        toolTipWidget_->showDelayed();

    }

}

//______________________________________________________________________
void RecentFilesWidget::_clean()
{

    Debug::Throw( "RecentFilesWidget:_clean.\n" );
    if( !QuestionDialog( window(), tr( "Remove invalid or duplicated files from list ?" ) ).exec() ) return;
    recentFiles_->clean();
    update();

}

//______________________________________________________________________
void RecentFilesWidget::_open()
{

    Debug::Throw( "RecentFilesWidget:_open.\n" );
    FileRecordModel::List validSelection;

    for( const auto& record:model_.get( list_->selectionModel()->selectedRows() ) )
    { if( record.isValid() ) emit fileActivated( record ); }

}

//______________________________________________________________________
void RecentFilesWidget::_itemSelected( const QModelIndex& index )
{
    Debug::Throw( "RecentFilesWidget::_itemSelected.\n" );
    if( !actionsLocked_ && index.isValid() )
    {  emit fileSelected( model_.get( index ) ); }
}

//______________________________________________________________________
void RecentFilesWidget::_itemActivated( const QModelIndex& index )
{
    Debug::Throw( "RecentFilesWidget::_itemActivated.\n" );
    if( !actionsLocked_ && index.isValid() )
    { emit fileActivated( model_.get( index ) ); }
}

//______________________________________________________________________
void RecentFilesWidget::_installActions()
{

    Debug::Throw( "RecentFilesWidget::_installActions.\n" );

    // clean
    addAction( cleanAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Clean" ), this ) );
    connect( cleanAction_, &QAction::triggered, this, &RecentFilesWidget::_clean );
    cleanAction_->setEnabled( false );
    cleanAction_->setToolTip( tr( "Clean invalid files" ) );

    // open
    addAction( openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open Selected Files" ), this ) );
    connect( openAction_, &QAction::triggered, this, &RecentFilesWidget::_open );

}
