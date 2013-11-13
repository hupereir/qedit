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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "FileModifiedDialog.h"
#include "FileModifiedDialog.moc"
#include "Icons.h"
#include "IconEngine.h"
#include "QtUtil.h"

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
FileModifiedDialog::FileModifiedDialog( QWidget* parent, const File& file ):
BaseDialog( parent ),
Counter( "FileModifiedDialog" )
{

    Debug::Throw( "FileModifiedDialog::FileModifiedDialog.\n" );

    setWindowTitle( tr( "File Modified - Qedit" ) );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout();
    layout->setSpacing(5);
    layout->setMargin(10);
    setLayout( layout );

    // create message
    QString buffer;
    QTextStream( &buffer ) << file.localName() << " has been modified by another application." << endl;

    //! try load Question icon
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
    button_layout->setMargin(0);
    button_layout->setSpacing( 5 );
    layout->addLayout( button_layout );
    button_layout->addStretch( 1 );

    // reload button.
    QPushButton* button;
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::RELOAD ), "&Reload", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _reLoad() ) );
    button->setToolTip( "Reload file from disc. Modifications will be lost" );

    // resave button
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::SAVE ), "&Save Again", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _reSave() ) );
    button->setToolTip( "Save file again. Disc modifications will be lost" );

    // save as button
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::SAVE_AS ), "&Save As", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _saveAs() ) );
    button->setToolTip( "Save file with a different name" );

    // ignore button.
    button_layout->addWidget( button = new QPushButton(  IconEngine::get( ICONS::DIALOG_CANCEL ), "&Ignore", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _ignore() ) );
    button->setToolTip( "Ignore warning" );

    adjustSize();

}
