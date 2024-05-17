#ifndef SessionFilesWidget_h
#define SessionFilesWidget_h

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
class MimeTypeIconProvider;

//* editor windows navigator
class SessionFilesWidget: public QWidget, private Base::Counter<SessionFilesWidget>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    explicit SessionFilesWidget( QWidget* );

    //* list
    TreeView& list() const
    { return *list_; }

    //* select file in list
    void select( const File& );

    //*@name actions
    //@{

    //* previous file
    QAction& previousFileAction() const
    { return *previousFileAction_; }

    //* previous file
    QAction& nextFileAction() const
    { return *nextFileAction_; }

    //@}

    //* model
    const SessionFilesModel& model() const
    { return model_; }

    //* update session files
    void updateFiles();

    Q_SIGNALS:

    //* signal emitted when a file is selected
    void fileSelected( FileRecord );

    //* signal emitted when a file is selected
    void fileActivated( FileRecord );

    //* signal emitted when file is asked to be closed
    void filesClosed( FileRecord::List );

    //* signal emitted when file is asked to be saved
    void filesSaved( FileRecord::List );

    private:

    //* previous file
    void _selectPreviousFile();

    //* next file
    void _selectNextFile();

    //* update session files
    void _updateActions();

    //* show tooltip
    void _showToolTip( const QModelIndex& );

    //* open
    void _open();

    //* save
    void _save();

    //* close
    void _close();

    //* sessionFilesItem selected
    void _itemSelected( const QModelIndex& index );

    //* sessionFilesItem selected
    void _itemActivated( const QModelIndex& index );

    //* install actions
    void _installActions();

    //* tooltip widget
    FileRecordToolTipWidget* toolTipWidget_ = nullptr;

    //* mime type icon provider
    MimeTypeIconProvider* mimeTypeIconProvider_ = nullptr;

    //* model
    SessionFilesModel model_;

    //* list
    TreeView* list_ = nullptr;

    //*@name actions
    //@{

    //* previous file
    QAction* previousFileAction_ = nullptr;

    //* next file
    QAction* nextFileAction_ = nullptr;

    //* open action
    QAction* openAction_ = nullptr;

    //* save action
    QAction* saveAction_ = nullptr;

    //* close action
    QAction *closeAction_ = nullptr;

    //@}

};

#endif
