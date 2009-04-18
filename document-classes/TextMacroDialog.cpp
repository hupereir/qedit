
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
  \file TextMacroDialog.cpp
  \brief Syntax highlighting macro editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>
#include <QLabel>

#include "GridLayout.h"
#include "AnimatedLineEditor.h"
#include "TextMacroDialog.h"
#include "TextMacroRuleList.h"

/* One should add validity checking */

using namespace std;

//________________________________________________________________________
TextMacroDialog::TextMacroDialog( QWidget* parent ):
  CustomDialog( parent )
{
  
  Debug::Throw( "TextMacroDialog::TextMacroDialog.\n" );
  
  mainLayout().setSpacing(5);
  
  GridLayout* grid_layout( new GridLayout() );
  grid_layout->setSpacing( 5 );
  grid_layout->setMargin( 0 );
  grid_layout->setMaxCount( 2 );
  grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
  mainLayout().addLayout( grid_layout );
 
  // name
  grid_layout->addWidget( new QLabel( "Name: ", this ) );
  grid_layout->addWidget( name_editor_ = new AnimatedLineEditor( this ) );
 
  // accelerator
  grid_layout->addWidget( new QLabel( "Accelerator: ", this ) );
  grid_layout->addWidget( accelerator_editor_ = new AnimatedLineEditor( this ) );

  // list
  mainLayout().addWidget( rule_list_ = new TextMacroRuleList( this ) );
  
}

//________________________________________________________________________
void TextMacroDialog::setMacro( const TextMacro& macro )
{ 
  Debug::Throw( "TextMacroDialog::setTextMacro.\n" );
  
  macro_ = macro;
  name_editor_->setText( macro.name() );
  accelerator_editor_->setText( macro.accelerator() );
  rule_list_->setRules( macro.rules() );
  return;
}

//________________________________________________________________________
TextMacro TextMacroDialog::macro( void )
{ 
  
  Debug::Throw( "TextMacroDialog::macro.\n" );
  macro_.setIsSeparator( false );
  macro_.setName( name_editor_->text() );
  macro_.setAccelerator( accelerator_editor_->text() );
  macro_.setRules( rule_list_->rules() );
    
  return macro_;
  
}
