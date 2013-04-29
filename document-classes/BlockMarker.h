#ifndef BlockMarker_h
#define BlockMarker_h

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

//! stores block position
//! needed to handle block geometry
class BlockMarker
{
    public:

    //! constructor
    BlockMarker( int id = 0, int cursor = 0, int position = -1 ):
        id_( id ),
        cursor_( cursor ),
        position_( position ),
        valid_( position >= 0 )
    {}

    //! equal to operator
    bool operator == ( const BlockMarker& marker ) const
    { return cursor() == marker.cursor(); }

    //! equal to operator
    bool operator != ( const BlockMarker& marker ) const
    { return cursor() != marker.cursor(); }

    //! less than operator
    bool operator < ( const BlockMarker& marker ) const
    { return cursor() < marker.cursor(); }

    //! id
    int id( void ) const
    { return id_; }

    //! cursor
    int cursor( void ) const
    { return cursor_; }

    //! position
    void setPosition( int position )
    {
        valid_ = (position >= 0);
        position_ = position;
    }

    //! position
    int position( void ) const
    { return position_; }

    //! validity
    bool isValid( void ) const
    { return valid_; }

    private:

    //! id
    int id_;

    //! cursor position
    int cursor_;

    //! position
    int position_;

    //! validity
    bool valid_;

    //! streamer
    friend QTextStream& operator << ( QTextStream& out, const BlockMarker& marker )
    {
        out << "(" << marker.id() << "," << marker.cursor() << "," << marker.position() << ")";
        return out;
    }

};


#endif
