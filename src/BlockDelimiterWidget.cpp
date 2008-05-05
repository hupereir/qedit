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

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QScrollBar>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>

#include <vector>

#include "BlockHighlight.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "Debug.h"
#include "BlockDelimiterWidget.h"
#include "HighlightBlockData.h"
#include "XmlOptions.h"

using namespace std;

//____________________________________________________________________________
BlockDelimiterWidget::BlockDelimiterWidget(TextDisplay* editor, QWidget* parent): 
  QWidget( parent),
  Counter( "BlockDelimiterWidget" ),
  editor_( editor ),
  need_segment_update_( true )
{
  Debug::Throw( "BlockDelimiterWidget::BlockDelimiterWidget.\n" );
  setAutoFillBackground( true );
  
  // actions
  _installActions();
  
  connect( _editor().verticalScrollBar(), SIGNAL( valueChanged( int ) ), SLOT( update() ) );
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( needSegmentUpdate() ) );
  connect( &_editor(), SIGNAL( textChanged() ), SLOT( update() ) );
  connect( &_editor().textHighlight(), SIGNAL( needSegmentUpdate() ), SLOT( needSegmentUpdate() ) );
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
  
  //Debug::Throw(0, "BlockDelimiterWidget::paintEvent.\n" );
  
  // check delimiters
  if( delimiters_.empty() ) return;
    
  /* update segments if needed */
  if( need_segment_update_ ) 
  { _updateSegments(); }
  
  // by default next paintEvent will require segment update
  need_segment_update_ = false;
    
  // calculate dimensions
  int y_offset = _editor().verticalScrollBar()->value();
  int height( QWidget::height() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) 
  { height -= _editor().horizontalScrollBar()->height(); }

  // create painter
  QPainter painter( this );
  painter.translate( 0, -y_offset );
  painter.setBrush( palette().color( QPalette::Base ) );

  // optimize drawing by not drawing overlapping segments
  BlockDelimiterSegment::List::const_reverse_iterator previous( segments_.rend() );
  for( BlockDelimiterSegment::List::const_reverse_iterator iter = segments_.rbegin(); iter != segments_.rend(); iter++ )
  {
    // skip this segment if included in previous
    if( previous != segments_.rend() && iter->first() >= previous->first() && iter->second() <= previous->second() ) 
    { continue; }
    
    previous = iter;
    
    // draw
    if( !iter->empty() ) 
    { 
      painter.drawLine( 
        half_width_, iter->first()+top_, 
        half_width_, min( height, iter->second() ) ); 
    } else { 
      painter.drawLine( 
        half_width_, iter->first()+top_, 
        half_width_, height ); 
    }
    
  }
  
  // draw ticks
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {
    
    // begin tick
    if( iter->first()+top_ < height ) 
    {
      iter->setActiveRect( QRect( rect_top_left_, iter->first() + rect_top_left_, rect_width_, rect_width_ ) );
      painter.drawRect( iter->activeRect() );
      painter.drawLine( 
        marker_top_left_, iter->first() + half_width_, 
        marker_bottom_right_, iter->first() + half_width_ );
      
      if( iter->flag( BlockDelimiterSegment::COLLAPSED ) ) 
      { 
        painter.drawLine( 
          half_width_, iter->first() + marker_top_left_, 
          half_width_, iter->first() + marker_bottom_right_ ); 
      }  

    }
    
    // end tick
    if( iter->second() < height && !( iter->flag( BlockDelimiterSegment::FIRST_ONLY ) || iter->empty() ) )
    { painter.drawLine( half_width_, iter->second(), width_, iter->second() ); }
    
  }
  
  painter.end();
  
}

//________________________________________________________
void BlockDelimiterWidget::mousePressEvent( QMouseEvent* event )
{

  Debug::Throw( "BlockDelimiterWidget::mousePressEvent.\n" );
    
  // check button
  if( !( event->button() == Qt::LeftButton ) ) return;

  // find segment matching event position
  BlockDelimiterSegment::List::const_iterator iter = find_if( 
    segments_.begin(), segments_.end(), 
    BlockDelimiterSegment::ContainsFTor( event->pos()+QPoint(0, _editor().verticalScrollBar()->value() ) ) );
  if( iter == segments_.end() ) return;
  
  // find matching begin and end blocks
  QTextBlock first_block;
  QTextBlock second_block;
  HighlightBlockData* first_block_data(0);
  
  QTextDocument &document( *_editor().document() );
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
      first_block_data = static_cast<HighlightBlockData*>( block.userData() );
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
  if( first_block_data->collapsed() ) _expand( first_block, first_block_data );
  else _collapse( first_block, second_block, first_block_data );
   
  // marck block as dirty to make sure it is re-highlighted
  need_segment_update_ = true;
  _editor().document()->markContentsDirty(first_block.position(), first_block.length()-1);

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
     
  // set dimensions needed to redraw marker and lines
  // this is done to minimize the amount of maths in the paintEvent method
  width_ = fontMetrics().lineSpacing();
  half_width_ = 0.5*width_;
  top_ = 0.8*width_;
  rect_top_left_ = 0.2*width_;
  rect_width_ = 0.6*width_;
  marker_top_left_ = 0.35*width_;
  marker_bottom_right_ = 0.65*width_; 
    
  // adjust size
  setFixedWidth( width_ );

}

//________________________________________________________
void BlockDelimiterWidget::_expandAllBlocks( void )
{
  Debug::Throw( "BlockDelimiterWidget::_expandAllBlocks.\n" );
  
  QTextDocument &document( *_editor().document() );
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next() ) 
  {
    
    // retrieve data and check if collapsed
    HighlightBlockData* data( static_cast<HighlightBlockData*>( block.userData() ) );
    if( !( data && data->collapsed() ) ) continue;
    _expand( block, data, true );
      
  }
    
  return;
  
}

//________________________________________________________
void BlockDelimiterWidget::_installActions( void )
{
  Debug::Throw( "BlockDelimiterWidget::_installActions.\n" );

  addAction( expand_all_action_ = new QAction( "&Expand all blocks", this ) );
  expand_all_action_->setToolTip( "expand all collapsed blocks" );
  connect( expand_all_action_, SIGNAL( triggered() ), SLOT( _expandAllBlocks() ) );
  expand_all_action_->setEnabled( false );
}

//________________________________________________________
void BlockDelimiterWidget::_updateSegments( void )
{

  // Debug::Throw(0, "BlockDelimiterWidget::_updateSegments.\n" );
  
  segments_.clear();
  
  // keep track of collapsed blocks
  bool has_collapsed_blocks( false ); 
  unsigned int collapsed_block_count(0);
  collapsed_blocks_.clear();
  
  // loop over delimiter types
  bool first( true );
  for( BlockDelimiter::List::const_iterator iter = delimiters_.begin(); iter != delimiters_.end(); iter++ )
  {
  
    // keep track of all starting points
    BlockDelimiterSegment::List start_points;
    int block_count(0);
    QTextDocument &document( *_editor().document() );
    for( QTextBlock block = document.begin(); block.isValid(); block = block.next(), block_count++ ) 
    {
      
      /*
      get block limits
      either use the block layout (if available)
      or the document layout, if the former is not ready yet. 
      One checks that the bounding rect if valid before proceeding.
      */      
      QRectF rect( _editor().document()->documentLayout()->blockBoundingRect( block ) );
      assert( !rect.isNull() );
            
      int block_begin( block.layout()->position().y() );
      int block_end( block_begin + block.layout()->boundingRect().height() );
            
      // retrieve data and check this block delimiter
      HighlightBlockData* data = (static_cast<HighlightBlockData*>( block.userData() ) );
      if( !data ) continue;

      // store "ignore" state
      bool ignored = _editor().ignoreBlock( block );
      
      // get delimiter data
      TextBlock::Delimiter delimiter( data->delimiter( iter->id() ) );
      if( delimiter.end() )
      {
        
        if( !(start_points.empty() ) && ignored == start_points.back().flag( BlockDelimiterSegment::IGNORED ) ) 
        { 
          // if block is both begin and end, only the begin flag is to be drawn.
          if( delimiter.begin() ) start_points.back().setFlag( BlockDelimiterSegment::FIRST_ONLY, true );
          segments_.push_back( start_points.back().setSecond( block_end ) ); 
        }
        
        // pop
        for( int i = 0; i < delimiter.end() && !start_points.empty() && ignored == start_points.back().flag( BlockDelimiterSegment::IGNORED ); i++ )
        { start_points.pop_back(); }
          
      }

      // store collapse state
      const bool& collapsed( data->collapsed() );
      if( collapsed || delimiter.begin() )
      {
        
        // prepare segment flags
        unsigned int flags( BlockDelimiterSegment::NONE );
        if( ignored ) flags |= BlockDelimiterSegment::IGNORED;
        if( collapsed ) flags |= BlockDelimiterSegment::COLLAPSED;
        
        // if block is collapsed, skip one start point (which is self contained)
        for( int i = (collapsed ? 1:0); i < delimiter.begin(); i++ )
        { start_points.push_back( BlockDelimiterSegment(block_begin, block_begin, flags ) ); }
    
        if( collapsed ) { 
 
          // store number of collapsed blocks for the current one
          if( first )
          {
            collapsed_blocks_.insert( make_pair( block_count, collapsed_block_count ) );
            collapsed_block_count += data->collapsedBlockCount();
          }
          
          // add one self contained segment
          has_collapsed_blocks = true;
          segments_.push_back( BlockDelimiterSegment( block_begin, block_end, flags ) );
        }
        
      }
            
    }
    
    // insert the remaining points as empty segments (that will extend to the end of the document)
    /* they are inserted in reverse order to optimize segment drawing in paintEvent */
    for( BlockDelimiterSegment::List::const_reverse_iterator iter = start_points.rbegin(); iter != start_points.rend(); iter++ )
    { segments_.push_back( *iter ); }
    
    // insert total number of collapsed block as last element
    if( first ) 
    {
      collapsed_blocks_.insert( make_pair( block_count, collapsed_block_count ) );
      first = false;
    }

  }
    
  // update expand all action
  expandAllAction().setEnabled( has_collapsed_blocks );
  
}

//________________________________________________________________________________________
void BlockDelimiterWidget::_expand( const QTextBlock& block, HighlightBlockData* data, const bool& recursive ) const
{ 
    
  data->setCollapsed( false );

  // create cursor
  QTextCursor cursor( block );      
  cursor.setPosition( block.position() + block.length() - 1, QTextCursor::MoveAnchor );      
  const CollapsedBlockData::List& collapsed_data( data->collapsedData() );
  if( collapsed_data.empty() ) return;

  bool modified( _editor().document()->isModified() );
  bool undo_enabled( _editor().document()->isUndoRedoEnabled() );
  _editor().document()->setUndoRedoEnabled( false );

  cursor.beginEditBlock();
  for( CollapsedBlockData::List::const_iterator iter = collapsed_data.begin(); iter != collapsed_data.end(); iter++ )
  {
    
    cursor.insertBlock();
    cursor.insertText( iter->text() );
    
    HighlightBlockData* current_data( new HighlightBlockData() );
    current_data->setDelimiters( iter->delimiters() );
    current_data->setCollapsed( iter->collapsed() );
    if( iter->collapsed() ) { current_data->setCollapsedData( iter->children() ); }
    cursor.block().setUserData( current_data );
    
    // also expands block if collapsed and recursive is set to true
    if( iter->collapsed() && recursive ) _expand( cursor.block(), current_data, true );
    
  }
  
  cursor.endEditBlock();
  _editor().document()->setUndoRedoEnabled( undo_enabled );
  _editor().document()->setModified( modified );
  
}

//________________________________________________________________________________________
void BlockDelimiterWidget::_collapse( const QTextBlock& first_block, const QTextBlock& second_block, HighlightBlockData* data ) const
{
   
  data->setCollapsed( true );
    
  // if next block is not valid, nothing is to be done
  if( !first_block.next().isValid() ) return;

  // get block associated to cursor
  // see if cursor belongs to collapsible block
  QTextBlock cursor_block( _editor().textCursor().block() );
  bool cursor_found( false );
    
  // create collapsed block data to be stored in current block before collapsed
  CollapsedBlockData::List collapsed_data_list;  
  for( QTextBlock current = first_block.next(); current.isValid(); current = current.next() )
  {
    
    // if current block match cursor, 
    // one need to move the cursor after the text gets deleted
    if( current == cursor_block ) cursor_found = true;

    // append collapsed data
    collapsed_data_list.push_back( CollapsedBlockData( current ) );
    
    if( current == second_block ) break;
    
  }
  
  // store in current block
  data->setCollapsedData( collapsed_data_list );
  
  bool modified( _editor().document()->isModified() );
  bool undo_enabled( _editor().document()->isUndoRedoEnabled() );
  _editor().document()->setUndoRedoEnabled( false );
  
  // create cursor and move at end of block
  QTextCursor cursor( first_block );
  cursor.setPosition( first_block.position() + first_block.length(), QTextCursor::MoveAnchor );
  if( second_block.isValid() ) { cursor.setPosition( second_block.position() + second_block.length(), QTextCursor::KeepAnchor ); }
  else { cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor ); }
  cursor.removeSelectedText();
  
  // restore state
  _editor().document()->setUndoRedoEnabled( undo_enabled );
  _editor().document()->setModified( modified );
 
  // move cursor to end of block
  if( cursor_found )
  { 
    cursor.setPosition( first_block.position() + first_block.length()-1 );
    _editor().setTextCursor( cursor );
  }
  
}

