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
#include "FileSystemIcons.h"
#include "ColumnSortingMenu.h"
#include "ColumnSelectionMenu.h"
#include "ContextMenu.h"
#include "CustomComboBox.h"
#include "CustomToolBar.h"
#include "FileInformationDialog.h"
#include "IconEngine.h"
#include "QtUtil.h"
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
    homePath_( Util::home() ),
    fileSystemWatcher_( this ),
    thread_( this )
{

    Debug::Throw( "FileSystemFrame::FileSystemFrame.\n" );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setMargin(0);
    setLayout( layout );

    // install actions
    _installActions();

    // toolbar
    CustomToolBar* toolbar = new CustomToolBar( "Navigation Toolbar", this, "NAVIGATION_TOOLBAR" );
    toolbar->addAction( &_parentDirectoryAction() );
    toolbar->addAction( &_previousDirectoryAction() );
    toolbar->addAction( &_nextDirectoryAction() );
    toolbar->addAction( &_homeDirectoryAction() );
    toolbar->addAction( &_reloadAction() );
    layout->addWidget( toolbar );

    // combo box
    layout->addWidget( pathComboBox_ = new CustomComboBox( this ) );
    _comboBox().setEditable( true );

    connect( &_comboBox(), SIGNAL( currentIndexChanged( const QString& ) ), SLOT( _updatePath( const QString& ) ) );
    connect( &_comboBox(), SIGNAL( currentIndexChanged( const QString& ) ), SLOT( _updatePath( const QString& ) ) );
    connect( _comboBox().lineEdit(), SIGNAL(returnPressed()), SLOT( _updatePath( void ) ) );

    // file list
    layout->addWidget( list_ = new AnimatedTreeView( this ), 1);
    list_->setModel( &model_ );
    list_->setSelectionMode( QAbstractItemView::ContiguousSelection );
    list_->setOptionName( "FILE_SYSTEM_LIST" );
    list_->header()->hide();

    // list menu
    // should move to proper _updateMenu method, and only show relevant actions
    // also, should use baseFileInfo model
    QMenu* menu( new ContextMenu( &_list() ) );
    menu->addMenu( new ColumnSortingMenu( menu, &_list() ) );
    menu->addMenu( new ColumnSelectionMenu( menu, &_list() ) );
    menu->addSeparator();
    menu->addAction( &_hiddenFilesAction() );
    menu->addSeparator();
    menu->addAction( &_openAction() );
    menu->addAction( &_renameAction() );
    menu->addAction( &_removeAction() );

    menu->addSeparator();
    menu->addAction( &_filePropertiesAction() );

    connect( &list_->transitionWidget().timeLine(), SIGNAL( finished() ), SLOT( _animationFinished() ) );

    // connections
    connect( &model_, SIGNAL( layoutChanged() ), &_list(), SLOT( updateMask() ) );
    connect( list_->selectionModel(), SIGNAL( currentRowChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _updateActions() ) );
    connect( list_->selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateActions() ) );
    connect( &_list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _itemActivated( const QModelIndex& ) ) );

    _updateNavigationActions();

    connect( &_fileSystemWatcher(), SIGNAL( directoryChanged( const QString& ) ), SLOT( _update( const QString& ) ) );
    connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
    _updateConfiguration();
    _updateActions();

}

//_________________________________________________________
void FileSystemFrame::setPath( File path, bool forced )
{

    Debug::Throw() << "FileSystemFrame::setPath - path: " << path << endl;

    // check if changed
    if( !forced && path_ == path ) return;

    // update path
    assert( path.isDirectory() );
    path_ = path;
    history_.add( path );

    if( list_->initializeAnimation() )
    {

        _update();
        list_->startAnimation();

    } else {

        _update();
        _animationFinished();

    }

}

//_________________________________________________________
void FileSystemFrame::setHome( const File& path )
{
    Debug::Throw( "FileSystemFrame::setHome.\n" );
    homePath_ = path;
    if( FileSystemFrame::path().isEmpty() ) setPath( path );
}

//_________________________________________________________
void FileSystemFrame::clear()
{

    Debug::Throw( "FileSystemFrame::Clear.\n" );
    model_.clear();

}

//______________________________________________________
void FileSystemFrame::customEvent( QEvent* event )
{

    if( event->type() != FileSystemEvent::eventType() ) return;

    FileSystemEvent* file_system_event( static_cast<FileSystemEvent*>(event) );
    if( !file_system_event ) return;

    // check path
    if( file_system_event->path() != path() )
    {
        _update();
        return;
    }

    // update model and list
    model_.update( file_system_event->files() );
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

        // make full name
        File path( record.file() );
        path = path.addPath( FileSystemFrame::path() );
        setPath( path );

    } else if( record.hasFlag( FileSystemModel::Navigator ) ) {

        _parentDirectoryAction().trigger();

    } else {

        emit fileActivated( record.setFile( record.file().addPath( path() ) ) );

    }

}

//______________________________________________________
void FileSystemFrame::_updateConfiguration( void )
{
    Debug::Throw( "FileSystemFrame::_updateConfiguration.\n" );
    _hiddenFilesAction().setChecked( XmlOptions::get().get<bool>( "SHOW_HIDDEN_FILES" ) );
}

//______________________________________________________
void FileSystemFrame::_updateNavigationActions( void )
{
    Debug::Throw( "FileSystemFrame::_updateNavigationActions.\n" );
    _previousDirectoryAction().setEnabled( history_.previousAvailable() );
    _nextDirectoryAction().setEnabled( history_.nextAvailable() );
    _parentDirectoryAction().setEnabled( !QDir( path() ).isRoot() );
    return;
}

//______________________________________________________
void FileSystemFrame::_updatePath( void )
{
    Debug::Throw( "FileSystemFrame::_updatePath.\n" );
    _updatePath( _comboBox().lineEdit()->text() );
}

//______________________________________________________
void FileSystemFrame::_updatePath( const QString& value )
{
    Debug::Throw() << "FileSystemFrame::_updatePath - value: " << value << endl;

    // check if path has changed
    if( value == path() ) return;

    // if path is empty set path to home directory
    if( value.isEmpty() )
    {
        _homeDirectoryAction().trigger();
        return;
    }

    // check if path exists and is a directory
    File path( value );
    if( !( path.exists() && path.isDirectory() ) ) { setPath( path_ ); }
    else setPath( path );

    Debug::Throw() << "FileSystemFrame::_updatePath - done." << endl;

}

//______________________________________________________
void FileSystemFrame::_update( const QString& value )
{
    if( !isVisible() ) return;
    if( value != path() ) return;
    _update();

}

//______________________________________________________
void FileSystemFrame::_update( void )
{
    Debug::Throw( "FileSystemFrame::_update.\n" );

    if( path().isEmpty() || !( path().exists() && path().isDirectory() ) ) return;
    if( thread_.isRunning() ) return;
    thread_.setPath( path(), _hiddenFilesAction().isChecked() );
    thread_.start();
    setCursor( Qt::WaitCursor );

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
void FileSystemFrame::_previousDirectory( void )
{
    Debug::Throw( "FileSystemFrame::_previousDirectory.\n" );
    if( !history_.previousAvailable() ) return;
    setPath( history_.previous() );
}

//______________________________________________________
void FileSystemFrame::_nextDirectory( void )
{
    Debug::Throw( "FileSystemFrame::_nextDirectory.\n" );
    if( !history_.nextAvailable() ) return;
    setPath( history_.next() );
}

//______________________________________________________
void FileSystemFrame::_parentDirectory( void )
{

    Debug::Throw( "FileSystemFrame::_parentDirectory.\n" );
    QDir dir( path() );
    dir.cdUp();
    setPath( dir.absolutePath() );

}

//______________________________________________________
void FileSystemFrame::_homeDirectory( void )
{
    Debug::Throw( "FileSystemFrame::_homeDirectory.\n" );
    setPath( home() );
}

//______________________________________________________
void FileSystemFrame::_reload( void )
{
    Debug::Throw( "FileSystemFrame::_homeDirectory.\n" );
    setPath( path_, true );
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
            copy.setFile( record.file().addPath( path() ) );
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
        copy.setFile( record.file().addPath( path() ) );
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

    File new_file( dialog.file() );
    if( new_file == record.file() ) return;

    // rename
    record.file().addPath( path() ).rename( new_file.addPath( path() ) );

}

//________________________________________
void FileSystemFrame::_fileProperties( void )
{

    Debug::Throw( "FileSystemFrame::_fileProperties.\n" );
    QModelIndex index( list_->selectionModel()->currentIndex() );
    if( !index.isValid() ) return;

    FileRecord record( model_.get( index ) );
    if( record.hasFlag( FileSystemModel::Navigator ) ) return;
    if( !record.file().isAbsolute() ) { record.setFile( record.file().addPath( path() ) ); }

    FileInformationDialog( this, record ).centerOnWidget( window() ).exec();

}

//_____________________________________________
void FileSystemFrame::_animationFinished( void )
{

    Debug::Throw( "FileSystemFrame::_animationFinished.\n" );
    _updateNavigationActions();

    // update combobox
    if( _comboBox().findText( path() ) < 0 )
    { _comboBox().addItem( path() ); }

    _comboBox().setEditText( path() );

    // reset file system watcher
    const QStringList directories( _fileSystemWatcher().directories() );
    if( !directories.isEmpty() ) _fileSystemWatcher().removePaths( directories );

    _fileSystemWatcher().addPath( path() );

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

    // previous directory
    addAction( previousDirectoryAction_ = new QAction( IconEngine::get( ICONS::PREVIOUS_DIRECTORY ), "Previous", this ) );
    connect( &_previousDirectoryAction(), SIGNAL( triggered() ), SLOT( _previousDirectory() ) );
    _previousDirectoryAction().setToolTip( "Change path to previous directory (from history)" );

    // next directory (from history)
    addAction( nextDirectoryAction_ = new QAction( IconEngine::get( ICONS::NEXT_DIRECTORY ), "Next", this ) );
    connect( &_nextDirectoryAction(), SIGNAL( triggered() ), SLOT( _nextDirectory() ) );
    _nextDirectoryAction().setToolTip( "Change path to next directory (from history)" );

    // parent directory in tree
    addAction( parentDirectoryAction_ = new QAction( IconEngine::get( ICONS::PARENT_DIRECTORY ), "Parent Directory", this ) );
    connect( &_parentDirectoryAction(), SIGNAL( triggered() ), SLOT( _parentDirectory() ) );
    _parentDirectoryAction().setToolTip( "Change path to parent directory" );

    // home directory
    addAction( homeDirectoryAction_ = new QAction( IconEngine::get( ICONS::HOME_DIRECTORY ), "Home", this ) );
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
    addAction( filePropertiesAction_ = new QAction( IconEngine::get( ICONS::INFO ), "Properties...", this ) );
    connect( &_filePropertiesAction(), SIGNAL( triggered() ), SLOT( _fileProperties() ) );
    _filePropertiesAction().setToolTip( "Display current file properties" );

}
