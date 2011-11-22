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

#include <cassert>
#include <QButtonGroup>
#include <QLabel>
#include <QLayout>

#include "HighlightPatternType.h"



//___________________________________________________
HighlightPatternType::HighlightPatternType( QWidget* parent ):
  QComboBox( parent ),
  Counter( "HighlightPatternType" )
{

  Debug::Throw( "HighlightPatternType::HighlightPatternType.\n" );
  setEditable( false );
  addItem( HighlightPattern::typeName( HighlightPattern::KEYWORD_PATTERN ) );
  addItem( HighlightPattern::typeName( HighlightPattern::RANGE_PATTERN ) );


  connect( this, SIGNAL( activated( const QString & ) ), SLOT( _typeChanged( const QString& ) ) );

}

//___________________________________________________
void HighlightPatternType::setType( const HighlightPattern::Type& type )
{

  Debug::Throw( "HighlightPatternType::setType.\n" );
  for( int index = 0; index < QComboBox::count(); index++ )
  {
    if( itemText( index ) == HighlightPattern::typeName( type ) )
    { setCurrentIndex( index ); }
  }

}

//___________________________________________________
HighlightPattern::Type HighlightPatternType::type( void ) const
{
  Debug::Throw( "HighlightPatternType::type.\n" );
  QString value( itemText( currentIndex() ) );
  if( value == HighlightPattern::typeName( HighlightPattern::RANGE_PATTERN ) ) return HighlightPattern::RANGE_PATTERN;
  else if(  value == HighlightPattern::typeName( HighlightPattern::KEYWORD_PATTERN ) ) return HighlightPattern::KEYWORD_PATTERN;
  else assert(0);
  return HighlightPattern::UNDEFINED;
}

//___________________________________________________
void HighlightPatternType::_typeChanged( const QString& value )
{
  Debug::Throw( "HighlightPatternType::_typeChanged.\n" );
  if( value == HighlightPattern::typeName( HighlightPattern::RANGE_PATTERN ) ) emit typeChanged( HighlightPattern::RANGE_PATTERN );
  else if(  value == HighlightPattern::typeName( HighlightPattern::KEYWORD_PATTERN ) ) emit typeChanged( HighlightPattern::KEYWORD_PATTERN );
  else assert(0);
}
