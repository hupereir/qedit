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
   \file HighlightPatternType.cpp
   \brief highlight pattern type selection
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <assert.h>
#include <QButtonGroup>
#include <QLabel>
#include <QLayout>

#include "HighlightPatternType.h"

using namespace std;

//___________________________________________________
HighlightPatternType::HighlightPatternType( QWidget* parent ):
  QWidget( parent ),
  Counter( "HighlightPatternType" )
{

  Debug::Throw( "HighlightPatternType::HighlightPatternType.\n" );
  setLayout( new QHBoxLayout() );
  layout()->setMargin(0);
  layout()->setSpacing(5);

  layout()->addWidget( new QLabel( "Type: ", this ) );
  QButtonGroup* group = new QButtonGroup();
  group->setExclusive( true );
  connect( group, SIGNAL( buttonClicked( QAbstractButton* ) ), SLOT( _typeChanged( QAbstractButton* ) ) );
  
  QCheckBox* checkbox;
  group->addButton( checkbox = new QCheckBox( HighlightPattern::typeName( HighlightPattern::KEYWORD_PATTERN ).c_str(), this ) );
  checkboxes_.insert( make_pair( checkbox, HighlightPattern::KEYWORD_PATTERN ) );
  layout()->addWidget( checkbox );
  checkbox->setChecked( true );
  
  group->addButton( checkbox = new QCheckBox( HighlightPattern::typeName( HighlightPattern::RANGE_PATTERN ).c_str(), this ) );
  checkboxes_.insert( make_pair( checkbox, HighlightPattern::RANGE_PATTERN ) );
  layout()->addWidget( checkbox );
  
}

//___________________________________________________
void HighlightPatternType::setType( const HighlightPattern::Type& type )
{
  Debug::Throw( "HighlightPatternType::setType.\n" );
  for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); iter++ )
  { iter->first->setChecked( iter->second == type ); }
}

//___________________________________________________
HighlightPattern::Type HighlightPatternType::type( void ) const
{
  Debug::Throw( "HighlightPatternType::type.\n" );
  for( CheckBoxMap::const_iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); iter++ )
  { if( iter->first->isChecked() ) return iter->second; }
  return HighlightPattern::UNDEFINED;
}

//___________________________________________________
void HighlightPatternType::_typeChanged( QAbstractButton* button )
{
  Debug::Throw( "HighlightPatternType::_typeChanged.\n" );
  if( !button->isChecked() ) return;
  
  CheckBoxMap::const_iterator iter( checkboxes_.find( button ) );
  assert( iter != checkboxes_.end() );
  emit typeChanged( iter->second );
}
