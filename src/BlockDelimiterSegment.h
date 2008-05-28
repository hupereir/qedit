#ifndef BlockDelimiterSegment_h
#define BlockDelimiterSegment_h

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
\file BlockDelimiterSegment.h
\brief store blocks limits and state
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <iostream>
#include <vector>

// used to draw block segment
class BlockDelimiterSegment: public Counter
{
  
  public:
  
  //! list
  typedef std::vector<BlockDelimiterSegment> List;
  
  //! constructor
  BlockDelimiterSegment( 
    const unsigned int& flags = NONE ):
    Counter( "BlockDelimiterSegment" ),
    flags_( flags )
  {}
  
  
  //!@name flags
  //@{
  
  //! flags
  enum Flag
  {
    NONE = 0,
    COLLAPSED = 1<<0,
    IGNORED = 1<<2,
    BEGIN_ONLY = 1<<3
  };
   
  //! flags
  bool flag( const Flag& flag ) const
  { return flags_ & flag; }
  
  //! flags
  BlockDelimiterSegment& setFlag( const Flag& flag, const bool& value )
  { 
    if( value ) flags_ |= flag;
    else flags_ &= (~flag);
    return *this;
  } 
  
  //@}  
  
  //! needed to handle block geometry
  class Marker
  {
    public:
    
    //! constructor
    Marker( const int& cursor = 0, const int& position = -1 ):
      cursor_( cursor ),
      position_( position ),
      valid_( position >= 0 )
    {}
    
    //! cursor
    void setCursor( const int& cursor )
    { 
      if( cursor != cursor_ ) {
        valid_ = false;
        cursor_ = cursor;
      }
    }
    
    //! cursor
    const int& cursor( void ) const
    { return cursor_; }
    
    //! position
    void setPosition( const int& position )
    {
      valid_ = position >= 0;
      position_ = position;
    }
    
    //! position
    const int& position( void ) const
    { return position_; }
    
    //! validity
    const bool& isValid( void ) const
    { return valid_; }
    
    private:
    
    //! cursor position
    int cursor_;
    
    //! position
    int position_;
    
    //! validity
    bool valid_;
      
  };
  
  //!@name geometry
  //@{
  
  //! begin point
  const int& begin( void ) const
  { return begin_.position(); }
  
  //! begin point
  BlockDelimiterSegment& setBegin( const int& begin )
  { 
    begin_.setPosition( begin ); 
    return *this;
  }
  
  //! end point
  const int& end( void ) const
  { return end_.position(); }
  
  //! end point
  BlockDelimiterSegment& setEnd( const int& end )
  { 
    end_.setPosition( end ); 
    return *this;
  }
      
  //! empty segment
  bool empty( void ) const
  { return begin() == end(); }
  
  //! active rect
  const QRect& activeRect( void ) const
  { return active_; }
  
  //! active rect
  void setActiveRect( const QRect& rect )
  { active_ = rect; }
  
  //@}
  
  //! used to find segment for which the active rect match a point
  class ActiveFTor
  {
    
    public:
    
    //! creator
    ActiveFTor( const QPoint& point ):
      point_( point )
      {}
      
    //! prediction
    bool operator() (const BlockDelimiterSegment& segment ) const
    { return segment.activeRect().contains( point_ ); }
    
    private:
    
    //! position
    QPoint point_; 
    
  };

  //! used to find segment that match the cursor location and collapse state
  class ContainsFTor
  {
    public:
    
    //! constructor
    ContainsFTor( const int& y, const bool& collapsed ):
    y_( y ),
    collapsed_( collapsed )
    {}
    
    //! prediction
    bool operator() (const BlockDelimiterSegment& segment ) const
    { 
      return 
        segment.flag( BlockDelimiterSegment::COLLAPSED ) == collapsed_ && 
        (( segment.begin() == segment.end() ) ? (y_ >= segment.begin()) : (y_ >= segment.begin() && y_ < segment.end())); 
    }
    
    private:
    
    //! position
    int y_;
    
    //! collapse state
    bool collapsed_; 
    
  };
  
  //! used to cound collapsed segments
  class CollapsedFTor
  {
    public:
    
    bool operator() ( const BlockDelimiterSegment& segment ) const
    { return segment.flag( COLLAPSED ); }
    
  };
  
  //! used to sort segments according to starting or ending points
  /*! top level segments should comme first */
  class SortFTor
  {
    
    public:
    
    bool operator() ( const BlockDelimiterSegment& first, const BlockDelimiterSegment& second ) const
    { return ( first.begin() > second.begin() || (first.begin() == second.begin() && first.end() < second.end() ) ); }
    
  };
  
  private:
    
  //! first position
  Marker begin_;
  
  //! end position
  Marker end_;
    
  //! active area (for mouse pointing)
  /*! it is set if drawFirstDelimiter() is called */
  QRect active_;

  //! flags
  unsigned int flags_;  

  //! streamer
  friend std::ostream& operator << ( std::ostream& out, const BlockDelimiterSegment& segment )
  {
    out << "(" << segment.begin() << "," << segment.end() << ") flags: " << segment.flags_;
    return out;
  }
  
  //! streamer
  friend std::ostream& operator << ( std::ostream& out, const BlockDelimiterSegment::List& segments )
  {
    for( BlockDelimiterSegment::List::const_iterator iter = segments.begin(); iter != segments.end(); iter++ )
    { out << *iter << std::endl; }
    return out;
  }
  
};

#endif
