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

#include "BlockDelimiter.h"

#include "Debug.h"
#include "XmlDef.h"
#include "XmlString.h"

//_________________________________________________________
BlockDelimiter::BlockDelimiter( const QDomElement& element, int id ):
    Counter( "BlockDelimiter" ),
    id_( id )
{

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == Xml::Begin ) first_ = XmlString( attribute.value() );
        else if( attribute.name() == Xml::End ) second_ = XmlString( attribute.value() );
        else if( attribute.name() == Xml::RegExp ) regexp_.setPattern( XmlString( attribute.value() ) );
        else Debug::Throw(0) << "BlockDelimiter::BlockDelimiter - unrecognized attribute: " << attribute.name() << endl;
    }

    // create regexp
    if( regexp_.pattern().isEmpty() )
    {
        QString pattern = QString("(") + first() + ")|(" + second() + ")";
        regexp_.setPattern( pattern );
    }

}


//_____________________________________________________
QDomElement BlockDelimiter::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "BlockDelimiter::DomElement.\n" );
    QDomElement out( parent.createElement( Xml::BlockDelimiter ) );

    // dump attributes
    out.setAttribute( Xml::Begin, first() );
    out.setAttribute( Xml::End, second() );
    out.setAttribute( Xml::RegExp, regexp().pattern() );
    return out;
}
