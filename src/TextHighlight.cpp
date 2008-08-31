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
  \file TextHighlight.cpp
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
  QSyntaxHighlighter( document ),
  Counter( "TextHighlight" ),
  highlight_enabled_( false ),
  parenthesis_enabled_( false ),
  block_delimiters_enabled_( true )
{ Debug::Throw( "TextHighlight::TextHighlight.\n" ); }

//_______________________________________________________
void TextHighlight::setParenthesis( const TextParenthesis::List& parenthesis )
{
  
  Debug::Throw( "TextHighlight::setParenthesis.\n" );
  parenthesis_ = parenthesis;
  
}

//_________________________________________________________
void TextHighlight::highlightBlock( const QString& text )
{
      
  // check if syntax highlighting is enabled
  bool highlight_enabled( isHighlightEnabled()  && !patterns_.empty() );
  #if WITH_ASPELL 
  highlight_enabled |= spellParser().isEnabled();
  #endif
  
  // retrieve active_id from last block state
  int active_id( previousBlockState() );
  PatternLocationSet locations;

  // try retrieve HighlightBlockData
  bool need_update( true );
  
  // try retrieve block data
  HighlightBlockData* data = dynamic_cast<HighlightBlockData*>( currentBlockUserData() );
  
  if( data ) { 
 
    // see if block needs update
    need_update = 
      data->hasFlag( TextBlock::MODIFIED ) || 
      (highlight_enabled && (locations = data->locations()).activeId().first != active_id );
   
  } else {
    
    // try retrieve data from parent type
    TextBlockData* text_data = static_cast<TextBlockData*>( currentBlockUserData() );
    data = text_data ? new HighlightBlockData( text_data ) : new HighlightBlockData();
    setCurrentBlockUserData( data );
        
  }

  // block delimiters parsing
  if( isBlockDelimitersEnabled() && need_update ) 
  {
    bool segment_changed( false );
    for( BlockDelimiter::List::const_iterator iter = block_delimiters_.begin(); iter != block_delimiters_.end(); iter++ )
    { segment_changed |= data->delimiters().set( iter->id(), _delimiter( *iter, text ) ); }
    if( segment_changed )  
    emit needSegmentUpdate();
  }
  
  // highlight patterns
  if( need_update && highlight_enabled )
  {
    
    // get new set of highlight locations
    locations = _highlightLocationSet( text, active_id );
    
    // update data modification state and highlight pattern locations
    data->setFlag( TextBlock::MODIFIED, false );
    data->setLocations( locations );
        
    // store active id
    /* this is disabled when  current block is collapsed */
    if( !data->hasFlag( TextBlock::COLLAPSED ) ) setCurrentBlockState( locations.activeId().second );    
    else setCurrentBlockState( 0 ); 
            
  }
    
  // before try applying the found locations see if automatic spellcheck is on
  #if WITH_ASPELL
  
  if( spellParser().isEnabled() )
  {
        
    // clear locations
    locations = _spellCheckLocationSet( text, data );
    data->setLocations( PatternLocationSet() );
    setCurrentBlockState( -1 );
    
  } 
  
  #endif
    
  // apply new location set
  if( !locations.empty() ) _applyPatterns( locations );
    
  // check if parenthesis need highlight
  if( isParenthesisEnabled() && data && data->hasParenthesis() )
  { 
    QTextCharFormat old( TextHighlight::format( data->parenthesis() ) );
    old.setBackground( parenthesis_highlight_format_.background() );
    setFormat( data->parenthesis(), data->parenthesisLength(), parenthesis_highlight_format_ ); 
  }
  
  return;
  
}
    
//_________________________________________________________
PatternLocationSet TextHighlight::locationSet( const QString& text, const int& active_id )
{
  
  if( spellParser().isEnabled() ) return _spellCheckLocationSet( text );
  else if( isHighlightEnabled()  && !patterns_.empty() ) return _highlightLocationSet( text, active_id );
  else return PatternLocationSet();
  
}

//_________________________________________________________
PatternLocationSet TextHighlight::_highlightLocationSet( const QString& text, const int& active_id ) const
{
      
  // location list
  PatternLocationSet locations;
  locations.activeId().first = active_id;
  locations.activeId().second = active_id;
  
  // check if pattern active_id is still active
  if( active_id > 0 )
  {    
    
    // look for matching pattern in list
    HighlightPattern::List::const_iterator pattern_iter = find_if( patterns_.begin(), patterns_.end(), HighlightPattern::SameIdFTor( active_id ) );
    assert( pattern_iter != patterns_.end() );
    
    const HighlightPattern &pattern( *pattern_iter );
    bool active=true;
    pattern.processText( locations, text, active );
      
    // if not active, break the loop to process the other patterns
    if( active )
    {
    
      // if still active. look for child patterns
      for( HighlightPattern::List::const_iterator child_iter = pattern.children().begin(); child_iter != pattern.children().end(); child_iter++ )
      { child_iter->processText( locations, text, active );}
   
      // remove patterns that overlap with others
      PatternLocationSet::iterator iter = locations.begin();
      PatternLocationSet::iterator prev = locations.begin();
      
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
          PatternLocationSet::iterator current = iter;
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
  for( HighlightPattern::List::const_iterator iter = patterns_.begin(); iter != patterns_.end(); iter++ )
  {
        
    const HighlightPattern &pattern( *iter );
    
    // do not reprocess active pattern (if any)
    // sincee it was already done
    if( (int)pattern.id() == active_id ) continue;
    
    // process pattern, store activity
    bool active = false;
    
    // here one could check if the pattern appears at least once (by checking return value of processText
    // and loop over children here (in place of main loop) if yes.
    pattern.processText( locations, text, active );
    if( active ) active_patterns |= pattern.id();
        
  }
  
  // check number of recorded locations
  if( locations.empty() ) return locations;
  
  // remove locations that are front and have parents
  while( locations.size() && locations.begin()->parentId() ) locations.erase(locations.begin());
  
  // remove patterns that overlap with others
  PatternLocationSet::iterator iter = locations.begin();
  PatternLocationSet::iterator prev = locations.begin();
  PatternLocationSet::iterator parent = locations.begin();
  
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
        PatternLocationSet::iterator current = iter;
        iter++;
        
        // remove pattern from active list
        active_patterns &= (~current->id());
        locations.erase( current );
      }
      
      // no overlap with prev. Check against parent
    } else if( iter->position() < parent->position()+(int)parent->length()  ) {
      
      if( iter->parentId() == parent->id() )
      {
        prev = iter; 
        iter++;
      } else {
        
        PatternLocationSet::iterator current = iter;
        iter++;
        
        // remove pattern from active list
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
  for( PatternLocationSet::iterator iter = locations.begin(); iter != locations.end(); iter++ )
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
PatternLocationSet TextHighlight::_spellCheckLocationSet( const QString& text, HighlightBlockData* data )
{

  PatternLocationSet locations;
  
  // insert highlight
  const SPELLCHECK::Word::Set& words( spellParser().parse( text ) );
  for( SPELLCHECK::Word::Set::const_iterator iter = words.begin(); iter != words.end(); iter++ )
  { locations.insert( PatternLocation( spellPattern(), iter->position(), iter->size() ) ); }
  
  // store misspelled words
  if( data ) data->setMisspelledWords( words );
  return locations;
  
}

//_________________________________________________________
void TextHighlight::_applyPatterns( const PatternLocationSet& locations )
{

  // initialize style
  int pattern_id(-1);
  QTextCharFormat current_format;
  for( PatternLocationSet::const_iterator iter = locations.begin(); iter != locations.end(); iter++ )
  {
    if( pattern_id != iter->id() )
    { 
      pattern_id = iter->id();
      current_format = iter->format(); 
    }

    QTextCharFormat format( current_format );
    QTextCharFormat old( TextHighlight::format( iter->position() ) );
    if( old.hasProperty( QTextFormat::BackgroundBrush ) ) format.setBackground( old.background() );
    setFormat( iter->position(), iter->length(), format );
    
  }

  return;
}

//_________________________________________________________
TextBlock::Delimiter TextHighlight::_delimiter( const BlockDelimiter& delimiter, const QString& text ) const
{
  
  TextBlock::Delimiter out;
  int position = 0;
  while( (position = text.indexOf( delimiter.regexp(), position ) ) >= 0 )
  {
    if( text.mid( position ).startsWith( delimiter.first() ) ) out.begin()++;
    else if( text.mid( position ).startsWith( delimiter.second() ) )
    {
      if( out.begin() > 0 ) out.begin()--;
      else out.end()++;
    }
    
    position++;
  }
  
  return out;
 
}
