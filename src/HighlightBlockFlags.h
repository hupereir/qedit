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

#include <iostream>

#include "TextBlockFlags.h"

namespace TextBlock
{
  
  enum
  {
  
    //! block is modified
    MODIFIED = 1<<2,
    
    //! block is a diff-wise added block
    DIFF_ADDED = 1<<3,
    
    //! block is a diff-wise conflict block 
    DIFF_CONFLICT = 1<<4,
    
    //! user tagged blocks
    USER_TAG = 1<<5,
 
    //! all tags
    ALL_TAGS = DIFF_ADDED | DIFF_CONFLICT | USER_TAG
    
  };
  
  //! counts how many times a block appears as a begin and a end block
  class Delimiter
  {
    public:
    
    //! constructor
    Delimiter( const unsigned int& begin = 0, const unsigned int& end = 0 ):
      begin_( begin ),
      end_( end )
    {}
    
    //! number of times the block is of type "begin"
    unsigned int begin_;
    
    //! number of times the block is of type "end"
    unsigned int end_;
    
    //! streamer
    friend std::ostream& operator << ( std::ostream& out, const Delimiter& dilimiter )
    {
      out << "(" << dilimiter.begin_ << "," << dilimiter.end_ << ")";
      return out;
    }
    
  };
  
};


#endif
