#ifndef TextMacroMenu_h
#define TextMacroMenu_h
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

#include "Counter.h"
#include "TextMacro.h"

#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QHash>

//* display available document classes
class TextMacroMenu: public QMenu, private Base::Counter<TextMacroMenu>
{

    Q_OBJECT

    public:

    //* constructor
    explicit TextMacroMenu( QWidget* );

    //* set enabled
    void setEnabled( bool );

    //* update macros
    void update( const TextMacro::List&, bool enableAutomatic );

    //* update state
    void updateState( bool );

    Q_SIGNALS:

    //* emitted every time a text macro is selected
    void textMacroSelected( QString );

    private:

    //* emitted when an action is selected
    void _processAction( QAction* action );

    //* container for macros
    class MacroContainer
    {
        public:

        //* constructor from text macro
        explicit MacroContainer( const TextMacro& macro ):
            name_( macro.name() ),
            automatic_( macro.isAutomatic() )
        {}

        //* name
        const QString& name() const
        { return name_; }

        //* automatic
        bool isAutomatic() const
        { return automatic_; }

        private:

        // name
        QString name_;

        //* automatic
        bool automatic_ = false;

    };

    //* map action to TextMacro
    using ActionMap = QHash<QAction*, MacroContainer >;
    ActionMap actions_;

};

#endif
