
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

#include "AskForSaveDialog.h"
#include "BaseIcons.h"
#include "GridLayout.h"
#include "IconEngine.h"
#include "QtUtil.h"

#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

//________________________________________________________
AskForSaveDialog::AskForSaveDialog( QWidget* parent, const File& file, const unsigned int& buttons ):
BaseDialog( parent ),
Counter( "AskForSaveDialog" )
{

    Debug::Throw( "AskForSaveDialog::AskForSaveDialog.\n" );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout();
    layout->setSpacing(5);
    layout->setMargin(10);
    setLayout( layout );

    // create message
    QString buffer;
    QTextStream what( &buffer );
    what << "File ";
    if( file.size() ) what << "\"" << file.localName() << "\" ";
    what << "has been modified." << endl << "Save ?";

    // insert main vertical box
    QHBoxLayout *hLayout( new QHBoxLayout() );
    layout->addLayout( hLayout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( ICONS::WARNING ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter );

    QFrame* frame( new QFrame( this ) );
    frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( frame );

    // button layout
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->setSpacing(5);
    button_layout->setMargin(0);
    layout->addLayout( button_layout );

    button_layout->addStretch( 1 );

    // yes button
    QPushButton* button;
    if( buttons & YES )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_OK ), "&Yes", this ) );
        connect( button, SIGNAL( clicked() ), SLOT( _yes() ) );
        button->setToolTip( "Save modified file to disk" );
    }

    // yes to all button
    if( buttons & YES_TO_ALL )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_OK_APPLY ), "Yes to &All", this ) );
        connect( button, SIGNAL( clicked() ), SLOT( _yesToAll() ) );
        button->setToolTip( "Save all modified files to disk" );
    }

    // no button
    if( buttons & NO )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&No", this ) );
        connect( button, SIGNAL( clicked() ), SLOT( _no() ) );
        button->setToolTip( "Ignore file modifications" );
    }

    // no button
    if( buttons & NO_TO_ALL )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "No to A&ll", this ) );
        connect( button, SIGNAL( clicked() ), SLOT( _noToAll() ) );
        button->setToolTip( "Ignore all files modifications" );
    }

    // cancel button
    if( buttons & CANCEL )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CANCEL ), "&Cancel", this ) );
        connect( button, SIGNAL( clicked() ), SLOT( _cancel() ) );
        button->setToolTip( "Cancel current action" );
    }

    adjustSize();

}
