
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

#include "TextMacroDialog.h"
#include "GridLayout.h"
#include "AnimatedLineEditor.h"
#include "TextMacroRuleList.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>

//________________________________________________________________________
TextMacroDialog::TextMacroDialog( QWidget* parent ):
CustomDialog( parent )
{

    Debug::Throw( "TextMacroDialog::TextMacroDialog.\n" );
    setWindowTitle( "Text Macro Settings - Qedit" );

    mainLayout().setSpacing(5);

    GridLayout* grid_layout( new GridLayout() );
    grid_layout->setSpacing( 5 );
    grid_layout->setMargin( 0 );
    grid_layout->setMaxCount( 2 );
    grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    mainLayout().addLayout( grid_layout );

    // name
    grid_layout->addWidget( new QLabel( "Name: ", this ) );
    grid_layout->addWidget( nameEditor_ = new AnimatedLineEditor( this ) );

    // accelerator
    grid_layout->addWidget( new QLabel( "Accelerator: ", this ) );
    grid_layout->addWidget( acceleratorEditor_ = new AnimatedLineEditor( this ) );

    // list
    mainLayout().addWidget( ruleList_ = new TextMacroRuleList( this ) );

}

//________________________________________________________________________
void TextMacroDialog::setMacro( const TextMacro& macro )
{
    Debug::Throw( "TextMacroDialog::setTextMacro.\n" );

    macro_ = macro;
    nameEditor_->setText( macro.name() );
    acceleratorEditor_->setText( macro.accelerator() );
    ruleList_->setRules( macro.rules() );
    return;
}

//________________________________________________________________________
TextMacro TextMacroDialog::macro( void )
{

    Debug::Throw( "TextMacroDialog::macro.\n" );
    macro_.setIsSeparator( false );
    macro_.setName( nameEditor_->text() );
    macro_.setAccelerator( acceleratorEditor_->text() );
    macro_.setRules( ruleList_->rules() );

    return macro_;

}
