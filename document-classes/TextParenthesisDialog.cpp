
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

/*!
\file TextParenthesisDialog.cpp
\brief Syntax highlighting parenthesis editing dialog
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "GridLayout.h"
#include "AnimatedLineEditor.h"
#include "TextParenthesisDialog.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>

//________________________________________________________________________
TextParenthesisDialog::TextParenthesisDialog( QWidget* parent ):
    CustomDialog( parent, OkButton|CancelButton|CustomDialog::Separator )
{

    Debug::Throw( "TextParenthesisDialog::TextParenthesisDialog.\n" );
    setWindowTitle( "Text Parenthesis Settings - Qedit" );

    mainLayout().setSpacing(5);

    GridLayout* gridLayout( new GridLayout() );
    gridLayout->setSpacing( 5 );
    gridLayout->setMargin( 0 );
    gridLayout->setMaxCount( 2 );
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    mainLayout().addLayout( gridLayout );

    //
    gridLayout->addWidget( new QLabel( "First parenthesis string: ", this ) );
    gridLayout->addWidget( first_editor_ = new AnimatedLineEditor( this ) );

    gridLayout->addWidget( new QLabel( "Second parenthesis string: ", this ) );
    gridLayout->addWidget( second_editor_ = new AnimatedLineEditor( this ) );

    gridLayout->addWidget( new QLabel( "Regular expression to match both parenthesis: ", this ) );
    gridLayout->addWidget( regexp_editor_ = new AnimatedLineEditor( this ) );

}

//________________________________________________________________________
void TextParenthesisDialog::setParenthesis( const TextParenthesis& parenthesis )
{
    Debug::Throw( "TextParenthesisDialog::setParenthesis.\n" );

    first_editor_->setText( parenthesis.first() );
    second_editor_->setText( parenthesis.second() );
    regexp_editor_->setText( parenthesis.regexp().pattern() );
    return;
}

//________________________________________________________________________
TextParenthesis TextParenthesisDialog::parenthesis( void )
{

    Debug::Throw( "TextParenthesisDialog::parenthesis.\n" );
    TextParenthesis out;
    out.setFirst( first_editor_->text() );
    out.setSecond( second_editor_->text() );
    out.setRegexp( regexp_editor_->text() );

    return out;

}
