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

#include <QPair>

//* text to Xml conversion pair list
using Conversion = QPair<QString, QString>;
static const auto conversions =
{
    Conversion( "\t", "XML_TAB" ),
    Conversion( "\n", "XML_ENDL" )
};

//__________________________________
XmlString::XmlString( const QString& other ):
    value_( other )
{
    for( const auto& conversion:conversions )
    { if( !conversion.second.isEmpty() ) value_ = value_.replace( conversion.second, conversion.first ); }
}
