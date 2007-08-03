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
  \file KeywordPattern.cc
  \brief single keyword syntax highlighting
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "Debug.h"
#include "KeywordPattern.h"
#include "Str.h"
#include "XmlDef.h"
#include "XmlUtil.h"

using namespace std;

//____________________________________________________________
KeywordPattern::KeywordPattern( const QDomElement& element ):
    HighlightPattern( element )
{
  
  Debug::Throw( "KeywordPattern::KeywordPattern.\n" );
  
  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( child_element.isNull() ) continue;
    string tag_name( qPrintable( child_element.tagName() ) );
    if( tag_name == XML::KEYWORD ) setKeyword( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
  }

  keyword_.setCaseSensitivity( flag( CASE_INSENSITIVE ) ? Qt::CaseInsensitive : Qt::CaseSensitive );

}

//____________________________________________________________
QDomElement KeywordPattern::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "KeywordPattern::DomElement.\n" );
  QDomElement out( HighlightPattern::domElement( parent ) );
  out.
    appendChild( parent.createElement( XML::KEYWORD.c_str() ) ).
    appendChild( parent.createTextNode( XmlUtil::textToXml( qPrintable( keyword().pattern() ) ).c_str() ) );
  return out;
}
  
//____________________________________________________________
void KeywordPattern::processText( LocationSet& locations, const QString& text, bool& active ) const
{
  //! disable activity
  active=false;

  //! check RegExp
  if( keyword().isEmpty() ) return;
  int position( 0 );
  while( position >= 0 )
  {
    position = keyword().indexIn( text, position );
    if( position >= 0 )
    {
      locations.insert( Location( *this, position, keyword().matchedLength() ) );
      position += keyword().matchedLength();
    }
  }
}
