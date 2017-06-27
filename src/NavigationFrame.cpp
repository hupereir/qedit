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

#include "NavigationFrame.h"

#include "Application.h"
#include "Debug.h"
#include "FileSystemFrame.h"
#include "RecentFilesFrame.h"
#include "SessionFilesFrame.h"

#include <QHeaderView>
#include <QLayout>

//_______________________________________________________________
NavigationFrame::NavigationFrame( QWidget* parent, FileList& files ):
    QStackedWidget( parent )
{

    Debug::Throw( "NavigationFrame:NavigationFrame.\n" );
    layout()->setMargin(2);
    layout()->setSpacing(2);

    // stack widget
    addWidget( sessionFilesFrame_ = new SessionFilesFrame(0) );
    addWidget( recentFilesFrame_ = new RecentFilesFrame(0, files) );
    addWidget( fileSystemFrame_ = new FileSystemFrame(0) );

    // current widget
    setCurrentWidget( &sessionFilesFrame() );

    // connections
    connect( this, SIGNAL(currentChanged(int)), SLOT(_updateCurrentWidget()) );

    // actions
    _installActions();

}

//______________________________________________________________________
void NavigationFrame::_installActions()
{

    Debug::Throw( "NavigationFrame::_installActions.\n" );
    addAction( visibilityAction_ = new QAction( tr( "Show &Navigation Panel" ), this ) );
    visibilityAction_->setCheckable( true );
    visibilityAction_->setChecked( true );
    connect( visibilityAction_, SIGNAL(toggled(bool)), SLOT(setVisible(bool)) );

}

//______________________________________________________________________
void NavigationFrame::_updateCurrentWidget()
{

    Debug::Throw( "NavigationFrame::_updateCurrentWidget.\n" );
    if( !currentWidget() ) return;

    if( currentWidget() == &sessionFilesFrame() ) { sessionFilesFrame().update(); }
    else if( currentWidget() == &recentFilesFrame() ) { recentFilesFrame().update(); }
    else if( currentWidget() == &fileSystemFrame() ) { fileSystemFrame().update(); }

}
