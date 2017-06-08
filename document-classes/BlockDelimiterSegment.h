#ifndef BlockDelimiterSegment_h
#define BlockDelimiterSegment_h

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

#include "BlockMarker.h"
#include <QList>
#include <QPair>
#include <QPoint>
#include <QRect>

// used to draw block segment
class BlockDelimiterSegment: private Base::Counter<BlockDelimiterSegment>
{

    public:

    //* list
    using List = QList<BlockDelimiterSegment>;
    using ListIterator = QListIterator<BlockDelimiterSegment>;
    using MutableListIterator = QMutableListIterator<BlockDelimiterSegment>;

    //* flags
    enum Flag
    {
        None = 0,
        Collapsed = 1<<0,
        Ignored = 1<<2,
        BeginOnly = 1<<3
    };

    Q_DECLARE_FLAGS( Flags, Flag )

    //* constructor
    explicit BlockDelimiterSegment(
        const BlockMarker& begin = BlockMarker(),
        const BlockMarker& end = BlockMarker(),
        Flags flags = None ):
        Counter( "BlockDelimiterSegment" ),
        begin_( begin ),
        end_( end ),
        flags_( flags )
    {}

    //*@name flags
    //@{

    //* flags
    bool hasFlag( const Flag& flag ) const
    { return flags_ & flag; }

    //* flags
    BlockDelimiterSegment& setFlag( const Flag& flag, bool value )
    {
        if( value ) flags_ |= flag;
        else flags_ &= (~flag);
        return *this;
    }

    //@}

    //* validity
    bool isValid( void ) const
    { return ( begin().isValid() && ( hasFlag( BeginOnly ) || end().isValid() ) ); }

    //*@name geometry
    //@{

    //* begin point
    const BlockMarker& begin( void ) const
    { return begin_; }

    //* begin point
    BlockMarker& begin( void )
    { return begin_; }

    //* begin point
    BlockDelimiterSegment& setBegin( const BlockMarker& begin )
    {
        begin_ = begin;
        return *this;
    }

    //* end point
    const BlockMarker& end( void ) const
    { return end_; }

    //* end point
    BlockMarker& end( void )
    { return end_; }

    //* end point
    BlockDelimiterSegment& setEnd( const BlockMarker& end )
    {
        end_ = end;
        return *this;
    }

    //* empty segment
    bool empty( void ) const
    { return begin().cursor() == end().cursor(); }

    //* active rect
    const QRect& activeRect( void ) const
    { return active_; }

    //* active rect
    void setActiveRect( const QRect& rect )
    { active_ = rect; }

    //@}

    //* used to find segment for which the active rect match a point
    class ActiveFTor
    {

        public:

        //* creator
        explicit ActiveFTor( const QPoint& point ):
            point_( point )
        {}

        //* prediction
        bool operator() (const BlockDelimiterSegment& segment ) const
        { return segment.activeRect().contains( point_ ); }

        private:

        //* position
        QPoint point_;

    };

    //* used to find segment that match the cursor location and collapse state
    class ContainsFTor
    {
        public:

        //* constructor
        explicit ContainsFTor( int cursor ):
            cursor_( cursor )
            {}

        //* prediction
        bool operator() (const BlockDelimiterSegment& segment ) const
        { return cursor_ >= segment.begin().cursor() && ( segment.empty() || cursor_ <= segment.end().cursor() ); }

        private:

        //* position
        int cursor_;

    };

    //* used to cound collapsed segments
    class CollapsedFTor
    {
        public:

        bool operator() ( const BlockDelimiterSegment& segment ) const
        { return segment.hasFlag( Collapsed ); }

    };

    //* used to sort segments according to starting or ending points
    /** top level segments should comme last */
    class SortFTor
    {

        public:

        bool operator() ( const BlockDelimiterSegment& first, const BlockDelimiterSegment& second ) const
        { return ( second.begin() < first.begin() || (first.begin() == second.begin() && first.end() < second.end() ) ); }

    };

    private:

    //* first position
    BlockMarker begin_;

    //* end position
    BlockMarker end_;

    //* active area (for mouse pointing)
    /** it is set if drawFirstDelimiter() is called */
    QRect active_;

    //* flags
    Flags flags_ = 0;

    //* streamer
    friend QTextStream& operator << ( QTextStream& out, const BlockDelimiterSegment& segment )
    {
        out << "begin: " << segment.begin() << " end: " << segment.end() << " flags: " << segment.flags_;
        return out;
    }

    //* streamer
    friend QTextStream& operator << ( QTextStream& out, const BlockDelimiterSegment::List& segments )
    {
        for( const auto& segment:segments )
        { out << segment << endl; }
        return out;
    }

};

Q_DECLARE_OPERATORS_FOR_FLAGS( BlockDelimiterSegment::Flags )

#endif
