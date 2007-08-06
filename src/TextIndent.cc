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
  \file TextIndent.cc
  \brief text indentation
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <qregexp.h>
#include "TextDisplay.h"
#include "TextIndent.h"

using namespace std;

//______________________________________________
TextIndent::TextIndent( TextDisplay* editor ):
  QObject( editor ),
  Counter( "TextIndent" ),
  enabled_( true ),
  editor_( editor ),
  base_indentation_( 0 )
{ Debug::Throw( "TextIndent::TextIndent.\n" ); }

//______________________________________________
void TextIndent::indent( QTextBlock block )
{
  if( !isEnabled() ) return;
  
  // ignore "empty" blocks
  // if( editor_->isEmptyBlock( block ) ) return;
  
  // store block and cursor
  current_cursor_ = editor_->textCursor();
  
  // disable updates during manipulations
  editor_->setUpdatesEnabled( false );
  
  // retrieve previous valid block to
  // determine the base indentation
  QTextBlock previous_block( block.previous() );
  while( previous_block.isValid() &&  editor_->ignoreBlock( previous_block ) ) 
  { previous_block = previous_block.previous(); }
  
  // _decrement if first paragraph of text
  if( !previous_block.isValid() ) _decrement( block );
  else {

    // get previous paragraph tabs
    int previous_tabs( _tabCount( previous_block ) );
    int new_tabs = previous_tabs;
    for( IndentPattern::List::iterator iter = patterns_.begin(); iter != patterns_.end(); iter++ )
    {
      if( _acceptPattern( block, **iter ) )
      {
        Debug::Throw() << "TextIndent::indent - accepted pattern: " << (*iter)->name() << endl;
        if( (*iter)->type() == IndentPattern::INCREMENT ) new_tabs += (*iter)->scale();
        else if( (*iter)->type() == IndentPattern::DECREMENT ) new_tabs -= (*iter)->scale();
        else if( (*iter)->type() == IndentPattern::DECREMENT_ALL ) new_tabs = 0;
        break;
      }
    }
    
    // make sure new_tabs is not negative
    new_tabs = max( new_tabs, 0 );
    _decrement( block );
    _increment( block, new_tabs );
    
  }

  // restore cursor
  editor_->setTextCursor( current_cursor_ );
  editor_->setUpdatesEnabled( true );
  
}

//____________________________________________
bool TextIndent::_acceptPattern( QTextBlock block, const IndentPattern& pattern ) const
{
  
  // retrieve rules associated to pattern
  bool accepted( true );
  const IndentPattern::RuleList& rules( pattern.rules() );
  int rule_id( 0 );
  for( IndentPattern::RuleList::const_iterator iter = rules.begin(); iter != rules.end() && accepted; iter++, rule_id++ )
  {
    
    QTextBlock local( block );
    
    // if working on current paragraph
    if( iter->paragraph() == 0 )
    { 
      
      if( !iter->accept( local.text() ) ) 
      {
        Debug::Throw() << "TextIndent::_acceptPattern - [" << pattern.name() << "," << rule_id << "] rejected" << endl;
        accepted = false; 
      }
      
    } else {

      // decrepent paragraph, skipping ignored lines
      int decrement = 0;
      int true_decrement = 0;
      do {
        local = local.previous();
        decrement--;
        true_decrement--;
        while( local.isValid() && editor_->ignoreBlock( local ) ) 
        { 
          local = local.previous(); 
          true_decrement--;
        }
        
      } while( local.isValid() && decrement > iter->paragraph() );
      
      Debug::Throw() << "TextIndent::_acceptPattern - [" << pattern.name() << "," << rule_id << "]"
        << " decrement: " << decrement << " true: " << true_decrement
        << endl;
      
      // check paragraph and regexp
      // here one could have a flag on the indentation pattern rules 
      // to check what is to be done when there is no valid paragraph
      // matching the request. This would allow to enable some patterns 
      // that cannot otherwise.
      if( !local.isValid() || !iter->accept( local.text() ) ) 
      {
        Debug::Throw() << "TextIndent::_acceptPattern - [" << pattern.name() << "," << rule_id << "] rejected" << endl;
        accepted = false;
      }
      
    }
    
  }

  return accepted;
}

//____________________________________________
int TextIndent::_tabCount( const QTextBlock& block )
{
   
  QString text( block.text() );
  int count = 0;
  
  // skip the characters matching base_indentation_
  int index = base_indentation_;
  
  // loop over next characters to identify tabs
  // both normal and emulated tabs are counted
  while( index < text.size() )
  {
    if( text.mid( index, editor_->normalTabCharacter().size() ) == editor_->normalTabCharacter() )
    { index += editor_->normalTabCharacter().size(); count++; }
    
    else if( text.mid( index, editor_->emulatedTabCharacter().size() ) == editor_->emulatedTabCharacter() )
    { index += editor_->emulatedTabCharacter().size(); count++; } 
    
    else break;
  }
  
  return count;
}

//____________________________________________
void TextIndent::_increment( QTextBlock block, const unsigned int& count )
{
  
  // first make sure that the line has at least base_indentation_ characters
  QTextCursor cursor( block );
  cursor.movePosition( QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );
  if( baseIndentation() )
  {
    cursor.insertText( QString( baseIndentation(), ' ' ) );
    if( block == current_cursor_.block() ) 
    { 
      current_cursor_.setPosition( current_cursor_.anchor() + baseIndentation(), QTextCursor::MoveAnchor ); 
      current_cursor_.setPosition( current_cursor_.position() + baseIndentation(), QTextCursor::KeepAnchor ); 
    }
  }
  
  // locate cursor at beginning of the paragraph + baseIndentation
  cursor.setPosition( block.position() + baseIndentation(), QTextCursor::MoveAnchor );
  for( unsigned int i=0; i < count; i++ )
  {    

    int position( current_cursor_.position() );
    int anchor( current_cursor_.anchor() );
    
    // insert tab characters
    cursor.insertText( editor_->tabCharacter() );
    if( block == current_cursor_.block() ) 
    { 
      current_cursor_.setPosition( anchor + editor_->tabCharacter().size(), QTextCursor::MoveAnchor ); 
      current_cursor_.setPosition( position + editor_->tabCharacter().size(), QTextCursor::KeepAnchor ); 
    }
    
  }
  
}

//____________________________________________
void TextIndent::_decrement( QTextBlock block )
{
  
  // set a cursor at beginning of block
  QTextCursor cursor( block );
  cursor.setPosition( block.position() + baseIndentation(), QTextCursor::MoveAnchor );

  // leading space characters regexp
  static const QRegExp regexp( "^\\s+" );

  // search text and remove characters
  if( regexp.indexIn( block.text().mid( baseIndentation() ) ) >= 0 )
  {
    int position( current_cursor_.position() );
    int anchor( current_cursor_.anchor() );
    int length( regexp.matchedLength() );
    Debug::Throw() << "TextIndent::_decrement - matched length: " << length << endl;
    
    cursor.setPosition( cursor.position() + length, QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
    if( current_cursor_.block() == block && current_cursor_.position() - block.position() > baseIndentation() ) 
    {
      current_cursor_.setPosition( max( 0, anchor - length ), QTextCursor::MoveAnchor ); 
      current_cursor_.setPosition( max( 0, position - length ), QTextCursor::KeepAnchor );
    }
  } else { Debug::Throw() << "TextIndent::_decrement - no match" << endl; } 
}
