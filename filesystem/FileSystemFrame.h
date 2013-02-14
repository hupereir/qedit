#ifndef FileSystemFrame_h
#define FileSystemFrame_h

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

#include "File.h"
#include "FileSystemModel.h"
#include "FileThread.h"

#include <QIcon>
#include <QWidget>
#include <QFileSystemWatcher>

class AnimatedTreeView;
class PathEditor;
class FileRecordToolTipWidget;

//! customized ListView for file/directory navigation
class FileSystemFrame: public QWidget, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! constructor
    FileSystemFrame( QWidget* parent );

    //! destructor
    ~FileSystemFrame( void )
    { Debug::Throw( "FileSystemFrame::~FileSystemFrame.\n" ); }

    //! home path
    void setHome( const File& path );

    //! clear list, add navigator button
    void clear();

    signals:

    //! file selected
    void fileActivated( FileRecord );

    public slots:

    //! base directory
    void setPath( File, bool forced = false );

    protected slots:

    //! custom event, used to retrieve file validity check event
    void _processFiles( const File::List& );

    //! item activated
    void _itemActivated( const QModelIndex& );

    //! show hidden files
    void _toggleShowHiddenFiles( bool state );

    //! update configuration
    void _updateConfiguration( void );

    //! update navigation actions based on current location and history
    void _updateNavigationActions( void );

    //! show tooltip
    void _showToolTip( const QModelIndex& );

    //! update directory
    void _update( const QString& );

    //! update directory
    void _update( void );

    //! update actions
    void _updateActions( void );

    //! home directory
    void _homeDirectory( void )
    { setPath( homePath_ ); }

    //! reload
    void _reload( void );

    //! open
    void _open( void );

    //! remove (from dialog)
    void _remove( void );

    //! remove (from dialog)
    void _rename( void );

    //! file properties
    void _fileProperties( void );

    private:

    //! install actions
    void _installActions( void );

     //! path editor
    PathEditor* pathEditor_;

    //! tooltip widget
    FileRecordToolTipWidget* toolTipWidget_;

    //! model
    FileSystemModel model_;

    //! list of files
    AnimatedTreeView* list_;

    //! size property id
    FileRecord::PropertyId::Id sizePropertyId_;

    //! true to show navigator in list
    bool showNavigator_;

    //!@name actions
    //@{

    //! show/hide hidden files
    QAction* hiddenFilesAction_;

    //! previous directory action
    QAction* previousDirectoryAction_;

    //! next directory action
    QAction* nextDirectoryAction_;

    //! parent directory action
    QAction* parentDirectoryAction_;

    //! home directory action
    QAction* homeDirectoryAction_;

    //! reload
    QAction* reloadAction_;

    //! open
    QAction* openAction_;

    //! remove
    QAction* removeAction_;

    //! rename
    QAction* renameAction_;

    //! properties
    QAction* filePropertiesAction_;

    //@}

    //! home directory
    File homePath_;

    //! file system watcher
    QFileSystemWatcher fileSystemWatcher_;

    //! thread to list files
    FileThread thread_;

};

#endif
