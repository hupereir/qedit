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

    signals:

    //! emmited every time a text macro is selected
    void textMacroSelected( QString );

    public slots:

    //! update macros
    void update( const TextMacro::List& );

    //! update state
    void updateState( bool );

    protected slots:

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
    typedef QMap<QAction*, MacroContainer > ActionMap;
    ActionMap actions_;

};

#endif
