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
  //painter.setRenderHints(QPainter::Antialiasing );
  
  painter.translate( 0, -y_offset );
  painter.setBrush( palette().color( QPalette::Base ) );
  
  // calculate height
  // int height( QWidget::height() - metric.lineSpacing() );
  int height( QWidget::height() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) { height -= _editor().horizontalScrollBar()->height(); }
  
  // keep track of all starting points
  vector<QPointF> start_points;
  vector<QPointF> all_start_points;
  for( QTextBlock block = document.begin(); block.isValid(); block = block.next() )
  {
    
    // draw tick if visible
    double block_begin( block.layout()->position().y() );
    double block_end( block_begin + block.layout()->boundingRect().height() );

    // retrieve data and check this block delimiter
    HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( data && ( data->delimiter().begin() || data->delimiter().end() ) ) 
    {
    
      if( data->delimiter().begin() )
      {
        
        // store block starting point
        QPointF point( 0.5*width(), block_begin + 0.2*metric.lineSpacing() );
        for( int i = 0; i < data->delimiter().begin(); i++ )
        { start_points.push_back( point ); }
        
        // draw tick
        if( block_begin >= y_offset && block_begin <= height ) 
        { all_start_points.push_back( point ); }
      
      } 
          
      if( data->delimiter().end() )
      {
        
        if( (!start_points.empty()) && ( int(start_points.size()) - data->delimiter().end() ) <= 0 )
        {
          // set end point and draw line
          QPointF end_point(  0.5*width(), min<double>( block_end, height ) );
          painter.drawLine( start_points.front(), end_point );
        }
        
        // draw tick
        if( (!data->delimiter().begin()) && (!start_points.empty() && block_end >= y_offset  && block_end <= height ) )
        { painter.drawLine( QPointF(0.5*width(), block_end), QPointF(width(), block_end) ); }
        
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
    QPointF end_point(  0.5*width(), height );
    painter.drawLine( start_points.front(), end_point );
  }

  // draw all start points
  for( vector<QPointF>::const_iterator iter = all_start_points.begin(); iter != all_start_points.end(); iter++ )
  {
    painter.drawRect( QRectF( 0.2*width(), iter->y(), 0.6*width(), 0.6*fontMetrics().lineSpacing() ) ); 
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
   
  // width
  setFixedWidth( fontMetrics().lineSpacing() );

}
