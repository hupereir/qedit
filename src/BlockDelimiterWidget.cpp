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
  
  if( delimiters_.empty() ) return;
  
  const QFontMetrics metric( fontMetrics() );
  int y_offset = _editor().verticalScrollBar()->value();
  QTextDocument &document( *_editor().document() );
    
  // brush/pen  
  QPainter painter( this );
  
  painter.translate( 0, -y_offset );
  painter.setBrush( palette().color( QPalette::Base ) );
  
  // calculate height
  int height( QWidget::height() + y_offset );
  if( _editor().horizontalScrollBar()->isVisible() ) 
  { height -= _editor().horizontalScrollBar()->height(); }
  
  // store begin/end points
  typedef std::pair<int, int> Segment;
  
  // store segments
  typedef std::vector<Segment> SegmentList;
  SegmentList segments;
  
  // loop over delimiters
  for( BlockDelimiter::List::const_iterator iter = delimiters_.begin(); iter != delimiters_.end(); iter++ )
  {
  
    // keep track of all starting points
    vector<int> start_points;
    for( QTextBlock block = document.begin(); block.isValid(); block = block.next() ) 
    {
      
      // get block limits
      double block_begin( block.layout()->position().y() );
      double block_end( block_begin + block.layout()->boundingRect().height() );

      // check if outside of window
      if( block_begin > height ) break;
      
      // retrieve data and check this block delimiter
      HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
      if( !data ) continue;

      TextBlock::Delimiter delimiter( data->delimiter( iter->id() ) );
      if( delimiter.end() )
      {
        
        if( !(start_points.empty() || delimiter.begin() ) ) 
        { segments.push_back( make_pair( start_points.back(), block_end ) ); }
        
        // pop
        for( int i = 0; i < delimiter.end() && !start_points.empty(); i++ )
        { start_points.pop_back(); }
          
      }
        
      for( int i = 0; i < delimiter.begin(); i++ )
      { start_points.push_back( block_begin ); }
    
    }
    
    for( vector<int>::iterator iter = start_points.begin(); iter != start_points.end(); iter++ )
    { segments.push_back( make_pair( *iter, height+1 ) ); }
  
  }
  
  // draw all vertical lines
  for( SegmentList::const_iterator iter = segments.begin(); iter != segments.end(); iter++ )
  { painter.drawLine( width()/2, iter->first+0.8*fontMetrics().lineSpacing(), width()/2, iter->second ); }
    
  // draw ticks
  for( SegmentList::const_iterator iter = segments.begin(); iter != segments.end(); iter++ )
  {
    
    if( iter->first < height ) painter.drawRect( 0.2*width(), iter->first + 0.2*fontMetrics().lineSpacing(), 0.6*width(), 0.6*fontMetrics().lineSpacing() );
    if( iter->second < height ) painter.drawLine( width()/2, iter->second, width(), iter->second );
    
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
