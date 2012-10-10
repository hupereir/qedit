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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "CloseFilesDialog.h"

#include "Debug.h"
#include "FileRecordProperties.h"
#include "Icons.h"
#include "IconEngine.h"

#include <QtGui/QLabel>

//__________________________________________________
CloseFilesDialog::CloseFilesDialog( QWidget* parent, FileRecord::List files ):
CustomDialog( parent, OkButton | CancelButton| Separator )
{

    Debug::Throw( "CloseFilesDialog::CloseFilesDialog.\n" );

    setWindowTitle( "Close Files - Qedit" );

    QHBoxLayout *hLayout( new QHBoxLayout() );
    hLayout->setSpacing(5);
    hLayout->setMargin( 0 );
    mainLayout().addLayout( hLayout );

    // add icon
    QLabel *label( new QLabel( this ) );
    label->setPixmap( IconEngine::get( ICONS::WARNING ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );

    // create label text
    static const unsigned int maxLineSize( 50 );
    unsigned int currentLine( 0 );
    QString buffer;
    QTextStream what( &buffer );
    what << "Editing: ";

    int index(0);
    foreach( const FileRecord& record, files )
    {
        what << record.file().localName();
        if( record.hasFlag( FileRecordProperties::MODIFIED ) ) what << "*";
        if( index < files.size()-2 ) what << ", ";
        else if( index == files.size()-2 ) what << " and ";
        else what << ".";

        if( buffer.size() >= int((currentLine+1)*maxLineSize) )
        {
            what << endl;
            currentLine++;
        }

        ++index;

    }

    what << endl << "Close ?";
    hLayout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter );

    // rename buttons
    okButton().setText( "Close" );
    okButton().setIcon( IconEngine::get( ICONS::DIALOG_CLOSE ) );
    adjustSize();
}

