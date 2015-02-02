#ifndef SessionFilesFrame_h
#define SessionFilesFrame_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "Counter.h"
#include "SessionFilesModel.h"
#include "TreeView.h"

#include <QPaintEvent>
#include <QAction>
#include <QBasicTimer>
#include <QTimerEvent>

class FileRecordToolTipWidget;

//* editor windows navigator
class SessionFilesFrame: public QWidget, public Counter
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    SessionFilesFrame( QWidget* parent );

    //* destructor
    ~SessionFilesFrame( void );

    //* list
    TreeView& list( void ) const
    { return *list_; }

    //* select file in list
    void select( const File& );

    //*@name actions
    //@{

    //* previous file
    QAction& previousFileAction( void ) const
    { return *previousFileAction_; }

    //* previous file
    QAction& nextFileAction( void ) const
    { return *nextFileAction_; }

    //@}

    //* model
    const SessionFilesModel& model( void ) const
    { return model_; }

    public Q_SLOTS:

    //* update session files
    void update( void );

    Q_SIGNALS:

    //* signal emitted when a file is selected
    void fileSelected( FileRecord );

    //* signal emitted when a file is selected
    void fileActivated( FileRecord );

    //* signal emitted when file is asked to be closed
    void filesClosed( FileRecord::List );

    //* signal emitted when file is asked to be saved
    void filesSaved( FileRecord::List );

    protected Q_SLOTS:

    //* previous file
    void _selectPreviousFile( void );

    //* next file
    void _selectNextFile( void );

    //* update session files
    void _updateActions( void );

    //* show tooltip
    void _showToolTip( const QModelIndex& );

    //* open
    void _open( void );

    //* save
    void _save( void );

    //* close
    void _close( void );

    //* sessionFilesItem selected
    void _itemSelected( const QModelIndex& index );

    //* sessionFilesItem selected
    void _itemActivated( const QModelIndex& index );

    private:

    //* install actions
    void _installActions( void );

    //* tooltip widget
    FileRecordToolTipWidget* toolTipWidget_ = nullptr;

    //* model
    SessionFilesModel model_;

    //* list
    TreeView* list_;

    //*@name actions
    //@{

    //* previous file
    QAction* previousFileAction_;

    //* next file
    QAction* nextFileAction_;

    //* open action
    QAction* openAction_;

    //* save action
    QAction* saveAction_;

    //* close action
    QAction *closeAction_;

    //@}

};

#endif
