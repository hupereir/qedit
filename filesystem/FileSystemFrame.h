#ifndef FileSystemFrame_h
#define FileSystemFrame_h

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

#include "File.h"
#include "FileSystemModel.h"
#include "FileThread.h"

#include <QIcon>
#include <QWidget>
#include <QFileSystemWatcher>

class TreeView;
class PathEditor;
class FileRecordToolTipWidget;

//* customized ListView for file/directory navigation
class FileSystemFrame: public QWidget, public Counter
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    FileSystemFrame( QWidget* parent );

    //* destructor
    ~FileSystemFrame( void )
    { Debug::Throw( "FileSystemFrame::~FileSystemFrame.\n" ); }

    //* working path
    void setWorkingPath( const File& );

    //* clear list, add navigator button
    void clear();

    Q_SIGNALS:

    //* file selected
    void fileActivated( FileRecord );

    public Q_SLOTS:

    //* base directory
    void setPath( File, bool forced = false );

    protected Q_SLOTS:

    //* custom event, used to retrieve file validity check event
    void _processFiles( const File::List& );

    //* item activated
    void _itemActivated( const QModelIndex& );

    //* show hidden files
    void _toggleShowHiddenFiles( bool state );

    //* update configuration
    void _updateConfiguration( void );

    //* update navigation actions based on current location and history
    void _updateNavigationActions( void );

    //* show tooltip
    void _showToolTip( const QModelIndex& );

    //* update directory
    void _update( const QString& );

    //* update directory
    void _update( void );

    //* update actions
    void _updateActions( void );

    //* working directory
    void _workingDirectory( void )
    { setPath( workingPath_ ); }

    //* home directory
    void _homeDirectory( void )
    { setPath( homePath_ ); }

    //* reload
    void _reload( void );

    //* open
    void _open( void );

    //* remove (from dialog)
    void _remove( void );

    //* remove (from dialog)
    void _rename( void );

    //* file properties
    void _fileProperties( void );

    private:

    //* install actions
    void _installActions( void );

     //* path editor
    PathEditor* pathEditor_ = nullptr;

    //* tooltip widget
    FileRecordToolTipWidget* toolTipWidget_ = nullptr;

    //* model
    FileSystemModel model_;

    //* list of files
    TreeView* list_ = nullptr;

    //* size property id
    FileRecord::PropertyId::Id sizePropertyId_ = 0;

    //* true to show navigator in list
    bool showNavigator_ = false;

    //*@name actions
    //@{

    //* show/hide hidden files
    QAction* hiddenFilesAction_ = nullptr;

    //* previous directory action
    QAction* previousDirectoryAction_ = nullptr;

    //* next directory action
    QAction* nextDirectoryAction_ = nullptr;

    //* parent directory action
    QAction* parentDirectoryAction_ = nullptr;

    //* home directory action
    QAction* homeDirectoryAction_ = nullptr;

    //* working directory action
    QAction* workingDirectoryAction_ = nullptr;

    //* reload
    QAction* reloadAction_ = nullptr;

    //* open
    QAction* openAction_ = nullptr;

    //* remove
    QAction* removeAction_ = nullptr;

    //* rename
    QAction* renameAction_ = nullptr;

    //* properties
    QAction* filePropertiesAction_ = nullptr;

    //@}

    //* home directory
    File homePath_;

    //* working directory
    File workingPath_;

    //* file system watcher
    QFileSystemWatcher fileSystemWatcher_;

    //* thread to list files
    FileThread thread_;

};

#endif
