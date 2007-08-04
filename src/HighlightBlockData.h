#ifndef HighlightBlockData_h
#define HighlightBlockData_h

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
  \file HighlightBlockData.h
  \brief TextBlock data for syntax highlighting
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/  

#include "TextBlockData.h"
#include "HighlightPattern.h"

//! TextBlock data
class HighlightBlockData: public TextBlockData
{
  
  public: 
  
  //! constructor
  HighlightBlockData():
    TextBlockData()
  { Debug::Throw( "HighlightBlockData::HighlightBlockData.\n" ); }
    
  //! constructor
  HighlightBlockData( const TextBlockData& reference ):
    TextBlockData( reference )
  { Debug::Throw( "HighlightBlockData::HighlightBlockData.\n" ); }
    
  //! constructor
  HighlightBlockData( const TextBlockData* pointer ):
    TextBlockData( *pointer )
  { Debug::Throw( "HighlightBlockData::HighlightBlockData.\n" ); }
    
  //! destructor
  virtual ~HighlightBlockData( void )
  { Debug::Throw( "HighlightBlockData::~HighlightBlockData.\n" ); }
  
  //! modification state
  const bool& isModified( void ) const
  { return modified_; }
  
  //! modification state
  void setModified( const bool& value )
  { modified_ = value; }
  
  //! syntax highlighting pattern locations
  const HighlightPattern::LocationSet& locations( void ) const
  { return locations_; }
  
  //! syntax highlighting pattern locations
  void setLocations( const HighlightPattern::LocationSet& locations )
  { locations_ = locations; }
  
  private:
  
  //! block modification state
  /*! it is used to tell if patterns must be re-calculated */
  bool modified_;
  
  //! locations and ids of matching syntax highlighting patterns
  HighlightPattern::LocationSet locations_;
  
};

#endif
  
