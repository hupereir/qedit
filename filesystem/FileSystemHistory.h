#ifndef FileSystemHistory_h
#define FileSystemHistory_h

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
   \file    FileSystemHistory.h
   \brief   Handles directory navigation history
   \author  Hugo Pereira
   \version $Revision$
   \date    $Date$
*/

#include <cassert>
#include <vector>

#include "Counter.h"
#include "File.h"

class FileSystemHistory: public Counter
{
  public:
  
  //! constructor
  FileSystemHistory( void ):
    Counter( "FileSystemHistory" ),
    index_(0)
  {}
  
  //! true if next path in history is valid
  bool nextAvailable( void ) const
  { return index_ + 1 < path_list_.size(); }
  
  //! retrieve next path in history  
  File next( void )
  {
    assert( nextAvailable() );
    index_++;
    return path_list_[index_];
  }

  //! true if previous path in history is valid
  bool previousAvailable( void ) const
  { return index_ > 0; }
    
  //! retrieve previous path in history
  File previous( void )
  { 
    assert( previousAvailable() );
    index_--; 
    return path_list_[index_];
  }
 
  //! add path to history
  /*! 
    Everything that is originally after the current index_ is removed;
    New path is then added after the current index.
    And the current index is moved to it.
  */
  void add( File );
  
  private:
  
  //! path list
  std::vector<File> path_list_;
  
  //! current index in list
  unsigned int index_;
      
};

#endif
