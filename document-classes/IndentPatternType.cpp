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
   \file IndentPatternType.cpp
   \brief highlight pattern type selection
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <cassert>
#include <QButtonGroup>
#include <QLabel>
#include <QLayout>

#include "IndentPatternType.h"



//___________________________________________________
IndentPatternType::IndentPatternType( QWidget* parent ):
  QComboBox( parent ),
  Counter( "IndentPatternType" )
{

  Debug::Throw( "IndentPatternType::IndentPatternType.\n" );
  setEditable( false );
  addItem( IndentPattern::typeName( IndentPattern::NOTHING ) );
  addItem( IndentPattern::typeName( IndentPattern::INCREMENT ) );
  addItem( IndentPattern::typeName( IndentPattern::DECREMENT ) );
  addItem( IndentPattern::typeName( IndentPattern::DECREMENT_ALL ) );

  connect( this, SIGNAL( activated( const QString & ) ), SLOT( _typeChanged( const QString& ) ) );

}

//___________________________________________________
void IndentPatternType::setType( const IndentPattern::Type& type )
{

  Debug::Throw( "IndentPatternType::setType.\n" );
  for( int index = 0; index < QComboBox::count(); index++ )
  {
    if( itemText( index ) == IndentPattern::typeName( type ) )
    { setCurrentIndex( index ); }
  }

}

//___________________________________________________
IndentPattern::Type IndentPatternType::type( void ) const
{
  Debug::Throw( "IndentPatternType::type.\n" );
  QString value( itemText( currentIndex() ) );
  if( value == IndentPattern::typeName( IndentPattern::NOTHING ) ) return IndentPattern::NOTHING;
  else if( value == IndentPattern::typeName( IndentPattern::INCREMENT ) ) return IndentPattern::INCREMENT;
  else if( value == IndentPattern::typeName( IndentPattern::DECREMENT ) ) return IndentPattern::DECREMENT;
  else if( value == IndentPattern::typeName( IndentPattern::DECREMENT_ALL ) ) return IndentPattern::DECREMENT_ALL;
  else assert(0);
  return IndentPattern::NOTHING;
}

//___________________________________________________
void IndentPatternType::_typeChanged( const QString& value )
{
  Debug::Throw( "IndentPatternType::_typeChanged.\n" );
  if( value == IndentPattern::typeName( IndentPattern::NOTHING ) ) emit typeChanged( IndentPattern::NOTHING );
  else if( value == IndentPattern::typeName( IndentPattern::INCREMENT ) ) emit typeChanged( IndentPattern::INCREMENT );
  else if( value == IndentPattern::typeName( IndentPattern::DECREMENT ) ) emit typeChanged( IndentPattern::DECREMENT );
  else if( value == IndentPattern::typeName( IndentPattern::DECREMENT_ALL ) ) emit typeChanged( IndentPattern::DECREMENT_ALL );
  else assert(0);
}
