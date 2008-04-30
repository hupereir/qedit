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

#include "BlockHighlight.h"
#include "CustomTextEdit.h"
#include "Debug.h"
#include "BlockDelimiterWidget.h"
#include "HighlightBlockData.h"
#include "XmlOptions.h"

using namespace std;

//____________________________________________________________________________
BlockDelimiterWidget::BlockDelimiterWidget(CustomTextEdit* editor, QWidget* parent): 
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
  
  // width
  setFixedWidth( fontMetrics().lineSpacing() );

}

//__________________________________________
BlockDelimiterWidget::~BlockDelimiterWidget()
{
  Debug::Throw( "BlockDelimiterWidget::~BlockDelimiterWidget.\n" );
}

//__________________________________________
void BlockDelimiterWidget::paintEvent( QPaintEvent* )
{  
  
  const QFontMetrics metric( fontMetrics() );
  int y_offset = _editor().verticalScrollBar()->value();
  QTextDocument &document( *_editor().document() );
    
  // brush/pen  
  QPainter painter( this );
  painter.translate( 0, -y_offset );
  
  // calculate height
  // int height( QWidget::height() - metric.lineSpacing() );
  int height( QWidget::height() );
  if( _editor().horizontalScrollBar()->isVisible() ) { height -= _editor().horizontalScrollBar()->height(); }
  
  int block_count = 0;
  QPoint start_point;
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next() )
  {
    
    // draw tick if visible
    int block_y( block.layout()->position().y() );
    int block_height( block.layout()->boundingRect().height() );

    // retrieve data and check this block delimiter
    HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( data && ( data->delimiter().begin_ || data->delimiter().end_ ) ) 
    { 
    
      if( block_y + 20 - y_offset >= 0 )
      {
        if( data->delimiter().begin_ ) painter.drawLine( width()/2, block_y, width(), block_y );
        else painter.drawLine( width()/2, block_y + block_height, width(), block_y + block_height );          
      }
            
      // now set lines
      if( block_count == 0 && data->delimiter().begin_ )
      { 
        // set begin point 
        start_point = QPoint( width()/2, block_y );
      }
      
      // update block count
      block_count = std::max<int>( 0, block_count + data->delimiter().begin_ - data->delimiter().end_ );
      if( block_count == 0 && data->delimiter().end_ )
      {
        // set end point and draw line
        QPoint end_point = QPoint(  width()/2, block_y + block_height );
        painter.drawLine( start_point, end_point );
      }
    
    }
    
    // check if outside of window
    if ( block_y - y_offset > height ) 
    {
      
      if( block_count > 0 )
      {
        // set end point and draw line
        QPoint end_point = QPoint(  width()/2, height + y_offset );
        painter.drawLine( start_point, end_point );
      }
      
      // exit loop
      break;
    }
  }

  painter.end();
  
}

//________________________________________________________
void BlockDelimiterWidget::_updateConfiguration( void )
{
  
  Debug::Throw( "BlockDelimiterWidget::_updateConfiguration.\n" );

  // font
  QFont font;
  font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ).c_str() );
  setFont( font );
  
}
