
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

#include "AskForSaveDialog.h"
#include "AskForSaveDialog.moc"
#include "BaseIconNames.h"
#include "GridLayout.h"
#include "IconEngine.h"
#include "QtUtil.h"

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
AskForSaveDialog::AskForSaveDialog( QWidget* parent, const File& file, ReturnCodes buttons ):
    BaseDialog( parent ),
    Counter( "AskForSaveDialog" )
{

    Debug::Throw( "AskForSaveDialog::AskForSaveDialog.\n" );

    setWindowTitle( tr( "File Modified - Qedit" ) );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout();
    layout->setSpacing(5);
    layout->setMargin(10);
    setLayout( layout );

    // create message
    QString buffer = file.QString::isEmpty() ? tr( "File has been modified.\nSave ?" ):QString( tr( "File '%1' has been modified.\nSave ?" ) ).arg( file.localName() );

    // insert main vertical box
    QHBoxLayout *hLayout( new QHBoxLayout() );
    layout->addLayout( hLayout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( IconNames::Warning ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter );

    QFrame* frame( new QFrame( this ) );
    frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( frame );

    // button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(5);
    buttonLayout->setMargin(0);
    layout->addLayout( buttonLayout );

    buttonLayout->addStretch( 1 );

    // yes button
    QPushButton* button;
    if( buttons & Yes )
    {
        buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogOk ), tr( "Yes" ), this ) );
        connect( button, SIGNAL(clicked()), SLOT(_yes()) );
        button->setToolTip( tr( "Save modified file to disk" ) );
    }

    // yes to all button
    if( buttons & YesToAll )
    {
        buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogAccept ), tr( "Yes to All" ), this ) );
        connect( button, SIGNAL(clicked()), SLOT(_yesToAll()) );
        button->setToolTip( tr( "Save all modified files to disk" ) );
    }

    // no button
    if( buttons & No )
    {
        buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogClose ), tr( "No" ), this ) );
        connect( button, SIGNAL(clicked()), SLOT(_no()) );
        button->setToolTip( tr( "Ignore file modifications" ) );
    }

    // no button
    if( buttons & NoToAll )
    {
        buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogClose ), tr( "No to All" ), this ) );
        connect( button, SIGNAL(clicked()), SLOT(_noToAll()) );
        button->setToolTip( tr( "Ignore all files modifications" ) );
    }

    // cancel button
    if( buttons & Cancel )
    {
        buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogCancel ), tr( "Cancel" ), this ) );
        button->setShortcut( Qt::Key_Escape );
        connect( button, SIGNAL(clicked()), SLOT(_cancel()) );
        button->setToolTip( tr( "Cancel current action" ) );
    }

    adjustSize();

}
