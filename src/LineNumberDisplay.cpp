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
  \file LineNumberDisplay.cpp
  \brief display line number of a text editor
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPainter>
#include <QScrollBar>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>

#include "BlockHighlight.h"
#include "TextEditor.h"
#include "Debug.h"
#include "LineNumberDisplay.h"
#include "HighlightBlockData.h"
#include "XmlOptions.h"

using namespace std;

//____________________________________________________________________________
LineNumberDisplay::LineNumberDisplay(TextEditor* editor): 
  QObject( editor ),
  Counter( "LineNumberDisplay" ),
  editor_( editor ),
  need_update_( true ),
  need_current_block_update_( false ),
  width_( 0 )
{
  
  Debug::Throw( "LineNumberDisplay::LineNumberDisplay.\n" );
 
  connect( &_editor().wrapModeAction(), SIGNAL( toggled( bool ) ), SLOT( _needUpdate() ) );
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
  connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );
  connect( &_editor().blockHighlight(), SIGNAL( highlightChanged() ), SLOT( _currentBlockChanged() ) );
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );

  // update configuration
  _updateConfiguration();
  
}

//__________________________________________
LineNumberDisplay::~LineNumberDisplay()
{ Debug::Throw( "LineNumberDisplay::~LineNumberDisplay.\n" );}

//__________________________________________
void LineNumberDisplay::synchronize( LineNumberDisplay* display )
{
 
  Debug::Throw( "LineNumberDisplay::synchronize.\n" );

  // copy members
  line_number_data_ = display->line_number_data_;
  need_update_ = display->need_update_;
  width_ = display->width_;
  
  // re-initialize connections
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
  connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );
  
}

//__________________________________________
bool LineNumberDisplay::updateWidth( const int& count )
{
  Debug::Throw( "LineNumberDisplay::updateWidth.\n" );
  int new_width( _editor().fontMetrics().width( QString::number( count ) ) + 14 );
  if( width() == new_width ) return false;
  width_ = new_width;
  return true;  
}

//__________________________________________
void LineNumberDisplay::paint( QPainter& painter )
{  

  // update line number data if needed
  if( need_update_ ) 
  { _updateLineNumberData(); }
  need_update_ = false;
  
  // font metric and offset
  const QFontMetrics metric( _editor().fontMetrics() );
  
  // calculate dimensions
  int y_offset = _editor().verticalScrollBar()->value() - _editor().frameWidth();
  int height( _editor().height() - 2*_editor().frameWidth() );
  if( _editor().horizontalScrollBar()->isVisible() ) { height -= _editor().horizontalScrollBar()->height() + 2; }
  
  // translate
  height += y_offset;
  
  // current block highlight
  if( need_current_block_update_ && highlight_color_.isValid() )
  {
    has_current_block_ = _updateCurrentBlockData();
    if( has_current_block_ ) 
    {
      
      // draw background
      painter.save();
      painter.setPen( Qt::NoPen );
      painter.setBrush( highlight_color_ );
      painter.drawRect( 0, current_block_data_.position(), width_, metric.lineSpacing() );
      painter.restore();
      
    } 
        
  }

  // get begin and end cursor positions
  int first_index = _editor().cursorForPosition( QPoint( 0, 0 ) ).position();
  int last_index = _editor().cursorForPosition( QPoint( 0, _editor().height() ) ).position();
  
  // loop over data
  QTextBlock block( _editor().document()->begin() );
  unsigned int id( 0 );
  
  for( LineNumberData::List::iterator iter = line_number_data_.begin(); iter != line_number_data_.end(); iter++ )
  {
    
    // skip if block is not (yet) in window
    if( iter->cursor() < first_index ) continue;
    
    // stop if block is outside (below) window
    if( iter->cursor() > last_index ) break;
        
    // check validity
    if( !iter->isValid() ) _updateLineNumberData( block, id, *iter );
    
    // check position
    if( iter->isValid() && iter->position() > height ) continue;
    
    QString numtext( QString::number( iter->lineNumber() ) );
    painter.drawText(
      0, iter->position(), width_-8,
      metric.lineSpacing(),
      Qt::AlignRight | Qt::AlignTop, 
      numtext );
        
  }
  
}

//________________________________________________________
void LineNumberDisplay::_updateConfiguration( void )
{
  
  Debug::Throw( "LineNumberDisplay::_updateConfiguration.\n" );
    
  // colors
  {
    QColor color( XmlOptions::get().get<string>( "HIGHLIGHT_COLOR" ).c_str() );
    highlight_color_ = color.isValid() ? color:_editor().palette().color( QPalette::Highlight );
  }

}

//________________________________________________________
void LineNumberDisplay::_contentsChanged( void )
{
  
  // if text is wrapped, line number data needs update at next update
  /* note: this could be further optimized if one retrieve the position at which the contents changed occured */
  if( _editor().lineWrapMode() != QTextEdit::NoWrap )
  { need_update_ = true; }
}

//________________________________________________________
void LineNumberDisplay::_blockCountChanged( void )
{
    
  // nothing to be done if wrap mode is not NoWrap, because
  // it is handled in the _contentsChanged slot.
  if( _editor().lineWrapMode() == QTextEdit::NoWrap )
  { need_update_ = true; }
  
}

//________________________________________________________
void LineNumberDisplay::_currentBlockChanged( void )
{ need_current_block_update_ = true; }

//________________________________________________________
void LineNumberDisplay::_updateLineNumberData( void )
{
  
  line_number_data_.clear();
  
  // get document
  unsigned int id( 0 );
  unsigned int block_count( 1 );
  QTextDocument &document( *_editor().document() );
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next(), id++, block_count++ )
  {
    
    // insert new data
    line_number_data_.push_back( LineNumberData( id, block_count, block.position() ) );
    
    QTextBlockFormat block_format( block.blockFormat() );
    if( block_format.boolProperty( TextBlock::Collapsed ) && block_format.hasProperty( TextBlock::CollapsedData ) )
    { block_count += block_format.property( TextBlock::CollapsedData ).value<CollapsedBlockData>().blockCount() - 1; }
    
  }
  
  return;
  
}

//________________________________________________________
void LineNumberDisplay::_updateLineNumberData( QTextBlock& block, unsigned int& id, LineNumberDisplay::LineNumberData& data ) const
{
  assert( !data.isValid() );

  // find block matching data id
  if( data.id() < id ) { for( ; data.id() < id && block.isValid(); block = block.previous(), id-- ) {} }
  else if( data.id() > id ) { for( ; data.id() > id && block.isValid(); block = block.next(), id++ ) {} }
  assert( block.isValid() );
  
  QRectF rect( _editor().document()->documentLayout()->blockBoundingRect( block ) );
  data.setPosition( block.layout()->position().y() );

}

//________________________________________________________
bool LineNumberDisplay::_updateCurrentBlockData( void )
{

  // do nothing if not enabled
  if( !_editor().blockHighlightAction().isChecked() ) return false;
  
  // font metric
  const QFontMetrics metric( _editor().fontMetrics() );
  
  // get begin and end cursor positions
  int first_index = _editor().cursorForPosition( QPoint( 0, 0 ) ).position();
  int last_index = _editor().cursorForPosition( QPoint( 0, _editor().height() ) ).position();

  // get document
  unsigned int block_count( 1 );
  QTextDocument &document( *_editor().document() );
  QTextBlock block = document.begin();
  LineNumberData::List::iterator iter( line_number_data_.begin() );
  for( ; block.isValid() && iter != line_number_data_.end(); block = block.next(), block_count++, iter++ )
  {
    
    // skip if block is not (yet) in window
    if( iter->cursor() < first_index ) continue;
    
    // stop if block is outside (below) window
    if( iter->cursor() > last_index ) break;
    
    // block data
    TextBlockData* data( static_cast<TextBlockData*>( block.userData() ) );
    
    // update block count in case of collapsed block
    if( data && data->hasFlag( TextBlock::CURRENT_BLOCK ) )
    { 
      current_block_data_ = *iter;
      return true;
    }
    
  }
  
  return false;
}
