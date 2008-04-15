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
  \file HighlightPattern.cpp
  \brief Base class for syntax highlighting
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <assert.h>

#include "HighlightPattern.h"
#include "PatternLocation.h"
#include "Str.h"
#include "XmlDef.h"
#include "XmlUtil.h"

using namespace std;

//___________________________________________________________________________
std::string HighlightPattern::no_parent_pattern_ = "None";


//___________________________________________________________________________
HighlightPattern::HighlightPattern( const QDomElement& element ):
  Counter( "HighlightPattern" ),
  id_( 0 ),
  type_( UNDEFINED ),
  name_( "default" ),
  parent_( "" ),
  parent_id_( 0 ),
  style_( HighlightStyle() ),
  flags_( NONE )
{  
  Debug::Throw( "HighlightPattern::HighlightPattern.\n" );
  if( element.tagName() == XML::KEYWORD_PATTERN.c_str() ) setType( KEYWORD_PATTERN );
  if( element.tagName() == XML::RANGE_PATTERN.c_str() ) setType( RANGE_PATTERN );
  
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    Str name( qPrintable( attribute.name() ) );
    Str value( qPrintable( attribute.value() ) );
      
    if( name == XML::NAME ) setName( value );
    else if( name == XML::PARENT ) setParent( value );
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
    string name( qPrintable( child_element.tagName() ) );
    if( name == XML::COMMENTS ) setComments( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
    else if( name == XML::KEYWORD ) setKeyword( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
    else if( name == XML::BEGIN ) setBegin( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
    else if( name == XML::END ) setEnd( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
  }
 
}

//______________________________________________________
QDomElement HighlightPattern::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "HighlightPattern::domElement.\n" );
  
  QDomElement out( parent.createElement( typeName().c_str() ) );
  out.setAttribute( XML::NAME.c_str(), name().c_str() );
  out.setAttribute( XML::PARENT.c_str(), HighlightPattern::parent().c_str() );
  out.setAttribute( XML::STYLE.c_str(), style().name().c_str() );
  
  // options:
  ostringstream what;
  if( flag( SPAN ) ) what << XML::OPTION_SPAN << " ";
  if( flag( NO_INDENT ) ) what << XML::OPTION_NO_INDENT << " ";
  if( flag( CASE_INSENSITIVE ) ) what << XML::OPTION_NO_CASE << " ";
  if( what.str().size() ) out.setAttribute( XML::OPTIONS.c_str(), what.str().c_str() );

  // comments
  out.
    appendChild( parent.createElement( XML::COMMENTS.c_str() ) ).
    appendChild( parent.createTextNode( XmlUtil::textToXml( comments() ).c_str() ) );

  // regexps
  if( type() == KEYWORD_PATTERN )
  {
    out.
      appendChild( parent.createElement( XML::KEYWORD.c_str() ) ).
      appendChild( parent.createTextNode( XmlUtil::textToXml( qPrintable( keyword().pattern() ) ).c_str() ) );
  }
  
  if( type() == RANGE_PATTERN )
  {
    out.
      appendChild( parent.createElement( XML::BEGIN.c_str() ) ).
      appendChild( parent.createTextNode( XmlUtil::textToXml( qPrintable( begin().pattern() ) ).c_str() ) );
    out.
      appendChild( parent.createElement( XML::END.c_str() ) ).
      appendChild( parent.createTextNode( XmlUtil::textToXml( qPrintable( end().pattern() ) ).c_str() ) );
  }
    
  return out;
}


//____________________________________________________________
bool HighlightPattern::differs( const HighlightPattern& pattern ) const
{ 
  return 
    name() != pattern.name() ||
    flags() != pattern.flags() ||
    type() != pattern.type() ||
    parent() != pattern.parent() ||
    style() != pattern.style() ||
    keyword() != pattern.keyword() ||
    ( type() == RANGE_PATTERN && end() != pattern.end() );
}
  
//____________________________________________________________
string HighlightPattern::typeName( const Type& type )
{
  switch( type )
  {
    case KEYWORD_PATTERN: return XML::KEYWORD_PATTERN;
    case RANGE_PATTERN: return XML::RANGE_PATTERN;
    default: assert(0);
  }
}   

//____________________________________________________________
bool HighlightPattern::_findKeyword( PatternLocationSet& locations, const QString& text, bool& active ) const
{
    
  // disable activity
  active=false;
  
  // check RegExp
  if( keyword().isEmpty() ) return false;

  // process text
  bool found( false );
  int position( 0 );
  while( position >= 0 )
  {
    position = keyword().indexIn( text, position );
    if( position >= 0 )
    {
      found = true;
      locations.insert( PatternLocation( *this, position, keyword().matchedLength() ) );
      position += keyword().matchedLength();
    }
  }
  
  return found;
  
}

//____________________________________________________________
bool HighlightPattern::_findRange( PatternLocationSet& locations, const QString& text, bool& active ) const
{
  
  // check RegExp
  if( begin().isEmpty() || end().isEmpty() ) return false;
  
  int begin(0);
  int end(0);
  
  bool found( false );
  
  // check if pattern spans over paragraphs
  // and was active in previous paragraph
  if( flag( SPAN ) && active )
  {
      
    // if active, look for end match
    end = HighlightPattern::end().indexIn( text, 0 );
    if( end < 0 )
    {
      
      // no match found.
      // pattern is still active for next paragraph
      // the whole paragraph match the pattern
      locations.insert( PatternLocation( *this, 0, text.size() ) );
      return true;  
      
    } else {
        
      // found matching end.
      // pattern is not active any more but one needs to check if it does not start again
      active = false;
      found = true;
      end+=HighlightPattern::end().matchedLength();
      locations.insert( PatternLocation( *this, 0, end ) );
      
    }
    
  }
  
  // look for begin and end in current paragraphs
  while( 1 )
  {
    
    // look for begin match
    // start from end index, which is either 0, or the last
    // found end in case of spanning active patterns    
    begin = end;
    begin = HighlightPattern::begin().indexIn( text, begin );
    if( begin < 0 ) 
    {
      active = false;
      break;
    }
    
    // look for end match     
    end = HighlightPattern::end().indexIn( text, begin );
    
    // avoid zero length match
    if( begin == end && HighlightPattern::begin().matchedLength() == HighlightPattern::end().matchedLength() )
    { end = HighlightPattern::end().indexIn( text, begin+HighlightPattern::begin().matchedLength() ); }
    
    if( end < 0 ) 
    {
      if( flag( SPAN ) )
      {
        // no end found. 
        // Pattern will still be active in next paragraph
        found = true;
        active = true;
        locations.insert( PatternLocation( *this, begin, text.size()-begin ) );
      }
      
      break;
    }
    
    // found end matching begin
    // append new text location
    found = true;
    end += HighlightPattern::end().matchedLength();
    locations.insert( PatternLocation( *this, begin, end-begin ) );
    
  }
  
  return found;
}
