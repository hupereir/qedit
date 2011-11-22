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
   \file HighlightPatternOptions.cpp
   \brief font formatting options
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QHBoxLayout>
#include <QLabel>

#include "Debug.h"
#include "HighlightPatternOptions.h"
#include "HighlightPattern.h"



//_____________________________________________
HighlightPatternOptions::HighlightPatternOptions( QWidget* parent ):
  QWidget( parent ),
  Counter( "HighlightPatternOptions" )
{
  Debug::Throw( "HighlightPatternOptions::HighlightPatternOptions.\n" );
  setLayout( new QHBoxLayout() );
  layout()->setMargin(0);
  layout()->setSpacing(5);
  layout()->addWidget( new QLabel( "Options: ", this ) );
  layout()->addWidget( span_ = new QCheckBox( "&Span", this ) );
  layout()->addWidget( no_indent_ = new QCheckBox( "&No indentation", this ) );
  layout()->addWidget( case_sensitive_ = new QCheckBox( "&Case sensitive", this ) );

  span_->setToolTip( "Highlight pattern spans over several paragraphs" );
  no_indent_->setToolTip(
    "Paragraphs matching the pattern are skipped from automatic indentation.\n"
    "This is typically the case for comments." );
  case_sensitive_->setToolTip( "Highlight pattern matching is case sensitive" );

}

//__________________________________________________
void HighlightPatternOptions::setOptions( const unsigned int& options )
{
  Debug::Throw( "HighlightPatternOptions::setOptions.\n" );
  span_->setChecked( options & HighlightPattern::SPAN );
  no_indent_->setChecked( options & HighlightPattern::NO_INDENT );
  case_sensitive_->setChecked( !(options & HighlightPattern::CASE_INSENSITIVE) );
}

//__________________________________________________
unsigned int HighlightPatternOptions::options( void ) const
{
  unsigned int out = HighlightPattern::NONE;
  if( span_->isChecked() ) out |= HighlightPattern::SPAN;
  if( no_indent_->isChecked() ) out |= HighlightPattern::NO_INDENT;
  if( !case_sensitive_->isChecked() ) out |= HighlightPattern::CASE_INSENSITIVE;
  return out;
}
