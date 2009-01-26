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

/*!
  \file BlockDelimiterDisplay.h
  \brief display block delimiters
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

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
#include "BlockDelimiterDisplay.h"
#include "HighlightBlockData.h"
#include "XmlOptions.h"

using namespace std;

//____________________________________________________________________________
BlockDelimiterDisplay::BlockDelimiterDisplay(TextDisplay* editor ): 
  QObject( editor ),
  Counter( "BlockDelimiterDisplay" ),
  editor_( editor ),
  need_update_( true ),
  custom_symbols_( false ),
  offset_(0)
{
  
  Debug::Throw( "BlockDelimiterDisplay::BlockDelimiterDisplay.\n" );
    
  // actions
  _installActions();
  
  // connections
  connect( &_editor().textHighlight(), SIGNAL( needSegmentUpdate() ), SLOT( _needUpdate() ) );
  connect( &_editor().wrapModeAction(), SIGNAL( toggled( bool ) ), SLOT( _needUpdate() ) );
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
  connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );

  // initialize width
  // the value is meaningless but needed to avoid early non-initialize variables
  setWidth(0);
  
}

//__________________________________________
BlockDelimiterDisplay::~BlockDelimiterDisplay()
{ Debug::Throw( "BlockDelimiterDisplay::~BlockDelimiterDisplay.\n" ); }

//__________________________________________
void BlockDelimiterDisplay::synchronize( const BlockDelimiterDisplay* display )
{
  Debug::Throw( "BlockDelimiterDisplay::synchronize.\n" );

  // copy members 
  delimiters_ = display->delimiters_;
  segments_ = display->segments_;
  collapsed_blocks_ = display->collapsed_blocks_;
  need_update_ = display->need_update_;
  offset_ = display->offset_;
  
  // geometry
  setWidth( display->width() );
  
  // re-initialize connections
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
  connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );

}

//__________________________________________
void BlockDelimiterDisplay::setActionVisibility( const bool& state )
{
  Debug::Throw( "BlockDelimiterDisplay::setActionVisibility.\n" );
  
  collapseCurrentAction().setVisible( state );
  expandCurrentAction().setVisible( state );
  collapseAction().setVisible( state );
  expandAllAction().setVisible( state );  
}

//__________________________________________
void BlockDelimiterDisplay::updateCurrentBlockActionState( void )
{
  
  // update segments if needed
  _updateSegments(); 
  
  // by default next paintEvent will require segment update
  int cursor( _editor().textCursor().position() );

  BlockDelimiterSegment::List::iterator expanded_iter = std::find_if( segments_.begin(), segments_.end(), BlockDelimiterSegment::ContainsFTor( cursor, false ) );
  collapseCurrentAction().setEnabled( expanded_iter != segments_.end() );

  BlockDelimiterSegment::List::iterator collapsed_iter = std::find_if( segments_.begin(), segments_.end(), BlockDelimiterSegment::ContainsFTor( cursor, true ) );
  expandCurrentAction().setEnabled( collapsed_iter != segments_.end() );
  
}

//__________________________________________
void BlockDelimiterDisplay::paint( QPainter& painter )
{  
        
  // check delimiters
  if( delimiters_.empty() ) return;
  
  // store colors
  foreground_ = painter.pen().color();
  background_ = painter.brush().color();
  
  /* update segments if needed */
  _updateSegments();
  
  // calculate dimensions
  int y_offset = _editor().verticalScrollBar()->value() - _editor().frameWidth();
  int height( _editor().height() - 2*_editor().frameWidth() );
  if( _editor().horizontalScrollBar()->isVisible() ) { height -= _editor().horizontalScrollBar()->height() + 2; }
  
  // get begin and end cursor positions
  int first_index = _editor().cursorForPosition( QPoint( 0, 0 ) ).position();
  int last_index = _editor().cursorForPosition( QPoint( 0,  height ) ).position() + 1;

  // add horizontal offset
  painter.translate( _offset(), 0 );    
  height += y_offset;    
  
  // retrieve matching segments
  QTextDocument &document( *_editor().document() );
  QTextBlock block( document.begin() );
  unsigned int id( 0 );
  
  // optimize drawing by not drawing overlapping segments
  BlockDelimiterSegment::List::reverse_iterator previous( segments_.rend() );
  for( BlockDelimiterSegment::List::reverse_iterator iter = segments_.rbegin(); iter != segments_.rend(); iter++ )
  {

    // skip segment if outside of visible limits
    if( iter->begin().cursor() > last_index+1 || iter->end().cursor() < first_index ) continue;
    
    // try update segments
    if( iter->begin().cursor() >= first_index && iter->begin().cursor() <= last_index )
    { _updateMarker( block, id, iter->begin(), BEGIN ); }
    
    // try update segments
    if( iter->end().cursor() >= first_index && iter->end().cursor() <= last_index )
    {  _updateMarker( block, id, iter->end(), END ); }
        
    // skip this segment if included in previous
    if( previous != segments_.rend() && !( iter->begin() < previous->begin() || previous->end() < iter->end() ) ) continue;
    else previous = iter;

    // draw
    int begin( iter->begin().isValid() ? iter->begin().position()+top_ : 0 );
    int end( ( (!iter->empty()) && iter->end().isValid() && iter->end().position() < height ) ? iter->end().position():height );
    painter.drawLine( half_width_, begin, half_width_, end ); 
    
  }
      
  // end tick
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {
    
    if( iter->end().isValid() && iter->end().cursor() < last_index && iter->end().cursor() >= first_index && !( iter->flag( BlockDelimiterSegment::BEGIN_ONLY ) || iter->empty() ) )
    { painter.drawLine( half_width_, iter->end().position(), width_ - 3, iter->end().position() ); }
    
  }
  
  // draw begin ticks
  // first draw empty square
  painter.save();
  painter.setPen( Qt::NoPen );
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {
    
    // check validity
    if( iter->begin().isValid() && iter->begin().cursor() < last_index && iter->begin().cursor() >= first_index )
    {
      iter->setActiveRect( QRect( rect_top_left_, iter->begin().position() + rect_top_left_, rect_width_, rect_width_ ) );
      painter.drawRect( iter->activeRect() );
    }
    
  }
  painter.restore();
  
  // draw delimiters
  painter.save();
  if( custom_symbols_ )
  {
    // local style for custom painting
    // should define this only once -per paint event-,
    // before entering the loop over delimiters
    painter.setBrush( foreground_ );
    QPen pen( foreground_ );
    pen.setWidth( 1.5 );
    painter.setPen( pen );
    painter.setRenderHints( QPainter::Antialiasing );
  } else painter.setBrush( Qt::NoBrush );
  
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {    
    if( iter->begin().isValid() && iter->begin().cursor() < last_index && iter->begin().cursor() >= first_index )
    { _drawDelimiter( painter, iter->activeRect(), iter->flag( BlockDelimiterSegment::COLLAPSED ) ); } 
  }
  painter.restore();
     
}

//________________________________________________________
void BlockDelimiterDisplay::mousePressEvent( QMouseEvent* event )
{

  Debug::Throw( "BlockDelimiterDisplay::mousePressEvent.\n" );
    
  // check button
  if( !( event->button() == Qt::LeftButton ) ) return;

  // find segment matching event position
  BlockDelimiterSegment::List::const_iterator iter = find_if( 
    segments_.begin(), segments_.end(), 
    BlockDelimiterSegment::ActiveFTor( event->pos()+QPoint( -_offset(), _editor().verticalScrollBar()->value() ) ) );
  if( iter == segments_.end() ) return;
  
  /* 
  clear box selection
  because it gets corrupted by the collapsed/expand process 
  */
  _editor().clearBoxSelection();
  
  // retrieve matching segments
  HighlightBlockData* data(0);
  TextBlockPair blocks( _findBlocks( *iter, data ) );
  
  // check if block is collapsed
  QTextBlockFormat block_format( blocks.first.blockFormat() );
  if( block_format.boolProperty( TextBlock::Collapsed ) ) 
  {
    
    Debug::Throw( "BlockDelimiterDisplay::mousePressEvent - collapsed block found.\n" );
    bool cursor_visible( _editor().isCursorVisible() );
    _expand( blocks.first, data );
    if( cursor_visible ) _editor().ensureCursorVisible();
    
  } else {
    
    Debug::Throw( "BlockDelimiterDisplay::mousePressEvent - expanded block found.\n" );
    _collapse( blocks.first, blocks.second, data );
    
  }
  
  // force segment update at next update
  need_update_ = true;

}

//________________________________________________________
void BlockDelimiterDisplay::setWidth( const int& width )
{
  
  Debug::Throw( "BlockDelimiterDisplay::setWidth.\n" );
  
  // set dimensions needed to redraw marker and lines
  // this is done to minimize the amount of maths in the paintEvent method
  width_ = width;
  if( !( width_ %2 ) ) { width_++; }
  
  rect_width_ = width_ - 4;
  half_width_ = int(0.5*width_);
  top_ = 2;
  rect_top_left_ = 2;
          
}

//________________________________________________________
void BlockDelimiterDisplay::_contentsChanged( void )
{
  
  // if text is wrapped, line number data needs update at next update
  /* note: this could be further optimized if one retrieve the position at which the contents changed occured */
  if( _editor().lineWrapMode() != QTextEdit::NoWrap )
  { 
    need_update_ = true; 
    _synchronizeBlockData();
  }
  
}

//________________________________________________________
void BlockDelimiterDisplay::_blockCountChanged( void )
{
  
  // nothing to be done if wrap mode is not NoWrap, because
  // it is handled in the _contentsChanged slot.
  if( _editor().lineWrapMode() == QTextEdit::NoWrap )
  { 
    need_update_ = true; 
    _synchronizeBlockData();
  }
  
}

//________________________________________________________
void BlockDelimiterDisplay::_collapseCurrentBlock( void )
{ 
  Debug::Throw( "BlockDelimiterDisplay::_collapseCurrentBlock.\n" );

  /* update segments if needed */
  _updateSegments(); 
  _updateSegmentMarkers();
  
  // get cursor position
  int cursor( _editor().textCursor().position() );

  // find matching segment
  BlockDelimiterSegment::List::iterator iter = std::find_if( segments_.begin(), segments_.end(), BlockDelimiterSegment::ContainsFTor( cursor, false ) );
  if( iter == segments_.end() ) return;

  // clear box selection
  _editor().clearBoxSelection();

  // find matching blocks
  HighlightBlockData *data(0);
  TextBlockPair blocks( _findBlocks( *iter, data ) );
  
  // collapse
  _collapse( blocks.first, blocks.second, data );
  return;
}
  
//________________________________________________________
void BlockDelimiterDisplay::_expandCurrentBlock( void )
{   
  
  Debug::Throw( "BlockDelimiterDisplay::_expandCurrentBlock.\n" );

  /* update segments if needed */
  _updateSegments(); 
  _updateSegmentMarkers();
  
  // get cursor position
  int cursor( _editor().textCursor().position() );

  // find matching segment
  BlockDelimiterSegment::List::iterator iter = std::find_if( segments_.begin(), segments_.end(), BlockDelimiterSegment::ContainsFTor( cursor, true ) );
  if( iter == segments_.end() ) return;
  
  // find matching blocks
  HighlightBlockData *data(0);
  TextBlockPair blocks( _findBlocks( *iter, data ) );
  
  // collapse
  bool cursor_visible( _editor().isCursorVisible() );
  _editor().clearBoxSelection();
  _expand( blocks.first, data );
  if( cursor_visible ) _editor().ensureCursorVisible();
  return;
}

//________________________________________________________
void BlockDelimiterDisplay::_collapseTopLevelBlocks( void )
{ 
  
  Debug::Throw( "BlockDelimiterDisplay::_collapseTopLevelBlocks.\n" );
  
  /* update segments if needed */
  _updateSegments(); 
  _updateSegmentMarkers();

  // list of QTextCursor needed to remove blocks
  typedef std::vector<QTextCursor> CursorList;
  CursorList cursors;

  // get first block
  QTextBlock block( _editor().document()->begin() );
  unsigned int id(0);
  
  // create Text cursor
  QTextCursor cursor( block );
  cursor.beginEditBlock();

  // loop over segments in reverse order
  BlockDelimiterSegment::List::reverse_iterator previous(segments_.rend() );
  for( BlockDelimiterSegment::List::reverse_iterator iter = segments_.rbegin(); iter != segments_.rend(); iter++ )
  {
    
    // skip this segment if included in previous
    if( previous != segments_.rend() && !( iter->begin() < previous->begin() || previous->end() < iter->end() ) ) 
    { continue; }
  
    // update "Previous" segment 
    previous = iter;

    // get matching blocks
    HighlightBlockData *data(0);
    TextBlockPair blocks( _findBlocks( block, id, *iter, data ) );
    
    // do nothing if block is already collapsed
    cursor.setPosition( blocks.first.position(), QTextCursor::MoveAnchor );
    QTextBlockFormat block_format( cursor.blockFormat() );
    if( block_format.boolProperty( TextBlock::Collapsed ) ) 
    { continue; }
        
    // update block format
    block_format.setProperty( TextBlock::Collapsed, true );
    QVariant variant;
    variant.setValue( _collapsedData( blocks.first, blocks.second ) );
    block_format.setProperty( TextBlock::CollapsedData, variant );
    cursor.setBlockFormat( block_format );  
    
    // mark contents dirty to force update of current block
    data->setFlag( TextBlock::MODIFIED, true );
    data->setFlag( TextBlock::COLLAPSED, true );
    _editor().document()->markContentsDirty(blocks.first.position(), blocks.first.length()-1);

    cursor.setPosition( blocks.first.position() + blocks.first.length(), QTextCursor::MoveAnchor );
    if( blocks.second.isValid() ) { 
    
      if( blocks.second.next().isValid() ) cursor.setPosition( blocks.second.position() + blocks.second.length(), QTextCursor::KeepAnchor ); 
      else cursor.setPosition( blocks.second.position() + blocks.second.length() - 1, QTextCursor::KeepAnchor ); 
      
    } else { 
      cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor );
    }
    
    // store cursor in list for removal of the edited text at the end of the loop
    cursors.push_back( cursor );
        
  }
  
  // now remove all text stored in cursor list
  for( CursorList::const_iterator iter = cursors.begin(); iter != cursors.end(); iter++ )
  {
    cursor.setPosition( iter->anchor() );
    cursor.setPosition( iter->position(), QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
  }  
  
  cursor.endEditBlock();
  
  return; 

}


//________________________________________________________
void BlockDelimiterDisplay::_expandAllBlocks( void )
{ 
  
  Debug::Throw( "BlockDelimiterDisplay::_expandAllBlocks.\n" );
  
  // clear box selection
  _editor().clearBoxSelection();

  /* update segments if needed */
  _updateSegments(); 
  _updateSegmentMarkers();

  bool cursor_visible( _editor().isCursorVisible() );
  QTextDocument &document( *_editor().document() );
  QTextCursor cursor( document.begin() );
  cursor.beginEditBlock();
  
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next() ) 
  {
    // retrieve data and check if collapsed
    if( block.blockFormat().boolProperty( TextBlock::Collapsed ) )
    { 
      HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
      _expand( block, data, true ); 
    }
      
  }
  
  cursor.endEditBlock();
  
  // set cursor position
  if( cursor_visible ) _editor().ensureCursorVisible();
  
  return;
  
}

//__________________________________________________________
void BlockDelimiterDisplay::_needUpdate( void )
{ need_update_ = true; }
  
//________________________________________________________
void BlockDelimiterDisplay::_installActions( void )
{
  
  Debug::Throw( "BlockDelimiterDisplay::_installActions.\n" );

  collapse_current_action_ = new QAction( "&Collapse Current Block", this );
  collapse_current_action_->setToolTip( "Collapse current collapsed block" );
  collapse_current_action_->setShortcut( Qt::CTRL + Qt::Key_Minus );
  connect( collapse_current_action_, SIGNAL( triggered() ), SLOT( _collapseCurrentBlock() ) );
  collapse_current_action_->setEnabled( false );

  expand_current_action_ = new QAction( "&Expand Current Block", this );
  expand_current_action_->setToolTip( "Expand current collapsed block" );
  expand_current_action_->setShortcut( Qt::CTRL + Qt::Key_Plus );
  connect( expand_current_action_, SIGNAL( triggered() ), SLOT( _expandCurrentBlock() ) );
  expand_current_action_->setEnabled( false );
    
  collapse_action_ = new QAction( "&Collapse Top-Level Blocks", this );
  collapse_action_->setToolTip( "Collapse all top level blocks" );
  collapse_action_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_Minus );
  connect( collapse_action_, SIGNAL( triggered() ), SLOT( _collapseTopLevelBlocks() ) );
  collapse_action_->setEnabled( true );
  
  expand_all_action_ = new QAction( "&Expand All Blocks", this );
  expand_all_action_->setToolTip( "Expand all collapsed blocks" );
  expand_all_action_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_Plus );
  connect( expand_all_action_, SIGNAL( triggered() ), SLOT( _expandAllBlocks() ) );
  expand_all_action_->setEnabled( false );
   
}

//________________________________________________________
void BlockDelimiterDisplay::_synchronizeBlockData( void ) const
{
  
  QTextDocument &document( *_editor().document() );
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next() ) 
  {
           
    // retrieve data and check this block delimiter
    HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( !data ) continue;
    
    // store collapse state
    QTextBlockFormat block_format( block.blockFormat() );
    bool collapsed( block_format.boolProperty( TextBlock::Collapsed ) );
    if( data->hasFlag( TextBlock::COLLAPSED ) != collapsed )
    {
      data->setFlag( TextBlock::COLLAPSED, collapsed );
      data->setFlag( TextBlock::MODIFIED, true );
      document.markContentsDirty(block.position(), block.length()-1);
    }
    
  }

}

//________________________________________________________
void BlockDelimiterDisplay::_updateSegments( void )
{
  
  if( !need_update_ ) return;
  need_update_ = false;

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

      // retrieve data and check this block delimiter
      HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
      if( !data ) continue;

      // store collapse state
      QTextBlockFormat block_format( block.blockFormat() );
      bool collapsed( block_format.boolProperty( TextBlock::Collapsed ) );

      // get delimiter data
      TextBlock::Delimiter delimiter( data->delimiters().get( iter->id() ) );
      if( collapsed ) 
      { 
        assert( block_format.hasProperty( TextBlock::CollapsedData ) );
        CollapsedBlockData block_collapsed_data( block_format.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );
        delimiter += block_collapsed_data.delimiters().get( iter->id() ); 
      }
      
      // check if something is to be done
      if( !( collapsed || delimiter.begin() || delimiter.end() ) ) continue;
         
      // get block limits
      BlockMarker block_begin( block_count, block.position() );
      BlockMarker block_end( block_count, block.position()+block.length() - 1 );      
            
      // store "Ignore" state
      bool ignored = data->ignoreBlock();
      if( delimiter.end() )
      {
        
        if( !(start_points.empty() ) && ignored == start_points.back().flag( BlockDelimiterSegment::IGNORED ) ) 
        { 
          // if block is both begin and end, only the begin flag is to be drawn.
          if( delimiter.begin() ) start_points.back().setFlag( BlockDelimiterSegment::BEGIN_ONLY, true );
          
          // store new segment
          segments_.push_back( start_points.back().setEnd( block_end ) ); 
        }
        
        // pop
        for( int i = 0; i < delimiter.end() && !start_points.empty() && ignored == start_points.back().flag( BlockDelimiterSegment::IGNORED ); i++ )
        { start_points.pop_back(); }
          
      }
      
      // add segment
      if( collapsed || delimiter.begin() )
      {
        
        // prepare segment flags
        unsigned int flags( BlockDelimiterSegment::NONE );
        if( ignored ) flags |= BlockDelimiterSegment::IGNORED;
        if( collapsed ) flags |= BlockDelimiterSegment::COLLAPSED;
        
        // if block is collapsed, skip one start point (which is self contained)
        //for( int i = (collapsed ? 1:0); i < delimiter.begin(); i++ )
        for( int i = 0; i < delimiter.begin(); i++ )
        { start_points.push_back( BlockDelimiterSegment( block_begin, block_end, flags ) ); }
    
        if( collapsed ) { 
           
          // store number of collapsed blocks for the current one
          if( first )
          {
            collapsed_blocks_.insert( make_pair( block_count, collapsed_block_count ) );
            
            // assert( block_format.hasProperty( TextBlock::CollapsedData ) );
            collapsed_block_count += block_format.property( TextBlock::CollapsedData ).value<CollapsedBlockData>().blockCount() - 1;
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
    // this is done only for the first block delimiter pair
    if( first ) 
    {
      collapsed_blocks_.insert( make_pair( block_count, collapsed_block_count ) );
      first = false;
    }

  }
    
  // sort segments so that top level comes last
  std::sort( segments_.begin(), segments_.end(), BlockDelimiterSegment::SortFTor() );
        
  // update expand all action
  expandAllAction().setEnabled( has_collapsed_blocks );
  collapseAction().setEnabled( has_expanded_blocks );
  
}


//________________________________________________________
void BlockDelimiterDisplay::_updateSegmentMarkers( void )
{

  QTextBlock block( _editor().document()->begin() );
  unsigned int id = 0;
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  { 
    _updateMarker( block, id, iter->begin(), BEGIN ); 
    _updateMarker( block, id, iter->end(), END );
  }

}


//________________________________________________________
void BlockDelimiterDisplay::_updateMarker( QTextBlock& block, unsigned int& id, BlockMarker& marker, const BlockMarkerType& flag ) const
{

  // find block matching marker id
  if( marker.id() < id ) { for( ; marker.id() < id && block.isValid(); block = block.previous(), id-- ) {} }
  else if( marker.id() > id ) { for( ; marker.id() > id && block.isValid(); block = block.next(), id++ ) {} }
  assert( block.isValid() );
  
  QRectF rect( _editor().document()->documentLayout()->blockBoundingRect( block ) );
  if( flag == BEGIN ) { marker.setPosition( (int) block.layout()->position().y() ); }
  else { marker.setPosition( (int) (block.layout()->position().y() + rect.height()) ); }
       
  return;
  
}

//_____________________________________________________________________________________
BlockDelimiterDisplay::TextBlockPair BlockDelimiterDisplay::_findBlocks( 
  const BlockDelimiterSegment& segment, 
  HighlightBlockData*& data ) const
{
  QTextBlock block( _editor().document()->begin() );
  unsigned int id( 0 );
  return _findBlocks( block, id, segment, data );
}

//_____________________________________________________________________________________
BlockDelimiterDisplay::TextBlockPair BlockDelimiterDisplay::_findBlocks( 
  QTextBlock& block,
  unsigned int& id,
  const BlockDelimiterSegment& segment, 
  HighlightBlockData*& data ) const
{

  Debug::Throw( "BlockDelimiterDisplay::_findBlocks.\n" );
  TextBlockPair out;
  
  // look for first block
  assert( id <= segment.begin().id() );
  for( ; block.isValid() && id < segment.begin().id(); block = block.next(), id++ ) 
  {}
  
  assert( block.isValid() );
    
  // get data and check
  data = dynamic_cast<HighlightBlockData*>( block.userData() );
  assert( data );

  // store
  out.first = block;

  // finish if block is collapsed
  if( block.blockFormat().boolProperty( TextBlock::Collapsed ) ) 
  { return out; }
  
  // look for second block
  for( ; block.isValid() && id < segment.end().id(); block = block.next(), id++ )
  {}
  
  // check if second block is also of "Begin" type
  if( block != out.first )
  {
    HighlightBlockData *second_data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( second_data )
    {
      
      const TextBlock::Delimiter::List& delimiters( second_data->delimiters() );
      for( TextBlock::Delimiter::List::const_iterator iter = delimiters.begin(); iter != delimiters.end(); iter++ )
      {
        if( !iter->begin() ) continue;
        block = block.previous();
        id--;
        break;
      }
      
    }
  }
  
  // store and return
  out.second = block;  
  return out;
  
}

//________________________________________________________________________________________
void BlockDelimiterDisplay::_expand( const QTextBlock& block, HighlightBlockData* data, const bool& recursive ) const
{ 
      
  // retrieve block format
  QTextBlockFormat block_format( block.blockFormat() );
  
  // retrieve collapsed block data
  assert( block_format.hasProperty( TextBlock::CollapsedData ) );
  CollapsedBlockData collapsed_data( block_format.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );
  
  // mark contents dirty to force update of current block
  data->setFlag( TextBlock::MODIFIED, true );
  data->setFlag( TextBlock::COLLAPSED, false );
  
  _editor().document()->markContentsDirty(block.position(), block.length()-1);
  
  // create cursor
  QTextCursor cursor( block );      
  cursor.beginEditBlock();
  cursor.setPosition( block.position() + block.length() - 1, QTextCursor::MoveAnchor );      

  // update collapsed flag associated to data
  block_format.setProperty( TextBlock::Collapsed, false );
  cursor.setBlockFormat( block_format );
  
  for( CollapsedBlockData::List::const_iterator iter = collapsed_data.children().begin(); iter != collapsed_data.children().end(); iter++ )
  {
    
    cursor.insertBlock();
    cursor.insertText( iter->text() );
        
    // update text block format
    QTextBlockFormat block_format( cursor.blockFormat() );
    block_format.setProperty( TextBlock::Collapsed, iter->collapsed() );

    if( iter->collapsed() ) { 
      QVariant variant;
      variant.setValue( *iter );
      block_format.setProperty( TextBlock::CollapsedData, variant );
    }

    cursor.setBlockFormat( block_format );
        
    // also expands block if collapsed and recursive is set to true
    if( iter->collapsed() && recursive ) 
    {
      HighlightBlockData *current_data =  new HighlightBlockData();
      cursor.block().setUserData( current_data );
      _expand( cursor.block(), current_data, true );
    }
    
  }
  
  cursor.endEditBlock();
  
}

//________________________________________________________________________________________
void BlockDelimiterDisplay::_collapse( const QTextBlock& first_block, const QTextBlock& second_block, HighlightBlockData* data ) const
{
   
  Debug::Throw( "BlockDelimiterDisplay::_collapse.\n" );
  
  // create cursor and move at end of block
  QTextCursor cursor( first_block );
   
  // update block format
  QTextBlockFormat block_format( cursor.blockFormat() );
  block_format.setProperty( TextBlock::Collapsed, true );
  
  QVariant variant;
  variant.setValue( _collapsedData( first_block, second_block ) );
  block_format.setProperty( TextBlock::CollapsedData, variant );
  
  // start edition
  cursor.beginEditBlock();
  cursor.setBlockFormat( block_format );  
  
  cursor.setPosition( first_block.position() + first_block.length(), QTextCursor::MoveAnchor );
  if( second_block.isValid() ) { 
    
    if( second_block.next().isValid() ) cursor.setPosition( second_block.position() + second_block.length(), QTextCursor::KeepAnchor ); 
    else cursor.setPosition( second_block.position() + second_block.length() - 1, QTextCursor::KeepAnchor ); 
    
  } else { cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor ); }
  
  cursor.removeSelectedText();
  cursor.endEditBlock();  
  
  // mark contents dirty to force update of current block
  data->setFlag( TextBlock::MODIFIED, true );
  data->setFlag( TextBlock::COLLAPSED, true );
   
  // mark contents dirty to force update
  _editor().document()->markContentsDirty(first_block.position(), first_block.length()-1);

}

//________________________________________________________________________________________
CollapsedBlockData BlockDelimiterDisplay::_collapsedData( const QTextBlock& first_block, const QTextBlock& second_block ) const
{

  CollapsedBlockData collapsed_data;
  TextBlock::Delimiter::List collapsed_delimiters;
  if( second_block != first_block )
  {
    for( QTextBlock current = first_block.next(); current.isValid(); current = current.next() )
    {

      // create collapse block data
      CollapsedBlockData current_collapsed_data( current );

      // update collapsed delimiters
      // retrieve the TextBlockData associated to this block
      HighlightBlockData* current_data( dynamic_cast<HighlightBlockData*>( current.userData() ) );
      if( current_data ) { collapsed_delimiters += current_data->delimiters(); }
      
      // also append possible collapsed delimiters
      collapsed_delimiters += current_collapsed_data.delimiters();

      // append collapsed data
      collapsed_data.children().push_back( current_collapsed_data );

      if( current == second_block ) break;
      
    }
    
  }
  
  // store collapsed delimiters in collapsed data
  collapsed_data.delimiters() = collapsed_delimiters;
  return collapsed_data;
}

//_________________________________________________________________________
void BlockDelimiterDisplay::_drawDelimiter( QPainter& painter, const QRect& rect, const bool& collapsed ) const
{
  
  if( custom_symbols_ )
  {
    QRectF local( rect );
    local.adjust( 1.5, 1.5, -1.5, -1.5 );
    if( collapsed )
    {
      double offset( local.height()/6 );
      const QPointF points[3] = {
         QPointF(local.topLeft()) + QPointF( offset, 0 ),
         QPointF(local.bottomLeft()) + QPointF( offset, 0 ),
         local.topLeft() + QPointF( local.width()*2/3, local.height()/2 ) + QPointF( offset, 0 )
      };

      painter.drawConvexPolygon(points, 3);
      
    } else {

      double offset( local.width()/6 );
      const QPointF points[3] = {
        QPointF(local.topLeft()) + QPointF( 0, offset ),
        QPointF(local.topRight()) + QPointF( 0, offset ),
        local.topLeft() + QPointF( local.width()/2, local.height()*2/3 ) + QPointF( 0, offset )
      };

      painter.drawConvexPolygon(points, 3);
    }
    
  } else {
    
    QStyleOption option;
    option.initFrom( &_editor() );
    option.palette.setColor( QPalette::Text, foreground_ );
    option.rect = rect;
    option.state |= QStyle::State_Children;
    if( !collapsed ) { option.state |= QStyle::State_Open; }
      
    _editor().style()->drawPrimitive( QStyle::PE_IndicatorBranch, &option, &painter );
    
  } 

  return;
}
