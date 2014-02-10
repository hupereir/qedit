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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
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



//_____________________________________________________
HighlightStyle::HighlightStyle( const QDomElement& element ):
      Counter( "HighlightStyle" ),
      format_( Format::Default ),
      color_( Qt::black )
{
  Debug::Throw( "HighlightStyle::HighlightStyle.\n" );

  // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;

    if( attribute.name() == Xml::NAME ) setName( attribute.value() );
    else if( attribute.name() == Xml::FORMAT ) setFontFormat( (Format::TextFormatFlags) attribute.value().toInt() );
    else if( attribute.name() == Xml::COLOR ) setColor( QColor( attribute.value() ) );
    else Debug::Throw(0) << "Option::Option - unrecognized attribute " << attribute.name() << ".\n";

  }
}

//_____________________________________________________
QDomElement HighlightStyle::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "HighlighStyle::DomElement.\n" );
  QDomElement out = parent.createElement( Xml::STYLE );
  out.setAttribute( Xml::NAME, name() );
  out.setAttribute( Xml::FORMAT, Str().assign<unsigned int>(fontFormat()) );
  out.setAttribute( Xml::COLOR, color().name() );
  return out;
}


//_____________________________________________________
bool HighlightStyle::operator == ( const HighlightStyle& other ) const
{
  return
    name() == other.name() &&
    fontFormat() == other.fontFormat() &&
    color() == other.color();
}
