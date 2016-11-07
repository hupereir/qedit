#ifndef RecentFilesFrame_h
#define RecentFilesFrame_h

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

class FileList;
class TreeView;
class FileRecordToolTipWidget;

//* editor windows navigator
/**
displays an up-to-date list of recent files
as well as files opened in current session
*/
class RecentFilesFrame: public QWidget, public Counter
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    RecentFilesFrame( QWidget* parent, FileList&  );

    //* list
    TreeView& list( void ) const
    { return *list_; }

    //* select file in list
    void select( const File& );

    public Q_SLOTS:

    //* update
    void update( void );

    Q_SIGNALS:

    //* signal emitted when a file is selected
    void fileSelected( FileRecord );

    //* signal emited when a file is selected
    void fileActivated( FileRecord );

    protected:

    //* enter event
    void enterEvent( QEvent* );

    private Q_SLOTS:

    //* update action
    void _updateActions( void );

    //* show tooltip
    void _showToolTip( const QModelIndex& );

    //* clean
    void _clean( void );

    //* open
    void _open( void );

    //* sessionFilesItem selected
    void _itemSelected( const QModelIndex& index );

    //* sessionFilesItem selected
    void _itemActivated( const QModelIndex& index );

    private:

    //*@name actions
    //@{

    //* install actions
    void _installActions( void );

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

    //* update
    QAction* updateAction_ = nullptr;

    //* clean action
    QAction* cleanAction_ = nullptr;

    //* open action
    QAction* openAction_ = nullptr;

    //@}

};

#endif
