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

#include "Config.h"

#if WITH_ASPELL
#include "Word.h"
#endif

//! TextBlock data
class HighlightBlockData: public TextBlockData
{
  
  public: 
  
  //! constructor
  HighlightBlockData():
    TextBlockData(),
    modified_( true )
  {}
    
  //! constructor
  HighlightBlockData( const TextBlockData& reference ):
    TextBlockData( reference ),
    modified_( true )
  {}
    
  //! constructor
  HighlightBlockData( const TextBlockData* pointer ):
    TextBlockData( *pointer ),
    modified_( true )
  {}
    
  //! destructor
  virtual ~HighlightBlockData( void )
  {}
  
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
  
  #if WITH_ASPELL
  //! set of misspelled words
  const SPELLCHECK::Word::Set& misspelledWords( void ) const
  { return words_; }
  
  //! set of misspelled words
  void setMisspelledWords( const SPELLCHECK::Word::Set& words )
  { words_ = words; }
  
  //! return misspelled word matching position, if any
  SPELLCHECK::Word misspelledWord( const int& position ) const;
  #endif
  
  private:
  
  //! block modification state
  /*! it is used to tell if patterns must be re-calculated */
  bool modified_;
  
  //! locations and ids of matching syntax highlighting patterns
  HighlightPattern::LocationSet locations_;
  
  #if WITH_ASPELL
  //! set of misspelled words and position in associated block
  SPELLCHECK::Word::Set words_;
  #endif
  
};

#endif
  
