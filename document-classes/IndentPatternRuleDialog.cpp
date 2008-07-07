
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
  \file IndentPatternRuleDialog.cpp
  \brief Syntax highlighting parenthesis editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>
#include <QLabel>

#include "CustomGridLayout.h"
#include "LineEditor.h"
#include "IndentPatternRuleDialog.h"

/* One should add validity checking */

using namespace std;

//________________________________________________________________________
IndentPatternRuleDialog::IndentPatternRuleDialog( QWidget* parent ):
  CustomDialog( parent )
{
  
  Debug::Throw( "IndentPatternRuleDialog::IndentPatternRuleDialog.\n" );
  
  mainLayout().setSpacing(5);
  
  CustomGridLayout* grid_layout( new CustomGridLayout() );
  grid_layout->setSpacing( 5 );
  grid_layout->setMargin( 0 );
  grid_layout->setMaxCount( 2 );
  mainLayout().addLayout( grid_layout );
 
  //
  grid_layout->addWidget( new QLabel( "Regular expression to match: ", this ) );
  grid_layout->addWidget( regexp_editor_ = new LineEditor( this ) );
 
  grid_layout->addWidget( new QLabel( "Paragraph to match (befor current): ", this ) );
  grid_layout->addWidget( paragraph_spinbox_ = new QSpinBox( this ) );
  paragraph_spinbox_->setMinimum( -10 );
  paragraph_spinbox_->setMaximum( 0 );
  paragraph_spinbox_->setValue( 0 );
  
  mainLayout().addWidget( case_checkbox_ = new QCheckBox( "Case sensitivity", this ) );
  case_checkbox_->setChecked( true );
  
}

//________________________________________________________________________
void IndentPatternRuleDialog::setRule( const IndentPattern::Rule& rule )
{ 
  Debug::Throw( "IndentPatternRuleDialog::setRule.\n" );
  
  regexp_editor_->setText( rule.pattern().pattern() );
  paragraph_spinbox_->setValue( rule.paragraph() );
  case_checkbox_->setChecked( !rule.flag( IndentPattern::Rule::CASE_INSENSITIVE ) );
  return;
}

//________________________________________________________________________
IndentPattern::Rule IndentPatternRuleDialog::rule( void )
{ 
  
  Debug::Throw( "IndentPatternRuleDialog::parenthesis.\n" );
  IndentPattern::Rule out;
  out.setPattern( regexp_editor_->text() );
  out.setParagraph( paragraph_spinbox_->value() );
  out.setFlag( IndentPattern::Rule::CASE_INSENSITIVE, !case_checkbox_->isChecked() );
    
  return out;
  
}
