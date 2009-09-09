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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA 02111-1307 USA
*
*
*******************************************************************************/

/*!
\file IndentPattern.cpp
\brief Base class for indentation pattern
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <cassert>

#include "IndentPattern.h"
#include "Str.h"
#include "XmlDef.h"
#include "XmlString.h"

using namespace std;

//___________________________________________________________________________
unsigned int& IndentPattern::_counter( void )
{
  static unsigned int counter( 0 );
  return counter;
}

//_____________________________________________________
IndentPattern::IndentPattern( void ):
  Counter( "IndentPattern" ),
  id_(0),
  name_( "" ),
  type_( NOTHING ),
  scale_( 1 )
{}
//{ assert( false ); }

//_____________________________________________________
IndentPattern::IndentPattern( const QDomElement& element ):
  Counter( "IndentPattern" ),
  id_( _counter()++ ),
  name_( "" ),
  type_( NOTHING ),
  scale_( 1 )
{
  Debug::Throw( "IndentPattern::IndentPattern.\n" );

  // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {

    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    if( attribute.name() == XML::TYPE ) setType( (Type) attribute.value().toInt() );
    else if( attribute.name() == XML::NAME ) setName( attribute.value() );
    else if( attribute.name() == XML::SCALE ) setScale( attribute.value().toInt() );
    else Debug::Throw(0) << "IndentPattern::IndentPattern - unrecognized attribute: " << attribute.name() << endl;

  }

  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
  {
    QDomElement child_element = child_node.toElement();
    if( child_element.isNull() ) continue;
    if( child_element.tagName() == XML::RULE )
    {
      Rule rule( child_element );
      if( rule.isValid() ) addRule( rule );
    } else if( child_element.tagName() == XML::COMMENTS ) {

      setComments( XmlString( child_element.text() ).toText() );

    } else Debug::Throw(0) << "IndentPattern::IndentPattern - unrecognized child: " << child_element.tagName() << endl;
  }

  assert( !rules().empty() );

}

//_____________________________________________________
QDomElement IndentPattern::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "IndentPattern::domElement.\n" );
  QDomElement out( parent.createElement( XML::INDENT_PATTERN ) );
  out.setAttribute( XML::TYPE, Str().assign<unsigned int>( type() ) );
  if( !name().isEmpty() ) out.setAttribute( XML::NAME, name() );
  if( scale() > 1 ) out.setAttribute( XML::SCALE, Str().assign<unsigned int>( scale() ) );

  for( Rule::List::const_iterator iter = rules().begin(); iter != rules().end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  if( !comments().isEmpty() )
  {
    out.
      appendChild( parent.createElement( XML::COMMENTS ) ).
      appendChild( parent.createTextNode( XmlString( comments() ).toXml() ) );
  }

  return out;
}

//____________________________________________________________
bool IndentPattern::operator == ( const IndentPattern& other ) const
{
  return
    name() == other.name() &&
    type() == other.type() &&
    scale() == other.scale() &&
    rules() == other.rules() &&
    comments() == other.comments();
}

//____________________________________________________________
QString IndentPattern::typeName( const Type& type )
{
  switch( type )
  {
    case NOTHING: return XML::INDENT_NOTHING;
    case INCREMENT: return XML::INDENT_INCREMENT;
    case DECREMENT: return XML::INDENT_DECREMENT;
    case DECREMENT_ALL: return XML::INDENT_DECREMENT_ALL;
    default: assert(0);
  }
}

//_____________________________________________________
IndentPattern::Rule::Rule( const QDomElement& element ):
  Counter( "IndentPattern::Rule" ),
  flags_( NONE )
{
  Debug::Throw( "Rule::Rule.\n" );

  // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    if( attribute.name() == XML::PAR ) setParagraph( attribute.value().toInt() );
    else if( attribute.name() == XML::OPTIONS )
    {
      if( attribute.value().indexOf( XML::OPTION_NO_CASE, 0, Qt::CaseInsensitive ) >= 0 ) setFlag( CASE_INSENSITIVE, true );
    }
    else Debug::Throw(0) << "Rule::Rule - unrecognized attribute: " << attribute.name() << endl;
  }

  // parse children
  // parse children elements
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
  {
    QDomElement child_element = child_node.toElement();
    if( child_element.tagName() == XML::REGEXP ) setPattern( XmlString( child_element.text() ).toText() );
    else Debug::Throw(0) << "Rule::Rule - unrecognized child: " << child_element.tagName() << endl;
  }

  regexp_.setCaseSensitivity( flag( CASE_INSENSITIVE ) ? Qt::CaseInsensitive : Qt::CaseSensitive );

}

//________________________________________________________
QDomElement IndentPattern::Rule::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "IndentPattern::Rule::DomElement.\n" );
  QDomElement out( parent.createElement( XML::RULE ) );
  out.setAttribute( XML::PAR, Str().assign<int>( paragraph() ) );
  if( flag( CASE_INSENSITIVE ) )  out.setAttribute( XML::OPTIONS, XML::OPTION_NO_CASE );

  out.
    appendChild( parent.createElement( XML::REGEXP ) ).
    appendChild( parent.createTextNode( XmlString( pattern().pattern() ).toXml() ) );

  return out;
}

//________________________________________________________
bool IndentPattern::Rule::accept( const QString& text ) const
{

  if( !pattern().isValid() ) return true;
  return pattern().indexIn( text ) >= 0;

}
