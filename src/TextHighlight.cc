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
  \file TextHighlight.cc
  \brief syntax highlighting based on text patterns
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QTextDocument>

#include "Debug.h"
#include "HighlightPattern.h"
#include "HighlightBlockData.h"
#include "TextParenthesis.h"
#include "TextHighlight.h"

using namespace std;

//_________________________________________________________
TextHighlight::TextHighlight( QTextDocument* document ):
  BaseTextHighlight( document ),
  highlight_enabled_( false ),
  parenthesis_enabled_( false ),
  local_parenthesis_( -1 ),
  absolute_parenthesis_( 0 )
{ Debug::Throw( "TextHighlight::TextHighlight.\n" ); }

//_______________________________________________________
void TextHighlight::setParenthesis( const TextParenthesis::List& parenthesis )
{
  Debug::Throw( "TextHighlight::setParenthesis.\n" );
  parenthesis_ = parenthesis;
  parenthesis_set_.clear();
  for( TextParenthesis::List::const_iterator iter = parenthesis_.begin(); iter != parenthesis_.end(); iter++ )
  {
    parenthesis_set_.insert( iter->first );
    parenthesis_set_.insert( iter->second );
  }
}

//_______________________________________________________
void TextHighlight::clearParenthesis( void )
{

  Debug::Throw() << "TextHighlight::clearParenthesis - local:"
    << " " << local_parenthesis_ 
    << " absolute: " << absolute_parenthesis_ 
    << endl;
  
  // clear previous parenthesis highlight
  if(  local_parenthesis_ == -1 ) return;
  local_parenthesis_ = -1;
  document()->markContentsDirty(absolute_parenthesis_,1); 
  
}
  
//_______________________________________________________
void TextHighlight::highlightParenthesis( const int& local, const int& absolute )
{

  local_parenthesis_ = local;
  absolute_parenthesis_ = absolute; 
  document()->markContentsDirty(absolute_parenthesis_,1); 

}

//_________________________________________________________
void TextHighlight::highlightBlock( const QString& text )
{
  
  // base class highlight
  // this is always performed
  BaseTextHighlight::highlightBlock( text );
  
  // highlight parenthesis highlight color, if any
  if( local_parenthesis_ != -1 ) 
  { setFormat( local_parenthesis_, 1, parenthesis_highlight_format_ ); }
  
  // check if enabled
  if( !isHighlightEnabled()  || patterns_.empty() ) return;
  
  // retrieve active_id from last block state
  int active_id( previousBlockState() );
  HighlightPattern::LocationSet locations;

  // try retrieve HighlightBlockData
  bool need_update( true );
  HighlightBlockData* data = dynamic_cast<HighlightBlockData*>( currentBlockUserData() );
  if( data ) { 
 
    need_update = ( data->isModified() || (locations = data->locations()).activeId().first != active_id );
        
  } else {
    
    // try retrieve data from parent type
    TextBlockData* text_data = dynamic_cast<TextBlockData*>( currentBlockUserData() );
    data = text_data ? new HighlightBlockData( text_data ) : new HighlightBlockData();
    setCurrentBlockUserData( data );
        
  }

  // create new location set if needed
  if( need_update ) 
  {
    locations = locationSet( text, active_id );
    
    // update data modification state and highlight pattern locations
    data->setModified( false );
    data->setLocations( locations );
    
    // store active id
    setCurrentBlockState( locations.activeId().second );

  }
  
  // before try applying the found locations see if automatic spellcheck is on
  #if WITH_ASPELL
  
  if( spellParser().isEnabled() ) 
  {
    
    // clear locations
    locations.clear();
    
    // insert highlight
    const SPELLCHECK::Word::Set& words( spellParser().parse( text ) );
    for( SPELLCHECK::Word::Set::const_iterator iter = words.begin(); iter != words.end(); iter++ )
    { locations.insert( HighlightPattern::Location( spellPattern(), iter->position(), iter->size() ) ); }
    
    // store active id
    data->setMisspelledWords( words );
    setCurrentBlockState( -1 );
    
  } 
  
  #endif
    
  // apply new location set
  if( !locations.empty() ) _applyPatterns( text, locations );
  
  return;
  
}  
    
//_________________________________________________________
HighlightPattern::LocationSet TextHighlight::locationSet( const QString& text, const int& active_id )
{
      
  // location list
  HighlightPattern::LocationSet locations;
  locations.activeId().first = active_id;
  locations.activeId().second = active_id;
  
  // check if pattern active_id is still active
  if( active_id > 0 )
  {    
    // look for matching pattern in list
    HighlightPattern::List::iterator pattern_iter = find_if( patterns_.begin(), patterns_.end(), HighlightPattern::SameIdFTor( active_id ) );
    Exception::check( pattern_iter != patterns_.end(), DESCRIPTION( "invalid pattern" ) );
    
    HighlightPattern &pattern( **pattern_iter );
    bool active=true;
    pattern.processText( locations, text, active );
      
    // if not active, break the loop to process the other patterns
    if( active )
    {
    
      // if still active. look for child patterns
      // this could be made faster by storing child patterns into this one
      for( HighlightPattern::List::const_iterator child_iter = pattern.children().begin(); child_iter != pattern.children().end(); child_iter++ )
      { (*child_iter)->processText( locations, text, active );}
   
      // remove patterns that overlap with others
      HighlightPattern::LocationSet::iterator iter = locations.begin();
      HighlightPattern::LocationSet::iterator prev = locations.begin();
      
      // first pattern is skipped because it must be the parent
      // so that prev is incremented once and current is incremented twice
      if( iter != locations.end() ) iter++;
      if( iter != locations.end() ) iter++; 
      if( prev != locations.end() ) prev++; 
      while(  iter != locations.end() )
      {
        
        // no need to compare prev and iter parent Ids because they are known to be the 
        // active parrent
        if( iter->position() < prev->position()+(int)prev->length() )
        {

          // current iterator overlaps with prev
          HighlightPattern::LocationSet::iterator current = iter;
          iter++;
          locations.erase( current );
          
        } else {
          
          prev = iter;
          iter++;
            
        }
      }
      
      return locations;
    }
    
  }
    
  // no active pattern
  // normal processing
  unsigned int active_patterns(0);
  for( HighlightPattern::List::iterator iter = patterns_.begin(); iter != patterns_.end(); iter++ )
  {
    
    HighlightPattern &pattern( **iter );
    
    // do not reprocess active pattern (if any)
    // sincee it was already done
    if( (int)pattern.id() == active_id ) continue;
    
    // process pattern, store activity
    bool active = false;
    pattern.processText( locations, text, active );
    if( active ) active_patterns |= pattern.id();
    
  }
  
  // check number of recorded locations
  if( locations.empty() ) return locations;
  
  // remove locations that are front and have parents
  while( locations.size() && locations.begin()->parentId() ) locations.erase(locations.begin());
  
  // remove patterns that overlap with others
  HighlightPattern::LocationSet::iterator iter = locations.begin();
  HighlightPattern::LocationSet::iterator prev = locations.begin();
  HighlightPattern::LocationSet::iterator parent = locations.begin();
  
  if( iter != locations.end() ) iter++;
  while(  iter != locations.end() )
  {
    
    // check if patterns overlap
    if( iter->position() < prev->position()+(int)prev->length() )
    {
      
      // check if iterator has parent that match
      if( iter->parentId() == prev->id() ) 
      {
        
        prev = iter;
        iter++;
        
      } else {
        
        // remove current pattern
        HighlightPattern::LocationSet::iterator current = iter;
        iter++;
        
        // remove pattern from active list
        active_patterns &= (~current->id());
        locations.erase( current );
      }
      
      // no overlap with prev. Check against parent
    } else if( iter->position() < parent->position()+(int)parent->length()  ) 
    {
      
      if( iter->parentId() == parent->id() )
      {
        prev = iter; 
        iter++;
      } else {
        
        HighlightPattern::LocationSet::iterator current = iter;
        iter++;
        
        // remove pattern from active list
        /* 
        this may not work in case there are 
        occurences of the same pattern later in the set
        besides it is useless since one loops again
        over patterns afterwards to decide which one is active
        */
        // active_patterns &= (~current->id());
        locations.erase( current );
      }
      
    } else {
     
      // no increment. Advance one.
      parent = iter;
      prev = iter;
      iter++;
      
    }
  }
        
  // check activity
  // one loop over the remaining locations
  // stop at the first one that is found in the list of possibly active
  locations.activeId().second = 0;
  for( HighlightPattern::LocationSet::iterator iter = locations.begin(); iter != locations.end(); iter++ )
  {
    if( active_patterns & iter->id() )
    {
      locations.activeId().second = iter->id();
      break;
    }
  }

  return locations;
  
}

//_________________________________________________________
void TextHighlight::_applyPatterns( const QString& text, const HighlightPattern::LocationSet& locations )
{

  // initialize style
  HighlightStyle current_style;
  QTextCharFormat current_format;
  for( HighlightPattern::LocationSet::const_iterator iter = locations.begin(); iter != locations.end(); iter++ )
  {
    if( current_style != iter->style() )
    {
      current_style = iter->style();
      current_format = current_style.format();
    }

    QTextCharFormat format( current_format );
    QTextCharFormat old( TextHighlight::format( iter->position() ) );
    if( old.hasProperty( QTextFormat::BackgroundBrush ) ) format.setBackground( old.background() );
    setFormat( iter->position(), iter->length(), format );
    
  }

  return;
}

//_________________________________________________________
void TextHighlight::_parseParenthesis( const QString& text )
{}
