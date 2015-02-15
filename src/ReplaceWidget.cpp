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

#include "ReplaceWidget.h"
#include "Debug.h"

#include <QLabel>
#include <QPushButton>

//________________________________________________________________________
ReplaceWidget::ReplaceWidget( QWidget* parent ):
    BaseReplaceWidget( parent )
{
    Debug::Throw( "ReplaceWidget::ReplaceWidget.\n" );

    // insert multiple file buttons
    QAction* action;
    _replaceAllMenu().addAction( action = new QAction( tr( "Files" ), this ) );
    connect( action, SIGNAL(triggered()), SIGNAL(replaceInFiles()) );

}
