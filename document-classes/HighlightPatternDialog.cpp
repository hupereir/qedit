
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

#include "GridLayout.h"
#include "AnimatedLineEditor.h"
#include "TextEditor.h"
#include "HighlightPatternDialog.h"
#include "HighlightPatternOptions.h"
#include "HighlightPatternType.h"
#include "InformationDialog.h"

using namespace std;

//________________________________________________________________________
HighlightPatternDialog::HighlightPatternDialog( QWidget* parent ):
  CustomDialog( parent )
{

  Debug::Throw( "HighlightPatternDialog::HighlightPatternDialog.\n" );
  setWindowTitle( "Highlight Pattern settings - qedit" );

  mainLayout().setSpacing(5);

  // name
  GridLayout* grid_layout( new GridLayout() );
  grid_layout->setSpacing( 5 );
  grid_layout->setMargin( 0 );
  grid_layout->setMaxCount( 2 );
  grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
  mainLayout().addLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "Name: ", this ) );
  grid_layout->addWidget( name_editor_ = new AnimatedLineEditor( this ) );

  // parent
  grid_layout->addWidget( new QLabel( "Parent pattern: ", this ) );
  grid_layout->addWidget( parent_combobox_ = new QComboBox( this ) );
  parent_combobox_->setEditable( false );
  parent_combobox_->addItem( HighlightPattern::no_parent_pattern_ );

  // styles
  grid_layout->addWidget( new QLabel( "Highlight style: ", this ) );
  grid_layout->addWidget( style_combobox_ = new QComboBox( this ) );
  style_combobox_->setEditable( false );

  // type
  grid_layout->addWidget( new QLabel( "Type: ", this ) );
  grid_layout->addWidget( pattern_type_ = new HighlightPatternType( this ) );

  // regular expressions
  grid_layout->addWidget( new QLabel( "Regular expression to match: ", this ) );
  grid_layout->addWidget( keyword_regexp_editor_ = new AnimatedLineEditor( this ) );

  grid_layout->addWidget( end_regexp_label_ = new QLabel( "Ending regular expression: ", this ) );
  grid_layout->addWidget( end_regexp_editor_ = new AnimatedLineEditor( this ) );

  // options
  mainLayout().addWidget( pattern_options_ = new HighlightPatternOptions( this ) );
  connect( pattern_type_, SIGNAL( typeChanged( HighlightPattern::Type ) ), SLOT( _updateEditors( HighlightPattern::Type ) ) );

  // comments
  mainLayout().addWidget( new QLabel( "Comments: ", this ) );
  mainLayout().addWidget( comments_editor_ = new TextEditor( this ) );

  _updateEditors( pattern_type_->type() );

}

//________________________________________________________________________
void HighlightPatternDialog::setPatterns( const HighlightPattern::List& patterns )
{
  Debug::Throw( "HighlightPatternDialog::setPatterns.\n" );

  // update parent_combobox
  parent_combobox_->clear();
  parent_combobox_->addItem( HighlightPattern::no_parent_pattern_ );
  for( HighlightPattern::List::const_iterator iter = patterns.begin(); iter != patterns.end(); iter++ )
  { parent_combobox_->addItem( iter->name() ); }

  //! select default parent pattern
  parent_combobox_->setCurrentIndex( parent_combobox_->findText( HighlightPattern::no_parent_pattern_ ) );
}

//________________________________________________________________________
void HighlightPatternDialog::setStyles( const HighlightStyle::Set& styles )
{
  Debug::Throw( "HighlightPatternDialog::setStyles.\n" );

  styles_ = styles;

  // update style_combobox
  style_combobox_->clear();
  for( HighlightStyle::Set::const_iterator iter = styles_.begin(); iter != styles_.end(); iter++ )
  { style_combobox_->addItem( iter->name() ); }

  return;

}

//________________________________________________________________________
void HighlightPatternDialog::setPattern( const HighlightPattern& pattern )
{
  Debug::Throw( "HighlightPatternDialog::setPattern.\n" );

  pattern_ = pattern;

  name_editor_->setText( pattern_.name() );
  pattern_type_->setType( pattern_.type() );
  pattern_options_->setOptions( pattern_.flags() );

  // regular expressions
  keyword_regexp_editor_->setText( pattern_.keyword().pattern() );

  if( pattern.type() == HighlightPattern::RANGE_PATTERN )
  { end_regexp_editor_->setText( pattern_.end().pattern() ); }

  // style
  style_combobox_->setCurrentIndex( style_combobox_->findText( pattern_.style().name() ) );

  // parent
  if( pattern.parent().isEmpty() ) { parent_combobox_->setCurrentIndex( parent_combobox_->findText( HighlightPattern::no_parent_pattern_ ) ); }
  else { parent_combobox_->setCurrentIndex( parent_combobox_->findText( pattern_.parent() ) ); }

  // comments
  comments_editor_->setPlainText( pattern.comments() );

  // editors enability
  _updateEditors( pattern_type_->type() );

  return;
}

//________________________________________________________________________
HighlightPattern HighlightPatternDialog::pattern( void )
{

  Debug::Throw( "HighlightPatternDialog::pattern.\n" );
  pattern_.setName( name_editor_->text() );

  QString parent( parent_combobox_->itemText( parent_combobox_->currentIndex() ) );
  pattern_.setParent( parent == HighlightPattern::no_parent_pattern_ ? "":parent );

  // style
  set<HighlightStyle>::iterator style_iter ( styles_.find( HighlightStyle( style_combobox_->itemText( style_combobox_->currentIndex() ) ) ) );
  if( style_iter != styles_.end() ) pattern_.setStyle( *style_iter );
  else InformationDialog( this, "invalid style name" ).exec();

  pattern_.setFlags( pattern_options_->options() );
  pattern_.setType( pattern_type_->type() );

  pattern_.setKeyword( keyword_regexp_editor_->text() );
  if( pattern_.type() == HighlightPattern::RANGE_PATTERN )
  { pattern_.setEnd( end_regexp_editor_->text() ); }

  pattern_.setComments( comments_editor_->toPlainText() );

  return pattern_;

}

//________________________________________________________________________
void HighlightPatternDialog::_updateEditors( HighlightPattern::Type type )
{
  Debug::Throw( "HighlightPatternDialog::_updateEditors.\n" );
  end_regexp_label_->setEnabled( type == HighlightPattern::RANGE_PATTERN );
  end_regexp_editor_->setEnabled( type == HighlightPattern::RANGE_PATTERN );
}
