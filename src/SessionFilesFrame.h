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

#include "Counter.h"
#include "SessionFilesModel.h"
#include "TreeView.h"

#include <QtGui/QPaintEvent>
#include <QtGui/QAction>
#include <QtCore/QBasicTimer>
#include <QtCore/QTimerEvent>
#include <cassert>

class FileRecordToolTipWidget;

//! editor windows navigator
class SessionFilesFrame: public QWidget, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! creator
    SessionFilesFrame( QWidget* parent );

    //! destructor
    ~SessionFilesFrame( void );

    //! list
    TreeView& list( void ) const
    {
        assert( list_ );
        return *list_;
    }

    //! select file in list
    void select( const File& );

    //!@name actions
    //@{

    //! previous file
    QAction& previousFileAction( void ) const
    { return *previousFileAction_; }

    //! previous file
    QAction& nextFileAction( void ) const
    { return *nextFileAction_; }

    //@}

    //! model
    const SessionFilesModel& model( void ) const
    { return model_; }

    public slots:

    //! update session files
    void update( void );

    signals:

    //! signal emitted when a file is selected
    void fileSelected( FileRecord );

    //! signal emitted when a file is selected
    void fileActivated( FileRecord );

    //! signal emitted when file is asked to be closed
    void filesClosed( FileRecord::List );

    //! signal emitted when file is asked to be saved
    void filesSaved( FileRecord::List );

    protected slots:

    //! previous file
    void _selectPreviousFile( void );

    //! next file
    void _selectNextFile( void );

    //! update session files
    void _updateActions( void );

    //! show tooltip
    void _showToolTip( const QModelIndex& );

    //! open
    void _open( void );

    //! save
    void _save( void );

    //! close
    void _close( void );

    //! sessionFilesItem selected
    void _itemSelected( const QModelIndex& index );

    //! sessionFilesItem selected
    void _itemActivated( const QModelIndex& index );

    private:

    //! install actions
    void _installActions( void );

    //!@name actions
    //@{

    //! open action
    QAction& _openAction( void ) const
    { return *openAction_; }

    //! save action
    QAction& _saveAction( void ) const
    { return *saveAction_; }

    //! close action
    QAction& _closeAction( void ) const
    { return *closeAction_; }

    //@}

    //! tooltip widget
    FileRecordToolTipWidget* toolTipWidget_;

    //! model
    SessionFilesModel model_;

    //! list
    TreeView* list_;

    //!@name actions
    //@{

    //! previous file
    QAction* previousFileAction_;

    //! next file
    QAction* nextFileAction_;

    //! open action
    QAction* openAction_;

    //! save action
    QAction* saveAction_;

    //! close action
    QAction *closeAction_;

    //@}

};

#endif
