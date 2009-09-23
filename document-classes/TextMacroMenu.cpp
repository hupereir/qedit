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
   \file TextMacroMenu.cpp
   \brief display available text macros
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include "TextMacroMenu.h"

using namespace std;

//___________________________________________________________
TextMacroMenu::TextMacroMenu( QWidget* parent ):
  QMenu( parent ),
  Counter( "TextMacroMenu" )
{
  Debug::Throw( "TextMacroMenu::TextMacroMenu.\n" );
  connect( this, SIGNAL( triggered( QAction* ) ), SLOT( _processAction( QAction* ) ) );
}

//___________________________________________________________
void TextMacroMenu::setTextMacros( const TextMacro::List& macros, bool enabled )
{
  Debug::Throw( "TextMacroMenu::setTextMacros.\n" );
  clear();
  actions_.clear();
  QAction* action;

  for( TextMacro::List::const_iterator iter = macros.begin(); iter != macros.end(); iter++ )
  {

    if( iter->isSeparator() ) addSeparator();
    else {

      // create menu entry
      action = iter->action();
      action->setEnabled( enabled || iter->isAutomatic() );
      addAction( action );

      // insert in map
      actions_.insert( make_pair( action, iter->name() ) );

    }
  }

}

//___________________________________________________________
void TextMacroMenu::setEnabled( bool enabled )
{
  for( std::map< QAction*, QString >::iterator iter = actions_.begin(); iter != actions_.end(); iter++ )
  { iter->first->setEnabled( enabled ); }
}

//___________________________________________________________
void TextMacroMenu::_processAction( QAction* action )
{

  // try retrieve id in map
  std::map< QAction*, QString >::iterator iter = actions_.find( action );
  if( iter != actions_.end() ) emit textMacroSelected( iter->second );

}
