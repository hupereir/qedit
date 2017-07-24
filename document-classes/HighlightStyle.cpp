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

#include "HighlightStyle.h"

#include "Debug.h"
#include "XmlDef.h"

//_____________________________________________________
HighlightStyle::HighlightStyle( const QDomElement& element ):
    Counter( "HighlightStyle" )
{
    Debug::Throw( "HighlightStyle::HighlightStyle.\n" );

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;

        if( attribute.name() == Xml::Name ) setName( attribute.value() );
        else if( attribute.name() == Xml::Format ) setFontFormat( (Format::TextFormatFlags) attribute.value().toInt() );
        else if( attribute.name() == Xml::Color ) setColor( QColor( attribute.value() ) );

    }
}

//_____________________________________________________
QDomElement HighlightStyle::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "HighlighStyle::DomElement.\n" );
    QDomElement out = parent.createElement( Xml::Style );
    out.setAttribute( Xml::Name, name_ );
    if( format_ != Format::Default ) out.setAttribute( Xml::Format, QString::number(format_) );
    if( color_.isValid() ) out.setAttribute( Xml::Color, color_.name() );
    return out;
}


//_____________________________________________________
bool operator == ( const HighlightStyle& first, const HighlightStyle& second)
{
    return
        first.name_ == second.name_ &&
        first.format_ == second.format_ &&
        first.color_ == second.color_;
}
