#ifndef BlockMarker_h
#define BlockMarker_h

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

//* stores block position
//* needed to handle block geometry
class BlockMarker
{
    public:

    //* constructor
    explicit BlockMarker( int id = 0, int cursor = 0, int position = -1 ):
        id_( id ),
        cursor_( cursor ),
        position_( position ),
        valid_( position >= 0 )
    {}

    //* id
    int id() const
    { return id_; }

    //* cursor
    int cursor() const
    { return cursor_; }

    //* position
    void setPosition( int position )
    {
        valid_ = (position >= 0);
        position_ = position;
    }

    //* position
    int position() const
    { return position_; }

    //* validity
    bool isValid() const
    { return valid_; }

    private:

    //* id
    int id_ = 0;

    //* cursor position
    int cursor_ = 0;

    //* position
    int position_ = 0;

    //* validity
    bool valid_  = false;

    //* streamer
    friend QTextStream& operator << ( QTextStream& out, const BlockMarker& marker )
    {
        out << "(" << marker.id() << "," << marker.cursor() << "," << marker.position() << ")";
        return out;
    }

};


//* equal to operator
inline bool operator == ( const BlockMarker& first, const BlockMarker& second )
{ return first.cursor() == second.cursor(); }

//* different from operator
inline bool operator != ( const BlockMarker& first, const BlockMarker& second )
{ return !( first == second ); }

//* less than operator
inline bool operator < ( const BlockMarker& first, const BlockMarker& second )
{ return first.cursor() < second.cursor(); }

#endif
