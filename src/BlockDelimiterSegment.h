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
    const int& first = 0, 
    const int& second = 0, 
    const unsigned int& flags = NONE ):
    Counter( "BlockDelimiterSegment" ),
    first_( first ),
    second_( second ),
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
    FIRST_ONLY = 1<<3
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
  
  //!@name geometry
  //@{
  
  //! first point
  const int& first( void ) const
  { return first_; }
  
  //! first point
  BlockDelimiterSegment& setFirst( const int& first )
  { 
    first_ = first; 
    return *this;
  }
  
  //! second point
  const int& second( void ) const
  { return second_; }
  
  //! second point
  BlockDelimiterSegment& setSecond( const int& second )
  { 
    second_ = second; 
    return *this;
  }
      
  //! empty segment
  bool empty( void ) const
  { return first() == second(); }
  
  //! active rect
  const QRect& activeRect( void ) const
  { return active_; }
  
  //! active rect
  void setActiveRect( const QRect& rect )
  { active_ = rect; }
  
  //@}
  
  //! used to find segment matching a point
  class ContainsFTor
  {
    
    public:
    
    //! creator
    ContainsFTor( const QPoint& point ):
      point_( point )
      {}
      
    //! prediction
    bool operator() (const BlockDelimiterSegment& segment ) const
    { return segment.activeRect().contains( point_ ); }
    
    private:
    
    //! position
    QPoint point_; 
    
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
    { return ( first.first() < second.first() || (first.first() == second.first() && first.second() > second.second() ) ); }
    
  };
  
  private:
  
  //! first position
  int first_;
  
  //! second position
  int second_;
    
  //! active area (for mouse pointing)
  /*! it is set if drawFirstDelimiter() is called */
  QRect active_;

  //! flags
  unsigned int flags_;  

  //! streamer
  friend std::ostream& operator << ( std::ostream& out, const BlockDelimiterSegment& segment )
  {
    out << "(" << segment.first() << "," << segment.second() << ") flags: " << segment.flags_;
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
