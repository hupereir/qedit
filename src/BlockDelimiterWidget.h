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
#include <QPaintEvent>
#include <QWidget>

#include "Counter.h"

class CustomTextEdit;

//! display block delimiters
class BlockDelimiterWidget: public QWidget, public Counter 
{

  //! Qt meta object
  Q_OBJECT  

  public:
    
  //! constructor
  BlockDelimiterWidget(CustomTextEdit*, QWidget* parent);

  //! destructor
  virtual ~BlockDelimiterWidget();
  
  protected:
  
  //! paint
  virtual void paintEvent( QPaintEvent* );

  private slots:
  
  //! configuration
  void _updateConfiguration( void );
    
  private:
  
  //! editor
  CustomTextEdit& _editor( void ) const
  { return *editor_; }
  
  //! associated editor
  CustomTextEdit* editor_;
  
};

#endif
