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
 * Place, Suite 330, Boston, MA  02111-1307 USA                           
 *                         
 *                         
 *******************************************************************************/

/*!
  \file HighlightPattern.cc
  \brief Base class for syntax highlighting
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <qregexp.h>
#include "HighlightPattern.h"
#include "Str.h"
#include "XmlDef.h"
#include "XmlUtil.h"

using namespace std;

//___________________________________________________________________________
HighlightPattern::HighlightPattern( const QDomElement& element ):
  Counter( "HighlightPattern" ),
  id_( 0 ),
  name_( "default" ),
  parent_( "" ),
  parent_id_( 0 ),
  style_( HighlightStyle() ),
  flags_( NONE )
{  
  Debug::Throw( "HighlightPattern::HighlightPattern.\n" );
  
  _setType( (element.tagName().isNull()) ? "":qPrintable( element.tagName() ) );
  
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    Str name( qPrintable( attribute.name() ) );
    Str value( qPrintable( attribute.value() ) );
      
    if( name == XML::NAME ) _setName( value );
    else if( name == XML::PARENT ) _setParent( value );
    else if( name == XML::STYLE ) setStyle( HighlightStyle( value ) );
    else if( name == XML::OPTIONS )
    {
      if( value.find( XML::OPTION_SPAN ) != string::npos ) setFlag( SPAN, true );
      if( value.find( XML::OPTION_NO_INDENT ) != string::npos ) setFlag( NO_INDENT, true );
      if( value.find( XML::OPTION_NO_CASE ) != string::npos ) setFlag( CASE_INSENSITIVE, true ); 
    } else 
    { Debug::Throw() << "HighlightPattern::HighlightPattern - unrecognized attribute " << name << endl; }
    
  }
  
  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( child_element.isNull() ) continue;
    string tag_name( qPrintable( child_element.tagName() ) );
    if( tag_name == XML::COMMENTS ) setComments( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
  }
 
}

//______________________________________________________
QDomElement HighlightPattern::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "HighlightPattern::domElement.\n" );
  QDomElement out( parent.createElement( _type().c_str() ) );
  out.setAttribute( XML::NAME.c_str(), name().c_str() );
  out.setAttribute( XML::PARENT.c_str(), HighlightPattern::parent().c_str() );
  out.setAttribute( XML::STYLE.c_str(), style().name().c_str() );
  
  // options:
  ostringstream what;
  if( flag( SPAN ) ) what << XML::OPTION_SPAN << " ";
  if( flag( NO_INDENT ) ) what << XML::OPTION_NO_INDENT << " ";
  if( flag( CASE_INSENSITIVE ) ) what << XML::OPTION_NO_CASE << " ";
  if( what.str().size() ) out.setAttribute( XML::OPTIONS.c_str(), what.str().c_str() );

  out.
    appendChild( parent.createElement( XML::COMMENTS.c_str() ) ).
    appendChild( parent.createTextNode( XmlUtil::textToXml( comments() ).c_str() ) );
  
  return out;
}
