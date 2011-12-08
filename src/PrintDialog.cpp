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

#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "FileDialog.h"
#include "GridLayout.h"
#include "PrintDialog.h"
#include "QtUtil.h"

#include <QtGui/QFrame>
#include <QtGui/QLayout>
#include <QtGui/QToolButton>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>

//__________________________________________________
PrintDialog::PrintDialog( QWidget* parent ):
CustomDialog( parent, OkButton | CancelButton )
{

    Debug::Throw( "PrintDialog::PrintDialog.\n" );
    setWindowTitle( "Print Document - Qedit" );

    setOptionName( "PRINT_DIALOG" );

    QButtonGroup* group = new QButtonGroup( this );
    group->setExclusive( true );

    // destination
    QGroupBox *box;
    box = new QGroupBox( "Destination", this );
    mainLayout().addWidget( box );

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMaxCount( 3 );
    box->setLayout( gridLayout );

    // file
    QLabel* label;
    gridLayout->addWidget( label = new QLabel( "File: ", box ), 0, 0, 1, 1 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    gridLayout->addWidget( destinationEditor_ = new BrowsedLineEditor( box ), 0, 1, 1, 2 );
    destinationEditor_->setMinimumSize( QSize( 150, 0 ) );

    // format
    gridLayout->addWidget( label = new QLabel( "Output format: ", box ), 1, 0, 1, 1 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    gridLayout->addWidget( pdfCheckBox_ = new QRadioButton( "PDF", box ), 1, 1, 1, 1 );
    group->addButton( pdfCheckBox_ );
    pdfCheckBox_->setChecked( true );

    gridLayout->addWidget( htmlCheckBox_ = new QRadioButton( "HTML", box ), 1, 2, 1, 1 );
    group->addButton( htmlCheckBox_ );
    htmlCheckBox_->setChecked( false );

    gridLayout->setColumnStretch( 2, 1 );

    // options
    box = new QGroupBox( "Options", this );
    gridLayout = new GridLayout();
    gridLayout->setMaxCount(2);
    box->setLayout( gridLayout );
    mainLayout().addWidget( box );

    gridLayout->addWidget( wrapCheckBox_ = new QCheckBox( "Wrap lines to maximum size:", box ) );
    wrapCheckBox_->setChecked( true );

    gridLayout->addWidget( maximumLineSize_ = new QSpinBox( box ) );
    maximumLineSize_->setMaximum( 1024) ;

    gridLayout->addWidget( commandCheckBox_ = new QCheckBox( "Open/print with: ", box ) );
    commandCheckBox_->setChecked( true );

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(2);
    hLayout->setMargin(0);
    gridLayout->addLayout( hLayout );

    hLayout->addWidget( commandEditor_ = new CustomComboBox( box ) );
    commandEditor_->setEditable( true );
    commandEditor_->setAutoCompletion( true, Qt::CaseSensitive  );
    commandEditor_->setMinimumSize( QSize( 150, 0 ) );

    // browse command button associated to the CustomComboBox
    QToolButton* button = new QToolButton( box );
    button->setIcon( IconEngine::get( ICONS::OPEN ) );
    button->setAutoRaise( false );
    hLayout->addWidget( button );
    connect( button, SIGNAL( clicked() ), SLOT( _browseCommand() ) );

    // connections
    connect( pdfCheckBox_, SIGNAL( toggled( bool ) ), SLOT( _updateFile() ) );
    connect( htmlCheckBox_, SIGNAL( toggled( bool ) ), SLOT( _updateFile() ) );
    connect( wrapCheckBox_, SIGNAL( toggled( bool ) ), SLOT( _updateCheckBoxes() ) );
    connect( commandCheckBox_, SIGNAL( toggled( bool ) ), SLOT( _updateCheckBoxes() ) );

    _updateFile();
    _updateCheckBoxes();

    // change button text
    okButton().setText( "&Print" );
    okButton().setIcon( IconEngine::get( ICONS::PRINT ));
}

//__________________________________________________
void PrintDialog::setFile( const File& file )
{
    Debug::Throw( "PrintDialog::setFile.\n" );
    destinationEditor_->editor().setText( file );
    _updateFile();
}

//__________________________________________________
void PrintDialog::_updateCheckBoxes( void )
{

    Debug::Throw( "PrintDialog::_updateCheckBoxes.\n" );
    maximumLineSize_->setEnabled( wrapCheckBox_->isChecked() );
    commandEditor_->setEnabled( commandCheckBox_->isChecked() );

}

//__________________________________________________
void PrintDialog::_updateFile( void )
{

    Debug::Throw( "PrintDialog::_updateFile.\n" );

    File file( destinationEditor_->editor().text() );
    file = file.isEmpty() ? File("document"):file.truncatedName();
    if( pdfCheckBox_->isChecked() ) file += ".pdf";
    else if( htmlCheckBox_->isChecked() ) file += ".html";

    destinationEditor_->editor().setText( file );

}

//__________________________________________________
void PrintDialog::_browseCommand( void )
{

    Debug::Throw( "PrintDialog::_browseCommand.\n" );

    // open FileDialog
    QString file( FileDialog(this).getFile() );
    if( !file.isNull() ) {
        commandEditor_->setEditText( file );
        commandEditor_->addItem( file );
    }

    return;

}
