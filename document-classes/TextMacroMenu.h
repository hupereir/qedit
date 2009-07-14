#ifndef TextMacroMenu_h
#define TextMacroMenu_h

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
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
****************************************************************************/

/*!
   \file TextMacroMenu.h
   \brief display available text macros
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <map>

#include "Counter.h"
#include "TextMacro.h"

//! display available document classes
class TextMacroMenu: public QMenu, public Counter
{
  
  Q_OBJECT
  
  public:
  
  //! constructor
  TextMacroMenu( QWidget* );

  //! destructor
  virtual ~TextMacroMenu( void )
  {}

  //! set macro
  void setTextMacros( const TextMacro::List&, bool );
  
  //! set enabled
  void setEnabled( bool );
  
  signals:
  
  //! emmited every time a text macro is selected
  void textMacroSelected( QString );

  protected slots:
  
  //! emited when an action is selected
  void _processAction( QAction* action );
  
  private:
  
  //! map action to TextMacro
  std::map<QAction*, QString > actions_;
  
};

#endif
