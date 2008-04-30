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
#include <QPaintEvent>
#include <QWidget>

#include "Counter.h"

class CustomTextEdit;

//! display line number of a text editor
class LineNumberWidget: public QWidget, public Counter
{

  Q_OBJECT  

  public:
    
  //! constructor
  LineNumberWidget(CustomTextEdit*, QWidget* parent);

  //! destructor
  virtual ~LineNumberWidget();
  
  protected:
  
  //! paint
  virtual void paintEvent( QPaintEvent* );
  
  private slots:
  
  //! configuration
  void _updateConfiguration( void );
  
  //! paragraph highlight
  void _highlightParagraph( void );
  
  private:
  
  //! editor
  CustomTextEdit& _editor( void ) const
  { return *editor_; }
  
  //! associated editor
  CustomTextEdit* editor_;
  
  //! current block highlight color
  QColor highlight_color_;

  //! keep track of current block
  int current_block_;
  
};

#endif
