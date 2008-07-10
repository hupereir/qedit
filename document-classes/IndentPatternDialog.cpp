
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
  \file IndentPatternDialog.cpp
  \brief Syntax highlighting pattern editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>
#include <QLabel>

#include "CustomGridLayout.h"
#include "LineEditor.h"
#include "IndentPatternDialog.h"
#include "IndentPatternRuleList.h"
#include "IndentPatternType.h"
#include "QtUtil.h"

using namespace std;

//________________________________________________________________________
IndentPatternDialog::IndentPatternDialog( QWidget* parent ):
  CustomDialog( parent )
{
  
  Debug::Throw( "IndentPatternDialog::IndentPatternDialog.\n" );
  
  mainLayout().setSpacing(5);
  
  // name 
  CustomGridLayout* grid_layout( new CustomGridLayout() );
  grid_layout->setSpacing( 5 );
  grid_layout->setMargin( 0 );
  grid_layout->setMaxCount( 2 );
  mainLayout().addLayout( grid_layout );
 
  grid_layout->addWidget( new QLabel( "Name: ", this ) );
  grid_layout->addWidget( name_editor_ = new LineEditor( this ) );
 
  // type
  grid_layout->addWidget( new QLabel( "Type: ", this ) );
  grid_layout->addWidget( pattern_type_ = new IndentPatternType( this ) );  
  
  // scale
  grid_layout->addWidget( new QLabel( "Scale: ", this ) );
  grid_layout->addWidget( scale_spinbox_ = new QSpinBox( this ) );  
  scale_spinbox_->setMinimum(0);
  scale_spinbox_->setMaximum(10);
  scale_spinbox_->setValue(0);
  
  // comments
  mainLayout().addWidget( list_ = new IndentPatternRuleList( this ) );
    
}

//________________________________________________________________________
void IndentPatternDialog::setPattern( const IndentPattern& pattern )
{ 
  Debug::Throw( "IndentPatternDialog::setPattern.\n" );
  
  pattern_ = pattern;
  
  name_editor_->setText( pattern_.name() );
  pattern_type_->setType( pattern_.type() );
  scale_spinbox_->setValue( pattern_.scale() );
  list_->setRules( pattern_.rules() );  
  return;
}

//________________________________________________________________________
IndentPattern IndentPatternDialog::pattern( void )
{ 
  
  Debug::Throw( "IndentPatternDialog::pattern.\n" );
  pattern_.setName( name_editor_->text() );
  pattern_.setScale( scale_spinbox_->value() );
  pattern_.setType( pattern_type_->type() );
  pattern_.setRules( list_->rules() );
  
  return pattern_;
  
}