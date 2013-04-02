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

#include "NewFileDialog.h"

#include "IconEngine.h"
#include "Icons.h"
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

    setWindowTitle( tr( "File Not Found - Qedit" ) );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout();
    layout->setSpacing(5);
    layout->setMargin(10);
    setLayout( layout );

    // create message
    const QString buffer = QString( tr( "Cannot open file '%1'.\nNo such file or directory" ) ).arg( file );

    QHBoxLayout *hLayout( new QHBoxLayout() );
    layout->addLayout( hLayout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( ICONS::WARNING ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter );

    // horizontal separator
    QFrame* frame( new QFrame( this ) );
    frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( frame );

    // button layout
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->setSpacing(5);
    button_layout->setMargin(0);
    layout->addLayout( button_layout );

    button_layout->addStretch(1);

    // yes button
    QPushButton* button;
    if( buttons & Create )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_ACCEPT ), tr( "Create" ), this ) );
        connect( button, SIGNAL( clicked() ), SLOT( _create() ) );
    }

    // cancel button.
    if( buttons & Cancel )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CANCEL ), tr( "Cancel" ), this ) );
        button->setShortcut( Qt::Key_Escape );
        connect( button, SIGNAL( clicked() ), SLOT( _cancel() ) );
    }

    // cancel button.
    if( buttons & Exit )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), tr( "Exit" ), this ) );
        connect( button, SIGNAL( clicked() ), SLOT( _exit() ) );
    }

    adjustSize();

}
