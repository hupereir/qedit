#ifndef LineNumberWidget_h
#define LineNumberWidget_h

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
  \file LineNumberWidget.h
  \brief display line number of a text editor
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QColor>
#include <QFont>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>
#include <QWheelEvent>

#include "Counter.h"

class TextEditor;

//! display line number of a text editor
class LineNumberWidget: public QWidget, public Counter
{

  //! Qt meta object
  Q_OBJECT  

  public:
    
  //! constructor
  LineNumberWidget(TextEditor*, QWidget* parent);

  //! destructor
  virtual ~LineNumberWidget();
  
  protected:
  
  //! paint
  virtual void paintEvent( QPaintEvent* );
  
  //! mouse press event
  /*! left button events are forwarded to the editor */
  virtual void mousePressEvent( QMouseEvent* );
  
  //! mouse press event
  /*! left button events are forwarded to the editor */
  virtual void mouseReleaseEvent( QMouseEvent* );

  //! wheel event
  /*! wheel events are forwarded to the editor */
  virtual void wheelEvent( QWheelEvent* );

  private slots:
  
  //! configuration
  void _updateConfiguration( void );
  
  //! need update
  void _needUpdate( void )
  { need_update_ = true; }
  
  //! contents changed
  void _contentsChanged( void );
  
  //! block count changed
  void _blockCountChanged( void );
    
  private:
  
  //! editor
  TextEditor& _editor( void ) const
  { return *editor_; }

  //! update line number data
  void _updateLineNumberData( void );
  
  //! map block number and position
  class LineNumberData
  {
    
    public:
    
    //! list of data
    typedef std::vector<LineNumberData> List;
    
    //! constructor
    LineNumberData( const unsigned int& line_number, const int& y ):
      line_number_( line_number ),
      y_( y )
    {}
    
    //! line number
    const unsigned int& lineNumber( void ) const
    { return line_number_; }
    
    //! y
    const int& y( void ) const
    { return y_; }
    
    private:
    
    //! line number
    unsigned int line_number_;
    
    //! position
    int y_;
    
  };  
  
  //! associated editor
  TextEditor* editor_;
  
  //! current block highlight color
  QColor highlight_color_;
  
  //! line number data
  LineNumberData::List line_number_data_;
  
  //! true when line number data update is needed
  bool need_update_;
  
};

#endif
