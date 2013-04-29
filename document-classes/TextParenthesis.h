#ifndef _TextParenthesis_h_
#define _TextParenthesis_h_

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

#include "Counter.h"
#include "Debug.h"
#include "Str.h"

#include <QDomElement>
#include <QDomDocument>
#include <QRegExp>
#include <QList>

//! text parenthesis (for highlighting)
class TextParenthesis: public Counter
{

    public:

    //! typedef for list of patterns
    typedef QList< TextParenthesis > List;

    //! constructor from DomElement
    TextParenthesis( const QDomElement& element = QDomElement() );

    //! dom element
    QDomElement domElement( QDomDocument& parent ) const;

    //! equal to operator
    bool operator == ( const TextParenthesis& parenthesis ) const
    {
        return
            first() == parenthesis.first() &&
            second() == parenthesis.second() &&
            regexp() == parenthesis.regexp();
    }

    //! less than operator
    bool operator < ( const TextParenthesis& parenthesis ) const
    {
        if( first() != parenthesis.first() ) return first() < parenthesis.first();
        if( second() != parenthesis.second() ) return second() < parenthesis.second();
        if( regexp().pattern() != parenthesis.regexp().pattern() ) return regexp().pattern() < parenthesis.regexp().pattern();
        return false;
    }

    //! block start
    const QString& first() const
    { return first_; }

    //! first
    void setFirst( const QString& value )
    { first_ = value; }

    //! block end
    const QString& second() const
    { return second_; }

    //! second
    void setSecond( const QString& value )
    { second_ = value; }

    //! regExp that match either block start or end
    const QRegExp& regexp() const
    { return regexp_; }

    //! regext
    void setRegexp( const QString& value )
    { regexp_.setPattern( value ); }

    //! used to find parenthesis for which first character match
    class MatchFTor
    {
        public:

        //! constructor
        MatchFTor( const QString& text ):
            text_( text )
        {}

        //! predicate
        bool operator() ( const TextParenthesis& parenthesis ) const
        {
            return
                text_.left( parenthesis.first().size() ) == parenthesis.first() ||
                text_.left( parenthesis.second().size() ) == parenthesis.second();
        }

        private:

        //! predicted character
        const QString& text_;

    };

    //! used to find parenthesis for which first character match
    class FirstElementFTor
    {
        public:

        //! constructor
        FirstElementFTor( const QString& text ):
            text_( text )
        {}

        //! predicate
        bool operator() ( const TextParenthesis& parenthesis ) const
        { return text_.right( parenthesis.first().size() ) == parenthesis.first(); }

        private:

        //! predicted character
        const QString& text_;

    };

    //! used to find parenthesis for which first character match
    class SecondElementFTor
    {
        public:

        //! constructor
        SecondElementFTor( const QString& text ):
            text_( text )
        {}

        //! predicate
        bool operator() ( const TextParenthesis& parenthesis ) const
        { return text_.right( parenthesis.second().size() ) == parenthesis.second(); }

        private:

        //! predicted character
        const QString& text_;

    };

    private:

    //! regular expression that match first character
    QString first_;

    //! regular expression that match second character
    QString second_;

    //! regular expression that match either of both characters
    QRegExp regexp_;

    //! streamer
    friend QTextStream& operator << ( QTextStream& out, const TextParenthesis& parenthesis )
    {
        out << " first: " << parenthesis.first()
            << " second: " << parenthesis.second()
            << " regexp: " << parenthesis.regexp().pattern();
        return out;
    }

};
#endif
