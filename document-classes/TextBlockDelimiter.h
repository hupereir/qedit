#ifndef TextBlockDelimiter_h
#define TextBlockDelimiter_h

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

#include <QTextStream>
#include <QVector>

namespace TextBlock
{

    //* counts how many times a block appears as a begin and a end block
    class Delimiter final: private Base::Counter<Delimiter>
    {
        public:

        //* constructor
        explicit Delimiter():
            Counter( QStringLiteral("TextBlock::Delimiter") )
        {}

        //*@name accessors
        //@{

        //* number of times the block is of type "begin"
        int begin( bool isCommented = false ) const
        { return (isCommented ? commentedPair_:pair_).begin_; }

        //* number of times the block is of type "end"
        int end( bool isCommented = false ) const
        { return (isCommented ? commentedPair_:pair_).end_; }

        //@}

        //*@name modifiers
        //@{

        //* sum operator (warning: this is not a reflexive operator)
        Delimiter& operator += ( const Delimiter& );

        //* increment
        void increment( bool isCommented )
        { (isCommented ? commentedPair_:pair_).increment(); }

        //* decrement
        void decrement( bool isCommented )
        { (isCommented ? commentedPair_:pair_).decrement(); }

        //@}

        //* delimiter list

        class List: private Base::Counter<List>
        {
            public:

            //* constructor
            explicit List():
                Counter( QStringLiteral("TextBlock::Delimiter::List") )
                {}

            //* sum operator (warning: this is not a reflexive operator)
            List& operator += (const List& );

            //* set value at index i. Resize if needed
            bool set( int, const TextBlock::Delimiter& );

            //* accessors
            const QVector<Delimiter>& get() const { return delimiters_; }
            QVector<Delimiter>& get() { return delimiters_; }

            //* get value at index i
            TextBlock::Delimiter get( int ) const;

            private:

            QVector<Delimiter> delimiters_;

        };


        private:

        class Pair
        {

            public:

            //* constructor
            explicit Pair()
            {}

            //* sum operator (warning: this is not a reflexive operator)
            Pair& operator += ( const Pair& );

            //* increment
            void increment()
            { begin_++; }

            //* decrement
            void decrement()
            {
                if( begin_ > 0 ) begin_--;
                else end_++;
            }

            //* number of times the block is of type "begin"
            int begin_ = 0;

            //* number of times the block is of type "end"
            int end_ = 0;

            //* equal to
            friend bool operator == (const Pair& first, const Pair& second)
            { return first.begin_ == second.begin_ && first.end_ == second.end_; }

        };

        Pair pair_;
        Pair commentedPair_;

        //* streamer
        friend QTextStream& operator << ( QTextStream& out, const Delimiter::Pair pair )
        {
            out << "(" << pair.begin_ << "," << pair.end_ << ")";
            return out;
        }

        //* streamer
        friend QTextStream& operator << ( QTextStream& out, const Delimiter& delimiter )
        {
            out << delimiter.pair_ << " " << delimiter.commentedPair_;
            return out;
        }

        //* equal to operator
        friend bool operator == (const Delimiter& first, const Delimiter& second )
        { return first.pair_ == second.pair_ && first.commentedPair_ == second.commentedPair_; }

    };

}

#endif
