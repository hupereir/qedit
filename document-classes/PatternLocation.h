#ifndef _PatternLocation_h_
#define _PatternLocation_h_

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
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        
 * for more details.                     
 *                          
 * You should have received a copy of the GNU General Public License along with 
 * software; if not, write to the Free Software Foundation, Inc., 59 Temple     
 * Place, Suite 330, Boston, MA  02111-1307 USA                           
 *                         
 *                         
 *******************************************************************************/

/*!
  \file PatternLocation.h
  \brief encapsulate highlight location, pattern and style
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QTextCharFormat>

#include "Counter.h"
#include "HighlightPattern.h"

//! encapsulate highlight location, pattern and style
class PatternLocation: public Counter
{
  public:        
  
  //! constructor
  PatternLocation( 
    const HighlightPattern& parent,
    const int& position, 
    const unsigned int& length ):
    Counter( "PatternLocation" ),
    id_( parent.id() ),
    parent_id_( parent.parentId() ),
    flags_( parent.flags() ),
    format_( parent.style().fontFormat() ),
    color_( parent.style().color() ),
    position_( position ),
    length_( length )
  { }
  
  //! less than operator
  bool operator < (const PatternLocation& location ) const
  { 
    return 
      (position() < location.position()) ||
      (position() == location.position() && parentId() < location.parentId() ) ; 
  }
    
  //!@name location
  //@{
  
  //! position
  const int& position( void ) const
  { return position_; }
  
  //! length
  const unsigned int& length( void ) const
  { return length_; }
  
    
  //! used to find a location matching index
  class ContainsFTor 
  {
    
    public:
    
    //! constructor
    ContainsFTor( const int& index ):
      index_( index )
      {}
      
    //! prediction
    bool operator() (const PatternLocation& location )
    { 
      return 
        index_ >= location.position() && 
        index_ < location.position()+int(location.length()); 
    }
      
    private:
    
    //! predicted index
    int index_;
    
  };
  
  // overlaps
  class OverlapFTor
  {
    
    public:
    
    //! predicate
    bool operator() (const PatternLocation& first, const PatternLocation& second ) 
    { return second.position() < first.position() + (int)first.length(); }
    
  };
  
  //@}
  
  //!@name highlight pattern
  //@{
  
  //! pattern id
  int id( void ) const
  { return id_; }
  
  //! parent pattern id
  int parentId( void ) const
  { return parent_id_; }

  //! flags
  unsigned int flags( void ) const
  { return flags_; }

  //! flags
  bool flag( const HighlightPattern::Flag& flag ) const
  { return flags() & flag; }

  //@}
  
  //!@name highlight style
  //@{

  //! format
  unsigned int fontFormat( void ) const
  { return format_; }

  //! color
  virtual QColor color( void ) const
  { return color_; }
  
  //! formated font
  virtual QTextCharFormat format() const
  {
    
    QTextCharFormat out;
    
    out.setFontWeight( (format_&FORMAT::BOLD) ? QFont::Bold : QFont::Normal );
    out.setFontItalic( format_&FORMAT::ITALIC );
    out.setFontUnderline( format_&FORMAT::UNDERLINE );
    out.setFontOverline( format_&FORMAT::OVERLINE );
    out.setFontStrikeOut( format_&FORMAT::STRIKE );
    if( color_.isValid() ) out.setForeground( color_ );
    
    return out;
  }
  
  //}

  private:
  
  //! pattern id
  int id_;
  
  //! pattern parent id
  int parent_id_;
  
  //! pattern flags
  unsigned int flags_;
  
  //! style font format
  unsigned int format_;
  
  //! style color
  QColor color_;
  
  //! position in text
  int position_;
  
  //! length of the pattern
  unsigned int length_;
  
  //! dump
  friend std::ostream& operator << (std::ostream& out, const PatternLocation& location )
  {  
    out << "id: " << location.id() << " parent id:" << location.parentId() << " position: " << location.position() << " length: " << location.length() ;
    return out;
  }
  
};

  
//! set of locations. 
class PatternLocationSet: public std::set<PatternLocation>
{
  
  public:
  
  //! default constructor
  PatternLocationSet():
    active_id_( std::make_pair( 0, 0 ) )
    {}
    
  //! active id
  const std::pair<int,int>& activeId( void ) const
  { return active_id_; }
    
  //! active id
  std::pair<int,int>& activeId( void )
  { return active_id_; }
  
  private:
  
  //! active patterns from previous and this paragraph
  std::pair<int, int> active_id_;
  
  //! dump
  friend std::ostream& operator << (std::ostream& out, const PatternLocationSet& locations )
  {  
    out << "[" << locations.activeId().first << "," << locations.activeId().second << "] ";
    for( PatternLocationSet::const_iterator iter = locations.begin(); iter != locations.end(); iter++ )
      out << *iter << std::endl;
    return out;
  }

};

#endif
