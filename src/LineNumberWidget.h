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

class CustomTextEdit;

//! display line number of a text editor
class LineNumberWidget: public QWidget, public Counter
{

  //! Qt meta object
  Q_OBJECT  

  public:
    
  //! constructor
  LineNumberWidget(CustomTextEdit*, QWidget* parent);

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
  
  private:
  
  //! editor
  CustomTextEdit& _editor( void ) const
  { return *editor_; }
  
  //! associated editor
  CustomTextEdit* editor_;
  
  //! current block highlight color
  QColor highlight_color_;
  
};

#endif
