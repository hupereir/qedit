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
#include "IconEngine.h"
#include "IconNames.h"
#include "QtUtil.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
FileModifiedWidget::FileModifiedWidget( QWidget* parent, const File& file ):
    MessageWidget( parent, MessageType::Information )
{

    Debug::Throw( QStringLiteral("FileModifiedWidget::FileModifiedWidget.\n") );

    setFile( file );

    {
        // reload button
        auto button = addButton( IconEngine::get( IconNames::Reload ), QStringLiteral("&Reload") );
        connect( button, &QAbstractButton::clicked, this, &FileModifiedWidget::_reLoad );
        connect( button, &QAbstractButton::clicked, this, &MessageWidget::animatedHide );
        button->setToolTip( QStringLiteral("Reload file from disc. Modifications will be lost") );
    }

    {
        // resave button
        auto button = addButton( IconEngine::get( IconNames::Save ), QStringLiteral("&Save Again") );
        connect( button, &QAbstractButton::clicked, this, &FileModifiedWidget::_reSave );
        connect( button, &QAbstractButton::clicked, this, &MessageWidget::animatedHide );
        button->setToolTip( QStringLiteral("Save file again. Disc modifications will be lost") );
    }

    {
        // save as button
        auto button = addButton( IconEngine::get( IconNames::SaveAs ), QStringLiteral("&Save As") );
        connect( button, &QAbstractButton::clicked, this, &FileModifiedWidget::_saveAs );
        connect( button, &QAbstractButton::clicked, this, &MessageWidget::animatedHide );
        button->setToolTip( QStringLiteral("Save file with a different name") );
    }

    {
        // ignore button.
        auto button = addButton(  IconEngine::get( IconNames::DialogCancel ), QStringLiteral("&Ignore") );
        connect( button, &QAbstractButton::clicked, this, &FileModifiedWidget::_ignore );
        connect( button, &QAbstractButton::clicked, this, &MessageWidget::animatedHide );
        button->setToolTip( QStringLiteral("Ignore warning") );
    }

    adjustSize();

    // delete on hide
    connect( this, &MessageWidget::hideAnimationFinished, this, &QObject::deleteLater );

}

//________________________________________________________
void FileModifiedWidget::setFile( const File& file )
{ setText( tr( "The file '%1' has been modified by another application." ).arg( file.localName() ) ); }

//________________________________________________________
void FileModifiedWidget::_reLoad()
{ emit actionSelected( ReturnCode::Reload ); }

//________________________________________________________
void FileModifiedWidget::_reSave()
{ emit actionSelected( ReturnCode::SaveAgain ); }

//________________________________________________________
void FileModifiedWidget::_saveAs()
{ emit actionSelected( ReturnCode::SaveAs ); }

//________________________________________________________
void FileModifiedWidget::_ignore()
{ emit actionSelected( ReturnCode::Ignore ); }
