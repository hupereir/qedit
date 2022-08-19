#ifndef RecentFilesWidget_h
#define RecentFilesWidget_h

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
#include "RecentFilesModel.h"

#include <QAction>
#include <QPaintEvent>
#include <QWidget>

class FileList;
class TreeView;
class FileRecordToolTipWidget;

//* editor windows navigator
/**
displays an up-to-date list of recent files
as well as files opened in current session
*/
class RecentFilesWidget: public QWidget, private Base::Counter<RecentFilesWidget>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    explicit RecentFilesWidget( QWidget* parent, FileList&  );

    //* list
    TreeView& list() const
    { return *list_; }

    //* select file in list
    void select( const File& );

    //* update recent files
    void updateFiles();

    Q_SIGNALS:

    //* signal emitted when a file is selected
    void fileSelected( FileRecord );

    //* signal emitted when a file is selected
    void fileActivated( FileRecord );

    protected:

    //* enter event
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void enterEvent( QEvent* ) override;
    #else
    void enterEvent( QEnterEvent* ) override;
    #endif
    
    private:

    //* update action
    void _updateActions();

    //* show tooltip
    void _showToolTip( const QModelIndex& );

    //* clean
    void _clean();

    //* open
    void _open();

    //* sessionFilesItem selected
    void _itemSelected( const QModelIndex& index );

    //* sessionFilesItem selected
    void _itemActivated( const QModelIndex& index );

    //*@name actions
    //@{

    //* install actions
    void _installActions();

    //@}

    //* true if actions are locked (to disable signal emission during update)
    bool actionsLocked_ = false;

    //* recent files
    FileList* recentFiles_ = nullptr;

    //* tooltip widget
    FileRecordToolTipWidget* toolTipWidget_ = nullptr;

    //* model
    RecentFilesModel model_;

    //* list
    TreeView* list_ = nullptr;

    //@}

    //*@name actions
    //@{

    //* clean action
    QAction* cleanAction_ = nullptr;

    //* open action
    QAction* openAction_ = nullptr;

    //@}

};

#endif
