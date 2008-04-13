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
  \file HighlightStyle.cpp
  \brief Base class for syntax highlighting style
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "HighlightStyle.h"
#include "Str.h"
#include "XmlDef.h"

using namespace std;

//_____________________________________________________
HighlightStyle::HighlightStyle( const QDomElement& element ):
      Counter( "HighlightStyle" ),
      format_( FORMAT::DEFAULT ),
      color_( Qt::black )
{
  Debug::Throw( "HighlightStyle::HighlightStyle.\n" );
      
  // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    Str name( qPrintable( attribute.name() ) );
    Str value( qPrintable( attribute.value() ) );
    
    if( name == XML::NAME ) setName( value );
    else if( name == XML::FORMAT ) setFontFormat( value.get<unsigned int>() );
    else if( name == XML::COLOR ) setColor( QColor( value.c_str() ) );
    else   cout << "Option::Option - unrecognized attribute " << name << ".\n";
  
  }
}

//_____________________________________________________
QDomElement HighlightStyle::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "HighlighStyle::DomElement.\n" );
  QDomElement out = parent.createElement( XML::STYLE.c_str() );
  out.setAttribute( XML::NAME.c_str(), name().c_str() );
  out.setAttribute( XML::FORMAT.c_str(), Str().assign<unsigned int>(fontFormat()).c_str() );
  out.setAttribute( XML::COLOR.c_str(), color().name() );
  return out;
}


//_____________________________________________________
bool HighlightStyle::differs( const HighlightStyle& style ) const
{ 
  return 
    name() != style.name() ||
    fontFormat() != style.fontFormat() ||
    color() != style.color();
}  
