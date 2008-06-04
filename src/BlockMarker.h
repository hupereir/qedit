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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software , Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
\file BlockMarker.h
\brief store blocks position
\author Hugo Pereira
\version $Revision$
\date $Date$
*/


//! stores block position
//! needed to handle block geometry
class BlockMarker
{
  public:
  
  //! constructor
  BlockMarker( const unsigned int& id = 0, const int& cursor = 0, const int& position = -1 ):
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
  const unsigned int& id( void ) const
  { return id_; }
  
  //! cursor
  const int& cursor( void ) const
  { return cursor_; }
  
  //! position
  void setPosition( const int& position )
  {
    valid_ = (position >= 0);
    position_ = position;
  }
  
  //! position
  const int& position( void ) const
  { return position_; }
  
  //! validity
  const bool& isValid( void ) const
  { return valid_; }
  
  private:
  
  //! id
  unsigned int id_;
  
  //! cursor position
  int cursor_;
  
  //! position
  int position_;
  
  //! validity
  bool valid_;
  
  //! streamer
  friend std::ostream& operator << ( std::ostream& out, const BlockMarker& marker )
  {
    out << "(" << marker.id() << "," << marker.cursor() << "," << marker.position() << ")";
    return out;
  }
  
};


#endif
