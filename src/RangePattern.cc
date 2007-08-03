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
  \file RangePattern.cc
  \brief single keyword syntax highlighting
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "Debug.h"
#include "RangePattern.h"
#include "Str.h"
#include "XmlDef.h"
#include "XmlUtil.h"

using namespace std;

//____________________________________________________________
RangePattern::RangePattern( const QDomElement& element ):
    HighlightPattern( element )
{
  
  Debug::Throw( "RangePattern::RangePattern.\n" );
  
  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( child_element.isNull() ) continue;
    string tag_name( qPrintable( child_element.tagName() ) );
    if( tag_name == XML::BEGIN ) setBegin( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
    else if( tag_name == XML::END ) setEnd( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
  }
  
  begin_.setCaseSensitivity( flag( CASE_INSENSITIVE ) ? Qt::CaseInsensitive : Qt::CaseSensitive );
  end_.setCaseSensitivity( flag( CASE_INSENSITIVE ) ? Qt::CaseInsensitive : Qt::CaseSensitive );
  
}

//____________________________________________________________
QDomElement RangePattern::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "RangePattern::domElement.\n" );
  QDomElement out( HighlightPattern::domElement( parent ) );
  out.
    appendChild( parent.createElement( XML::BEGIN.c_str() ) ).
    appendChild( parent.createTextNode( XmlUtil::textToXml( qPrintable(begin().pattern() ) ).c_str() ) );
  out.
    appendChild( parent.createElement( XML::END.c_str() ) ).
    appendChild( parent.createTextNode( XmlUtil::textToXml( qPrintable(end().pattern() ) ).c_str() ) );
  return out;
}

//____________________________________________________________
void RangePattern::processText( LocationSet& locations, const QString& text, bool& active ) const
{

  // check RegExp
  if( begin().isEmpty() || end().isEmpty() ) return;
  
  int begin(0);
  int end(0);
  
  // check if pattern spans over paragraphs
  // and was active in previous paragraph
  if( flag( SPAN ) && active )
  {
      
    // if active, look for end match
    end = RangePattern::end().indexIn( text, 0 );
    if( end < 0 )
    {
      // no match found.
      // pattern is still active for next paragraph
      // the whole paragraph match the pattern
      locations.insert( Location( *this, 0, text.size() ) );
      return;  
      
    } else {
        
      // found matching end.
      // pattern is not active any more but one needs to check if it does not start again
      active = false;
      end+=RangePattern::end().matchedLength();
      locations.insert( Location( *this, 0, end ) );
    }
    
  }
  
  // look for begin and end in current paragraphs
  while( 1 )
  {
    
    // look for begin match
    // start from end index, which is either 0, or the last
    // found end in case of spanning active patterns
    begin = end;
    begin = RangePattern::begin().indexIn( text, begin );
    if( begin < 0 ) 
    {
      active = false;
      break;
    }
    
    // look for end match     
    int end = RangePattern::end().indexIn( text, begin );
    
    // avoid zero length match
    if( begin == end && RangePattern::begin().matchedLength() == RangePattern::end().matchedLength() )
    { end = RangePattern::end().indexIn( text, begin+RangePattern::begin().matchedLength() ); }
    
    if( end < 0 ) 
    {
      if( flag( SPAN ) )
      {
        // no end found. 
        // Pattern will still be active in next paragraph
        active = true;
        locations.insert( Location( *this, begin, text.size()-begin ) );
      }
      
      break;
    }
    
    // found end matching begin
    // append new text location
    end+=RangePattern::end().matchedLength();
    locations.insert( Location( *this, begin, end-begin ) );
  }
  
  return;
}
