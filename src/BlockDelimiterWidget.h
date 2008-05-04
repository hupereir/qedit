#ifndef BlockDelimiterWidget_h
#define BlockDelimiterWidget_h

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
  \file BlockDelimiterWidget.h
  \brief display block delimiters
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QAction>
#include <QColor>
#include <QFont>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTextBlock>
#include <QWheelEvent>
#include <QWidget>

#include "BlockDelimiter.h"
#include "BlockDelimiterSegment.h"
#include "Counter.h"

class HighlightBlockData;
class TextDisplay;

//! display block delimiters
class BlockDelimiterWidget: public QWidget, public Counter 
{

  //! Qt meta object
  Q_OBJECT

  public:
  
  //! constructor
  BlockDelimiterWidget(TextDisplay*, QWidget* parent);

  //! destructor
  virtual ~BlockDelimiterWidget();
  
  //! block delimiters
  void setBlockDelimiters( const BlockDelimiter::List& delimiters )
  { delimiters_ = delimiters; }
  
  //! synchronization
  void synchronize( const BlockDelimiterWidget* );
  
  //!@name actions
  //@{
  
  //! expand all
  QAction& expandAllAction( void ) const
  { return *expand_all_action_; }
  
  //@}
  
  protected:
  
  //! paint
  virtual void paintEvent( QPaintEvent* );

  //! mouse press event
  virtual void mousePressEvent( QMouseEvent* );
    
  //! wheel event
  /*! wheel events are forwarded to the editor */
  virtual void wheelEvent( QWheelEvent* );
  
  private slots:
  
  //! configuration
  void _updateConfiguration( void );
      
  //! expand all blocks
  void _expandAllBlocks( void );
  
  //! update position to match scrollbar
  void _scrollBarPositionChanged( void )
  {
    need_segment_update_ = false;
    update();
  }
  
  private:
  
  //! install actions
  void _installActions( void );
  
  //! update segments
  /*! this might go into a slot, linked to document being modified */
  void _updateSegments( void );
  
  //! expand current block
  void _expand( const QTextBlock&, HighlightBlockData*, const bool& recursive = false ) const;

  //! collapse blocks 
  void _collapse( const QTextBlock&, const QTextBlock&, HighlightBlockData* ) const;
  
  //! editor
  TextDisplay& _editor( void ) const
  { return *editor_; }
  
  //! associated editor
  TextDisplay* editor_;
  
  //! block delimiters
  BlockDelimiter::List delimiters_;
  
  //! block segments
  BlockDelimiterSegment::List segments_;
  
  //! true when _updateSegments needs to be called in paintEvent
  bool need_segment_update_; 
  
  //!@name marker dimension
  //@{

  /* 
  note: to speed-up the code, one could store all positions as members 
  they are:
    half_width = 0.5*width
    top = 0.8*width
    top_left = 0.2*width
    rect_width = rect_height = 0.6*width
    marker_left = 0.35*width;
    marker_right = 0.65*width; 
  */
  
  int width_;
  int half_width_;
  int top_;
  int rect_top_left_;
  int rect_width_;
  int marker_top_left_;
  int marker_bottom_right_;
  
  //@}
  
  //!@name actions
  //@{
  
  //! expand all
  QAction* expand_all_action_;
  
  //@}
  
};

#endif







