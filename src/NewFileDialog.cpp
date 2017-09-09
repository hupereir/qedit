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

#include "NewFileDialog.h"

#include "IconEngine.h"
#include "IconNames.h"
#include "QtUtil.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
NewFileDialog::NewFileDialog( QWidget* parent, const File& file, ReturnCodes buttons ):
    BaseDialog( parent ),
    Counter( "NewFileDialog" )
{

    Debug::Throw( "NewFileDialog::NewFileDialog.\n" );

    setWindowTitle( tr( "File Not Found" ) );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout;
    layout->setSpacing(5);
    layout->setMargin(10);
    setLayout( layout );

    // create message
    const QString buffer = tr( "Cannot open file '%1'.\nNo such file or directory" ).arg( file );

    QHBoxLayout *hLayout( new QHBoxLayout );
    layout->addLayout( hLayout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( IconNames::DialogWarning ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter );

    // horizontal separator
    QFrame* frame( new QFrame( this ) );
    frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( frame );

    // button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(5);
    buttonLayout->setMargin(0);
    layout->addLayout( buttonLayout );

    buttonLayout->addStretch(1);

    // yes button
    QPushButton* button;
    if( buttons & Create )
    {
        buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogAccept ), tr( "Create" ), this ) );
        connect( button, SIGNAL(clicked()), SLOT(_create()) );
    }

    // cancel button.
    if( buttons & Cancel )
    {
        buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogCancel ), tr( "Cancel" ), this ) );
        button->setShortcut( Qt::Key_Escape );
        connect( button, SIGNAL(clicked()), SLOT(_cancel()) );
    }

    // cancel button.
    if( buttons & Exit )
    {
        buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogClose ), tr( "Exit" ), this ) );
        connect( button, SIGNAL(clicked()), SLOT(_exit()) );
    }

    adjustSize();

}
