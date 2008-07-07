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

#include <assert.h>

#include "IndentPattern.h"
#include "Str.h"
#include "XmlDef.h"
#include "XmlUtil.h"

using namespace std;

//___________________________________________________________________________
unsigned int IndentPattern::id_counter_ = 0;

//_____________________________________________________
IndentPattern::IndentPattern( const QDomElement& element ):
  Counter( "IndentPattern" ),
  id_( id_counter_++ ),
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
    else cout << "IndentPattern::IndentPattern - unrecognized attribute: " << qPrintable( attribute.name() ) << endl;
    
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
    }
    else cout << "IndentPattern::IndentPattern - unrecognized child: " << qPrintable( child_element.tagName() ) << endl;
  }
  
  assert( !rules().empty() );
  
}

//_____________________________________________________
QDomElement IndentPattern::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "IndentPattern::domElement.\n" );
  QDomElement out( parent.createElement( XML::INDENT_PATTERN ) );
  out.setAttribute( XML::TYPE, Str().assign<unsigned int>( type() ).c_str() );
  if( !name().isEmpty() ) out.setAttribute( XML::NAME, name() );
  if( scale() > 1 ) out.setAttribute( XML::SCALE, Str().assign<unsigned int>( scale() ).c_str() );
  for( Rule::List::const_iterator iter = rules().begin(); iter != rules().end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }
  return out;
}

//____________________________________________________________
bool IndentPattern::differs( const IndentPattern& pattern ) const
{ 
  return 
    name() != pattern.name() ||
    type() != pattern.type() ||
    scale() != pattern.scale() ||
    rules() != pattern.rules();
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
      if( attribute.value().indexOf( XML::OPTION_NO_CASE, 0, Qt::CaseInsensitive ) > 0 ) setFlag( CASE_INSENSITIVE, true ); 
    } 
    else cout << "Rule::Rule - unrecognized attribute: " << qPrintable( attribute.name() ) << endl;
  }
  
  // parse children
  // parse children elements
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( child_element.tagName() == XML::REGEXP ) setPattern( XmlUtil::xmlToText( child_element.text() ) );
    else cout << "Rule::Rule - unrecognized child: " << qPrintable( child_element.tagName() ) << endl;
  }
  
  regexp_.setCaseSensitivity( flag( CASE_INSENSITIVE ) ? Qt::CaseInsensitive : Qt::CaseSensitive );
  
}

//________________________________________________________
QDomElement IndentPattern::Rule::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "IndentPattern::Rule::DomElement.\n" );
  QDomElement out( parent.createElement( XML::RULE ) );
  out.setAttribute( XML::PAR, Str().assign<int>( paragraph() ).c_str() );
  ostringstream what;
  if( flag( CASE_INSENSITIVE ) )  out.setAttribute( XML::OPTIONS, XML::OPTION_NO_CASE );
  
  out.
    appendChild( parent.createElement( XML::REGEXP ) ).
    appendChild( parent.createTextNode( XmlUtil::textToXml( pattern().pattern() ) ) );
  
  return out;
}

//________________________________________________________
bool IndentPattern::Rule::accept( const QString& text ) const
{
  
  if( !pattern().isValid() ) return true;
  return pattern().indexIn( text ) >= 0;
  
}
