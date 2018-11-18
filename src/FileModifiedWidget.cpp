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

#include "FileModifiedWidget.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "QtUtil.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
FileModifiedWidget::FileModifiedWidget( QWidget* parent, const File& file ):
InformationWidget( parent )
{

    Debug::Throw( "FileModifiedWidget::FileModifiedWidget.\n" );

    setText( tr( " The file %1 has been modified by another application." ).arg( file.localName() ) );

    // reload button.
    QPushButton* button;
    buttonLayout().addWidget( button = new QPushButton( IconEngine::get( IconNames::Reload ), "&Reload", this ) );
    connect( button, SIGNAL(clicked()), SLOT(_reLoad()) );
    button->setToolTip( "Reload file from disc. Modifications will be lost" );

    // resave button
    buttonLayout().addWidget( button = new QPushButton( IconEngine::get( IconNames::Save ), "&Save Again", this ) );
    connect( button, SIGNAL(clicked()), SLOT(_reSave()) );
    button->setToolTip( "Save file again. Disc modifications will be lost" );

    // save as button
    buttonLayout().addWidget( button = new QPushButton( IconEngine::get( IconNames::SaveAs ), "&Save As", this ) );
    connect( button, SIGNAL(clicked()), SLOT(_saveAs()) );
    button->setToolTip( "Save file with a different name" );

    // ignore button.
    buttonLayout().addWidget( button = new QPushButton(  IconEngine::get( IconNames::DialogCancel ), "&Ignore", this ) );
    connect( button, SIGNAL(clicked()), SLOT(_ignore()) );
    button->setToolTip( "Ignore warning" );

    adjustSize();

}
