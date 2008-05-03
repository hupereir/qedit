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
* software; if not, write to the Free Software , Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file BlockDelimiterWidget.cpp
  \brief display block delimiters
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QApplication>
#include <QPainter>
#include <QScrollBar>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>

#include <vector>

#include "BlockHighlight.h"
#include "TextDisplay.h"
#include "Debug.h"
#include "BlockDelimiterWidget.h"
#include "HighlightBlockData.h"
#include "XmlOptions.h"

using namespace std;

//____________________________________________________________________________
BlockDelimiterWidget::BlockDelimiterWidget(TextDisplay* editor, QWidget* parent): 
  QWidget( parent),
  Counter( "BlockDelimiterWidget" ),
  editor_( editor )
{
  Debug::Throw( "BlockDelimiterWidget::BlockDelimiterWidget.\n" );
  setAutoFillBackground( true );
  
  connect( _editor().verticalScrollBar(), SIGNAL( valueChanged( int ) ), SLOT( update() ) );
  connect( &_editor(), SIGNAL( textChanged() ), SLOT( update() ) );  
  
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
    
  // update configuration
  _updateConfiguration();
  
}

//__________________________________________
BlockDelimiterWidget::~BlockDelimiterWidget()
{
  Debug::Throw( "BlockDelimiterWidget::~BlockDelimiterWidget.\n" );
}

//__________________________________________
void BlockDelimiterWidget::synchronize( const BlockDelimiterWidget* widget )
{
  Debug::Throw( "BlockDelimiterWidget::synchronize.\n" );
  delimiters_ = widget->delimiters_;
  segments_ = widget->segments_;
}

//__________________________________________
void BlockDelimiterWidget::paintEvent( QPaintEvent* )
{  
  
  if( delimiters_.empty() ) return;
  
  const QFontMetrics metric( fontMetrics() );
  int y_offset = _editor().verticalScrollBar()->value();
  QTextDocument &document( *_editor().document() );
      
  // calculate height
  int height( QWidget::height() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) 
  { height -= _editor().horizontalScrollBar()->height(); }
  
  // clear existing segments
  segments_.clear();
  
  // loop over delimiters
  for( BlockDelimiter::List::const_iterator iter = delimiters_.begin(); iter != delimiters_.end(); iter++ )
  {
  
    // keep track of all starting points
    Segment::List start_points;
    for( QTextBlock block = document.begin(); block.isValid(); block = block.next() ) 
    {

      // get block limits
      int block_begin( block.layout()->position().y() );
      int block_end( block_begin + block.layout()->boundingRect().height() );
      
      // check if outside of window
      if( block_begin > height ) break;
      
      // retrieve data and check this block delimiter
      HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
      if( !data ) continue;

      // store "ignore" state
      bool ignored = _editor().ignoreBlock( block );
      
      TextBlock::Delimiter delimiter( data->delimiter( iter->id() ) );
      if( delimiter.end() && block_end < height )
      {
        
        if( !(start_points.empty() || delimiter.begin() ) && ignored == start_points.back().ignored() ) 
        { segments_.push_back( start_points.back().setSecond( block_end ) ); }
        
        // pop
        for( int i = 0; i < delimiter.end() && !start_points.empty() && ignored == start_points.back().ignored(); i++ )
        { start_points.pop_back(); }
          
      }
        
      // if block is collapsed, skip one start point (which is self contained)
      const bool& collapsed( data->collapsed() );
      for( int i = (collapsed ? 1:0); i < delimiter.begin(); i++ )
      { start_points.push_back( Segment( block_begin, height, ignored, collapsed ) ); }

      // if block is collapsed add one self contained segment
      if( collapsed ) { segments_.push_back( Segment( block_begin, block_begin, ignored, true ) ); }
    }
    
    for( Segment::List::iterator iter = start_points.begin(); iter != start_points.end(); iter++ )
    { segments_.push_back( *iter ); }
  
  }
  
  // painter
  QPainter painter( this );
  painter.translate( 0, -y_offset );
  painter.setBrush( palette().color( QPalette::Base ) );

  // draw all vertical lines
  for( Segment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  { if( iter->second() <= height ) iter->drawLine( painter, width() ); }
    
  // draw ticks
  for( Segment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {
    
    if( iter->first() < height ) iter->drawFirstDelimiter( painter, width() );    
    if( iter->second() < height ) iter->drawSecondDelimiter( painter, width() );    
    
  }
  
  painter.end();
  
}

//________________________________________________________
void BlockDelimiterWidget::mousePressEvent( QMouseEvent* event )
{
  
  return;
  
  // check button
  if( !( event->button() == Qt::LeftButton ) ) return;

  // find segment matching event position
  Segment::List::const_iterator iter = find_if( 
    segments_.begin(), segments_.end(), 
    Segment::ContainsFTor( event->pos()+QPoint(0, _editor().verticalScrollBar()->value() ) ) );
  if( iter == segments_.end() ) return;
  
  // find matching begin and end blocks
  QTextDocument &document( *_editor().document() );
  QTextBlock first_block;
  QTextBlock second_block;
  HighlightBlockData* first_block_data(0);
  
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next() ) 
  {
    
    // get block limits
    int block_begin( block.layout()->position().y() );
    int block_end( block_begin + block.layout()->boundingRect().height() );
    
    // check if first block match
    if( block_begin == iter->first() )
    {
      first_block = block;
      
      // if segment is collapsed, break loop
      first_block_data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
      assert( first_block_data );
      
      if( first_block_data->collapsed() ) break;
      
    }
    
    // check if second block match
    if( first_block.isValid() && block_end == iter->second() )
    {
      second_block = block;
      break;
    }
    
  }

  // make sure first block was found
  assert( first_block.isValid() );
  
  // check if block is collapsed
  if( first_block_data->collapsed() )
  { 
    
    first_block_data->setCollapsed( false );
    if( !first_block_data->collapsedText().isEmpty() )
    {
      
      // create cursor
      QTextCursor cursor( first_block );
      
      // move at end of first_block
      cursor.setPosition( first_block.position() + first_block.length(), QTextCursor::MoveAnchor );      
      cursor.insertText( first_block_data->collapsedText() );
      
    } 
    
    // marck first_block as dirty to make sure it is re-highlighted
    _editor().document()->markContentsDirty(first_block.position(), first_block.length()-1);
    update();
    
  } else {
    
    first_block_data->setCollapsed( true );
    
    // if next block is not valid, nothing is to be done
    if( first_block.next().isValid() )
    {
      
      // create cursor
      QTextCursor cursor( first_block );
      
      // move at end of first_block
      cursor.setPosition( first_block.position() + first_block.length(), QTextCursor::MoveAnchor );
      
      // move at end of last block if valid
      if( second_block.isValid() ) { cursor.setPosition( second_block.position() + second_block.length(), QTextCursor::KeepAnchor ); }
      else { cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor ); }
 
      // store selected text in data
      first_block_data->setCollapsedText( cursor.selectedText() );
      cursor.removeSelectedText();
      
    } 
    
    _editor().document()->markContentsDirty(first_block.position(), first_block.length()-1);    
    update();
          
  }
     
}

//________________________________________________________
void BlockDelimiterWidget::mouseReleaseEvent( QMouseEvent* event )
{
}

//___________________________________________________________
void BlockDelimiterWidget::wheelEvent( QWheelEvent* event )
{ qApp->sendEvent( _editor().viewport(), event ); }

//________________________________________________________
void BlockDelimiterWidget::_updateConfiguration( void )
{
  
  Debug::Throw( "BlockDelimiterWidget::_updateConfiguration.\n" );
  
  // font
  QFont font;
  font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ).c_str() );
  setFont( font );
   
  // width
  setFixedWidth( fontMetrics().lineSpacing() );

}

//________________________________________________________
void BlockDelimiterWidget::Segment::drawLine( QPainter& painter, const int& width )
{ if( first() != second() ) painter.drawLine( width/2, first_+0.8*width, width/2, second_ ); }

//________________________________________________________
void BlockDelimiterWidget::Segment::drawFirstDelimiter( QPainter& painter, const int& width )
{ 
  active_ = QRect( 0.2*width, first() + 0.2*width, 0.6*width, 0.6*width );
  painter.drawRect( activeRect() );
  painter.drawLine( 0.35*width, first() + 0.5*width, 0.65*width, first() + 0.5*width );
  if( collapsed() )
  { painter.drawLine( 0.5*width, first() + 0.35*width, 0.5*width, first() + 0.65*width ); }
  
}

//________________________________________________________
void BlockDelimiterWidget::Segment::drawSecondDelimiter( QPainter& painter, const int& width )
{ 
  if( first() != second() )
  { painter.drawLine( 0.5*width, second(), width, second() ); }
}
