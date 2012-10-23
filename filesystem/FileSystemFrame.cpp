// $Id$

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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "FileSystemFrame.h"

#include "AnimatedTreeView.h"
#include "FileRecordProperties.h"
#include "FileSystemIcons.h"
#include "ColumnSortingMenu.h"
#include "ColumnSelectionMenu.h"
#include "ContextMenu.h"
#include "CustomToolBar.h"
#include "FileInformationDialog.h"
#include "IconEngine.h"
#include "PathEditor.h"
#include "RemoveFilesDialog.h"
#include "RenameFileDialog.h"
#include "Singleton.h"
#include "TransitionWidget.h"
#include "Util.h"
#include "XmlOptions.h"

#include <QtGui/QApplication>
#include <QtGui/QHeaderView>
#include <QtGui/QLayout>
#include <QtCore/QDir>

//_____________________________________________
FileSystemFrame::FileSystemFrame( QWidget *parent ):
    QWidget( parent ),
    Counter( "FileSystemFrame" ),
    sizePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::SIZE ) ),
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
    CustomToolBar* toolbar = new CustomToolBar( "Navigation Toolbar", this, "NAVIGATION_TOOLBAR" );
    layout->addWidget( toolbar );

    // path editor
    layout->addWidget( pathEditor_ = new PathEditor( this ) );
    pathEditor_->setHomePath( Util::home() );

    // initialize local root path list
    File::List rootPathList;
    foreach( const QFileInfo& fileInfo, QDir::drives() )
    { rootPathList << fileInfo.path(); }
    pathEditor_->setRootPathList( rootPathList );

    connect( pathEditor_, SIGNAL( pathChanged( const File& ) ), SLOT( _update( void ) ) );
    connect( pathEditor_, SIGNAL( pathChanged( const File& ) ), SLOT( _updateNavigationActions( void ) ) );

    // install actions
    _installActions();
    toolbar->addAction( &_parentDirectoryAction() );
    toolbar->addAction( &_previousDirectoryAction() );
    toolbar->addAction( &_nextDirectoryAction() );
    toolbar->addAction( &_homeDirectoryAction() );
    toolbar->addAction( &_reloadAction() );

    // file list
    layout->addWidget( list_ = new AnimatedTreeView( this ), 1);
    list_->setModel( &model_ );
    list_->setItemMargin( 2 );
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
    menu->addAction( &_hiddenFilesAction() );
    menu->addSeparator();
    menu->addAction( &_openAction() );
    menu->addAction( &_renameAction() );
    menu->addAction( &_removeAction() );
    menu->addSeparator();
    menu->addAction( &_filePropertiesAction() );

    // connections
    connect( &model_, SIGNAL( layoutChanged() ), list_, SLOT( updateMask() ) );
    connect( list_->selectionModel(), SIGNAL( currentRowChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _updateActions() ) );
    connect( list_->selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateActions() ) );
    connect( list_, SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemActivated( const QModelIndex& ) ) );

    // connect filesystem watcher
    connect( &fileSystemWatcher_, SIGNAL( directoryChanged( const QString& ) ), SLOT( _update( const QString& ) ) );

    // connect thread
    connect( &thread_, SIGNAL( filesAvailable( const File::List& ) ), SLOT( _processFiles( const File::List& ) ) );

    connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
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

        // reset file system watcher
        const QStringList directories( fileSystemWatcher_.directories() );
        if( !directories.isEmpty() ) fileSystemWatcher_.removePaths( directories );

        fileSystemWatcher_.addPath( path );
    }

}

//_________________________________________________________
void FileSystemFrame::setHome( const File& path )
{
    Debug::Throw( "FileSystemFrame::setHome.\n" );
    homePath_ = path;
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
        record.setFlags( FileSystemModel::Navigator );
        records << record;
    }

    // add files
    foreach( const File& file, files )
    {
        // skip hidden files
        if( file.isHidden() && !_hiddenFilesAction().isChecked() ) continue;

        // create file record
        FileRecord record( file, file.lastModified() );

        // assign size
        record.addProperty( sizePropertyId_, QString().setNum(file.fileSize()) );

        // assign type
        record.setFlag( file.isDirectory() ? FileSystemModel::Folder : FileSystemModel::Document );
        if( file.isLink() ) record.setFlag( FileSystemModel::Link );

        // add to model
        records << record;

    }

    // update model
    model_.update( records );

    // update list
    list_->updateMask();
    list_->resizeColumnToContents( FileSystemModel::FILE );

    unsetCursor();

}

//______________________________________________________
void FileSystemFrame::_itemActivated( const QModelIndex& index )
{
    Debug::Throw( "FileSystemFrame::_itemActivated.\n" );

    if( !index.isValid() ) return;

    // retrieve file
    FileRecord record( model_.get( index ) );
    if( record.hasFlag( FileSystemModel::Folder ) )
    {

        setPath( record.file().addPath( pathEditor_->path() ) );

    } else if( record.hasFlag( FileSystemModel::Navigator ) ) {

        _parentDirectoryAction().trigger();

    } else {

        emit fileActivated( record.setFile( record.file().addPath( pathEditor_->path() ) ) );

    }

}

//______________________________________________________
void FileSystemFrame::_updateConfiguration( void )
{
    Debug::Throw( "FileSystemFrame::_updateConfiguration.\n" );
    _hiddenFilesAction().setChecked( XmlOptions::get().get<bool>( "SHOW_HIDDEN_FILES" ) );

    // show navigator in list
    showNavigator_ = XmlOptions::get().get<bool>( "SHOW_NAVIGATOR" );

}

//______________________________________________________
void FileSystemFrame::_updateNavigationActions( void )
{
    Debug::Throw( "FileSystemFrame::_updateNavigationActions.\n" );
    _previousDirectoryAction().setEnabled( pathEditor_->hasPrevious() );
    _nextDirectoryAction().setEnabled( pathEditor_->hasNext() );
    _parentDirectoryAction().setEnabled( pathEditor_->hasParent() );
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
    if( thread_.isRunning() ) return;

    // setup thread
    thread_.setFile( path );
    thread_.setCommand( FileThread::List );
    thread_.setFlags(  _hiddenFilesAction().isChecked() ? File::ShowHiddenFiles : File::None );
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
    foreach( const FileRecord& record, selection )
    {
        if( !record.hasFlag( FileSystemModel::Navigator ) ) hasRemovableSelection = true;
        if( record.hasFlag( FileSystemModel::Document ) )
        {
            hasEditableSelection = true;
            break;
        }
    }

    _openAction().setEnabled( hasEditableSelection );
    _removeAction().setEnabled( hasRemovableSelection );

    QModelIndex index( list_->selectionModel()->currentIndex() );
    bool hasValidFile( selection.size() == 1 && index.isValid() && !model_.get( index ).hasFlag( FileSystemModel::Navigator ) );
    _filePropertiesAction().setEnabled( hasValidFile );
    _renameAction().setEnabled( hasValidFile );
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
    Debug::Throw( "FileSystemFrame::_homeDirectory.\n" );
    setPath( pathEditor_->path(), true );
}

//______________________________________________________________________
void FileSystemFrame::_open( void )
{

    Debug::Throw( "FileSystemFrame:_open.\n" );
    FileSystemModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    FileSystemModel::List validSelection;
    foreach( const FileRecord& record, selection )
    {
        if( record.hasFlag( FileSystemModel::Document ) )
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
    foreach( const FileRecord& record, selection )
    {
        if( record.hasFlag( FileSystemModel::Navigator ) ) continue;
        FileRecord copy( record );
        copy.setFile( record.file().addPath( pathEditor_->path() ) );
        validSelection << copy;

    }

    if( validSelection.empty() ) return;
    RemoveFilesDialog dialog( this, validSelection );
    dialog.setWindowTitle( "Delete Files - Qedit" );
    if( !dialog.exec() ) return;

    // loop over selected files and remove
    // retrieve selected items and remove corresponding files
    foreach( const FileRecord& record, validSelection )
    { if( record.file().exists() ) record.file().removeRecursive(); }


}

//________________________________________
void FileSystemFrame::_rename( void )
{

    Debug::Throw( "FileSystemFrame::_rename.\n" );
    QModelIndex index( list_->selectionModel()->currentIndex() );
    if( !index.isValid() ) return;

    FileRecord record( model_.get( index ) );
    if( record.hasFlag( FileSystemModel::Navigator ) ) return;
    RenameFileDialog dialog( this, record );
    dialog.setWindowTitle( "Rename Item - Qedit" );
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
    if( record.hasFlag( FileSystemModel::Navigator ) ) return;
    if( !record.file().isAbsolute() ) { record.setFile( record.file().addPath( pathEditor_->path() ) ); }

    FileInformationDialog( this, record ).centerOnWidget( window() ).exec();

}

//_____________________________________________
void FileSystemFrame::_installActions( void )
{

    Debug::Throw( "FileSystemFrame::_installActions.\n" );

    // hidden files
    addAction( hiddenFilesAction_ = new QAction( "Show Hidden Files", this ) );
    _hiddenFilesAction().setCheckable( true );
    connect( &_hiddenFilesAction(), SIGNAL( toggled( bool ) ), SLOT( _update() ) );
    connect( &_hiddenFilesAction(), SIGNAL( toggled( bool ) ), SLOT( _toggleShowHiddenFiles( bool ) ) );

    // previous directory (from history)
    addAction( previousDirectoryAction_ = new QAction( IconEngine::get( ICONS::PREVIOUS_DIRECTORY ), "Previous", this ) );
    connect( previousDirectoryAction_, SIGNAL( triggered( void ) ), pathEditor_, SLOT( selectPrevious( void ) ) );

    // next directory (from history)
    addAction( nextDirectoryAction_ = new QAction( IconEngine::get( ICONS::NEXT_DIRECTORY ), "Next", this ) );
    connect( nextDirectoryAction_, SIGNAL( triggered( void ) ), pathEditor_, SLOT( selectNext( void ) ) );

    // parent directory in tree
    addAction( parentDirectoryAction_ = new QAction( IconEngine::get( ICONS::PARENT ), "Parent Directory", this ) );
    connect( parentDirectoryAction_, SIGNAL( triggered( void ) ), pathEditor_, SLOT( selectParent( void ) ) );
    _parentDirectoryAction().setToolTip( "Change path to parent directory" );

    // home directory
    addAction( homeDirectoryAction_ = new QAction( IconEngine::get( ICONS::HOME ), "Home", this ) );
    connect( &_homeDirectoryAction(), SIGNAL( triggered() ), SLOT( _homeDirectory() ) );
    _homeDirectoryAction().setToolTip( "Change path to current file working directory" );

    // home directory
    addAction( reloadAction_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Reload", this ) );
    connect( &_reloadAction(), SIGNAL( triggered() ), SLOT( _reload() ) );
    _reloadAction().setToolTip( "Reload current directory contents" );

    // open
    addAction( openAction_ = new QAction( IconEngine::get( ICONS::OPEN ), "Open Selected Files", this ) );
    connect( &_openAction(), SIGNAL( triggered() ), SLOT( _open() ) );
    _openAction().setToolTip( "Edit selected files" );

    // remove
    addAction( removeAction_ = new QAction( IconEngine::get( ICONS::DELETE ), "Delete", this ) );
    connect( &_removeAction(), SIGNAL( triggered() ), SLOT( _remove() ) );
    _removeAction().setShortcut( QKeySequence::Delete );

    // rename
    addAction( renameAction_ = new QAction( IconEngine::get( ICONS::RENAME ), "Rename", this ) );
    connect( &_renameAction(), SIGNAL( triggered() ), SLOT( _rename() ) );
    _renameAction().setShortcut( Qt::Key_F2 );
    _renameAction().setToolTip( "Change selected file name" );

    // file properties
    addAction( filePropertiesAction_ = new QAction( IconEngine::get( ICONS::INFORMATION ), "Properties...", this ) );
    connect( &_filePropertiesAction(), SIGNAL( triggered() ), SLOT( _fileProperties() ) );
    _filePropertiesAction().setToolTip( "Display current file properties" );

}
