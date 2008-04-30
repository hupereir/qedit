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

#include <map>

#include "HighlightBlockFlags.h"
#include "TextBlockData.h"
#include "PatternLocation.h"

#include "Config.h"

#if WITH_ASPELL
#include "Word.h"
#endif

//! TextBlock data for syntax highlighting
class HighlightBlockData: public TextBlockData
{
  
  public: 
  
  //! constructor
  HighlightBlockData():
    TextBlockData(),
    parenthesis_( -1 )
  {}
    
  //! constructor
  HighlightBlockData( const TextBlockData& reference ):
    TextBlockData( reference ),
    parenthesis_( -1 )
  {}
    
  //! constructor
  HighlightBlockData( const TextBlockData* pointer ):
    TextBlockData( *pointer ),
    parenthesis_( -1 )
  {}
    
  //! destructor
  virtual ~HighlightBlockData( void )
  {}
    
  //! syntax highlighting pattern locations
  const PatternLocationSet& locations( void ) const
  { return locations_; }
  
  //! syntax highlighting pattern locations
  void setLocations( const PatternLocationSet& locations )
  { locations_ = locations; }
  
  //!@name parenthesis
  //@{
  
  bool hasParenthesis( void ) const
  { return parenthesis_ != -1; }
  
  //! highlighted parenthesis
  const int& parenthesis( void ) const
  { return parenthesis_; }
  
  //! set parenthesis
  void setParenthesis( const int& value )
  { parenthesis_ = value; }
  
  //! clear parenthesis
  void clearParenthesis( void )
  { parenthesis_ = -1; }
    
  //@}
  
  //!@name block limits
  //@{
  
  //! delimiter
  const TextBlock::Delimiter& delimiter( void ) const
  { return delimiter_; }

  //! delimiter
  void setDelimiter( const TextBlock::Delimiter& limit )
  { delimiter_ = limit; }
  
  //@}
  
  #if WITH_ASPELL
  //!@name spelling
  //@{
  //! set of misspelled words
  const SPELLCHECK::Word::Set& misspelledWords( void ) const
  { return words_; }
  
  //! set of misspelled words
  void setMisspelledWords( const SPELLCHECK::Word::Set& words )
  { words_ = words; }
  
  //! return misspelled word matching position, if any
  SPELLCHECK::Word misspelledWord( const int& position ) const;
  //@}
  #endif
  
  private:
    
  //! locations and ids of matching syntax highlighting patterns
  PatternLocationSet locations_;
  
  //! highlighted parenthesis location
  /*! local with respect to the block */
  int parenthesis_;
     
  //! delimiter
  TextBlock::Delimiter delimiter_;
  
  #if WITH_ASPELL
  //! set of misspelled words and position in associated block
  SPELLCHECK::Word::Set words_;
  #endif
  
};

#endif
  
