#ifndef PatternLocation_h
#define PatternLocation_h

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
#include "HighlightPattern.h"
#include "QOrderedSet.h"
#include "TextFormat.h"

#include <QTextCharFormat>

//! encapsulate highlight location, pattern and style
class PatternLocation: public Counter
{
    public:


    using SetIterator = QOrderedSetIterator<PatternLocation>;

    //! construtor
    PatternLocation( void );

    //! constructor
    PatternLocation( const HighlightPattern&, int, int );

    //! destructor
    virtual ~PatternLocation( void )
    {}

    //! less than operator
    bool operator < (const PatternLocation& location ) const
    {
        return
            (position() < location.position()) ||
            (position() == location.position() && parentId() < location.parentId() ) ;
    }

    //! equal to operator
    bool operator == (const PatternLocation& location ) const
    { return position() == location.position() && parentId() == location.parentId(); }

    //! valid
    bool isValid( void ) const
    { return id_ >= 0 && position_ >= 0 && length_ > 0; }

    //!@name location
    //@{

    //! position
    int position( void ) const
    { return position_; }

    //! length
    int length( void ) const
    { return length_; }


    //! used to find a location matching index
    class ContainsFTor
    {

        public:

        //! constructor
        ContainsFTor( const int& index ):
            index_( index )
        {}

        //! prediction
        bool operator() (const PatternLocation& location ) const
        {
            return
                index_ >= location.position() &&
                index_ < location.position()+int(location.length());
        }

        private:

        //! predicted index
        int index_;

    };

    // overlaps
    class OverlapFTor
    {

        public:

        //! predicate
        bool operator() (const PatternLocation& first, const PatternLocation& second )
        { return second.position() < first.position() + (int)first.length(); }

    };

    //@}

    //!@name highlight pattern
    //@{

    //! pattern id
    int id( void ) const
    { return id_; }

    //! parent pattern id
    int parentId( void ) const
    { return parentId_; }

    //! flags
    HighlightPattern::Flags flags( void ) const
    { return flags_; }

    //! flags
    bool hasFlag( const HighlightPattern::Flag& flag ) const
    { return flags() & flag; }

    //@}

    //!@name highlight style
    //@{

    //! format
    Format::TextFormatFlags fontFormat( void ) const
    { return format_; }

    //! color
    virtual QColor color( void ) const
    { return color_; }

    //! formated font
    virtual QTextCharFormat format() const;

    //}

    private:

    //! pattern id
    int id_;

    //! pattern parent id
    int parentId_;

    //! pattern flags
    HighlightPattern::Flags flags_;

    //! style font format
    Format::TextFormatFlags format_;

    //! style color
    QColor color_;

    //! position in text
    int position_;

    //! length of the pattern
    unsigned int length_;

    //! dump
    friend QTextStream& operator << (QTextStream& out, const PatternLocation& location )
    {
        out << "id: " << location.id() << " parent id:" << location.parentId() << " position: " << location.position() << " length: " << location.length() ;
        return out;
    }

};

#endif
