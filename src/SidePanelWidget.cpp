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

#include "Debug.h"
#include "BaseFileSystemWidget.h"
#include "RecentFilesWidget.h"
#include "SessionFilesWidget.h"

#include <QHeaderView>
#include <QLayout>

//_______________________________________________________________
SidePanelWidget::SidePanelWidget( QWidget* parent, FileList& files ):
    QStackedWidget( parent )
{

    Debug::Throw( "SidePanelWidget:SidePanelWidget.\n" );
    layout()->setMargin(2);
    layout()->setSpacing(2);

    // stack widget
    addWidget( sessionFilesWidget_ = new SessionFilesWidget( nullptr ) );
    addWidget( recentFilesWidget_ = new RecentFilesWidget( nullptr, files) );
    addWidget( fileSystemWidget_ = new BaseFileSystemWidget( nullptr ) );

    // current widget
    setCurrentWidget( sessionFilesWidget_ );

    // connections
    connect( this, SIGNAL(currentChanged(int)), SLOT(_updateCurrentWidget()) );

    // actions
    _installActions();

}

//______________________________________________________________________
void SidePanelWidget::_installActions()
{

    Debug::Throw( "SidePanelWidget::_installActions.\n" );
    addAction( visibilityAction_ = new QAction( tr( "Show &Navigation Panel" ), this ) );
    visibilityAction_->setCheckable( true );
    visibilityAction_->setChecked( true );
    connect( visibilityAction_, SIGNAL(toggled(bool)), SLOT(setVisible(bool)) );

}

//______________________________________________________________________
void SidePanelWidget::_updateCurrentWidget()
{

    Debug::Throw( "SidePanelWidget::_updateCurrentWidget.\n" );
    if( !currentWidget() ) return;

    if( currentWidget() == sessionFilesWidget_ ) { sessionFilesWidget_->update(); }
    else if( currentWidget() == recentFilesWidget_ ) { recentFilesWidget_->update(); }
    else if( currentWidget() == fileSystemWidget_ ) { fileSystemWidget_->update(); }

}
