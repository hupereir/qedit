#ifndef PatternLocationSet_h
#define PatternLocationSet_h

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

#include "PatternLocation.h"

#include "QOrderedSet.h"

//* set of pattern locations
class PatternLocationSet: public QOrderedSet<PatternLocation>
{

    public:

    //* default constructor
    PatternLocationSet():
        activeId_( std::make_pair( 0, 0 ) )
    {}

    //* active id
    const std::pair<int,int>& activeId( void ) const
    { return activeId_; }

    //* active id
    std::pair<int,int>& activeId( void )
    { return activeId_; }

    // return true if current position corresponds to commented text
    bool isCommented( int ) const;

    private:

    //* active patterns from previous and this paragraph
    std::pair<int, int> activeId_;

    //* dump
    friend QTextStream& operator << (QTextStream& out, const PatternLocationSet& locations )
    {
        out << "[" << locations.activeId().first << "," << locations.activeId().second << "] ";
        foreach( const PatternLocation& location, locations )
        { out << location << endl; }
        return out;
    }

};

#endif
