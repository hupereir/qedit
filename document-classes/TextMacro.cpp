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
  \file TextMacro.cpp
  \brief Text modification macro
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <cassert>
#include <QStringList>
#include <vector>

#include "Str.h"
#include "TextMacro.h"
#include "XmlDef.h"
#include "XmlString.h"

using namespace std;

//___________________________________________________________________________
unsigned int TextMacro::id_counter_ = 0;

//_____________________________________________________
TextMacro::TextMacro( const QDomElement& element ):
  Counter( "TextMacro" ), 
  id_( id_counter_++ ),
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
    if( attribute.name() == XML::NAME ) setName( attribute.value() );
    else if( attribute.name() == XML::ACCELERATOR ) setAccelerator( attribute.value() );
    else if( attribute.name() == XML::OPTIONS )
    {
      if( attribute.value().indexOf( XML::OPTION_SEPARATOR, 0, Qt::CaseInsensitive ) >= 0 ) setIsSeparator();
    } else Debug::Throw(0) << "TextMacro::TextMacro - unrecognized attribute: " << attribute.name() << endl;
  }

  Debug::Throw() << "TextMacro::TextMacro - name: " << name() << endl;
  
  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( child_element.isNull() ) continue;
    if( child_element.tagName() == XML::RULE ) addRule( Rule( child_element ) );
    else Debug::Throw(0) << "TextMacro::TextMacro - unrecognized child: " << child_element.tagName() << endl;
  }
  
  assert( isSeparator() || !rules_.empty() );
  
}

//_____________________________________________________
QDomElement TextMacro::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "TextMacro::domElement.\n" );
  QDomElement out( parent.createElement( XML::MACRO ) );
  
  // dump attributes
  out.setAttribute( XML::NAME, name() );
  if( !accelerator().isEmpty() ) out.setAttribute( XML::ACCELERATOR, accelerator() );
  
  if( isSeparator() ) out.setAttribute( XML::OPTIONS, XML::OPTION_SEPARATOR );
  
  // dump children
  for( Rule::List::const_iterator iter = rules_.begin(); iter != rules_.end(); iter++ )
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
    if( attribute.name() == XML::OPTIONS ) 
    {
      if( attribute.value().indexOf( XML::OPTION_NO_SPLIT, 0, Qt::CaseInsensitive ) >= 0 ) setNoSplitting();
    } else Debug::Throw(0) << "TextMacro::Rule::Rule - unrecognized attribute: " << attribute.name() << endl;
  }
  
  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( child_element.isNull() ) continue;
    if( child_element.tagName() == XML::REGEXP ) setPattern( XmlString( child_element.text() ).toText() );
    else if( child_element.tagName() == XML::REPLACEMENT ) setReplaceText( XmlString( child_element.text() ).toText() ); 
    else Debug::Throw(0) << "TextMacro::Rule::Rule - unrecognized child: " << child_element.tagName() << endl;
  }
  
}

//_____________________________________________________
QDomElement TextMacro::Rule::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "TextMacro::Rule::domElement.\n" );
  
  QDomElement out( parent.createElement( XML::RULE ) );
  
  // options
  if( no_splitting_ ) out.setAttribute( XML::OPTIONS, XML::OPTION_NO_SPLIT );
  
  // child
  out.
    appendChild( parent.createElement( XML::REGEXP ) ).
    appendChild( parent.createTextNode( XmlString( pattern().pattern() ).toXml() ) );
  
  out.
    appendChild( parent.createElement( XML::REPLACEMENT ) ).
    appendChild( parent.createTextNode( XmlString( replaceText() ).toXml() ) );
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
    for( QStringList::iterator iter = lines.begin(); iter != lines.end(); iter++ )
    { iter->replace( pattern_, replace_text_ ); }
    
    // update output string
    text = lines.join( "\n" );
    
  }  
  
  // check if strings are different
  // and return
  return text != copy;
  
}
