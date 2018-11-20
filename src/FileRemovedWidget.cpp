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

#include "FileRemovedWidget.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "QtUtil.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
FileRemovedWidget::FileRemovedWidget( QWidget* parent, const File& file ):
InformationWidget( parent )
{

    Debug::Throw( "FileRemovedWidget::FileRemovedWidget.\n" );
    setText( tr( " The file '%1' has been deleted by another application." ).arg( file.localName() ) );

    {
        // resave button
        auto button = addButton( IconEngine::get( IconNames::Save ), tr( "Save Again" ) );
        connect( button, SIGNAL(clicked()), SLOT(_reSave()) );
        button->setToolTip( tr( "Save file again. Disc modifications will be lost" ) );
    }

    {
        // save as button
        auto button = addButton( IconEngine::get( IconNames::SaveAs ), tr( "Save As" ) );
        connect( button, SIGNAL(clicked()), SLOT(_saveAs()) );
        button->setToolTip( tr( "Save file with a different name" ) );
    }

    {
        // close button.
        auto button = addButton( IconEngine::get( IconNames::DialogClose ), tr( "Close" ) );
        connect( button, SIGNAL(clicked()), SLOT(_close()) );
        button->setToolTip( tr( "Close window" ) );
    }

    {
        // ignore button.
        auto button = addButton( IconEngine::get( IconNames::DialogCancel ), tr( "Ignore" ) );
        connect( button, SIGNAL(clicked()), SLOT(_ignore()) );
        button->setToolTip( tr( "Ignore warning" ) );
    }

    adjustSize();

    // delete on hide
    connect( this, SIGNAL(hideAnimationFinished()), SLOT(deleteLater()) );

}

//________________________________________________________
void FileRemovedWidget::_reSave()
{
    emit actionSelected( ReturnCode::SaveAgain );
    animatedHide();
}

//________________________________________________________
void FileRemovedWidget::_saveAs()
{
    emit actionSelected( ReturnCode::SaveAs );
    animatedHide();
}

//________________________________________________________
void FileRemovedWidget::_close()
{
    emit actionSelected( ReturnCode::Close );
    animatedHide();
}

//________________________________________________________
void FileRemovedWidget::_ignore()
{
    emit actionSelected( ReturnCode::Ignore );
    animatedHide();
}
