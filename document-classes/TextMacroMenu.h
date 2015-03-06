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
#include <QMap>

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

    //! set enabled
    void setEnabled( bool );

    Q_SIGNALS:

    //! emmited every time a text macro is selected
    void textMacroSelected( QString );

    public Q_SLOTS:

    //! update macros
    void update( const TextMacro::List& );

    //! update state
    void updateState( bool );

    protected Q_SLOTS:

    //! emited when an action is selected
    void _processAction( QAction* action );

    private:

    //! container for macros
    class MacroContainer
    {
        public:

        //! constructor from text macro
        MacroContainer( const TextMacro& macro ):
            name_( macro.name() ),
            automatic_( macro.isAutomatic() )
        {}

        //! name
        const QString& name( void ) const
        { return name_; }

        //! automatic
        bool isAutomatic( void ) const
        { return automatic_; }

        private:

        QString name_;
        bool automatic_;

    };

    //! map action to TextMacro
    using ActionMap = QMap<QAction*, MacroContainer >;
    ActionMap actions_;

};

#endif
