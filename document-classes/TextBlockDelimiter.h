#ifndef TextBlockDelimiter_h
#define TextBlockDelimiter_h

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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA 02111-1307 USA
*
*
*******************************************************************************/

#include "Counter.h"

#include <QTextStream>
#include <QVector>

namespace TextBlock
{

    //! counts how many times a block appears as a begin and a end block
    class Delimiter: public Counter
    {
        public:

        //! constructor
        Delimiter( void ):
            Counter( "TextBlock::Delimiter" ),
            begin_( 0 ),
            end_( 0 )
        {}


        //! equal to operator
        bool operator == (const Delimiter& other ) const
        { return begin_ == other.begin_ && end_ == other.end_; }

        //! different operator
        bool operator != (const Delimiter& other ) const
        { return !( *this == other ); }

        //!@name accessors
        //@{

        //! number of times the block is of type "begin"
        const int& begin( void ) const
        { return begin_; }

        //! number of times the block is of type "end"
        const int& end( void ) const
        { return end_; }

        //@}

        //!@name modifiers
        //@{

        //! sum operator (warning: this is not a reflexive operator)
        Delimiter operator + ( const Delimiter& ) const;

        //! sum operator (warning: this is not a reflexive operator)
        Delimiter& operator += ( const Delimiter& delimiter )
        { return *this = *this + delimiter; }

        //! increment
        void increment( void )
        { begin_++; }

        //! decrement
        void decrement( void )
        {
            if( begin_ > 0 ) begin_--;
            else end_++;
        }

        //@}

        //! delimiter list
        class List: public QVector<Delimiter>, public Counter
        {
            public:

            //! constructor
            List( void ):
                Counter( "TextBlock::Delimiter::List" )
                {}

            //! sum operator (warning: this is not a reflexive operator)
            List operator + (const List& list ) const;

            //! sum operator (warning: this is not a reflexive operator)
            List& operator += (const List& list )
            {
                *this = *this + list;
                return *this;
            }

            //! set value at index i. Resize if needed
            bool set( int, const TextBlock::Delimiter& );

            //! get value at index i
            TextBlock::Delimiter get( int ) const;

            //! streamer
            friend QTextStream& operator << ( QTextStream& out, const List& list )
            {
                foreach( const Delimiter& delimiter, list )
                { out << " " << delimiter; }
                return out;
            }

        };


        private:

        //! number of times the block is of type "begin"
        int begin_;

        //! number of times the block is of type "end"
        int end_;

        //! streamer
        friend QTextStream& operator << ( QTextStream& out, const Delimiter& delimiter )
        {
            out << "(" << delimiter.begin_ << "," << delimiter.end_ << ")";
            return out;
        }

    };

}

#endif
