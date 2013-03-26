
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

#include "RecentFilesFrame.h"

#include "AnimatedTreeView.h"
#include "ColumnSortingMenu.h"
#include "ColumnSelectionMenu.h"
#include "ContextMenu.h"
#include "Debug.h"
#include "Icons.h"
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
    list_->setModel( &_model() );
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
    connect( &_model(), SIGNAL( layoutChanged() ), list_, SLOT( updateMask() ) );
    connect( list_, SIGNAL( customContextMenuRequested( const QPoint& ) ), SLOT( _updateActions( void ) ) );
    connect( list_->selectionModel(), SIGNAL( currentRowChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _itemSelected( const QModelIndex& ) ) );
    connect( list_, SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemActivated( const QModelIndex& ) ) );
    connect( list_, SIGNAL( hovered( const QModelIndex& ) ), SLOT( _showToolTip( const QModelIndex& ) ) );

    connect( &_recentFiles(), SIGNAL( validFilesChecked( void ) ), SLOT( update( void ) ) );
    connect( &_recentFiles(), SIGNAL( contentsChanged( void ) ), SLOT( update( void ) ) );

}

//______________________________________________________________________
RecentFilesFrame::~RecentFilesFrame( void )
{ Debug::Throw( "RecentFilesFrame::~RecentFilesFrame.\n" ); }

//____________________________________________
void RecentFilesFrame::select( const File& file )
{

    Debug::Throw() << "RecentFilesFrame::select - file: " << file << ".\n";

    // find model index that match the file
    QModelIndex index( _model().index( FileRecord( file ) ) );

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

    // update records
    FileRecordModel::List records;
    foreach( const FileRecord& record, _recentFiles().records() )
    { records << record; }
    _model().update( records );

    list_->updateMask();
    list_->resizeColumns();
    list_->update();

    // clean action enability
    _cleanAction().setEnabled( _recentFiles().cleanEnabled() );
    Debug::Throw( "RecentFilesFrame:update - done.\n" );

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
void RecentFilesFrame::_updateActions( void )
{
    Debug::Throw( "RecentFilesFrame:_updateActions.\n" );
    FileRecordModel::List selection( _model().get( list_->selectionModel()->selectedRows() ) );

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
    _recentFiles().clean();
    update();

}

//______________________________________________________________________
void RecentFilesFrame::_open( void )
{

    Debug::Throw( "RecentFilesFrame:_open.\n" );
    FileRecordModel::List validSelection;

    foreach( const FileRecord& record, _model().get( list_->selectionModel()->selectedRows() ) )
    { if( record.isValid() ) validSelection << record; }

    // one should check the number of files to be edited
    foreach( const FileRecord& record, validSelection )
    { emit fileActivated( record ); }

}

//______________________________________________________________________
void RecentFilesFrame::_itemSelected( const QModelIndex& index )
{

    Debug::Throw( "RecentFilesFrame::_itemSelected.\n" );
    if( !index.isValid() ) return;
    emit fileSelected( model_.get( index ) );

}

//______________________________________________________________________
void RecentFilesFrame::_itemActivated( const QModelIndex& index )
{
    Debug::Throw( "RecentFilesFrame::_itemActivated.\n" );
    if( !index.isValid() ) return;
    emit fileActivated( _model().get( index ) );

}

//______________________________________________________________________
void RecentFilesFrame::_installActions( void )
{

    Debug::Throw( "RecentFilesFrame::_installActions.\n" );

    // clean
    addAction( cleanAction_ = new QAction( IconEngine::get( ICONS::DELETE ), tr( "Clean" ), this ) );
    connect( &_cleanAction(), SIGNAL( triggered() ), SLOT( _clean() ) );
    _cleanAction().setEnabled( false );
    _cleanAction().setToolTip( tr( "Clean invalid files" ) );

    // open
    addAction( openAction_ = new QAction( IconEngine::get( ICONS::OPEN ), tr( "Open Selected Files" ), this ) );
    connect( &_openAction(), SIGNAL( triggered() ), SLOT( _open() ) );
    _openAction().setToolTip( tr( "Show selected files" ) );

}
