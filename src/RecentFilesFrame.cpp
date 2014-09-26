
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
#include "RecentFilesFrame.moc"

#include "AnimatedTreeView.h"
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
#include "Util.h"
#include "XmlOptions.h"

#include <QButtonGroup>
#include <QHeaderView>
#include <QLayout>

//_______________________________________________________________
RecentFilesFrame::RecentFilesFrame( QWidget* parent, FileList& files ):
    QWidget( parent ),
    Counter( "RecentFilesFrame" ),
    actionsLocked_( false ),
    recentFiles_( &files )
{

    Debug::Throw( "RecentFilesFrame:RecentFilesFrame.\n" );

    // layout
    setLayout( new QVBoxLayout() );
    layout()->setMargin(0);
    layout()->setSpacing(2);

    // tooltip widget
    toolTipWidget_ = new FileRecordToolTipWidget( this );

    // list
    layout()->addWidget( list_ = new AnimatedTreeView( this ) );
    list_->setMouseTracking( true );
    list_->setModel( &model_ );
    list_->setSelectionMode( QAbstractItemView::ContiguousSelection );
    list_->setOptionName( "RECENT_FILES_LIST" );
    list_->header()->hide();

    // actions
    _installActions();

    // add actions to list
    QMenu* menu( new ContextMenu( list_ ) );
    menu->addMenu( new ColumnSortingMenu( menu, list_ ) );
    menu->addMenu( new ColumnSelectionMenu( menu, list_ ) );
    menu->addAction( &_openAction() );
    menu->addSeparator();
    menu->addAction( &_cleanAction() );

    // connections
    connect( &model_, SIGNAL(layoutChanged()), list_, SLOT(updateMask()) );
    connect( list_, SIGNAL(customContextMenuRequested(QPoint)), SLOT(_updateActions()) );
    connect( list_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(_itemSelected(QModelIndex)) );
    connect( list_, SIGNAL(activated(QModelIndex)), SLOT(_itemActivated(QModelIndex)) );
    connect( list_, SIGNAL(hovered(QModelIndex)), SLOT(_showToolTip(QModelIndex)) );

    connect( recentFiles_, SIGNAL(validFilesChecked()), SLOT(update()) );
    connect( recentFiles_, SIGNAL(contentsChanged()), SLOT(update()) );

}

//______________________________________________________________________
RecentFilesFrame::~RecentFilesFrame( void )
{ Debug::Throw( "RecentFilesFrame::~RecentFilesFrame.\n" ); }

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
void RecentFilesFrame::update( void )
{

    Debug::Throw( "RecentFilesFrame:update.\n" );

    // lock signal emission
    actionsLocked_ = true;

    model_.update( recentFiles_->records() );
    list_->updateMask();
    list_->resizeColumns();
    list_->update();

    // clean action enability
    _cleanAction().setEnabled( recentFiles_->cleanEnabled() );

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
void RecentFilesFrame::_updateActions( void )
{
    Debug::Throw( "RecentFilesFrame:_updateActions.\n" );
    FileRecordModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );

    bool has_validSelection( std::find_if( selection.begin(), selection.end(), FileRecord::ValidFTor() ) != selection.end() );
    _openAction().setEnabled( has_validSelection );

}

//______________________________________________________
void RecentFilesFrame::_showToolTip( const QModelIndex& index )
{

    if( !index.isValid() ) toolTipWidget_->hide();
    else {

        // fileInfo
        const FileRecord record( model_.get( index ) );

        // icon
        QIcon icon;
        QVariant iconVariant( model_.data( index, Qt::DecorationRole ) );
        if( iconVariant.canConvert( QVariant::Icon ) ) icon = iconVariant.value<QIcon>();

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
void RecentFilesFrame::_clean( void )
{

    Debug::Throw( "RecentFilesFrame:_clean.\n" );
    if( !QuestionDialog( window(), tr( "Remove invalid or duplicated files from list ?" ) ).exec() ) return;
    recentFiles_->clean();
    update();

}

//______________________________________________________________________
void RecentFilesFrame::_open( void )
{

    Debug::Throw( "RecentFilesFrame:_open.\n" );
    FileRecordModel::List validSelection;

    foreach( const FileRecord& record, model_.get( list_->selectionModel()->selectedRows() ) )
    { if( record.isValid() ) validSelection << record; }

    // one should check the number of files to be edited
    foreach( const FileRecord& record, validSelection )
    { emit fileActivated( record ); }

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
void RecentFilesFrame::_installActions( void )
{

    Debug::Throw( "RecentFilesFrame::_installActions.\n" );

    // clean
    addAction( cleanAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Clean" ), this ) );
    connect( &_cleanAction(), SIGNAL(triggered()), SLOT(_clean()) );
    _cleanAction().setEnabled( false );
    _cleanAction().setToolTip( tr( "Clean invalid files" ) );

    // open
    addAction( openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open Selected Files" ), this ) );
    connect( &_openAction(), SIGNAL(triggered()), SLOT(_open()) );

}
