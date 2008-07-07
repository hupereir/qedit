
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
  \file TextMacroRuleDialog.cpp
  \brief Syntax highlighting parenthesis editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>
#include <QLabel>

#include "CustomGridLayout.h"
#include "LineEditor.h"
#include "TextMacroRuleDialog.h"

/* One should add validity checking */

using namespace std;

//________________________________________________________________________
TextMacroRuleDialog::TextMacroRuleDialog( QWidget* parent ):
  CustomDialog( parent )
{
  
  Debug::Throw( "TextMacroRuleDialog::TextMacroRuleDialog.\n" );
  
  mainLayout().setSpacing(5);
  
  CustomGridLayout* grid_layout( new CustomGridLayout() );
  grid_layout->setSpacing( 5 );
  grid_layout->setMargin( 0 );
  grid_layout->setMaxCount( 2 );
  mainLayout().addLayout( grid_layout );
 
  //
  grid_layout->addWidget( new QLabel( "Regular expression to match: ", this ) );
  grid_layout->addWidget( regexp_editor_ = new LineEditor( this ) );
 
  grid_layout->addWidget( new QLabel( "Replacement text: ", this ) );
  grid_layout->addWidget( text_editor_ = new LineEditor( this ) );

  mainLayout().addWidget( split_checkbox_ = new QCheckBox( "Perform replacement line by line", this ) );
  split_checkbox_->setChecked( true );
  
}

//________________________________________________________________________
void TextMacroRuleDialog::setRule( const TextMacro::Rule& rule )
{ 
  Debug::Throw( "TextMacroRuleDialog::setRule.\n" );
  
  regexp_editor_->setText( rule.pattern().pattern() );
  text_editor_->setText( rule.replaceText() );
  split_checkbox_->setChecked( rule.split() );
  return;
}

//________________________________________________________________________
TextMacro::Rule TextMacroRuleDialog::rule( void )
{ 
  
  Debug::Throw( "TextMacroRuleDialog::rule.\n" );
  TextMacro::Rule out;
  out.setPattern( regexp_editor_->text() );
  out.setReplaceText( text_editor_->text() );
  if( !split_checkbox_->isChecked() ) out.setNoSplitting( );
    
  return out;
  
}
