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
#include <QSyntaxHighlighter>

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
class TextHighlight: public QSyntaxHighlighter, public Counter
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

  //! parenthesis highlight color
  void setParenthesisHighlightColor( const QColor& color )
  { parenthesis_highlight_format_.setBackground(color); }
  
  //! parenthesis highlight color
  QColor parenthesisHighlightColor( void )
  { return parenthesis_highlight_format_.background().color(); }

  //@}  
  
  //! patterns
  void clear( void )
  { 
    Debug::Throw( "TextHighlight.clear.\n" ); 
    patterns_.clear(); 
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
  
  //! apply locations to current block
  void _applyPatterns( const HighlightPattern::LocationSet& locations );
 
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

  //! parenthesis highlight format
  QTextCharFormat parenthesis_highlight_format_;
  
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
