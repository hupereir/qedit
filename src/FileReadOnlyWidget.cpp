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

#include "FileReadOnlyWidget.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "QtUtil.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
FileReadOnlyWidget::FileReadOnlyWidget( QWidget* parent, const File& file ):
    MessageWidget( parent, MessageType::Information )
{

    Debug::Throw( QStringLiteral("FileReadOnlyWidget::FileReadOnlyWidget.\n") );

    setFile( file );
    addDefaultCloseButton();

    // delete on hide
    connect( this, &MessageWidget::hideAnimationFinished, this, &QObject::deleteLater );

}

//________________________________________________________
void FileReadOnlyWidget::setFile( const File& file )
{ setText( tr( "The file '%1' is open in read-only mode." ).arg( file.localName() ) ); }
