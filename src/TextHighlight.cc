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

#include "Debug.h"
#include "HighlightPattern.h"
#include "TextHighlight.h"
#include "TextDisplay.h"

using namespace std;

//! local debugging verbosity level for this class
static const int debug_level = 1;

//_________________________________________________________
TextHighlight::TextHighlight( TextDisplay* display ):
  QSyntaxHighlighter( display ),
  #if WITH_ASPELL==0
  Counter( "TextHighlight" ),
  #endif
  enabled_( true ),
  editor_( display ),
  previous_paragraphs_( 0 )
{ Debug::Throw( "TextHighlight::TextHighlight.\n" ); }

//_________________________________________________________
int TextHighlight::highlightParagraph( const QString& text, int active_id )
{
  
  setFormat( 0, text.length(), Qt::black );
  if( currentParagraph() == 0 ) active_id = 0;

  // check paragraph length
  // calculate paragraph increment
  // update previous paragraphs
  int paragraphs( editor_->paragraphs() );
  int increment( paragraphs - previous_paragraphs_ );
  previous_paragraphs_ = paragraphs;

  // if paragraph increment is non zero, update editor location map
  if( increment != 0 )
  {
    const TextDisplay::LocationMap& previous_map( editor_->GetLocations() );
    TextDisplay::LocationMap new_map;
    
    // retrieve all locations that correspond to a paragraph not smaller than current_paragraph
    TextDisplay::LocationMap::const_iterator iter = previous_map.upper_bound( currentParagraph() );
    new_map.insert( previous_map.begin(), iter );
    for(; iter != previous_map.end(); iter++ )
    {
      if( iter->first + increment > currentParagraph() )
      { new_map.insert( make_pair( iter->first + increment, iter->second ) ); }
    }

    editor_->SetLocations( new_map );
  }
  
  // prepare new set of locations
  HighlightPattern::LocationSet locations;

  // retrieve current paragraph and decide if updates are needed
  int current_paragraph = editor_->GetPosition().Paragraph();
  bool need_update(currentParagraph() <= current_paragraph || !editor_->HasLocations( currentParagraph() ) );
  
  if( !need_update ) 
  { 
    
    // retrieve old (shifted) locations
    locations = editor_->GetLocations( currentParagraph() );
    
    // check if active ID match
    if( locations.ActiveId().first != active_id ) 
    {
      need_update = true; 
      locations = GetLocationSet( text, active_id );
    }
    
  } else locations = GetLocationSet( text, active_id );
    
  // apply new location set
  if( !locations.empty() ) _Apply( text, locations );
  
  // return updated active ID
  return locations.ActiveId().second;

}
    
//_________________________________________________________
HighlightPattern::LocationSet TextHighlight::GetLocationSet( const QString& text, const int& active_id )
{
    
  // location list
  HighlightPattern::LocationSet locations;
  locations.ActiveId().first = active_id;
  locations.ActiveId().second = active_id;
  
  // retrieve editor current font
  #if WITH_ASPELL
  if( enabled_ && !patterns_.empty() && !AutoSpellEnabled() ) 
  #else
  if( enabled_ && !patterns_.empty() ) 
  #endif
  {
    // check if Pattern active_id is still active
    if( active_id )
    for( PatternList::iterator iter = patterns_.begin(); iter != patterns_.end(); iter++ )
    {
      HighlightPattern &pattern( **iter );
      if( (int)pattern.Id() != active_id ) continue;
      
      bool active=true;
      pattern.ProcessText( locations, (const char*) text, active );
      
      // if not active, break the loop to process the other patterns
      if( !active ) break;
      
      // if still active. look for child patterns
      for( PatternList::iterator iter = patterns_.begin(); iter != patterns_.end(); iter++ )
      {
        
        // keep only child patterns
        HighlightPattern &pattern( **iter ); 
        if( (int)pattern.ParentId() != active_id ) continue;
        pattern.ProcessText( locations, (const char*) text, active ); 
        
      }
      
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
        if( iter == locations.end() ) break;
        
        // no need to compare prev and iter parent Ids because they are known to be the 
        // active parrent
        if( iter->Position() < prev->Position()+(int)prev->Length() )
        {
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
    
    unsigned int active_patterns(0);
    for( PatternList::iterator iter = patterns_.begin(); iter != patterns_.end(); iter++ )
    {
      HighlightPattern &pattern( **iter );
      
      // do not reprocess active pattern (if any)
      if( (int)pattern.Id() == active_id ) continue;
      
      // process pattern, store activity
      bool active = false;
      pattern.ProcessText( locations, (const char*) text, active );
      if( active ) active_patterns |= pattern.Id();
      
    }
    
    // check number of recorded locations
    if( locations.empty() ) return locations;
    
    // remove locations that are at fronts and have parents
    while( locations.size() && locations.begin()->ParentId() ) locations.erase(locations.begin());
    
    // remove patterns that overlap with others
    HighlightPattern::LocationSet::iterator iter = locations.begin();
    HighlightPattern::LocationSet::iterator prev = locations.begin();
    HighlightPattern::LocationSet::iterator parent = locations.begin();
    
    if( iter != locations.end() ) iter++;
    while(  iter != locations.end() )
    {
      
      // check if patterns overlap
      if( iter->Position() < prev->Position()+(int)prev->Length() )
      {
        
        // check if iterator has parent that match
        if( iter->ParentId() == prev->Id() ) 
        {
          prev = iter;
          iter++;
        } else {
          
          // remove current pattern
          HighlightPattern::LocationSet::iterator current = iter;
          iter++;
          
          // remove pattern from active list
          active_patterns &= (~current->Id());
          locations.erase( current );
        }
      
      // no overlap with prev. Check against parent
      } else if( iter->Position() < parent->Position()+(int)parent->Length()  ) {
        
        if( iter->ParentId() == parent->Id() )
        {
          prev = iter; 
          iter++;
        } else {
        
          HighlightPattern::LocationSet::iterator current = iter;
          iter++;
        
          // remove pattern from active list
          active_patterns &= (~current->Id());
          locations.erase( current );
        }
        
      } else {
        
        parent = iter;
        prev = iter;
        iter++;
        
      }
    }
    
    // check activity
    locations.ActiveId().second = 0;
    for( HighlightPattern::LocationSet::iterator iter = locations.begin(); iter != locations.end(); iter++ )
    if( active_patterns & iter->Id() )
    {
      locations.ActiveId().second = iter->Id();
      break;
    }
    
    // apply style for all recorded locations
    if( locations.empty() ) {
      locations.ActiveId().second = 0;
      return locations;
    }
  }
  
  #if WITH_ASPELL
  else if( AutoSpellEnabled() ) 
  {

    const SPELLCHECK::Word::Set& words( Parse( currentParagraph(), text ) );
    for( SPELLCHECK::Word::Set::const_iterator iter = words.begin(); iter != words.end(); iter++ )
    { locations.insert( HighlightPattern::Location( spell_pattern_, iter->position_, iter->size() ) ); }
    
  } 
  #endif
  
  return locations;
  
}

//_________________________________________________________
void TextHighlight::_Apply( const QString& text, const HighlightPattern::LocationSet& locations, const bool& update_editor )
{

  // initialize style
  QFont base_font( textEdit()->font() );
  HighlightStyle current_style;
  QFont font;
  QColor color;
  for( HighlightPattern::LocationSet::const_iterator iter = locations.begin(); iter != locations.end(); iter++ )
  {
    if( current_style != iter->Style() )
    {
      current_style = iter->Style();
      font = current_style.Font( base_font );
      color = current_style.Color();
    }

    // loop over locations and apply
    setFormat( iter->Position(), iter->Length(), font, color );
  }

  // store locations in editor
  if( update_editor ) editor_->SetLocations( currentParagraph(), locations );

  return;
}
