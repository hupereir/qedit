#ifndef MenuBar_h
#define MenuBar_h

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
#include "File.h"

#include <QActionGroup>
#include <QHash>
#include <QMenu>
#include <QMenuBar>

class BaseMenu;
class RecentFilesMenu;
class TextMacroMenu;

//* menu
class MenuBar:public QMenuBar, private Base::Counter<MenuBar>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    explicit MenuBar( QWidget* );

    //* recent files menu
    RecentFilesMenu& recentFilesMenu() const
    { return *recentFilesMenu_; }

    //* document class Menu
    QMenu& documentClassMenu() const
    { return *documentClassMenu_; }

    //* macro menu
    TextMacroMenu& macroMenu() const
    { return *macroMenu_; }

    //* update macro menu
    void updateMacroMenu();

    private:

    //* update recent files menu
    /** this is needed so that the current file appears checked in the menu */
    void _updateRecentFilesMenu();

    //* update edit menu
    void _updateEditMenu();

    //* update search menu
    void _updateSearchMenu();

    //* update preference menu
    void _updatePreferenceMenu();

    //* update tools menu
    void _updateToolsMenu();

    //* update windows menu
    void _updateWindowsMenu();

    //* select macro from menu
    void _selectMacro( const QString &);

    //* select file from windows menu
    void _selectFile( QAction* );

    //* local action group
    /** used to track memory leaks */
    class ActionGroup: public QActionGroup, private Base::Counter<ActionGroup>
    {

        public:

        //* constructor
        explicit ActionGroup( QObject* parent ):
            QActionGroup( parent ),
            Counter( QStringLiteral("Menu::ActionGroup") )
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
    BaseMenu* windowsMenu_ = nullptr;

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
