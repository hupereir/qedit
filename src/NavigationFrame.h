#ifndef NavigationFrame_h
#define NavigationFrame_h

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

#include "FileList.h"
#include "FileRecord.h"

#include <QAction>
#include <QShowEvent>
#include <QStackedWidget>
#include <QToolButton>

class SessionFilesFrame;
class RecentFilesFrame;
class FileSystemFrame;

//* editor windows navigator
class NavigationFrame: public QStackedWidget
{

    Q_OBJECT

    public:

    //* creator
    explicit NavigationFrame( QWidget* parent, FileList&  );

    //* default size
    void setDefaultWidth( int value )
    { defaultWidth_ = value; }

    //* default width
    int defaultWidth() const
    { return defaultWidth_; }

    //* size
    QSize sizeHint() const
    { return (defaultWidth_ ) >= 0 ? QSize( defaultWidth_, 0 ):QWidget::sizeHint(); }

    //* session files
    SessionFilesFrame& sessionFilesFrame() const
    { return *sessionFilesFrame_; }

    //* recent files
    RecentFilesFrame& recentFilesFrame() const
    { return *recentFilesFrame_; }

    //* file system
    FileSystemFrame& fileSystemFrame() const
    { return *fileSystemFrame_; }

    //*@name actions
    //@{

    //* visibility
    QAction& visibilityAction() const
    { return *visibilityAction_; }

    //@}

    private Q_SLOTS:

    //* update current widget
    void _updateCurrentWidget();

    private:

    //* install actions
    void _installActions();

    //* default width;
    int defaultWidth_ = -1;

    //* session files
    SessionFilesFrame *sessionFilesFrame_;

    //* recent files
    RecentFilesFrame *recentFilesFrame_;

    //* file system
    FileSystemFrame* fileSystemFrame_;

    //@}

    //* visibility action
    QAction* visibilityAction_ = nullptr;

};

#endif
