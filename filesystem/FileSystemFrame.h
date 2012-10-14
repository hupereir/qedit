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
#include "FileSystemHistory.h"
#include "FileSystemThread.h"

#include <QtGui/QIcon>
#include <QtGui/QWidget>
#include <QtCore/QFileSystemWatcher>

class CustomComboBox;
class AnimatedTreeView;

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

    //! base directory
    const File& path( void ) const
    { return path_; }

    //! home path
    const File& home( void ) const
    { return homePath_; }

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

    protected:

    //! custom event, used to retrieve file validity check event
    void customEvent( QEvent* );

    private slots:

    //! item activated
    void _itemActivated( const QModelIndex& );

    //! show hidden files
    void _toggleShowHiddenFiles( bool state );

    //! update configuration
    void _updateConfiguration( void );

    //! update navigation actions based on current location and history
    void _updateNavigationActions( void );

    //! update path
    void _updatePath( void );

    //! update path
    void _updatePath( const QString& );

    //! update directory
    void _update( const QString& );

    //! update directory
    void _update( void );

    //! update actions
    void _updateActions( void );

    //! previous directory
    void _previousDirectory( void );

    //! next directory
    void _nextDirectory( void );

    //! parent directory
    void _parentDirectory( void );

    //! home directory
    void _homeDirectory( void );

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

    //! animation finished
    void _animationFinished( void );

    private:

    //! install actions
    void _installActions( void );

    //!@name actions
    //@{

    //! hidden files
    QAction& _hiddenFilesAction( void ) const
    { return *hiddenFilesAction_; }

    //! previous directory
    QAction& _previousDirectoryAction( void ) const
    { return *previousDirectoryAction_; }

    //! next directory
    QAction& _nextDirectoryAction( void ) const
    { return *nextDirectoryAction_; }

    // directory above
    QAction& _parentDirectoryAction( void ) const
    { return *parentDirectoryAction_; }

    // home directory
    QAction& _homeDirectoryAction( void ) const
    { return *homeDirectoryAction_; }

    // reload action
    QAction& _reloadAction( void ) const
    { return *reloadAction_; }

    //! open action
    QAction& _openAction( void ) const
    { return *openAction_; }

    //! remove action
    QAction& _removeAction( void ) const
    { return *removeAction_; }

    //! rename action
    QAction& _renameAction( void ) const
    { return *renameAction_; }

    //! file properties action
    QAction& _filePropertiesAction( void ) const
    { return *filePropertiesAction_; }

    //@}

    //! path comboBox
    CustomComboBox* pathComboBox_;

    //! model
    FileSystemModel model_;

    //! list of files
    AnimatedTreeView* list_;

    //! current path
    File path_;

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

    //! path naviagtion history
    FileSystemHistory history_;

    //! file system watcher
    QFileSystemWatcher fileSystemWatcher_;

    //! thread to list files
    FileSystemThread thread_;

};

#endif
