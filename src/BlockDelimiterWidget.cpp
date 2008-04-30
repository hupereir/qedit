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
  int height( QWidget::height() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) { height -= _editor().horizontalScrollBar()->height(); }
  
  // keep track of all starting points
  vector<QPoint> start_points;

  for( QTextBlock block = document.begin(); block.isValid(); block = block.next() )
  {
    
    // draw tick if visible
    int block_begin( block.layout()->position().y() );
    int block_end( block_begin + block.layout()->boundingRect().height() );

    // retrieve data and check this block delimiter
    HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( data && ( data->delimiter().begin() || data->delimiter().end() ) ) 
    {
    
      if( data->delimiter().begin() )
      {
        
        // store block starting point
        QPoint point( width()/2, block_begin );
        for( int i = 0; i < data->delimiter().begin(); i++ )
        { start_points.push_back( point ); }
        
        // draw tick
        if( block_begin >= y_offset && block_begin <= height ) 
        { painter.drawLine( width()/2, block_begin, width(), block_begin ); }
      
      } 
          
      if( data->delimiter().end() )
      {
        
        if( (!start_points.empty()) && ( int(start_points.size()) - data->delimiter().end() ) <= 0 )
        {
          // set end point and draw line
          QPoint end_point = QPoint(  width()/2, min( block_end, height ) );
          painter.drawLine( start_points.front(), end_point );
        }
        
        // draw tick
        if( (!data->delimiter().begin()) && (!start_points.empty() && block_end >= y_offset  && block_end <= height ) )
        { painter.drawLine( width()/2, block_end, width(), block_end ); }
        
        // pop
        for( int i = 0; i < data->delimiter().end() && !start_points.empty(); i++ )
        { start_points.pop_back(); }
 
      }
      
    }
    
    // check if outside of window
    if( block_begin > height ) break;
  }

  // draw vertical line if needed
  if( (!start_points.empty() ) && start_points.front().y() < height )
  {
    // set end point and draw line
    QPoint end_point = QPoint(  width()/2, height );
    painter.drawLine( start_points.front(), end_point );
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
