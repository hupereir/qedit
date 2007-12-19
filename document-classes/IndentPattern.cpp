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
    Str name( qPrintable( attribute.name() ) );
    Str value( qPrintable( attribute.value() ) );
    if( name == XML::TYPE ) _setType( (Type) value.get<unsigned int>() );
    else if( name == XML::NAME ) _setName( value );
    else if( name == XML::SCALE ) _setScale( value.get<unsigned int>() );
    else cout << "IndentPattern::IndentPattern - unrecognized attribute: " << name << endl;
  
  }
  
  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement();
    if( child_element.isNull() ) continue;
    string tag_name( qPrintable( child_element.tagName() ) );
    if( tag_name == XML::RULE ) 
    {
      Rule rule( child_element );
      if( rule.isValid() ) _addRule( rule );
    }
    else cout << "IndentPattern::IndentPattern - unrecognized child: " << tag_name << endl;
  }
  
  assert( !rules().empty() );
  
}

//_____________________________________________________
QDomElement IndentPattern::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "IndentPattern::domElement.\n" );
  QDomElement out( parent.createElement( XML::INDENT_PATTERN.c_str() ) );
  out.setAttribute( XML::TYPE.c_str(), Str().assign<unsigned int>( type() ).c_str() );
  if( !name().empty() ) out.setAttribute( XML::NAME.c_str(), name().c_str() );
  if( scale() > 1 ) out.setAttribute( XML::SCALE.c_str(), Str().assign<unsigned int>( scale() ).c_str() );
  for( RuleList::const_iterator iter = rules().begin(); iter != rules().end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }
  return out;
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
    Str name( qPrintable( attribute.name() ) );
    Str value( qPrintable( attribute.value() ) );
    if( name == XML::PAR ) _setParagraph( value.get<int>() );
    else if( name == XML::OPTIONS )
    {
      if( value.find( XML::OPTION_NO_CASE ) != string::npos ) setFlag( CASE_INSENSITIVE, true ); 
    } 
    else cout << "Rule::Rule - unrecognized attribute: " << name << endl;
  }
  
  // parse children
  // parse children elements
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    string tag_name( qPrintable( child_element.tagName() ) );
    if( tag_name == XML::REGEXP ) _setRegExp( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
    else cout << "Rule::Rule - unrecognized child: " << tag_name << endl;
  }
  
  regexp_.setCaseSensitivity( flag( CASE_INSENSITIVE ) ? Qt::CaseInsensitive : Qt::CaseSensitive );

}

//________________________________________________________
QDomElement IndentPattern::Rule::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "IndentPattern::Rule::DomElement.\n" );
  QDomElement out( parent.createElement( XML::RULE.c_str() ) );
  out.setAttribute( XML::PAR.c_str(), Str().assign<int>( paragraph() ).c_str() );
  ostringstream what;
  if( flag( CASE_INSENSITIVE ) ) what << XML::OPTION_NO_CASE << " ";
  if( what.str().size() ) out.setAttribute( XML::OPTIONS.c_str(), what.str().c_str() );

  out.
    appendChild( parent.createElement( XML::REGEXP.c_str() ) ).
    appendChild( parent.createTextNode( XmlUtil::textToXml( qPrintable( regexp_.pattern() ) ).c_str() ) );
  
  return out;
}

//________________________________________________________
bool IndentPattern::Rule::accept( const QString& text ) const
{
  
  if( !regexp_.isValid() ) return true;
  return regexp_.indexIn( text ) >= 0;
  
}
