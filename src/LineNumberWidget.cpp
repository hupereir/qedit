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
  \file LineNumberWidget.cpp
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
#include "LineNumberWidget.h"
#include "HighlightBlockData.h"
#include "XmlOptions.h"

//____________________________________________________________________________
LineNumberWidget::LineNumberWidget(TextEditor* editor, QWidget* parent): 
  QWidget( parent),
  Counter( "LineNumberWidget" ),
  editor_( editor ),
  need_update_( true ),
  need_current_block_update_( false )
{
  
  Debug::Throw( "LineNumberWidget::LineNumberWidget.\n" );

  setAutoFillBackground( true );  

  // change background color
  // the same brush is used as for scrollbars
  QPalette palette( LineNumberWidget::palette() );
  palette.setBrush( QPalette::Window, QBrush( palette.color( QPalette::Base ), Qt::Dense4Pattern ) );
  setPalette( palette );  
 
  connect( _editor().verticalScrollBar(), SIGNAL( valueChanged( int ) ), SLOT( update() ) );
  connect( &_editor().wrapModeAction(), SIGNAL( toggled( bool ) ), SLOT( _needUpdate() ) );
  connect( &_editor().wrapModeAction(), SIGNAL( toggled( bool ) ), SLOT( update() ) );
  
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
  connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );
  
  connect( &_editor().blockHighlight(), SIGNAL( highlightChanged() ), SLOT( _currentBlockChanged() ) );
  connect( &_editor(), SIGNAL( textChanged() ), SLOT( update() ) );  
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );

  // update configuration
  _updateConfiguration();
  
  // width
  setFixedWidth( fontMetrics().width( "000" ) + 14 );

}

//__________________________________________
LineNumberWidget::~LineNumberWidget()
{ Debug::Throw( "LineNumberWidget::~LineNumberWidget.\n" );}

//__________________________________________
void LineNumberWidget::synchronize( LineNumberWidget* widget )
{
 
  Debug::Throw( "LineNumberWidget::synchronize.\n" );

  // copy members
  line_number_data_ = widget->line_number_data_;
  need_update_ = widget->need_update_;
  
  // re-initialize connections
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
  connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );
  
}

//__________________________________________
void LineNumberWidget::paintEvent( QPaintEvent* )
{  

  // update line number data if needed
  if( need_update_ ) 
  { _updateLineNumberData(); }
  need_update_ = false;
  
  // font metric and offset
  const QFontMetrics metric( fontMetrics() );
  
  // calculate dimensions
  int y_offset = _editor().verticalScrollBar()->value();
  int height( QWidget::height() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) 
  { height -= _editor().horizontalScrollBar()->height(); }

  // brush/pen  
  QPainter painter( this );
  painter.translate( 0, -y_offset );
  painter.setClipRect( 0, 0, width(), height );
    
  // current block highlight
  if( need_current_block_update_ && highlight_color_.isValid() )
  {
    has_current_block_ = _updateCurrentBlockData();
    if( has_current_block_ ) {
      
      // draw background
      painter.save();
      painter.setPen( Qt::NoPen );
      painter.setBrush( highlight_color_ );
      painter.drawRect( 0, current_block_data_.position(), width(), metric.lineSpacing() );
      painter.restore();
      
    } 
        
  }

  // maximum text length
  int max_length=0;  

  // get begin and end cursor positions
  int first_index = _editor().cursorForPosition( QPoint( 0, 0 ) ).position();
  int last_index = _editor().cursorForPosition( QPoint( 0, QWidget::height() ) ).position();
  
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
      0, iter->position(), width()-8,
      metric.lineSpacing(),
      Qt::AlignRight | Qt::AlignTop, 
      numtext );
    
    //max_length = std::max( max_length, metric.width(numtext)+metric.width("0")+10 );
    if( metric.width(numtext) + 14 > max_length )
    {
      max_length = std::max( max_length, metric.width(numtext) + 14 );
      painter.setClipRect( 0, 0, max_length, height );
    }
    
  }

  // resize
  if( max_length != width() && max_length > 0 ) 
  { setFixedWidth( max_length ); }
    
  painter.end();
  
}

//___________________________________________________________
void LineNumberWidget::mousePressEvent( QMouseEvent* event )
{ if( event->button() ==  Qt::LeftButton ) qApp->sendEvent( _editor().viewport(), event ); }

//___________________________________________________________
void LineNumberWidget::mouseReleaseEvent( QMouseEvent* event )
{ if( event->button() ==  Qt::LeftButton ) qApp->sendEvent( _editor().viewport(), event ); }

//___________________________________________________________
void LineNumberWidget::wheelEvent( QWheelEvent* event )
{ qApp->sendEvent( _editor().viewport(), event ); }

//________________________________________________________
void LineNumberWidget::_updateConfiguration( void )
{
  
  Debug::Throw( "LineNumberWidget::_updateConfiguration.\n" );

  // font
  QFont font;
  font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ).c_str() );
  setFont( font );

  // paragraph highlighting
  highlight_color_ = QColor( XmlOptions::get().raw( "HIGHLIGHT_COLOR" ).c_str() );
  
}

//________________________________________________________
void LineNumberWidget::_contentsChanged( void )
{
  
  // if text is wrapped, line number data needs update at next update
  /* note: this could be further optimized if one retrieve the position at which the contents changed occured */
  if( _editor().lineWrapMode() != QTextEdit::NoWrap )
  { need_update_ = true; }
}

//________________________________________________________
void LineNumberWidget::_blockCountChanged( void )
{
    
  // nothing to be done if wrap mode is not NoWrap, because
  // it is handled in the _contentsChanged slot.
  if( _editor().lineWrapMode() == QTextEdit::NoWrap )
  { need_update_ = true; }
  
}

//________________________________________________________
void LineNumberWidget::_currentBlockChanged( void )
{
  need_current_block_update_ = true;
  update();
}

//________________________________________________________
void LineNumberWidget::_updateLineNumberData( void )
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
void LineNumberWidget::_updateLineNumberData( QTextBlock& block, unsigned int& id, LineNumberWidget::LineNumberData& data ) const
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
bool LineNumberWidget::_updateCurrentBlockData( void )
{

  // do nothing if not enabled
  if( !_editor().blockHighlightAction().isChecked() ) return false;
  
  // font metric
  const QFontMetrics metric( fontMetrics() );
  
  // get begin and end cursor positions
  int first_index = _editor().cursorForPosition( QPoint( 0, 0 ) ).position();
  int last_index = _editor().cursorForPosition( QPoint( 0,  QWidget::height() ) ).position();

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
