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
#include "TextBlockData.h"
#include "XmlOptions.h"

//____________________________________________________________________________
LineNumberWidget::LineNumberWidget(TextEditor* editor, QWidget* parent): 
  QWidget( parent),
  Counter( "LineNumberWidget" ),
  editor_( editor )
{
  
  Debug::Throw( "LineNumberWidget::LineNumberWidget.\n" );
  setAutoFillBackground( true );
  
  connect( _editor().verticalScrollBar(), SIGNAL( valueChanged( int ) ), SLOT( update() ) );
  connect( &_editor().blockHighlight(), SIGNAL( highlightChanged() ), SLOT( update() ) );
  connect( &_editor(), SIGNAL( textChanged() ), SLOT( update() ) );  
  
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );

  // update configuration
  _updateConfiguration();
  
  // width
  setFixedWidth( fontMetrics().width( "000" ) + 14 );

}

//__________________________________________
LineNumberWidget::~LineNumberWidget()
{
  Debug::Throw( "LineNumberWidget::~LineNumberWidget.\n" );
}

//__________________________________________
void LineNumberWidget::paintEvent( QPaintEvent* )
{  
  
  const QFontMetrics metric( fontMetrics() );
  int y_offset = _editor().verticalScrollBar()->value();
  QTextDocument &document( *_editor().document() );
    
  // maximum text length
  int max_length=0;

  // brush/pen  
  QPainter painter( this );
  painter.translate( 0, -y_offset );
  
  int height( QWidget::height() - metric.lineSpacing() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) { height -= _editor().horizontalScrollBar()->height(); }
  
  int block_count(1);
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next(), block_count++ )
  {
    int block_y( block.layout()->position().y() );
    if ( block_y < y_offset ) continue;    
    if ( block_y > height ) break;
    
    // block highlight
    TextBlockData* data = 0;
    if( 
      _editor().blockHighlightAction().isChecked() && 
      ( data = static_cast<TextBlockData*>( block.userData() ) ) &&  
      data->hasFlag( TextBlock::CURRENT_BLOCK ) )
    {
      
      painter.setBrush( highlight_color_ );
      
      painter.setPen( Qt::NoPen );
      painter.drawRect( QRect( 
        0, block_y, width(),
        metric.lineSpacing() ) );
      
      painter.setPen( palette().color( QPalette::Text ) );
      
    } 
    
    QString numtext( QString::number(block_count) );
    
    painter.drawText(
      0, block_y, width()-8,
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
