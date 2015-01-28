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

#include "ReplaceDialog.h"
#include "Debug.h"
#include "QtUtil.h"

#include <QLabel>
#include <QPushButton>

//________________________________________________________________________
ReplaceDialog::ReplaceDialog( QWidget* parent, Qt::WindowFlags flags ):
BaseReplaceDialog( parent, flags )
{
    Debug::Throw( "ReplaceDialog::ReplaceDialog.\n" );

    // insert multiple file buttons
    QPushButton* button = new QPushButton( tr( "Files" ), this );
    connect( button, SIGNAL(clicked()), SIGNAL(replaceInFiles()) );
    button->setToolTip( tr( "Replace all occurence of the search string in the selected files" ) );
    _addDisabledButton( button );
    _locationLayout().addWidget( button );
    button->setAutoDefault( false );
    replaceInFilesButton_ = button;

    // tab order
    setTabOrder( &_replaceWindowButton(), button );

}
