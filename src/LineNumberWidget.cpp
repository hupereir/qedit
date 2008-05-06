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

/*
Note: this code to be optimized by storing the mapping between line number and position in an array, and updating the later only 
when the QTextDocument::blockCountChanged signal is emmited. 
No: that won't work, notably when the text is wrapped. Need to update every time the text is modified.
One could use the QTextDocument::contentsChange( int position, int charsRemoved, int charsAdded ) signal, and only update
blocks that are _after_ position.
*/

//____________________________________________________________________________
LineNumberWidget::LineNumberWidget(TextEditor* editor, QWidget* parent): 
  QWidget( parent),
  Counter( "LineNumberWidget" ),
  editor_( editor ),
  need_update_( true )
{
  
  Debug::Throw( "LineNumberWidget::LineNumberWidget.\n" );
  setAutoFillBackground( true );
  
  connect( _editor().verticalScrollBar(), SIGNAL( valueChanged( int ) ), SLOT( update() ) );
  connect( &_editor().wrapModeAction(), SIGNAL( toggled( bool ) ), SLOT( _needUpdate() ) );
  connect( &_editor().wrapModeAction(), SIGNAL( toggled( bool ) ), SLOT( update() ) );
  connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
  connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );

  connect( &_editor(), SIGNAL( textChanged() ), SLOT( update() ) );  

  // this is needed to update current paragraph highlight
  // it has not been re-implemented yet
  // connect( &_editor().blockHighlight(), SIGNAL( highlightChanged() ), SLOT( update() ) );
  
  
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
  int y_offset = _editor().verticalScrollBar()->value();
  

  // brush/pen  
  QPainter painter( this );
  painter.translate( 0, -y_offset );

  // maximum text length
  int max_length=0;  
  int height( QWidget::height() - metric.lineSpacing() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) { height -= _editor().horizontalScrollBar()->height(); }

  
  // loop over data
  for( LineNumberData::List::const_iterator iter = line_number_data_.begin(); iter != line_number_data_.end(); iter++ )
  {
    
    // skip if block is not (yet) in window
    if( iter->y() + metric.lineSpacing() < y_offset ) continue;
    
    // stop if block is outside (below) window
    if( iter->y() > height ) break;
        
    QString numtext( QString::number( iter->lineNumber() ) );
    painter.drawText(
      0, iter->y(), width()-8,
      metric.lineSpacing(),
      Qt::AlignRight | Qt::AlignTop, 
      numtext );
    
    max_length = std::max( max_length, metric.width(numtext)+metric.width("0")+10 );
  }

  setFixedWidth( max_length );
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
void LineNumberWidget::_updateLineNumberData( void )
{
  
  line_number_data_.clear();
  
  // get document
  unsigned int block_count( 1 );
  QTextDocument &document( *_editor().document() );
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next(), block_count++ )
  {
    
    // insert new data
    line_number_data_.push_back( LineNumberData( block_count, document.documentLayout()->blockBoundingRect( block ).y() ) );
    
    // block data
    HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    
    // update block count in case of collapsed block
    if( data && data->collapsed() ) { block_count += data->collapsedBlockCount(); }
    
  }
  
  return;
  
}
