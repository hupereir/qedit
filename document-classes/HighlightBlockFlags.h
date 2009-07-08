#ifndef HighlightBlockFlags_h
#define HighlightBlockFlags_h

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
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     
* Place, Suite 330, Boston, MA 02111-1307 USA                           
*                         
*                         
*******************************************************************************/
 
/*!
  \file HighlightBlockFlags.h
  \brief handles block tags
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/  

#include <QTextFormat>
#include <QTextStream>

#include <map>

#include "TextBlockFlags.h"

namespace TextBlock
{
  
  enum
  {
      
    //! block is modified
    MODIFIED = 1<<2,
 
    //! block is collapsed
    COLLAPSED = 1<<3,
   
    //! block is a diff-wise added block
    DIFF_ADDED = 1<<4,
    
    //! block is a diff-wise conflict block 
    DIFF_CONFLICT = 1<<5,
    
    //! user tagged blocks
    USER_TAG = 1<<6,
 
    //! all tags
    ALL_TAGS = DIFF_ADDED | DIFF_CONFLICT | USER_TAG
    
  };
  
  //! block format properties
  enum BlockFormatProperties
  {
    
    Collapsed = QTextFormat::UserProperty | (1<<0),
    CollapsedData = QTextFormat::UserProperty | (1<<1)  
    
  };

  //! counts how many times a block appears as a begin and a end block
  class Delimiter
  {
    public:
        
    //! constructor
    Delimiter( const int& begin = 0, const int& end = 0 ):
      begin_( begin ),
      end_( end )
    {}

    //! different operator
    bool operator != (const Delimiter& delimiter ) const
    { return begin_ != delimiter.begin_ || end_ != delimiter.end_; }

    //! equal to operator
    bool operator == (const Delimiter& delimiter ) const
    { return begin_ == delimiter.begin_ && end_ == delimiter.end_; }

    //! sum operator (warning: this is not a reflexive operator)
    Delimiter operator + ( const Delimiter& ) const;
    
    //! sum operator (warning: this is not a reflexive operator)
    Delimiter& operator += ( const Delimiter& delimiter )
    { 
      *this = *this + delimiter;
      return *this;
    }
    
    //! number of times the block is of type "begin"
    const int& begin( void ) const
    { return begin_; }
    
    //! number of times the block is of type "begin"
    int& begin( void )
    { return begin_; }

    //! number of times the block is of type "end"
    const int& end( void ) const
    { return end_; }
        
    //! number of times the block is of type "end"
    int& end( void )
    { return end_; }

    //! delimiter list
    class List: public std::vector<Delimiter> 
    {
      public:
            
      //! sum operator (warning: this is not a reflexive operator)
      List operator + (const List& list ) const;
      
      //! sum operator (warning: this is not a reflexive operator)
      List& operator += (const List& list )
      { 
        *this = *this + list;
        return *this;
      }
      
      //! set value at index i. Resize if needed
      bool set( const unsigned int&, const TextBlock::Delimiter& );
      
      //! get value at index i
      TextBlock::Delimiter get( const unsigned int& ) const;

      //! streamer
      friend QTextStream& operator << ( QTextStream& out, const List& list )
      {
        for( List::const_iterator iter = list.begin(); iter != list.end(); iter++ )
        { out << " " << *iter; }
        return out;
      }
      
    };
    
    
    private:
    
    //! number of times the block is of type "begin"
    int begin_;
    
    //! number of times the block is of type "end"
    int end_;
    
    //! streamer
    friend QTextStream& operator << ( QTextStream& out, const Delimiter& delimiter )
    {
      out << "(" << delimiter.begin_ << "," << delimiter.end_ << ")";
      return out;
    }
    
  };
  
};


#endif
