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
class PatternLocationSet final
{

    public:

    //* default constructor
    explicit PatternLocationSet():
        activeId_( std::make_pair( 0, 0 ) )
    {}

    //*@name accessors
    //@{

    //* set
    const QOrderedSet<PatternLocation>& get() const { return set_; }

    //* active id
    const std::pair<int,int>& activeId() const
    { return activeId_; }

    //* return true if current position corresponds to commented text
    bool isCommented( int ) const;

    using const_iterator = QOrderedSet<PatternLocation>::const_iterator;
    const_iterator begin() const { return set_.begin(); }
    const_iterator end() const { return set_.end(); }
    const_iterator find( const PatternLocation& location ) const { return set_.find( location ); }

    int size() const { return set_.size(); }
    bool empty() const { return set_.empty(); }

    //@}

    //*@name modifiers
    //@{

    //* set
    QOrderedSet<PatternLocation>& get() { return set_; }

    //* active id
    std::pair<int,int>& activeId()
    { return activeId_; }

    using iterator = QOrderedSet<PatternLocation>::iterator;
    iterator begin() { return set_.begin(); }
    iterator end() { return set_.end(); }

    const_iterator insert( const PatternLocation& location ) { return set_.insert( location ); }
    bool remove( const PatternLocation& location ) { return set_.remove( location ); }

    //* clear
    void clear() { set_.clear(); }

    //* erase
    template<class T> iterator erase(const T& t) { return set_.erase( t ); }

    //@}

    private:

    //* set
    QOrderedSet<PatternLocation> set_;

    //* active patterns from previous and this paragraph
    std::pair<int, int> activeId_;

    //* dump
    friend QTextStream& operator << (QTextStream& out, const PatternLocationSet& locations )
    {
        out << "[" << locations.activeId().first << "," << locations.activeId().second << "] ";
        for( const auto& location:locations )
        { out << location << Qt::endl; }
        return out;
    }

};

#endif
