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

#include <QSyntaxHighlighter>
#include <list>

#include "Config.h"
#include "Counter.h"
#include "Debug.h"
#include "Key.h"
#include "HighlightPattern.h"

class HighlightPattern;

//! syntax highlighting based on text patterns
class TextHighlight: public QSyntaxHighlighter, public BASE::Key, public Counter
{
  
  public:
  
  //! constructor
  TextHighlight( QTextDocument* );
  
  //! highlight paragraph
  virtual void highlightBlock( const QString& text );
  
  //! retrieve highlight location for given text
  HighlightPattern::LocationSet locationSet( const QString& text, const int& active_id );
  
  //! enable highlight. Returns true if changed
  bool setEnabled( const bool& state )
  { 
    if( enabled_ == state ) return false;
    enabled_ = state; 
    return true;
  }
  
  //! true if enabled
  const bool& isEnabled( void ) const
  { return enabled_; }
  
  //! patterns
  
  //! patterns
  const HighlightPattern::List& patterns( void ) const
  { return patterns_; }
  
  //! patterns
  void setPatterns( const HighlightPattern::List& patterns )
  { patterns_ = patterns; } 
  
  //! patterns
  void clear( void )
  { 
    Debug::Throw( "TextHighlight.clear.\n" ); 
    patterns_.clear(); 
  }
  
  private:
  
  //! apply locations to text
  void _apply( const QString& text, const HighlightPattern::LocationSet& locations );

  //! true if highlight is enabled
  bool enabled_;

  //! list of highlight patterns
  HighlightPattern::List patterns_;
  
};

#endif
