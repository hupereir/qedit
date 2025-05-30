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

#include "SidePanelWidget.h"
#include "BaseFileSystemWidget.h"
#include "Debug.h"
#include "QtUtil.h"
#include "PathEditor.h"
#include "RecentFilesWidget.h"
#include "SessionFilesWidget.h"

#include <QHeaderView>
#include <QLayout>

//_______________________________________________________________
SidePanelWidget::SidePanelWidget( QWidget* parent, FileList& files ):
    QStackedWidget( parent )
{

    Debug::Throw( QStringLiteral("SidePanelWidget:SidePanelWidget.\n") );
    QtUtil::setMargin(layout(), 2);
    layout()->setSpacing(2);

    // stack widget
    addWidget( sessionFilesWidget_ = new SessionFilesWidget( nullptr ) );
    addWidget( recentFilesWidget_ = new RecentFilesWidget( nullptr, files) );
    addWidget( fileSystemWidget_ = new BaseFileSystemWidget( nullptr ) );

    // widget edges
    QtUtil::setWidgetSides( &sessionFilesWidget_->list(), Qt::LeftEdge );
    QtUtil::setWidgetSides( &recentFilesWidget_->list(), Qt::LeftEdge );
    QtUtil::setWidgetSides( &fileSystemWidget_->list(), Qt::TopEdge|Qt::LeftEdge );

    fileSystemWidget_->pathEditor().setWidgetSides( Qt::TopEdge|Qt::LeftEdge );
    fileSystemWidget_->layout()->setSpacing(0);

    // current widget
    setCurrentWidget( sessionFilesWidget_ );

    // connections
    connect( this, &QStackedWidget::currentChanged, this, &SidePanelWidget::_updateCurrentWidget );

    // actions
    _installActions();

}

//______________________________________________________________________
void SidePanelWidget::_installActions()
{
    Debug::Throw( QStringLiteral("SidePanelWidget::_installActions.\n") );
    addAction( visibilityAction_ = new QAction( tr( "Show &Navigation Panel" ), this ) );
    visibilityAction_->setCheckable( true );
    visibilityAction_->setChecked( true );
    connect( visibilityAction_, &QAction::toggled, this, &QWidget::setVisible );
}

//______________________________________________________________________
void SidePanelWidget::_updateCurrentWidget()
{
    Debug::Throw( QStringLiteral("SidePanelWidget::_updateCurrentWidget.\n") );
    if( currentWidget() == sessionFilesWidget_ ) { sessionFilesWidget_->updateFiles(); }
    else if( currentWidget() == recentFilesWidget_ ) { recentFilesWidget_->updateFiles(); }
}
