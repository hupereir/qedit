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

#include "TextBlockDelimiter.h"
#include "Debug.h"

namespace TextBlock
{

    //_______________________________________________
    Delimiter Delimiter::operator + (const Delimiter& other ) const
    {
        Debug::Throw( "Delimiter::operator +=\n" );
        Delimiter out = *this;
        out.pair_ = out.pair_ + other.pair_;
        out.commentedPair_ = out.commentedPair_ + other.commentedPair_;
        return out;
    }

    //_______________________________________________
    Delimiter::Pair Delimiter::Pair::operator + (const Delimiter::Pair& other ) const
    {
        Debug::Throw( "Delimiter::operator +=\n" );
        Delimiter::Pair out = *this;

        for( int i=0; i < other.end_; i++ )
        {
            if( out.begin_ > 0 ) out.begin_--;
            else out.end_++;
        }

        out.begin_ += other.begin_;
        return out;

    }

    //_______________________________________________
    Delimiter::List Delimiter::List::operator + (const Delimiter::List& list ) const
    {
        Debug::Throw( "Delimiter::List::operator +=\n" );
        Delimiter::List out( *this );
        Delimiter::List::iterator first = out.begin();
        Delimiter::List::const_iterator second = list.begin();
        for(; first != out.end(); first++ )
        {
            if( second != list.end() )
            {
                *first += *second;
                second ++;
            }
        }

        for(; second != list.end(); second++ )
        { out << *second; }

        return out;

    }

    //_______________________________________________
    bool Delimiter::List::set( int i, const Delimiter& delimiter )
    {
        if( size() > i && (*this)[i] == delimiter ) return false;
        if( size() <= i ) { resize( i+1 ); }
        (*this)[i] = delimiter;
        return true;
    }

    //_______________________________________________
    Delimiter Delimiter::List::get( int i ) const
    {
        if( size() <= i ) return TextBlock::Delimiter();
        return (*this)[i];
    }

}
