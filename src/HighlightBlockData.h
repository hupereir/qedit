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

#include "CollapsedBlockData.h"
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
  HighlightBlockData();
    
  //! constructor
  HighlightBlockData( const TextBlockData& reference ):
    TextBlockData( reference ),
    parenthesis_( -1 ),
    parenthesis_length_(0)
  {}
    
  //! constructor
  HighlightBlockData( const TextBlockData* pointer ):
    TextBlockData( *pointer ),
    parenthesis_( -1 ),
    parenthesis_length_(0)
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
  
  //! highlighted parenthesis
  const int& parenthesisLength( void ) const
  { return parenthesis_length_; }

  //! set parenthesis
  void setParenthesis( const int& value, const int& length )
  { 
    parenthesis_ = value;
    parenthesis_length_ = length;
  }
  
  //! clear parenthesis
  void clearParenthesis( void )
  { 
    parenthesis_ = -1; 
    parenthesis_length_= 0;
  }
    
  //@}
  
  //!@name block limits
  //@{
  
  //! delimiters
  const TextBlock::Delimiter::Map& delimiters( void ) const
  { return delimiters_; }

  //! delimiters
  void setDelimiters( const TextBlock::Delimiter::Map& delimiters )
  { delimiters_ = delimiters; }
  
  //! delimiter
  TextBlock::Delimiter delimiter( const unsigned int& id ) const
  { 
    TextBlock::Delimiter::Map::const_iterator iter( delimiters_.find(id) );
    return (iter == delimiters_.end() ) ? TextBlock::Delimiter():iter->second;
  }

  //! delimiter
  void setDelimiter( const unsigned int& id, const TextBlock::Delimiter& delimiter )
  { delimiters_[id] = delimiter; }
  
  //! true if block is collapsed
  const bool& collapsed( void ) const
  { return collapsed_; }
  
  //! true if block is collapsed
  void setCollapsed( const bool& value )
  { collapsed_ = value; }
  
  //! collapsed data
  const CollapsedBlockData::List& collapsedData( void ) const
  { return collapsed_data_; }
  
  //! collapsed data
  void clearCollapsedData( void )
  { collapsed_data_.clear(); }
  
  //! collapsed data
  void setCollapsedData( const CollapsedBlockData::List& data ) 
  { collapsed_data_ = data; }
  
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
     
  //! parenthesis length
  int parenthesis_length_;
    
  //!@name block delimiters
  //@{
  
  //! delimiter
  TextBlock::Delimiter::Map delimiters_;
  
  //! true if block is collapsed
  bool collapsed_;
  
  //! collapsed data
  CollapsedBlockData::List collapsed_data_;
  
  //! collapsed text
  // QString collapsed_text_;
  
  //@}
  
  #if WITH_ASPELL
  //! set of misspelled words and position in associated block
  SPELLCHECK::Word::Set words_;
  #endif
  
};

#endif
  
