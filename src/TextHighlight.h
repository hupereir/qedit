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
#include <qsyntaxhighlighter.h>

#include "Config.h"
#include "Counter.h"
#include "Debug.h"
#include "HighlightPattern.h"

#if WITH_ASPELL
#include "SpellParser.h"
#endif

class HighlightPattern;
class TextDisplay;

//! syntax highlighting based on text patterns
#if WITH_ASPELL
class TextHighlight: public QSyntaxHighlighter, public SPELLCHECK::SpellParser
#else
class TextHighlight: public QSyntaxHighlighter, public Counter
#endif  
{
  
  public:
  
  //! constructor
  TextHighlight( TextDisplay* );
  
  //! highlight paragraph
  virtual int highlightParagraph( const QString& text, int endState );
  
  //! retrieve highlight location for given text
  HighlightPattern::LocationSet GetLocationSet( const QString& text, const int& active_id );
  
  //! enable highlight. Returns true if changed
  bool SetEnabled( const bool& state )
  { 
    if( enabled_ == state ) return false;
    enabled_ = state; 
    return true;
  }
  
  //! patterns
  typedef std::list< HighlightPattern* > PatternList;
  
  //! patterns
  const PatternList& GetPatterns( void ) const
  { return patterns_; }
  
  //! patterns
  void SetPatterns( const PatternList& patterns )
  { patterns_ = patterns; } 
  
  //! patterns
  void ClearPatterns( void )
  { patterns_.clear(); }
  
  //! update spell pattern (when compiled against Aspell)
  #if WITH_ASPELL 
  void UpdateSpellPattern( void )
  { spell_pattern_.SetStyle( HighlightStyle( "spell_style", FontFormat(), Color() ) ); }
  #endif 
 
  private:
  
  //! apply locations to text
  void _Apply( const QString& text, const HighlightPattern::LocationSet& locations, const bool& update_editor = true );  

  //! true if highlight is enabled
  bool enabled_;

  //! associated Text display
  TextDisplay* editor_;
  
  //! previous number of paragraphs
  int previous_paragraphs_;
  
  //! list of highlight patterns
  PatternList patterns_;
  
  #if WITH_ASPELL
  HighlightPattern spell_pattern_;
  #endif
  
    
};

#endif
