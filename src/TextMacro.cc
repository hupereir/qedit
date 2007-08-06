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
  \file TextMacro.cc
  \brief Text modification macro
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QStringList>
#include <vector>

#include "Str.h"
#include "TextMacro.h"
#include "XmlDef.h"
#include "XmlUtil.h"

using namespace std;

//_____________________________________________________
TextMacro::TextMacro( const QDomElement& element ):
  Counter( "TextMacro" ), 
  name_( "generic" ),
  accelerator_( "" ),
  is_separator_( false )
{
  Debug::Throw( "TextMacro::TextMacro.\n" );
  
  // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    Str name( qPrintable( attribute.name() ) );
    Str value( qPrintable( attribute.value() ) );
    if( name == XML::NAME ) _setName( value );
    else if( name == XML::ACCELERATOR ) _setAccelerator( value );
    else if( name == XML::OPTIONS )
    {
      if( value.find( XML::OPTION_SEPARATOR ) != string::npos ) _setIsSeparator();
    } else cout << "TextMacro::TextMacro - unrecognized attribute: " << name << endl;
  }

  Debug::Throw() << "TextMacro::TextMacro - name: " << name() << endl;
  
  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( child_element.isNull() ) continue;
    string tag_name( qPrintable( child_element.tagName() ) );
    if( tag_name == XML::RULE ) _addRule( Rule( child_element ) );
    else cout << "TextMacro::TextMacro - unrecognized child: " << tag_name << endl;
  }
  
  Exception::check( isSeparator() || !rules_.empty(), DESCRIPTION( "Empty macro") );
  
}

//_____________________________________________________
QDomElement TextMacro::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "TextMacro::domElement.\n" );
  QDomElement out( parent.createElement( XML::MACRO.c_str() ) );
  
  // dump attributes
  out.setAttribute( XML::NAME.c_str(), name().c_str() );
  if( !_accelerator().empty() ) out.setAttribute( XML::ACCELERATOR.c_str(), _accelerator().c_str() );
  
  ostringstream what;
  if( isSeparator() ) what << XML::OPTION_SEPARATOR << " ";
  if( what.str().size() ) out.setAttribute( XML::OPTIONS.c_str(), what.str().c_str() );
  
  // dump children
  for( list<Rule>::const_iterator iter = rules_.begin(); iter != rules_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }
  return out;  
}

//_______________________________________________________
TextMacro::Rule::Rule( const QDomElement& element ):
  Counter( "TextMacro::Rule" ),
  no_splitting_( false )
{
  
  Debug::Throw( "TextMacro::Rule::Rule.\n" );
  
  // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    Str name( qPrintable( attribute.name() ) );
    Str value( qPrintable( attribute.value() ) );
    if( name == XML::OPTIONS ) 
    {
      if( value.find( XML::OPTION_NO_SPLIT ) != string::npos ) _setNoSplitting();
    } else cout << "TextMacro::Rule::Rule - unrecognized attribute: " << name << endl;
  }
  
  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( child_element.isNull() ) continue;
    string tag_name( qPrintable( child_element.tagName() ) );
    if( tag_name == XML::REGEXP ) _setPattern( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
    else if( tag_name == XML::REPLACEMENT ) 
    { _setReplaceText( XmlUtil::xmlToText( qPrintable( child_element.text() ) ).c_str() ); }
    else cout << "TextMacro::Rule::Rule - unrecognized child: " << tag_name << endl;
  }
  
}

//_____________________________________________________
QDomElement TextMacro::Rule::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "TextMacro::Rule::domElement.\n" );
  
  QDomElement out( parent.createElement( XML::RULE.c_str() ) );
  
  // options
  ostringstream what;
  if( no_splitting_ ) what << XML::OPTION_NO_SPLIT << " ";
  if( what.str().size() ) out.setAttribute( XML::OPTIONS.c_str(), what.str().c_str() );
  
  // child
  out.
    appendChild( parent.createElement( XML::REGEXP.c_str() ) ).
    appendChild( parent.createTextNode( XmlUtil::textToXml( qPrintable( _pattern().pattern() ) ).c_str() ) );
  
  out.
    appendChild( parent.createElement( XML::REPLACEMENT.c_str() ) ).
    appendChild( parent.createTextNode( XmlUtil::textToXml( qPrintable( _replaceText() ) ).c_str() ) );
  return out;
}

//_____________________________________________________
bool TextMacro::Rule::processText( QString& text ) const
{
  Debug::Throw( "TextMacro::Rule::ProcessText.\n" );
  if( !isValid() ) return false;
  
  QString copy( text );
  if( no_splitting_ )
  { text.replace( pattern_, replace_text_ ); }
  else {
    
    // split text line by line
    QStringList lines( text.split( '\n' ) );
          
    // process each line
    /* note: the index is incremented in the loop to check for the last line */
    ostringstream what;
    for( QStringList::iterator iter = lines.begin(); iter != lines.end(); iter++ )
    { iter->replace( pattern_, replace_text_ ); }
    
    // update output string
    text = lines.join( "\n" );
    
  }  
  
  // check if strings are different
  // and return
  return text == copy;
  
}
