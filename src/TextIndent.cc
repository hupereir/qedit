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

  if( !enabled() ) return;

  // store block and cursor
  current_cursor_ = editor_->textCursor();
  
  // disable updates during manipulations
  editor_->setUpdatesEnabled( false );
  
  // decrement if first paragraph of text
  QTextBlock previous_block( block.previous() );
  if( !previous_block.isValid() ) decrement( block );
  else {

    // get previous paragraph tabs
    int previous_tabs( _tabCount( previous_block ) );
    int new_tabs = previous_tabs;
    for( IndentPattern::List::iterator iter = patterns_.begin(); iter != patterns_.end(); iter++ )
    {
      if( _acceptPattern( block, **iter ) )
      {
        
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
  editor_->updateContents();
  
}

//____________________________________________
bool TextIndent::_acceptPattern( QTextBlock block, const IndentPattern& pattern ) const
{
  
  // retrieve rules associated to pattern
  bool accepted( true );
  const IndentPattern::RuleList& rules( pattern.Rules() );
  for( IndentPattern::RuleList::const_iterator iter = rules.begin(); iter != rules.end() && accepted; iter++ )
  {

    // if working on current paragraph
    if( iter->paragraph() == 0 )
    { if( !iter->accept( block.text() ) ) accepted = false; }
    else {

      // decrepent paragraph, skipping ignored lines
      int decrement = 0;
      do {
        block = block.previous()
        decrement--;
        while( block.isValid() && editor_->ignoreBlock( block ) ) 
        { block = block.previous(); }
        
      } while( block.isValid() && decrement > iter->paragraph() );

      // check paragraph and regexp
      // here one could have a flag on the indentation pattern rules 
      // to check what is to be done when there is no valid paragraph
      // matching the request. This would allow to enable some patterns 
      // that cannot otherwise.
      if( !block.isValid() || !iter->accept( block.text() ) ) accepted = false;
    
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
  unsigned int index = base_indentation_;
  
  // loop over next characters to identify tabs
  // both normal and emulated tabs are counted
  while( index < text.size() )
  {
    if( text.mid( index, editor_->normalTab().size() ) == editor_->normalTab() )
    { index += editor_->normalTab().size(); count++; }
    
    else if( text.mid( index, editor_->emulatedTab().size() ) == editor_->emulatedTab() )
    { index += editor_->emulatedTab().size(); count++; } 
    
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
    
    // insert tab characters
    cursor.insertText( editor_->tab() );
    if( block == current_cursor_.block() ) 
    { 
      current_cursor_.setPosition( current_cursor_.anchor() + editor_->tab().size(), QTextCursor::MoveAnchor ); 
      current_cursor_.setPosition( current_cursor_.position() + editor_->tab().size(), QTextCursor::KeepAnchor ); 
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
  static const QRegExp regexp( "^(\\s+)(?!$)" );

  // search text and remove characters
  if( regexp.indexIn( block.text().mid( baseIndentation() ) ) >= 0 )
  {
    int length( regexp.matchedLength() );
    cursor.setPosition( cursor.position() + length, QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
    if( current_cursor.block() == block && current_cursor.position() - block.position() > baseIndentation() ) ) 
    {
      current_cursor_.setPosition( max( 0, current_cursor_.anchor() - regexp.matchedLength() ), QTextCursor::MoveAnchor ); 
      current_cursor_.setPosition( max( 0, current_cursor_.position() - regexp.matchedLength() ), QTextCursor::KeepAnchor );
    }
  }
}
