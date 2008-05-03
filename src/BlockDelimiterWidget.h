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

#include <QColor>
#include <QFont>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QWidget>

#include "BlockDelimiter.h"
#include "Counter.h"

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
  
  protected:
  
  //! paint
  virtual void paintEvent( QPaintEvent* );

  //! mouse press event
  virtual void mousePressEvent( QMouseEvent* );
  
  //! mouse release event
  virtual void mouseReleaseEvent( QMouseEvent* );
  
  //! wheel event
  /*! wheel events are forwarded to the editor */
  virtual void wheelEvent( QWheelEvent* );
  
  private slots:
  
  //! configuration
  void _updateConfiguration( void );
    
  private:
  
  // used to draw block segment
  class Segment
  {
    
    public:
    
    //! list
    typedef std::vector<Segment> List;
    
    //! constructor
    Segment( const int& first = 0, const int& second = 0, const bool& ignored = false ):
      first_( first ),
      second_( second ),
      ignored_( ignored )
    {}
        
    //! first point
    const int& first( void ) const
    { return first_; }
    
    //! first point
    Segment& setFirst( const int& first )
    { 
      first_ = first; 
      return *this;
    }

    //! first point
    const int& second( void ) const
    { return second_; }
    
    //! first point
    Segment& setSecond( const int& second )
    { 
      second_ = second; 
      return *this;
    }

    //! ignored
    const bool& ignored( void ) const
    { return ignored_; }
    
    //! ignored
    Segment& setIgnored( const bool& ignored ) 
    { 
      ignored_ = ignored; 
      return *this;
    }
      
    //! active rect
    const QRect& activeRect( void ) const
    { return active_; }
    
    //! draw line
    void drawLine( QPainter&, const int& );
    
    //! draw delimiters
    void drawFirstDelimiter( QPainter&, const int& );

    //! draw delimiters
    void drawSecondDelimiter( QPainter&, const int& );
    
    //! used to find segment matching a point
    class ContainsFTor
    {
      
      public:
      
      //! creator
      ContainsFTor( const QPoint& point ):
        point_( point )
        {}
      
      //! prediction
      bool operator() (const Segment& segment ) const
      { return segment.activeRect().contains( point_ ); }
      
      private:
      
      //! position
      QPoint point_; 
      
    };
    
    private:
    
    //! first position
    int first_;
    
    //! second position
    int second_;
    
    //! ignore flag
    bool ignored_;
    
    //! active area (for mouse pointing)
    /*! it is set if drawFirstDelimiter() is called */
    QRect active_;

  };
  
  //! editor
  TextDisplay& _editor( void ) const
  { return *editor_; }
  
  //! associated editor
  TextDisplay* editor_;
  
  //! block delimiters
  BlockDelimiter::List delimiters_;
  
  //! block segments
  Segment::List segments_;
  
};

#endif
