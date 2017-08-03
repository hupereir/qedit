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
    static ConversionList conversions( {

        Conversion( "<", "XML_LT" ),
        Conversion( ">", "XML_GT" ),
        Conversion( "&", "XML_AND" ),
        Conversion( "\"", "XML_QUOTE" ),

        Conversion( QString::fromUtf8( "à" ), "XML_AGRAVE" ),
        Conversion( QString::fromUtf8( "â" ), "XML_AHAT" ),
        Conversion( QString::fromUtf8( "ä" ), "XML_ATREM" ),
        Conversion( QString::fromUtf8( "é" ), "XML_ECUTE" ),
        Conversion( QString::fromUtf8( "è" ), "XML_EGRAVE" ),
        Conversion( QString::fromUtf8( "ê" ), "XML_EHAT" ),
        Conversion( QString::fromUtf8( "ë" ), "XML_EYTRM" ),
        Conversion( QString::fromUtf8( "î" ), "XML_IHAT" ),
        Conversion( QString::fromUtf8( "ï" ), "XML_ITREM" ),
        Conversion( QString::fromUtf8( "ô" ), "XML_OHAT" ),
        Conversion( QString::fromUtf8( "ö" ), "XML_OTREM" ),
        Conversion( QString::fromUtf8( "ù" ), "XML_UGRAVE" ),
        Conversion( QString::fromUtf8( "û" ), "XML_UHAT" ),
        Conversion( QString::fromUtf8( "ç" ), "XML_CCED" ),

        Conversion( "\t", "XML_TAB" ),
        Conversion( "\n", "XML_ENDL" ),

        // this conversion is needed for XML not to remove entries that consist of empty spaces only
        // it is used in xmlToText but not in textToXml
        Conversion( "", "XML_NONE" )
    } );

    return conversions;
}

//__________________________________
XmlString::XmlString( const QString& other ):
    value_( other )
{

    // HTML style conversions (escape characters)
    ConversionListIterator iter( _conversions() );
    iter.toBack();
    while( iter.hasPrevious() )
    {
        const Conversion& current( iter.previous() );
        if( !current.second.isEmpty() ) value_ = value_.replace( current.second, current.first );
    }

}
