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
#include "IconEngine.h"
#include "IconNames.h"
#include "QtUtil.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
FileRemovedWidget::FileRemovedWidget( QWidget* parent, const File& file ):
    MessageWidget( parent, MessageType::Information )
{

    Debug::Throw( QStringLiteral("FileRemovedWidget::FileRemovedWidget.\n") );
    setFile( file );

    {
        // resave button
        auto button = addButton( IconEngine::get( IconNames::Save ), tr( "Save Again" ) );
        connect( button, &QAbstractButton::clicked, this, &FileRemovedWidget::_reSave );
        connect( button, &QAbstractButton::clicked, this, &MessageWidget::animatedHide );
        button->setToolTip( tr( "Save file again. Disc modifications will be lost" ) );
    }

    {
        // save as button
        auto button = addButton( IconEngine::get( IconNames::SaveAs ), tr( "Save As" ) );
        connect( button, &QAbstractButton::clicked, this, &FileRemovedWidget::_saveAs );
        connect( button, &QAbstractButton::clicked, this, &MessageWidget::animatedHide );
        button->setToolTip( tr( "Save file with a different name" ) );
    }

    {
        // close button.
        auto button = addButton( IconEngine::get( IconNames::DialogClose ), tr( "Close" ) );
        connect( button, &QAbstractButton::clicked, this, &FileRemovedWidget::_close );
        connect( button, &QAbstractButton::clicked, this, &MessageWidget::animatedHide );
        button->setToolTip( tr( "Close window" ) );
    }

    {
        // ignore button.
        auto button = addButton( IconEngine::get( IconNames::DialogCancel ), tr( "Ignore" ) );
        connect( button, &QAbstractButton::clicked, this, &FileRemovedWidget::_ignore );
        connect( button, &QAbstractButton::clicked, this, &MessageWidget::animatedHide );
        button->setToolTip( tr( "Ignore warning" ) );
    }

    adjustSize();

    // delete on hide
    connect( this, &MessageWidget::hideAnimationFinished, this, &QObject::deleteLater );

}

//________________________________________________________
void FileRemovedWidget::setFile( const File& file )
{ setText( tr( "The file '%1' has been deleted by another application." ).arg( file.localName() ) ); }

//________________________________________________________
void FileRemovedWidget::_reSave()
{ emit actionSelected( ReturnCode::SaveAgain ); }

//________________________________________________________
void FileRemovedWidget::_saveAs()
{ emit actionSelected( ReturnCode::SaveAs ); }

//________________________________________________________
void FileRemovedWidget::_close()
{ emit actionSelected( ReturnCode::Close ); }

//________________________________________________________
void FileRemovedWidget::_ignore()
{ emit actionSelected( ReturnCode::Ignore ); }
