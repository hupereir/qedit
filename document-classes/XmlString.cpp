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

#include "XmlString.h"

//__________________________________
XmlString::ConversionList& XmlString::_conversions()
{
    static ConversionList conversions;
    return conversions;
}

//__________________________________
void XmlString::_initConversions() const
{

    _conversions() << Conversion( "<", "XML_LT" );
    _conversions() << Conversion( ">", "XML_GT" );
    _conversions() << Conversion( "&", "XML_AND" );
    _conversions() << Conversion( "\"", "XML_QUOTE" );

    _conversions() << Conversion( QString::fromUtf8( "à" ), "XML_AGRAVE" );
    _conversions() << Conversion( QString::fromUtf8( "â" ), "XML_AHAT" );
    _conversions() << Conversion( QString::fromUtf8( "ä" ), "XML_ATREM" );
    _conversions() << Conversion( QString::fromUtf8( "é" ), "XML_ECUTE" );
    _conversions() << Conversion( QString::fromUtf8( "è" ), "XML_EGRAVE" );
    _conversions() << Conversion( QString::fromUtf8( "ê" ), "XML_EHAT" );
    _conversions() << Conversion( QString::fromUtf8( "ë" ), "XML_EYTRM" );
    _conversions() << Conversion( QString::fromUtf8( "î" ), "XML_IHAT" );
    _conversions() << Conversion( QString::fromUtf8( "ï" ), "XML_ITREM" );
    _conversions() << Conversion( QString::fromUtf8( "ô" ), "XML_OHAT" );
    _conversions() << Conversion( QString::fromUtf8( "ö" ), "XML_OTREM" );
    _conversions() << Conversion( QString::fromUtf8( "ù" ), "XML_UGRAVE" );
    _conversions() << Conversion( QString::fromUtf8( "û" ), "XML_UHAT" );
    _conversions() << Conversion( QString::fromUtf8( "ç" ), "XML_CCED" );

    _conversions() << Conversion( "\t", "XML_TAB" );
    _conversions() << Conversion( "\n", "XML_ENDL" );

    // this conversion is needed for XML not to remove entries that consist of empty spaces only
    // it is used in xmlToText but not in textToXml
    _conversions() << Conversion( "", "XML_NONE" );

    return;

}

//__________________________________
XmlString::XmlString( const QString& other ):
    value_( other )
{
    if( !( _conversions().size() ) ) _initConversions();

    // HTML style conversions (escape characters)
    ConversionListIterator iter( _conversions() );
    iter.toBack();
    while( iter.hasPrevious() )
    {
        const Conversion& current( iter.previous() );
        if( !current.second.isEmpty() ) value_ = value_.replace( current.second, current.first );
    }

}
