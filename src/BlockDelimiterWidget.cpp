// $Id$

/*****************************************************************************
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
#include <QStyleOption>
#include <QStyle>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>
#include <QTreeView>

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
  need_update_( true )
{
  
  Debug::Throw( "BlockDelimiterWidget::BlockDelimiterWidget.\n" );
  setAutoFillBackground( true );
  
  // actions
  _installActions();
  
  // connections
  connect( _editor().verticalScrollBar(), SIGNAL( valueChanged( int ) ), SLOT( update() ) );
  connect( &_editor(), SIGNAL( textChanged() ), SLOT( update() ) );
  connect( &_editor().textHighlight(), SIGNAL( needSegmentUpdate() ), SLOT( _needUpdate() ) );
  connect( &_editor().wrapModeAction(), SIGNAL( toggled( bool ) ), SLOT( _needUpdate() ) );
  connect( &_editor().wrapModeAction(), SIGNAL( toggled( bool ) ), SLOT( update() ) );
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
  connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );

  // update configuration
  _updateConfiguration();
  
}

//__________________________________________
BlockDelimiterWidget::~BlockDelimiterWidget()
{ Debug::Throw( "BlockDelimiterWidget::~BlockDelimiterWidget.\n" ); }

//__________________________________________
void BlockDelimiterWidget::synchronize( const BlockDelimiterWidget* widget )
{
  Debug::Throw( "BlockDelimiterWidget::synchronize.\n" );

  // copy members 
  delimiters_ = widget->delimiters_;
  segments_ = widget->segments_;
  collapsed_blocks_ = widget->collapsed_blocks_;
  need_update_ = widget->need_update_;
  
  // re-initialize connections
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
  connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );

}

//__________________________________________
void BlockDelimiterWidget::setActionVisibility( const bool& state )
{
  Debug::Throw( "BlockDelimiterWidget::setActionVisibility.\n" );
  
  collapseCurrentAction().setVisible( state );
  expandCurrentAction().setVisible( state );
  collapseAction().setVisible( state );
  expandAllAction().setVisible( state );  
}

//__________________________________________
void BlockDelimiterWidget::updateCurrentBlockActionState( void )
{
  
  /* update segments if needed */
  if( need_update_ ) 
  { 
    _updateSegments(); 
    need_update_ = false;
  }
  
  // by default next paintEvent will require segment update
  int y( _editor().document()->documentLayout()->blockBoundingRect( _editor().textCursor().block() ).y() );

  BlockDelimiterSegment::ContainsFTor expanded_ftor( y, false );
  BlockDelimiterSegment::List::iterator expanded_iter = std::find_if( segments_.begin(), segments_.end(), expanded_ftor );
  collapseCurrentAction().setEnabled( expanded_iter != segments_.end() );

  BlockDelimiterSegment::ContainsFTor collapsed_ftor( y, true );  
  BlockDelimiterSegment::List::iterator collapsed_iter = std::find_if( segments_.begin(), segments_.end(), collapsed_ftor );
  expandCurrentAction().setEnabled( collapsed_iter != segments_.end() );
  
}

//__________________________________________
void BlockDelimiterWidget::paintEvent( QPaintEvent*)
{  
    
  // check delimiters
  if( delimiters_.empty() ) return;
    
  /* update segments if needed */
  if( need_update_ ) 
  { 
    _updateSegments(); 
    need_update_ = false;
  }
    
  // calculate dimensions
  int y_offset = _editor().verticalScrollBar()->value();
  int height( QWidget::height() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) 
  { height -= _editor().horizontalScrollBar()->height(); }

  // create painter
  QPainter painter( this );
  painter.translate( 0, -y_offset );
  
  painter.save();
  QPen pen;
  pen.setStyle( Qt::DotLine );
  painter.setPen( pen );
  
  // optimize drawing by not drawing overlapping segments
  BlockDelimiterSegment::List::reverse_iterator previous( segments_.rend() );
  for( BlockDelimiterSegment::List::reverse_iterator iter = segments_.rbegin(); iter != segments_.rend(); iter++ )
  {
          
    // skip this segment if included in previous
    if( previous != segments_.rend() && iter->begin() >= previous->begin() && iter->end() <= previous->end() ) 
    { continue; }
    
    previous = iter;
    
    // draw
    if( iter->begin()+top_ < height )
    {
      
      if( !iter->empty() ) 
      { 
        painter.drawLine( 
          half_width_, iter->begin()+top_, 
          half_width_, min( height, iter->end() ) ); 
        
      } else { 
        painter.drawLine( 
          half_width_, iter->begin()+top_, 
          half_width_, height ); 
      }
        
    }
    
  }
    
  // end tick
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {
    
    if( iter->end() <= height && !( iter->flag( BlockDelimiterSegment::BEGIN_ONLY ) || iter->empty() ) )
    { painter.drawLine( half_width_, iter->end(), width_, iter->end() ); }
    
  }

  painter.restore();
  
  // draw begin ticks
  // first draw empty square
  painter.save();
  painter.setPen( Qt::NoPen );
  painter.setBrush( palette().color( QPalette::Base ) );
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {
    
    if( iter->begin()+top_ >= height ) { continue; } 

    iter->setActiveRect( QRect( rect_top_left_, iter->begin() + rect_top_left_, rect_width_, rect_width_ ) );
    painter.drawRect( iter->activeRect() );
  
  }
  painter.restore();
  
  // use the QStyle primitive elements for TreeViews
  QStyleOption option;
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {
    
    if( iter->begin()+top_ >= height ) { continue; } 

    option.initFrom( this );
    option.rect = iter->activeRect();
    option.state |= QStyle::State_Children;
    if( !iter->flag( BlockDelimiterSegment::COLLAPSED ) ) { option.state |= QStyle::State_Open; }
    
    style()->drawPrimitive( QStyle::PE_IndicatorBranch, &option, &painter );
    
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
    BlockDelimiterSegment::ActiveFTor( event->pos()+QPoint(0, _editor().verticalScrollBar()->value() ) ) );
  if( iter == segments_.end() ) return;
  
  // retrieve matching segments
  QTextDocument &document( *_editor().document() );
  HighlightBlockData* data(0);
  TextBlockPair blocks( _findBlocks( document.begin(), *iter, data ) );
  
  // check if block is collapsed
  if( data->collapsed() ) 
  {
    bool cursor_visible( _editor().isCursorVisible() );
    _editor().clearBoxSelection();
    _expand( blocks.first, data );
    if( cursor_visible ) _editor().ensureCursorVisible();
  } else {
    _editor().clearBoxSelection();
    _collapse( blocks.first, blocks.second, data );
  }
  
  // force segment update at next update()
  need_update_ = true;

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
void BlockDelimiterWidget::_contentsChanged( void )
{
  // if text is wrapped, line number data needs update at next update
  /* note: this could be further optimized if one retrieve the position at which the contents changed occured */
  if( _editor().lineWrapMode() != QTextEdit::NoWrap )
  { need_update_ = true; }
}

//________________________________________________________
void BlockDelimiterWidget::_blockCountChanged( void )
{
  
  // nothing to be done if wrap mode is not NoWrap, because
  // it is handled in the _contentsChanged slot.
  if( _editor().lineWrapMode() == QTextEdit::NoWrap )
  { need_update_ = true; }
  
}

//________________________________________________________
void BlockDelimiterWidget::_collapseCurrentBlock( void )
{
  Debug::Throw( "BlockDelimiterWidget::_collapseCurrentBlock.\n" );

  /* update segments if needed */
  if( need_update_ ) 
  { 
    _updateSegments(); 
    need_update_ = false;
  }
 
  // sort segments so that top level comes last
  std::sort( segments_.begin(), segments_.end(), BlockDelimiterSegment::SortFTor() );

  // get cursor position
  QTextDocument& document( *_editor().document() );
  int y( document.documentLayout()->blockBoundingRect( _editor().textCursor().block() ).y() );

  // find matching segment
  BlockDelimiterSegment::ContainsFTor expanded_ftor( y, false );
  BlockDelimiterSegment::List::iterator iter = std::find_if( segments_.begin(), segments_.end(), expanded_ftor );
  if( iter == segments_.end() ) return;

  // find matching blocks
  HighlightBlockData *data(0);
  TextBlockPair blocks( _findBlocks( document.begin(), *iter, data ) );
  
  // collapse
  _editor().clearBoxSelection();
  _collapse( blocks.first, blocks.second, data );
  return;

}
  
//________________________________________________________
void BlockDelimiterWidget::_expandCurrentBlock( void )
{
  Debug::Throw( "BlockDelimiterWidget::_expandCurrentBlock.\n" );

  /* update segments if needed */
  if( need_update_ ) 
  { 
    _updateSegments(); 
    need_update_ = false;
  }

  // sort segments so that top level comes last
  std::sort( segments_.begin(), segments_.end(), BlockDelimiterSegment::SortFTor() );

  // get cursor position
  QTextDocument& document( *_editor().document() );
  int y( document.documentLayout()->blockBoundingRect( _editor().textCursor().block() ).y() );

  // find matching segment
  BlockDelimiterSegment::ContainsFTor collapsed_ftor( y, true );  
  BlockDelimiterSegment::List::iterator iter = std::find_if( segments_.begin(), segments_.end(), collapsed_ftor );
  if( iter == segments_.end() ) return;
  
  // find matching blocks
  HighlightBlockData *data(0);
  TextBlockPair blocks( _findBlocks( document.begin(), *iter, data ) );
  
  // collapse
  bool cursor_visible( _editor().isCursorVisible() );
  _editor().clearBoxSelection();
  _expand( blocks.first, data );
  if( cursor_visible ) _editor().ensureCursorVisible();
  return;
  
}

//________________________________________________________
void BlockDelimiterWidget::_collapseTopLevelBlocks( void )
{
  
  Debug::Throw( "BlockDelimiterWidget::_collapseTopLevelBlocks.\n" );
  
  /* update segments if needed */
  if( need_update_ ) 
  { 
    _updateSegments(); 
    need_update_ = false;
  }

  // sort segments so that top level comes last
  std::sort( segments_.begin(), segments_.end(), BlockDelimiterSegment::SortFTor() );
  
  // list of QTextCursor needed to remove blocks
  typedef std::vector<QTextCursor> CursorList;
  CursorList cursors;
  
  // get first block
  QTextBlock block( _editor().document()->begin() );
  HighlightBlockData* previous_block_data(0);
  
  // loop over segments in reverse order
  BlockDelimiterSegment::List::reverse_iterator previous(segments_.rend() );
  for( BlockDelimiterSegment::List::reverse_iterator iter = segments_.rbegin(); iter != segments_.rend(); iter++ )
  {
    
    // skip this segment if included in previous
    if( previous != segments_.rend() && iter->begin() >= previous->begin() && iter->end() <= previous->end() ) 
    { continue; }
  
    // update "previous" segment 
    previous = iter;
    
    // get matching blocks
    HighlightBlockData *data(0);
    TextBlockPair blocks( _findBlocks( block, *iter, data ) );

    // do nothing if block is already collapsed
    if( data->collapsed() ) {
      block = blocks.first;
      continue;
    }
    
    // store cursor
    cursors.push_back( _collapsedCursor( blocks.first, blocks.second, data ) ); 

    /* 
    this hack is needed to avoid loss of information in 
    case of adjacent blocks to be collapsed
    the collapsed block data are also added to the block above (prior to its being collapsed)
    */
    if( previous_block_data && blocks.first == block )
    {
      CollapsedBlockData::List collapsed_data( previous_block_data->collapsedData() );
      collapsed_data.insert( collapsed_data.end(), data->collapsedData().begin(), data->collapsedData().end() );
      previous_block_data->setCollapsedData( collapsed_data );
    } else previous_block_data = data;        
    
    // increment
    block = blocks.second;
    
  }
  
  // remove all text stored in cursor list
  bool modified( _editor().document()->isModified() );
  bool undo_enabled( _editor().document()->isUndoRedoEnabled() );
  _editor().document()->setUndoRedoEnabled( false );
  
  // clear box selection
  _editor().clearBoxSelection();

  // create cursor and move at end of block
  QTextCursor cursor( _editor().textCursor() );
  cursor.beginEditBlock();
  for( CursorList::const_iterator iter = cursors.begin(); iter != cursors.end(); iter++ )
  {
    cursor.setPosition( iter->anchor() );
    cursor.setPosition( iter->position(), QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
  }  
  cursor.endEditBlock();
  
  // restore state
  _editor().document()->setUndoRedoEnabled( undo_enabled );
  _editor().document()->setModified( modified );
    
}

//________________________________________________________
void BlockDelimiterWidget::_expandAllBlocks( void )
{
  Debug::Throw( "BlockDelimiterWidget::_expandAllBlocks.\n" );
  
  // clear box selection
  _editor().clearBoxSelection();

  /* update segments if needed */
  if( need_update_ ) 
  { 
    _updateSegments(); 
    need_update_ = false;
  }

  bool cursor_visible( _editor().isCursorVisible() );
  QTextDocument &document( *_editor().document() );
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next() ) 
  {
    
    // retrieve data and check if collapsed
    HighlightBlockData* data( static_cast<HighlightBlockData*>( block.userData() ) );
    if( !( data && data->collapsed() ) ) continue;
    _expand( block, data, true );
      
  }
    
  // set cursor position
  if( cursor_visible ) _editor().ensureCursorVisible();
  
  return;
  
}

//________________________________________________________
void BlockDelimiterWidget::_installActions( void )
{
  
  Debug::Throw( "BlockDelimiterWidget::_installActions.\n" );

  addAction( collapse_current_action_ = new QAction( "&Collapse current block", this ) );
  collapse_current_action_->setToolTip( "collapse current collapsed block" );
  collapse_current_action_->setShortcut( Qt::CTRL + Qt::Key_Minus );
  connect( collapse_current_action_, SIGNAL( triggered() ), SLOT( _collapseCurrentBlock() ) );
  collapse_current_action_->setEnabled( false );

  addAction( expand_current_action_ = new QAction( "&Expand current block", this ) );
  expand_current_action_->setToolTip( "expand current collapsed block" );
  expand_current_action_->setShortcut( Qt::CTRL + Qt::Key_Plus );
  connect( expand_current_action_, SIGNAL( triggered() ), SLOT( _expandCurrentBlock() ) );
  expand_current_action_->setEnabled( false );
    
  addAction( collapse_action_ = new QAction( "&Collapse top level blocks", this ) );
  collapse_action_->setToolTip( "collapse all top level blocks" );
  collapse_action_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_Minus );
  connect( collapse_action_, SIGNAL( triggered() ), SLOT( _collapseTopLevelBlocks() ) );
  collapse_action_->setEnabled( true );
  
  addAction( expand_all_action_ = new QAction( "&Expand all blocks", this ) );
  expand_all_action_->setToolTip( "expand all collapsed blocks" );
  expand_all_action_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_Plus );
  connect( expand_all_action_, SIGNAL( triggered() ), SLOT( _expandAllBlocks() ) );
  expand_all_action_->setEnabled( false );
   
}

//________________________________________________________
void BlockDelimiterWidget::_updateSegments( void )
{
    
  segments_.clear();
  
  // keep track of collapsed blocks
  bool has_collapsed_blocks( false ); 
  bool has_expanded_blocks( false );
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
      QRectF rect( document.documentLayout()->blockBoundingRect( block ) );
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
          if( delimiter.begin() ) start_points.back().setFlag( BlockDelimiterSegment::BEGIN_ONLY, true );
          segments_.push_back( start_points.back().setEnd( block_end ) ); 
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
          
        } else has_expanded_blocks = true;
        
      }
            
    }
    
    // insert the remaining points as empty segments (that will extend to the end of the document)
    /* they are inserted in reverse order to optimize segment drawing in paintEvent */
    for( BlockDelimiterSegment::List::reverse_iterator iter = start_points.rbegin(); iter != start_points.rend(); iter++ )
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
  collapseAction().setEnabled( has_expanded_blocks );
  
}

//_____________________________________________________________________________________
BlockDelimiterWidget::TextBlockPair BlockDelimiterWidget::_findBlocks( 
  QTextBlock block, 
  const BlockDelimiterSegment& segment, 
  HighlightBlockData*& data ) const
{

  TextBlockPair out;
  
  // look for first block
  for( ; block.isValid() && block.layout()->position().y() != segment.begin(); block = block.next() ) 
  {}
  
  assert( block.isValid() );
  
  // get data and check
  data = static_cast<HighlightBlockData*>( block.userData() );
  assert( data );
  
  // store
  out.first = block;

  // finish if block is collapsed
  if( data->collapsed() ) return out;
  
  // look for second block
  for( ; block.isValid() && block.layout()->position().y() + block.layout()->boundingRect().height() != segment.end(); block = block.next() )
  {}
  
  // store and return
  out.second = block;
  return out;
  
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

  // mark contents dirty to force update of current block
  data->setFlag( TextBlock::MODIFIED, true );
  _editor().document()->markContentsDirty(block.position(), block.length()-1);
  
  // add expanded block
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
         
  bool modified( _editor().document()->isModified() );
  bool undo_enabled( _editor().document()->isUndoRedoEnabled() );
  _editor().document()->setUndoRedoEnabled( false );

  // create cursor and move at end of block
  QTextCursor cursor( _collapsedCursor( first_block, second_block, data ) );
  cursor.beginEditBlock();
  cursor.removeSelectedText();
  cursor.endEditBlock();
  
  // restore state
  _editor().document()->setUndoRedoEnabled( undo_enabled );
  _editor().document()->setModified( modified );
      
}

//________________________________________________________________________________________
QTextCursor BlockDelimiterWidget::_collapsedCursor( const QTextBlock& first_block, const QTextBlock& second_block, HighlightBlockData* data ) const
{
   
  data->setCollapsed( true );
    
  // if next block is not valid, nothing is to be done
  if( !first_block.next().isValid() ) return QTextCursor();

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
  
  // mark contents dirty to force update of current block
  data->setFlag( TextBlock::MODIFIED, true );
  _editor().document()->markContentsDirty(first_block.position(), first_block.length()-1);
   
  // move cursor to end of block
  if( cursor_found )
  { 
    QTextCursor cursor( _editor().textCursor() );
    cursor.setPosition( first_block.position() + first_block.length()-1 );
    _editor().setTextCursor( cursor );
  }
  
  // create cursor and move at end of block
  QTextCursor cursor( first_block );
  cursor.setPosition( first_block.position() + first_block.length(), QTextCursor::MoveAnchor );
  if( second_block.isValid() ) { cursor.setPosition( second_block.position() + second_block.length(), QTextCursor::KeepAnchor ); }
  else { cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor ); }
    
  return cursor;
  
}
