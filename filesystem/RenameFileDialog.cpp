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

#include "Debug.h"
#include "LineEditor.h"
#include "RenameFileDialog.h"

#include <QLabel>

//_____________________________________________________
RenameFileDialog::RenameFileDialog( QWidget* parent, FileRecord record ):
    CustomDialog( parent )
{

    Debug::Throw( "RenameFileDialog::RenameFileDialog.\n" );

    QString buffer = tr( "Rename the item '%1' to:" ).arg( record.file() );
    mainLayout().addWidget( new QLabel( buffer, this ) );
    mainLayout().addWidget( editor_ = new LineEditor( this ) );

    // set editor text
    _editor().setText( record.file() );
    _editor().setFocus();

    // get short name and select
    File shortFile( record.file().truncatedName() );
    if( !shortFile.isEmpty() ) _editor().setSelection( 0, shortFile.get().size() );
    else _editor().selectAll();
    connect( &_editor(), SIGNAL(textChanged(QString)), SLOT(_updateButtons()) );

    // rename buttons
    okButton().setText( tr( "Rename" ) );

    setMinimumSize( QSize( 320, 0 ) );

}

//_____________________________________________________
File RenameFileDialog::file() const
{

    QString text( _editor().text() );
    return ( text.isEmpty() || text.isNull() ) ? File():File( text );

}

//_____________________________________________________
void RenameFileDialog::_updateButtons()
{

    Debug::Throw( "RenameFileDialog::_updateButtons.\n" );
    okButton().setEnabled( !_editor().text().isEmpty() );

}
