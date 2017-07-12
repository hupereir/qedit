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
    Delimiter& Delimiter::operator += (const Delimiter& other )
    {
        pair_ += other.pair_;
        commentedPair_ += other.commentedPair_;
        return *this;
    }

    //_______________________________________________
    Delimiter::Pair& Delimiter::Pair::operator += (const Delimiter::Pair& other )
    {
        Debug::Throw( "Delimiter::operator +=\n" );

        for( int i=0; i < other.end_; i++ )
        {
            if( begin_ > 0 ) begin_--;
            else end_++;
        }

        begin_ += other.begin_;
        return *this;

    }

    //_______________________________________________
    Delimiter::List& Delimiter::List::operator += (const Delimiter::List& other )
    {
        auto first = delimiters_.begin();
        auto second = other.delimiters_.begin();
        for(; first != delimiters_.end(); first++ )
        {
            if( second != other.delimiters_.end() )
            {
                *first += *second;
                second ++;
            }
        }

        for(; second != other.delimiters_.end(); second++ )
        { delimiters_.append( *second ); }

        return *this;

    }

    //_______________________________________________
    bool Delimiter::List::set( int i, const Delimiter& delimiter )
    {
        if( delimiters_.size() > i && delimiters_[i] == delimiter ) return false;
        if( delimiters_.size() <= i ) { delimiters_.resize( i+1 ); }
        delimiters_[i] = delimiter;
        return true;
    }

    //_______________________________________________
    Delimiter Delimiter::List::get( int i ) const
    {
        if( delimiters_.size() <= i ) return TextBlock::Delimiter();
        return delimiters_[i];
    }

}
