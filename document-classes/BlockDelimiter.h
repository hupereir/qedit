#ifndef BlockDelimiter_h
#define BlockDelimiter_h

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


#include "Counter.h"
#include "Debug.h"

#include <QDomElement>
#include <QDomDocument>
#include <QRegExp>
#include <QList>

//* text delimiter (for highlighting)
class BlockDelimiter final: private Base::Counter<BlockDelimiter>
{

    public:

    //* typedef for list of patterns
    using List = QList<BlockDelimiter>;

    //* constructor from DomElement
    explicit BlockDelimiter( const QDomElement& element = QDomElement(), int id = 0 );

    //* dom element
    QDomElement domElement( QDomDocument& parent ) const;

    //* Id
    int id() const
    { return id_; }

    //* block start
    const QString& first() const
    { return first_; }

    //* first
    void setFirst( const QString& value )
    { first_ = value; }

    //* block end
    const QString& second() const
    { return second_; }

    //* second
    void setSecond( const QString& value )
    { second_ = value; }

    //* regExp that match either block start or end
    const QRegExp& regexp() const
    { return regexp_; }

    //* regext
    void setRegexp( const QString& value )
    { regexp_.setPattern( value ); }


    private:

    //* unique id
    int id_ = 0;

    //* regular expression that match first character
    QString first_;

    //* regular expression that match second character
    QString second_;

    //* regular expression that match either of both characters
    QRegExp regexp_;

    //* streamer
    friend QTextStream& operator << ( QTextStream& out, const BlockDelimiter& delimiter )
    {
        out << " first: " << delimiter.first()
            << " second: " << delimiter.second()
            << " regexp: " << delimiter.regexp().pattern();
        return out;
    }

};

//* equal to operator
inline bool operator == ( const BlockDelimiter& first, const BlockDelimiter& second )
{
    return
        first.first() == second.first() &&
        first.second() == second.second() &&
        first.regexp() == second.regexp();
}

#endif
