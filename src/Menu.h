#ifndef Menu_h
#define Menu_h

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

#include "Debug.h"
#include "Counter.h"
#include "File.h"

#include <QMenu>
#include <QMenuBar>
#include <QHash>

class CustomMenu;
class RecentFilesMenu;
class TextMacroMenu;

//* menu
class Menu:public QMenuBar, public Counter
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    Menu( QWidget* );

    //* recent files menu
    RecentFilesMenu& recentFilesMenu( void ) const
    { return *recentFilesMenu_; }

    //* document class Menu
    QMenu& documentClassMenu( void ) const
    { return *documentClassMenu_; }

    //* macro menu
    TextMacroMenu& macroMenu( void ) const
    { return *macroMenu_; }

    public Q_SLOTS:

    //* update macro menu
    void updateMacroMenu( void );

    private Q_SLOTS:

    //* update recent files menu
    /** this is needed so that the current file appears checked in the menu */
    void _updateRecentFilesMenu( void );

    //* update edit menu
    void _updateEditMenu( void );

    //* update search menu
    void _updateSearchMenu( void );

    //* update preference menu
    void _updatePreferenceMenu( void );

    //* update tools menu
    void _updateToolsMenu( void );

    //* update windows menu
    void _updateWindowsMenu( void );

    //* select macro from menu
    void _selectMacro( QString );

    //* select file from windows menu
    void _selectFile( QAction* );

    private:

    //* local action group
    /** used to track memory leaks */
    class ActionGroup: public QActionGroup, public Counter
    {

        public:

        //* constructor
        ActionGroup( QObject* parent ):
            QActionGroup( parent ),
            Counter( "Menu::ActionGroup" )
        {}

    };

    //*@name children
    //@{

    //* recent files menu
    RecentFilesMenu* recentFilesMenu_ = nullptr;

    //* document class menu
    QMenu* documentClassMenu_ = nullptr;

    //* edit menu
    QMenu* editMenu_ = nullptr;

    //* search menu
    QMenu* searchMenu_ = nullptr;

    //* preference menu
    QMenu* preferenceMenu_ = nullptr;

    //* toold menu
    QMenu* toolsMenu_ = nullptr;

    //* macro menu
    TextMacroMenu* macroMenu_ = nullptr;

    //* windows menu
    CustomMenu* windowsMenu_ = nullptr;

    //@}

    //*@name action groups
    //@{

    QActionGroup* windowsActionGroup_ = nullptr;

    //@}


    //*@name actions
    //@{

    //* map windows menu ID to file name
    using ActionMap = QHash< QAction*, File >;
    ActionMap fileActions_;

    //@}

};

#endif
