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
  \file BlockDelimiterWidget.h
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
void BlockDelimiterWidget::paintEvent( QPaintEvent*)
{  
    
  // check delimiters
  if( delimiters_.empty() ) return;
    
  /* update segments if needed */
  _updateSegments();
  // get begin and end cursor positions
  int first_index = _editor().cursorForPosition( QPoint( 0, 0 ) ).position();
  int last_index = _editor().cursorForPosition( QPoint( 0,  QWidget::height() + fontMetrics().lineSpacing() ) ).position();
  
  // calculate dimensions
  int y_offset = _editor().verticalScrollBar()->value();
  int height( QWidget::height() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) 
  { height -= _editor().horizontalScrollBar()->height(); }
  
  // create painter
  QPainter painter( this );
  painter.translate( 0, -y_offset );
  
  //painter.save();
  QPen pen;
  pen.setStyle( Qt::DotLine );
  painter.setPen( pen );
  
  // optimize drawing by not drawing overlapping segments
  BlockDelimiterSegment::List::reverse_iterator previous( segments_.rend() );
  for( BlockDelimiterSegment::List::reverse_iterator iter = segments_.rbegin(); iter != segments_.rend(); iter++ )
  {

    // skip segment if outside of visible limits
    if( iter->begin().cursor() > last_index || iter->end().cursor() < first_index ) continue;
    
    // try update segment
    if( iter->begin().cursor() >= first_index && iter->begin().cursor() <= last_index )
    { _updateSegmentMarkers( *iter, BEGIN ); }
    
    if( iter->end().cursor() >= first_index && iter->end().cursor() <= last_index )
    { _updateSegmentMarkers( *iter, END ); }
    
    // skip this segment if included in previous
    if( previous != segments_.rend() && !( iter->begin() < previous->begin() || previous->end() < iter->end() ) ) continue;
    else previous = iter;

    // draw
    int begin( iter->begin().isValid() ? iter->begin().position()+top_ : 0 );
    int end( ( (!iter->empty()) && iter->end().isValid() && iter->end().cursor() < last_index ) ? iter->end().position():height );
    painter.drawLine( half_width_, begin, half_width_, end ); 
    
  }
    
  // end tick
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {
    
    if( iter->end().isValid() && iter->end().cursor() < last_index && iter->end().cursor() >= first_index && !( iter->flag( BlockDelimiterSegment::BEGIN_ONLY ) || iter->empty() ) )
    { painter.drawLine( half_width_, iter->end().position(), width_, iter->end().position() ); }
    
  }

  //painter.restore();
  
  // draw begin ticks
  // first draw empty square
  painter.save();
  painter.setPen( Qt::NoPen );
  painter.setBrush( palette().color( QPalette::Base ) );
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
  
  // use the QStyle primitive elements for TreeViews
  QStyleOption option;
  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  {
    
    if( iter->begin().isValid() && iter->begin().cursor() < last_index && iter->begin().cursor() >= first_index )
    {

      option.initFrom( this );
      option.rect = iter->activeRect();
      option.state |= QStyle::State_Children;
      if( !iter->flag( BlockDelimiterSegment::COLLAPSED ) ) { option.state |= QStyle::State_Open; }
      
      style()->drawPrimitive( QStyle::PE_IndicatorBranch, &option, &painter );
    }
    
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
  TextBlockData* data(0);
  TextBlockPair blocks( _findBlocks( document.begin(), *iter, data ) );
  
  // check if block is collapsed
  QTextBlockFormat block_format( blocks.first.blockFormat() );
  if( block_format.boolProperty( TextBlock::Collapsed ) ) 
  {
    
    Debug::Throw( "BlockDelimiterWidget::mousePressEvent - collapsed block found.\n" );
    bool cursor_visible( _editor().isCursorVisible() );
    _editor().clearBoxSelection();
    _expand( blocks.first, data );
    if( cursor_visible ) _editor().ensureCursorVisible();
    
  } else {
    
    Debug::Throw( "BlockDelimiterWidget::mousePressEvent - expanded block found.\n" );
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
  { 
    need_update_ = true; 
    _synchronizeBlockData();
  }
  
}

//________________________________________________________
void BlockDelimiterWidget::_blockCountChanged( void )
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
void BlockDelimiterWidget::_collapseCurrentBlock( void )
{ 
  Debug::Throw( "BlockDelimiterWidget::_collapseCurrentBlock.\n" );

  /* update segments if needed */
  _updateSegments(); 
  _updateSegmentMarkers();
  
  // sort segments so that top level comes last
  std::sort( segments_.begin(), segments_.end(), BlockDelimiterSegment::SortFTor() );

  // get cursor position
  QTextDocument& document( *_editor().document() );
  int cursor( _editor().textCursor().position() );

  // find matching segment
  BlockDelimiterSegment::List::iterator iter = std::find_if( segments_.begin(), segments_.end(), BlockDelimiterSegment::ContainsFTor( cursor, false ) );
  if( iter == segments_.end() ) return;

  // clear box selection
  _editor().clearBoxSelection();

  // find matching blocks
  TextBlockData *data(0);
  TextBlockPair blocks( _findBlocks( document.begin(), *iter, data ) );
  
  // collapse
  _collapse( blocks.first, blocks.second, data );
  return;
}
  
//________________________________________________________
void BlockDelimiterWidget::_expandCurrentBlock( void )
{   
  
  Debug::Throw( "BlockDelimiterWidget::_expandCurrentBlock.\n" );

  /* update segments if needed */
  _updateSegments(); 
  _updateSegmentMarkers();
  
  // sort segments so that top level comes last
  std::sort( segments_.begin(), segments_.end(), BlockDelimiterSegment::SortFTor() );

  // get cursor position
  QTextDocument& document( *_editor().document() );
  int cursor( _editor().textCursor().position() );

  // find matching segment
  BlockDelimiterSegment::List::iterator iter = std::find_if( segments_.begin(), segments_.end(), BlockDelimiterSegment::ContainsFTor( cursor, true ) );
  if( iter == segments_.end() ) return;
  
  // find matching blocks
  TextBlockData *data(0);
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
  _updateSegments(); 
  _updateSegmentMarkers();

  // sort segments so that top level comes last
  std::sort( segments_.begin(), segments_.end(), BlockDelimiterSegment::SortFTor() );
  
  // list of QTextCursor needed to remove blocks
  typedef std::vector<QTextCursor> CursorList;
  CursorList cursors;

  // get first block
  QTextBlock block( _editor().document()->begin() );
  
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
  
    // update "previous" segment 
    previous = iter;

    // get matching blocks
    TextBlockData *data(0);
    TextBlockPair blocks( _findBlocks( block, *iter, data ) );

    // do nothing if block is already collapsed
    QTextBlockFormat block_format( blocks.first.blockFormat() );
    if( block_format.boolProperty( TextBlock::Collapsed ) ) 
    {
      
      block = blocks.first;
      continue;
      
    }
    
    // create collapsed block data to be stored in current block before collapsed
    CollapsedBlockData collapsed_data;
    if( blocks.second != blocks.first )
    {
      for( QTextBlock current = blocks.first.next(); current.isValid(); current = current.next() )
      {
        
        // append collapsed data
        collapsed_data.children().push_back( CollapsedBlockData( current ) );
        if( current == blocks.second ) break;
        
      }
    }
    
    // move cursor to first block
    cursor.setPosition( blocks.first.position(), QTextCursor::MoveAnchor );

    // update block format
    block_format.setProperty( TextBlock::Collapsed, true );
    QVariant variant;
    variant.setValue( collapsed_data );
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
    
    // increment
    block = blocks.second;
    
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
void BlockDelimiterWidget::_expandAllBlocks( void )
{ 
  
  Debug::Throw( "BlockDelimiterWidget::_expandAllBlocks.\n" );
  
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
    HighlightBlockData* data( static_cast<HighlightBlockData*>( block.userData() ) );
    if( block.blockFormat().boolProperty( TextBlock::Collapsed ) )
    { _expand( block, data, true ); }
      
  }
  
  cursor.endEditBlock();
  
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
void BlockDelimiterWidget::_synchronizeBlockData( void ) const
{
  
  QTextDocument &document( *_editor().document() );
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next() ) 
  {
           
    // retrieve data and check this block delimiter
    HighlightBlockData* data = (static_cast<HighlightBlockData*>( block.userData() ) );
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
void BlockDelimiterWidget::_updateSegments( void )
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
      HighlightBlockData* data = (static_cast<HighlightBlockData*>( block.userData() ) );
      if( !data ) continue;

      // get delimiter data
      TextBlock::Delimiter delimiter( data->delimiter( iter->id() ) );

      // store collapse state
      QTextBlockFormat block_format( block.blockFormat() );
      bool collapsed( block_format.boolProperty( TextBlock::Collapsed ) );
      
      // check if something is to be done
      if( !( collapsed || delimiter.begin() || delimiter.end() ) ) continue;
                  
      // get block limits
      // QRectF rect( _editor().document()->documentLayout()->blockBoundingRect( block ) );
      // BlockMarker block_begin( block.position(), block.layout()->position().y() );
      // BlockMarker block_end( block.position()+block.length() - 1, block.layout()->position().y() + rect.height() );      
  
      // get block limits
      BlockMarker block_begin( block.position() );
      BlockMarker block_end( block.position()+block.length() - 1 );      
            
      // store "ignore" state
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
        for( int i = (collapsed ? 1:0); i < delimiter.begin(); i++ )
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
    
  // update expand all action
  expandAllAction().setEnabled( has_collapsed_blocks );
  collapseAction().setEnabled( has_expanded_blocks );
  
}


//________________________________________________________
void BlockDelimiterWidget::_updateSegmentMarkers( void )
{

  for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); iter++ )
  { _updateSegmentMarkers( *iter, ALL ); }

}


//________________________________________________________
void BlockDelimiterWidget::_updateSegmentMarkers( BlockDelimiterSegment& segment, const unsigned int& flag ) const
{
  
  if( (flag & BEGIN) && !segment.begin().isValid() )
  {
    
    // get block matching begin position
    QTextBlock block( _editor().document()->findBlock( segment.begin().cursor() ) );
    assert( block.isValid() );
    
    QRectF rect( _editor().document()->documentLayout()->blockBoundingRect( block ) );
    segment.begin().setPosition( block.layout()->position().y() );
    
  }
  
  if( (flag & END) && !segment.end().isValid() )
  {
    
    // get block matching begin position
    QTextBlock block( _editor().document()->findBlock( segment.end().cursor() ) );
    assert( block.isValid() );
    
    QRectF rect( _editor().document()->documentLayout()->blockBoundingRect( block ) );
    segment.end().setPosition( block.layout()->position().y() + rect.height() );
       
  }
  
  return;
  
}

//_____________________________________________________________________________________
BlockDelimiterWidget::TextBlockPair BlockDelimiterWidget::_findBlocks( 
  QTextBlock block, 
  const BlockDelimiterSegment& segment, 
  TextBlockData*& data ) const
{

  Debug::Throw( "BlockDelimiterWidget::_findBlocks.\n" );
  TextBlockPair out;
  
  // look for first block
  for( ; block.isValid() && block.position() != segment.begin().cursor(); block = block.next() ) 
  {}
  
  assert( block.isValid() );
    
  // get data and check
  data = static_cast<TextBlockData*>( block.userData() );
  assert( data );

  // store
  out.first = block;

  // finish if block is collapsed
  if( block.blockFormat().boolProperty( TextBlock::Collapsed ) ) 
  {
    Debug::Throw( "BlockDelimiterWidget::_findBlocks - done.\n" );
    return out;
  }
  
  // look for second block
  for( ; block.isValid() && block.position()+block.length()-1 != segment.end().cursor(); block = block.next() )
  {}
  
  // check if second block is also of "begin" type
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
        break;
      }
      
    }
  }
  
  // store and return
  out.second = block;
  Debug::Throw( "BlockDelimiterWidget::_findBlocks - done.\n" );
  return out;
  
}

//________________________________________________________________________________________
void BlockDelimiterWidget::_expand( const QTextBlock& block, TextBlockData* data, const bool& recursive ) const
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
      TextBlockData *current_data =  new TextBlockData();
      cursor.block().setUserData( current_data );
      _expand( cursor.block(), current_data, true );
    }
    
  }
  
  cursor.endEditBlock();
  
}

//________________________________________________________________________________________
void BlockDelimiterWidget::_collapse( const QTextBlock& first_block, const QTextBlock& second_block, TextBlockData* data ) const
{
   
  Debug::Throw( "BlockDelimiterWidget::_collapse.\n" );
  
  // get block associated to cursor
  // see if cursor belongs to collapsible block
  QTextBlock cursor_block( _editor().textCursor().block() );  

  // create cursor and move at end of block
  QTextCursor cursor( first_block );
  
  // create collapsed block data to be stored in current block before collapsed
  CollapsedBlockData collapsed_data;
  if( second_block != first_block )
  {
    for( QTextBlock current = first_block.next(); current.isValid(); current = current.next() )
    {
      
      // append collapsed data
      collapsed_data.children().push_back( CollapsedBlockData( current ) );
      if( current == second_block ) break;
      
    }
  }
  
  // update block format
  QTextBlockFormat block_format( cursor.blockFormat() );
  block_format.setProperty( TextBlock::Collapsed, true );
  QVariant variant;
  variant.setValue( collapsed_data );
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
  _editor().document()->markContentsDirty(first_block.position(), first_block.length()-1);
  
}
