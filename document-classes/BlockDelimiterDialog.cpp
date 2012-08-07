
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

#include "GridLayout.h"
#include "AnimatedLineEditor.h"
#include "BlockDelimiterDialog.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>

//________________________________________________________________________
BlockDelimiterDialog::BlockDelimiterDialog( QWidget* parent ):
CustomDialog( parent, OkButton|CancelButton|CustomDialog::Separator )
{

    Debug::Throw( "BlockDelimiterDialog::BlockDelimiterDialog.\n" );

    setWindowTitle( "Block Delimiter Settings - Qedit" );
    mainLayout().setSpacing(5);

    GridLayout* grid_layout( new GridLayout() );
    grid_layout->setSpacing( 5 );
    grid_layout->setMargin( 0 );
    grid_layout->setMaxCount( 2 );
    grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    mainLayout().addLayout( grid_layout );

    //
    grid_layout->addWidget( new QLabel( "First delimiter string: ", this ) );
    grid_layout->addWidget( firstEditor_ = new AnimatedLineEditor( this ) );

    grid_layout->addWidget( new QLabel( "Second delimiter string: ", this ) );
    grid_layout->addWidget( secondEditor_ = new AnimatedLineEditor( this ) );

    grid_layout->addWidget( new QLabel( "Regular expression to match both delimiters: ", this ) );
    grid_layout->addWidget( regexpEditor_ = new AnimatedLineEditor( this ) );

}

//________________________________________________________________________
void BlockDelimiterDialog::setDelimiter( const BlockDelimiter& delimiter )
{
    Debug::Throw( "BlockDelimiterDialog::setDelimiter.\n" );

    firstEditor_->setText( delimiter.first() );
    secondEditor_->setText( delimiter.second() );
    regexpEditor_->setText( delimiter.regexp().pattern() );
    return;
}

//________________________________________________________________________
BlockDelimiter BlockDelimiterDialog::delimiter( void )
{

    Debug::Throw( "BlockDelimiterDialog::delimiter.\n" );
    BlockDelimiter out;
    out.setFirst( firstEditor_->text() );
    out.setSecond( secondEditor_->text() );
    out.setRegexp( regexpEditor_->text() );

    return out;

}
