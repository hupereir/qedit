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

namespace
{   
    QAction* createAction( const TextMacro& macro, bool enableAutomatic )
    {
        // create action label
        const QString label = ( macro.isAutomatic() && enableAutomatic ) ? 
            QString( QObject::tr( "%1 (automatic)" ) ).arg( macro.name() ):
            macro.name();
        
        auto out( new QAction( label, nullptr ) );
        if( !macro.accelerator().isEmpty() )
        { out->setShortcut( QKeySequence( macro.accelerator() ) ); }
        
        return out;
    }
}

//___________________________________________________________
TextMacroMenu::TextMacroMenu( QWidget* parent ):
QMenu( parent ),
Counter( QStringLiteral("TextMacroMenu") )
{
    Debug::Throw( QStringLiteral("TextMacroMenu::TextMacroMenu.\n") );
    connect( this, &QMenu::triggered, this, &TextMacroMenu::_processAction );
}

//___________________________________________________________
void TextMacroMenu::update( const TextMacro::List& macros, bool enableAutomatic )
{
    Debug::Throw( QStringLiteral("TextMacroMenu::update.\n") );
    clear();
    actions_.clear();

    for( const auto& macro:macros )
    {

        if( macro.isSeparator() ) addSeparator();
        else {
            // create menu entry
            auto action = createAction( macro, enableAutomatic );
            addAction( action );

            // insert in map
            actions_.insert( action, MacroContainer( macro ) );
        }
    }

}

//___________________________________________________________
void TextMacroMenu::updateState( bool value )
{
    Debug::Throw( QStringLiteral("TextMacroMenu::updateState.\n") );
    for( auto&& iter = actions_.begin(); iter != actions_.end(); iter++ )
    { iter.key()->setEnabled( value || iter.value().isAutomatic() ); }
}

//___________________________________________________________
void TextMacroMenu::setEnabled( bool enabled )
{
    for( auto&& iter = actions_.begin(); iter != actions_.end(); ++iter )
    { iter.key()->setEnabled( enabled ); }
}

//___________________________________________________________
void TextMacroMenu::_processAction( QAction* action )
{

    // try retrieve id in map
    const auto iter = actions_.find( action );
    if( iter != actions_.end() ) emit textMacroSelected( iter.value().name() );

}
