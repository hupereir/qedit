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

#include "FileSystemFrame.h"

#include "BaseFileInfo.h"
#include "BaseFileInfoItemDelegate.h"
#include "ColumnSortingMenu.h"
#include "ColumnSelectionMenu.h"
#include "ContextMenu.h"
#include "CustomToolBar.h"
#include "FileInformationDialog.h"
#include "FileRecordProperties.h"
#include "FileRecordToolTipWidget.h"
#include "FileSystemIconNames.h"
#include "IconEngine.h"
#include "PathEditor.h"
#include "RemoveFilesDialog.h"
#include "RenameFileDialog.h"
#include "Singleton.h"
#include "TreeView.h"
#include "Util.h"
#include "XmlOptions.h"

#include <QApplication>
#include <QHeaderView>
#include <QLayout>
#include <QDir>

//_____________________________________________
FileSystemFrame::FileSystemFrame( QWidget *parent ):
    QWidget( parent ),
    Counter( "FileSystemFrame" ),
    sizePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::Size ) ),
    showNavigator_( false ),
    homePath_( Util::home() ),
    fileSystemWatcher_( this ),
    thread_( this )
{

    Debug::Throw( "FileSystemFrame::FileSystemFrame.\n" );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setMargin(0);
    setLayout( layout );

    // toolbar
    CustomToolBar* toolbar = new CustomToolBar( tr( "Navigation Toolbar" ), this, "NAVIGATION_TOOLBAR" );
    layout->addWidget( toolbar );

    // path editor
    layout->addWidget( pathEditor_ = new PathEditor( this ) );
    pathEditor_->setHomePath( Util::home() );

    // tooltip widget
    toolTipWidget_ = new FileRecordToolTipWidget( this );

    // initialize local root path list
    File::List rootPathList;
    for( const auto& fileInfo:QDir::drives() )
    { rootPathList << fileInfo.path(); }
    pathEditor_->setRootPathList( rootPathList );

    connect( pathEditor_, SIGNAL(pathChanged(File)), SLOT(_update()) );
    connect( pathEditor_, SIGNAL(pathChanged(File)), SLOT(_updateNavigationActions()) );

    // install actions
    _installActions();
    toolbar->addAction( parentDirectoryAction_ );
    toolbar->addAction( previousDirectoryAction_ );
    toolbar->addAction( nextDirectoryAction_ );
    toolbar->addAction( homeDirectoryAction_ );
    toolbar->addAction( workingDirectoryAction_ );
    toolbar->addAction( reloadAction_ );

    // file list
    layout->addWidget( list_ = new TreeView( this ), 1);
    if( list_->itemDelegateForColumn(0) ) list_->itemDelegateForColumn(0)->deleteLater();
    list_->setItemDelegateForColumn(0, new BaseFileInfoItemDelegate( list_ ) );

    list_->setMouseTracking( true );
    list_->setDragEnabled( true );
    list_->setModel( &model_ );
    list_->setSelectionMode( QAbstractItemView::ContiguousSelection );
    list_->setOptionName( "FILE_SYSTEM_LIST" );
    list_->header()->hide();

    // list menu
    // should move to proper _updateMenu method, and only show relevant actions
    // also, should use baseFileInfo model
    QMenu* menu( new ContextMenu( list_ ) );
    menu->addMenu( new ColumnSortingMenu( menu, list_ ) );
    menu->addMenu( new ColumnSelectionMenu( menu, list_ ) );
    menu->addSeparator();
    menu->addAction( hiddenFilesAction_ );
    menu->addSeparator();
    menu->addAction( openAction_ );
    menu->addAction( renameAction_ );
    menu->addAction( removeAction_ );
    menu->addSeparator();
    menu->addAction( filePropertiesAction_ );

    // connections
    connect( &model_, SIGNAL(layoutChanged()), list_, SLOT(updateMask()) );
    connect( list_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(_updateActions()) );
    connect( list_->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(_updateActions()) );
    connect( list_, SIGNAL(activated(QModelIndex)), SLOT(_itemActivated(QModelIndex)) );
    connect( list_, SIGNAL(hovered(QModelIndex)), SLOT(_showToolTip(QModelIndex)) );

    // connect filesystem watcher
    connect( &fileSystemWatcher_, SIGNAL(directoryChanged(QString)), SLOT(_update(QString)) );

    // connect thread
    connect( &thread_, SIGNAL(filesAvailable(const File::List&)), SLOT(_processFiles(const File::List&)) );

    connect( Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    _updateConfiguration();
    _updateNavigationActions();
    _updateActions();

}

//_________________________________________________________
void FileSystemFrame::setPath( File path, bool forced )
{

    Debug::Throw() << "FileSystemFrame::setPath - path: " << path << endl;

    // check if changed
    if( forced || pathEditor_->path() != path )
    {
        pathEditor_->setPath( path );
        _update();
        _update();
        _updateNavigationActions();

        // reset file system watcher
        const QStringList directories( fileSystemWatcher_.directories() );
        if( !directories.isEmpty() ) fileSystemWatcher_.removePaths( directories );

        fileSystemWatcher_.addPath( path );
    }

}

//_________________________________________________________
void FileSystemFrame::setWorkingPath( const File& path )
{
    Debug::Throw( "FileSystemFrame::setHome.\n" );
    workingPath_ = path;
    if( pathEditor_->path().isEmpty() ) setPath( path );
}

//_________________________________________________________
void FileSystemFrame::clear()
{

    Debug::Throw( "FileSystemFrame::Clear.\n" );
    model_.clear();

}

//______________________________________________________
void FileSystemFrame::_processFiles( const File::List& files )
{

    // check path
    if( thread_.file() != pathEditor_->path() )
    {
        _update();
        return;
    }

    // build records
    FileRecord::List records;

    // add navigator if needed and update model
    if( showNavigator_ )
    {
        FileRecord record( File("..") );
        record.setFlags( BaseFileInfo::Navigator );
        records << record;
    }

    // add files
    for( const auto& file:files )
    {
        // skip hidden files
        if( file.isHidden() && !hiddenFilesAction_->isChecked() ) continue;

        // create file record
        FileRecord record( file, file.lastModified() );

        // assign size
        record.addProperty( sizePropertyId_, QString::number(file.fileSize()) );

        // assign type
        record.setFlag( file.isDirectory() ? BaseFileInfo::Folder : BaseFileInfo::Document );
        if( file.isLink() ) record.setFlag( BaseFileInfo::Link );
        if( file.isHidden() ) record.setFlag( BaseFileInfo::Hidden );

        // add to model
        records << record;

    }

    // update model
    model_.update( records );

    // update list
    list_->updateMask();
    list_->resizeColumnToContents( FileSystemModel::Filename );

    unsetCursor();

}

//______________________________________________________
void FileSystemFrame::_itemActivated( const QModelIndex& index )
{
    Debug::Throw( "FileSystemFrame::_itemActivated.\n" );

    if( !index.isValid() ) return;

    // retrieve file
    FileRecord record( model_.get( index ) );
    if( record.hasFlag( BaseFileInfo::Folder ) )
    {

        setPath( record.file().addPath( pathEditor_->path() ) );

    } else if( record.hasFlag( BaseFileInfo::Navigator ) ) {

        pathEditor_->selectParent();

    } else {

        emit fileActivated( record.setFile( record.file().addPath( pathEditor_->path() ) ) );

    }

}

//______________________________________________________
void FileSystemFrame::_updateConfiguration( void )
{
    Debug::Throw( "FileSystemFrame::_updateConfiguration.\n" );
    hiddenFilesAction_->setChecked( XmlOptions::get().get<bool>( "SHOW_HIDDEN_FILES" ) );

    // show navigator in list
    showNavigator_ = XmlOptions::get().get<bool>( "SHOW_NAVIGATOR" );

}

//______________________________________________________
void FileSystemFrame::_updateNavigationActions( void )
{
    Debug::Throw( "FileSystemFrame::_updateNavigationActions.\n" );
    previousDirectoryAction_->setEnabled( pathEditor_->hasPrevious() );
    nextDirectoryAction_->setEnabled( pathEditor_->hasNext() );
    parentDirectoryAction_->setEnabled( pathEditor_->hasParent() );
    return;
}

//______________________________________________________
void FileSystemFrame::_update( const QString& value )
{ if( isVisible() && value == pathEditor_->path() ) _update(); }

//______________________________________________________
void FileSystemFrame::_update( void )
{

    const File path( pathEditor_->path() );
    if( path.isEmpty() || !( path.exists() && path.isDirectory() ) ) return;
    if( thread_.isRunning() ) thread_.wait();

    // setup thread
    thread_.setFile( path );
    thread_.setCommand( FileThread::Command::List );
    thread_.setFlags(  hiddenFilesAction_->isChecked() ? File::ShowHiddenFiles : File::None );
    setCursor( Qt::WaitCursor );

    thread_.start();

}

//______________________________________________________________________
void FileSystemFrame::_updateActions( void )
{
    Debug::Throw( "FileSystemFrame:_updateActions.\n" );
    FileRecord::List selection( model_.get( list_->selectionModel()->selectedRows() ) );

    bool hasEditableSelection( false );
    bool hasRemovableSelection( false );
    for( const auto& record:selection )
    {
        if( !record.hasFlag( BaseFileInfo::Navigator ) ) hasRemovableSelection = true;
        if( record.hasFlag( BaseFileInfo::Document ) )
        {
            hasEditableSelection = true;
            break;
        }
    }

    openAction_->setEnabled( hasEditableSelection );
    removeAction_->setEnabled( hasRemovableSelection );

    QModelIndex index( list_->selectionModel()->currentIndex() );
    bool hasValidFile( selection.size() == 1 && index.isValid() && !model_.get( index ).hasFlag( BaseFileInfo::Navigator ) );
    filePropertiesAction_->setEnabled( hasValidFile );
    renameAction_->setEnabled( hasValidFile );
}

//______________________________________________________
void FileSystemFrame::_showToolTip( const QModelIndex& index )
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

//______________________________________________________
void FileSystemFrame::_toggleShowHiddenFiles( bool state )
{
    Debug::Throw( "FileSystemFrame::_toggleShowHiddenFiles.\n" );
    XmlOptions::get().set( "SHOW_HIDDEN_FILES", state );
}

//______________________________________________________
void FileSystemFrame::_reload( void )
{
    Debug::Throw( "FileSystemFrame::_reload.\n" );
    setPath( pathEditor_->path(), true );
}

//______________________________________________________________________
void FileSystemFrame::_open( void )
{

    Debug::Throw( "FileSystemFrame:_open.\n" );
    FileSystemModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    FileSystemModel::List validSelection;
    for( const auto& record:selection )
    {
        if( record.hasFlag( BaseFileInfo::Document ) )
        {
            FileRecord copy( record );
            copy.setFile( record.file().addPath( pathEditor_->path() ) );
            emit fileActivated( copy );
        }

    }

}

//______________________________________________________________________
void FileSystemFrame::_remove( void )
{

    Debug::Throw( "FileSystemFrame::_remove.\n" );

    // get selection
    FileSystemModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    FileSystemModel::List validSelection;
    for( const auto& record:selection )
    {
        if( record.hasFlag( BaseFileInfo::Navigator ) ) continue;
        FileRecord copy( record );
        copy.setFile( record.file().addPath( pathEditor_->path() ) );
        validSelection << copy;

    }

    if( validSelection.empty() ) return;
    RemoveFilesDialog dialog( this, validSelection );
    dialog.setWindowTitle( tr( "Delete Files - Qedit" ) );
    if( !dialog.exec() ) return;

    // loop over selected files and remove
    // retrieve selected items and remove corresponding files
    for( const auto& record:validSelection )
    { if( record.file().exists() ) record.file().removeRecursive(); }


}

//________________________________________
void FileSystemFrame::_rename( void )
{

    Debug::Throw( "FileSystemFrame::_rename.\n" );
    QModelIndex index( list_->selectionModel()->currentIndex() );
    if( !index.isValid() ) return;

    FileRecord record( model_.get( index ) );
    if( record.hasFlag( BaseFileInfo::Navigator ) ) return;
    RenameFileDialog dialog( this, record );
    dialog.setWindowTitle( tr( "Rename Item - Qedit" ) );
    if( !dialog.centerOnWidget( window() ).exec() ) return;

    File newFile( dialog.file() );
    if( newFile == record.file() ) return;

    // rename
    record.file().addPath( pathEditor_->path() ).rename( newFile.addPath( pathEditor_->path() ) );

}

//________________________________________
void FileSystemFrame::_fileProperties( void )
{

    Debug::Throw( "FileSystemFrame::_fileProperties.\n" );
    QModelIndex index( list_->selectionModel()->currentIndex() );
    if( !index.isValid() ) return;

    FileRecord record( model_.get( index ) );
    if( record.hasFlag( BaseFileInfo::Navigator ) ) return;
    if( !record.file().isAbsolute() ) { record.setFile( record.file().addPath( pathEditor_->path() ) ); }

    // icon
    QIcon icon;
    QVariant iconVariant( model_.data( index, Qt::DecorationRole ) );
    if( iconVariant.canConvert( QVariant::Icon ) ) icon = iconVariant.value<QIcon>();

    FileInformationDialog dialog( this, record );
    dialog.setIcon( icon );
    dialog.centerOnWidget( window() ).exec();

}

//_____________________________________________
void FileSystemFrame::_installActions( void )
{

    Debug::Throw( "FileSystemFrame::_installActions.\n" );

    // hidden files
    addAction( hiddenFilesAction_ = new QAction( tr( "Show Hidden Files" ), this ) );
    hiddenFilesAction_->setCheckable( true );
    connect( hiddenFilesAction_, SIGNAL(toggled(bool)), SLOT(_update()) );
    connect( hiddenFilesAction_, SIGNAL(toggled(bool)), SLOT(_toggleShowHiddenFiles(bool)) );

    // previous directory (from history)
    addAction( previousDirectoryAction_ = new QAction( IconEngine::get( IconNames::PreviousDirectory ), tr( "Back" ), this ) );
    connect( previousDirectoryAction_, SIGNAL(triggered()), pathEditor_, SLOT(selectPrevious()) );

    // next directory (from history)
    addAction( nextDirectoryAction_ = new QAction( IconEngine::get( IconNames::NextDirectory ), tr( "Forward" ), this ) );
    connect( nextDirectoryAction_, SIGNAL(triggered()), pathEditor_, SLOT(selectNext()) );

    // parent directory in tree
    addAction( parentDirectoryAction_ = new QAction( IconEngine::get( IconNames::Parent ), tr( "Parent Directory" ), this ) );
    parentDirectoryAction_->setIconText( tr( "Up" ) );
    connect( parentDirectoryAction_, SIGNAL(triggered()), pathEditor_, SLOT(selectParent()) );
    parentDirectoryAction_->setToolTip( tr( "Change path to parent directory" ) );

    // home directory
    addAction( homeDirectoryAction_ = new QAction( IconEngine::get( IconNames::Home ), tr( "Home" ), this ) );
    connect( homeDirectoryAction_, SIGNAL(triggered()), SLOT(_homeDirectory()) );
    homeDirectoryAction_->setToolTip( tr( "Change path to current user home directory" ) );

    // working directory
    addAction( workingDirectoryAction_ = new QAction( IconEngine::get( IconNames::Find ), tr( "Working Directory" ), this ) );
    connect( workingDirectoryAction_, SIGNAL(triggered()), SLOT(_workingDirectory()) );
    workingDirectoryAction_->setToolTip( tr( "Change path to current file working directory" ) );

    // reload
    addAction( reloadAction_ = new QAction( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this ) );
    connect( reloadAction_, SIGNAL(triggered()), SLOT(_reload()) );
    reloadAction_->setToolTip( tr( "Reload current directory contents" ) );

    // open
    addAction( openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open Selected Files" ), this ) );
    connect( openAction_, SIGNAL(triggered()), SLOT(_open()) );
    openAction_->setToolTip( tr( "Edit selected files" ) );

    // remove
    addAction( removeAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Delete" ), this ) );
    connect( removeAction_, SIGNAL(triggered()), SLOT(_remove()) );
    removeAction_->setShortcut( QKeySequence::Delete );

    // rename
    addAction( renameAction_ = new QAction( IconEngine::get( IconNames::Rename ), tr( "Rename" ), this ) );
    connect( renameAction_, SIGNAL(triggered()), SLOT(_rename()) );
    renameAction_->setShortcut( Qt::Key_F2 );
    renameAction_->setToolTip( tr( "Change selected file name" ) );

    // file properties
    addAction( filePropertiesAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Properties..." ), this ) );
    connect( filePropertiesAction_, SIGNAL(triggered()), SLOT(_fileProperties()) );
    filePropertiesAction_->setToolTip( tr( "Display current file properties" ) );

}
