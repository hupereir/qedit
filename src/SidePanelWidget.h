#ifndef SidePanelWidget_h
#define SidePanelWidget_h

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

class SessionFilesWidget;
class RecentFilesWidget;
class FileSystemWidget;

//* editor windows navigator
class SidePanelWidget: public QStackedWidget
{

    Q_OBJECT

    public:

    //* creator
    explicit SidePanelWidget( QWidget* parent, FileList&  );

    //*@name accessors
    //@{

    //* size
    QSize sizeHint() const override
    {
        return (defaultWidth_ ) >= 0 ?
            QSize( defaultWidth_, 0 ):
            QWidget::sizeHint();
    }

    //* session files
    SessionFilesWidget& sessionFilesWidget() const
    { return *sessionFilesWidget_; }

    //* recent files
    RecentFilesWidget& recentFilesWidget() const
    { return *recentFilesWidget_; }

    //* file system
    FileSystemWidget& fileSystemWidget() const
    { return *fileSystemWidget_; }

    //* visibility
    QAction& visibilityAction() const
    { return *visibilityAction_; }

    //@}

    //*@name modifiers
    //@{

    //* default size
    void setDefaultWidth( int value )
    { defaultWidth_ = value; }

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
    SessionFilesWidget *sessionFilesWidget_;

    //* recent files
    RecentFilesWidget *recentFilesWidget_;

    //* file system
    FileSystemWidget* fileSystemWidget_;

    //@}

    //* visibility action
    QAction* visibilityAction_ = nullptr;

};

#endif
