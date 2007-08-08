#ifndef _TextHighlight_h_
#define _TextHighlight_h_

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
* Place, Suite 330, Boston, MA  02111-1307 USA                           
*                         
*                         
*******************************************************************************/

/*!
  \file TextHighlight.h
  \brief syntax highlighting based on text patterns
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <list>

#include "BaseTextHighlight.h"
#include "Config.h"
#include "Counter.h"
#include "Debug.h"
#include "Key.h"
#include "HighlightPattern.h"
#include "TextParenthesis.h"

#if WITH_ASPELL
#include "SpellParser.h"
#endif

class HighlightPattern;
class TextParenthesis;

//! syntax highlighting based on text patterns
class TextHighlight: public BaseTextHighlight
{
  
  public:
  
  //! constructor
  TextHighlight( QTextDocument* );
  
  //! highlight paragraph
  virtual void highlightBlock( const QString& text );
  
  //! retrieve highlight location for given text
  HighlightPattern::LocationSet locationSet( const QString& text, const int& active_id );
    
  //!@name highlight patterns
  //@{
  
  //! enable highlight. Returns true if changed
  bool setHighlightEnabled( const bool& state )
  { 
    if( highlight_enabled_ == state ) return false;
    highlight_enabled_ = state; 
    return true;
  }
  
  //! true if enabled
  const bool& isHighlightEnabled( void ) const
  { return highlight_enabled_; }

  //! patterns
  const HighlightPattern::List& patterns( void ) const
  { return patterns_; }
  
  //! patterns
  void setPatterns( const HighlightPattern::List& patterns )
  { patterns_ = patterns; } 
  
  //@}
  
  //!@name parenthesis
  //@{
  
  //! parenthesis enabled
  const bool& isParenthesisEnabled( void ) const
  { return parenthesis_enabled_; }
  
  //! parenthesis enabled
  bool setParenthesisEnabled( const bool& state )
  { 
    if( parenthesis_enabled_ == state ) return false;
    parenthesis_enabled_ = state; 
    return true;
  }
    
  //! parenthesis
  const TextParenthesis::List& parenthesis( void ) const
  { return parenthesis_; }
  
  //! set parenthesis
  void setParenthesis( const TextParenthesis::List& );
 
  //! current parenthesis color
  void setParenthesisHighlightColor( const QColor& color )
  { parenthesis_highlight_format_.setBackground(color); }

  //! current parenthesis
  /*! returns true if changed */
  void highlightParenthesis( const int& local, const int& absolute );
     
  //! clear highlighted parenthesis if any
  void clearParenthesis( void );
  
  //@}
    
  //! patterns
  void clear( void )
  { 
    Debug::Throw( "TextHighlight.clear.\n" ); 
    patterns_.clear(); 
    parenthesis_.clear();
    parenthesis_set_.clear();
  }

  #if WITH_ASPELL
  
  //! embedded spellcheck parser
  SPELLCHECK::SpellParser& spellParser( void ) 
  { return spellparser_; }
  
  //! highlight pattern associated to auto-spell
  const HighlightPattern& spellPattern( void ) const
  { return spell_pattern_; }
  
  //! update highlight pattern associated to auto-spell
  void updateSpellPattern( void )
  { spell_pattern_.setStyle( HighlightStyle( "spellcheck_style", spellParser().fontFormat(), spellParser().color() ) ); }

  #endif
  
  private:

  //!@name syntax highlighting
  //@{
  
  //! apply locations to text
  void _applyPatterns( const QString& text, const HighlightPattern::LocationSet& locations );
 
  //! parse text to store pairs of matching parenthesis
  void _parseParenthesis( const QString& text );
  
  //! true if highlight is enabled
  bool highlight_enabled_;

  //! list of highlight patterns
  HighlightPattern::List patterns_;
  
  //@}
  
  //!@name text parenthesis
  //@{
  
  //! parenthesis enabled
  bool parenthesis_enabled_;
  
  //! text parenthesis
  TextParenthesis::List parenthesis_;

  //! keep track of all parenthesis in a single set for fast access
  TextParenthesis::Set parenthesis_set_;
  
  //! parenthesis highlight format
  QTextCharFormat parenthesis_highlight_format_;
  
  //! local (i.e. block wise) position of current parenthesis to highlight, if any
  /*! -1 if none set */
  int local_parenthesis_;
  
  //! absolute position of current parenthesis to highlight (needed for clearing)
  int absolute_parenthesis_;
  
  //@}
  
  //!@name spell checking
  //@{

  #if WITH_ASPELL
  
  //! spell check parser
  SPELLCHECK::SpellParser spellparser_; 
  
  //! spellcheck highlight pattern
  HighlightPattern spell_pattern_;
  
  #endif
  
  //@}
  
};

#endif
