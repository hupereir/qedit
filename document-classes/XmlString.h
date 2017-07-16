#ifndef XmlString_h
#define XmlString_h

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

#include <QString>
#include <QPair>
#include <QList>

//* some Xml utilities
class XmlString final
{

    public:

    //* constructor
    explicit XmlString( const QString& );

    //* conversion
    operator const QString& () const { return value_; }

    private:

    //* text to Xml conversion pair type
    using Conversion = QPair<QString, QString>;

    //* text to Xml conversion pair type
    using ConversionList = QList< Conversion >;
    using ConversionListIterator = QListIterator< Conversion >;

    //* initialize Xml to text conversion pair list
    void _initConversions() const;

    //* text to Xml conversion pair list
    static ConversionList& _conversions();

    //* value
    QString value_;

};

#endif
