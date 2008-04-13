
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
  \file HighlightPatternDialog.cpp
  \brief Syntax highlighting pattern editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>
#include <QLabel>

#include "HighlightPatternDialog.h"
#include "CustomGridLayout.h"
#include "CustomLineEdit.h"
#include "HighlightPatternOptions.h"
#include "HighlightPatternType.h"
#include "QtUtil.h"

using namespace std;

//________________________________________________________________________
std::string HighlightPatternDialog::no_parent_pattern_ = "None";

//________________________________________________________________________
HighlightPatternDialog::HighlightPatternDialog( QWidget* parent ):
  CustomDialog( parent )
{
  
  Debug::Throw( "HighlightPatternDialog::HighlightPatternDialog.\n" );
  
  // name 
  CustomGridLayout* grid_layout( new CustomGridLayout() );
  grid_layout->setSpacing( 5 );
  grid_layout->setMargin( 0 );
  grid_layout->setMaxCount( 2 );
  mainLayout().addLayout( grid_layout );
 
  grid_layout->addWidget( new QLabel( "Name: ", this ) );
  grid_layout->addWidget( name_editor_ = new CustomLineEdit( this ) );
 
  // parent
  grid_layout->addWidget( new QLabel( "Parent pattern: ", this ) );
  grid_layout->addWidget( parent_combobox_ = new QComboBox( this ) );
  parent_combobox_->setEditable( false );
  parent_combobox_->addItem( no_parent_pattern_.c_str() );
   
  // styles
  grid_layout->addWidget( new QLabel( "Highlight style: ", this ) );
  grid_layout->addWidget( style_combobox_ = new QComboBox( this ) );
  style_combobox_->setEditable( false );

  // options and type
  mainLayout().addWidget( pattern_options_ = new HighlightPatternOptions( this ) );
  mainLayout().addWidget( pattern_type_ = new HighlightPatternType( this ) );  
  connect( pattern_type_, SIGNAL( typeChanged( HighlightPattern::Type ) ), SLOT( _updateEditors( HighlightPattern::Type ) ) );
  
  // regular expression
  QGroupBox* box = new QGroupBox( "Regular expressions", this );
  mainLayout().addWidget( box );
  grid_layout = new CustomGridLayout();
  grid_layout->setSpacing( 5 );
  grid_layout->setMargin( 5 );
  grid_layout->setMaxCount( 2 );
  box->setLayout( grid_layout );  
  
  grid_layout->addWidget( new QLabel( "Regular expression to match: ", this ) );
  grid_layout->addWidget( keyword_regexp_editor_ = new CustomLineEdit( this ) ); 

  grid_layout->addWidget( end_regexp_label_ = new QLabel( "Ending regular expression: ", this ) );
  grid_layout->addWidget( end_regexp_editor_ = new CustomLineEdit( this ) ); 
  
  _updateEditors( pattern_type_->type() );
  
}

//________________________________________________________________________
void HighlightPatternDialog::setPatterns( const HighlightPattern::List& patterns )
{ 
  Debug::Throw( "HighlightPatternDialog::setPatterns.\n" );
  
  // update parent_combobox
  parent_combobox_->clear();
  parent_combobox_->addItem( no_parent_pattern_.c_str() );
  for( HighlightPattern::List::const_iterator iter = patterns.begin(); iter != patterns.end(); iter++ )
  { parent_combobox_->addItem( iter->name().c_str() ); }
  
  //! select default parent pattern
  parent_combobox_->setCurrentIndex( parent_combobox_->findText( no_parent_pattern_.c_str() ) );
}

//________________________________________________________________________
void HighlightPatternDialog::setStyles( const HighlightStyle::Set& styles )
{ 
  Debug::Throw( "HighlightPatternDialog::setStyles.\n" );
 
  styles_ = styles;
  
  // update style_combobox
  style_combobox_->clear();
  for( HighlightStyle::Set::const_iterator iter = styles_.begin(); iter != styles_.end(); iter++ )
  { style_combobox_->addItem( iter->name().c_str() ); }

  return;
  
}


//________________________________________________________________________
void HighlightPatternDialog::setPattern( const HighlightPattern& pattern )
{ 
  Debug::Throw( "HighlightPatternDialog::setPattern.\n" );
  
  pattern_ = pattern;
  
  name_editor_->setText( pattern_.name().c_str() );
  pattern_type_->setType( pattern_.type() );
  pattern_options_->setOptions( pattern_.flags() );
  
  // regular expressions
  keyword_regexp_editor_->setText( pattern_.keyword().pattern() );
  
  if( pattern.type() == HighlightPattern::RANGE_PATTERN )
  { end_regexp_editor_->setText( pattern_.end().pattern() ); }
    
  // style
  style_combobox_->setCurrentIndex( style_combobox_->findText( pattern_.style().name().c_str() ) );
  
  // parent
  if( pattern.parent().empty() ) { parent_combobox_->setCurrentIndex( parent_combobox_->findText( no_parent_pattern_.c_str() ) ); }
  else { parent_combobox_->setCurrentIndex( parent_combobox_->findText( pattern_.parent().c_str() ) ); }

  _updateEditors( pattern_type_->type() );
  
  return;
}

//________________________________________________________________________
HighlightPattern HighlightPatternDialog::pattern( void )
{ 
  
  Debug::Throw( "HighlightPatternDialog::pattern.\n" );
  pattern_.setName( qPrintable( name_editor_->text() ) );
  pattern_.setParent( qPrintable( parent_combobox_->itemText( parent_combobox_->currentIndex() ) ) );

  // style
  set<HighlightStyle>::iterator style_iter ( styles_.find( HighlightStyle( qPrintable( style_combobox_->itemText( style_combobox_->currentIndex() ) ) ) ) );
  if( style_iter != styles_.end() ) pattern_.setStyle( *style_iter );
  else QtUtil::infoDialog( this, "invalid style name" );
  
  pattern_.setFlags( pattern_options_->options() );
  pattern_.setType( pattern_type_->type() );
  
  pattern_.setKeyword( qPrintable( keyword_regexp_editor_->text() ) );
  if( pattern_.type() == HighlightPattern::RANGE_PATTERN ) 
  { pattern_.setEnd( qPrintable( end_regexp_editor_->text() ) ); }
  
  return pattern_;
  
}
  
//________________________________________________________________________
void HighlightPatternDialog::_updateEditors( HighlightPattern::Type type )
{
  Debug::Throw( "HighlightPatternDialog::_updateEditors.\n" );
  end_regexp_label_->setEnabled( type == HighlightPattern::RANGE_PATTERN );
  end_regexp_editor_->setEnabled( type == HighlightPattern::RANGE_PATTERN );
}
