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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "TextMacroMenu.h"

//___________________________________________________________
TextMacroMenu::TextMacroMenu( QWidget* parent ):
QMenu( parent ),
Counter( "TextMacroMenu" )
{
    Debug::Throw( "TextMacroMenu::TextMacroMenu.\n" );
    connect( this, SIGNAL(triggered(QAction*)), SLOT(_processAction(QAction*)) );
}

//___________________________________________________________
void TextMacroMenu::update( const TextMacro::List& macros )
{
    Debug::Throw( "TextMacroMenu::update.\n" );
    clear();
    actions_.clear();

    foreach( const TextMacro& macro, macros )
    {

        if( macro.isSeparator() ) addSeparator();
        else {

            // create menu entry
            QAction* action = macro.action();
            addAction( action );

            // insert in map
            actions_.insert( action, macro );

        }
    }

}

//___________________________________________________________
void TextMacroMenu::updateState( bool value )
{
    Debug::Throw( "TextMacroMenu::updateState.\n" );
    for( ActionMap::const_iterator iter = actions_.begin(); iter != actions_.end(); iter++ )
    { iter.key()->setEnabled( value || iter.value().isAutomatic() ); }
}

//___________________________________________________________
void TextMacroMenu::setEnabled( bool enabled )
{
    for( ActionMap::iterator iter = actions_.begin(); iter != actions_.end(); ++iter )
    { iter.key()->setEnabled( enabled ); }
}

//___________________________________________________________
void TextMacroMenu::_processAction( QAction* action )
{

    // try retrieve id in map
    ActionMap::iterator iter = actions_.find( action );
    if( iter != actions_.end() ) emit textMacroSelected( iter.value().name() );

}
