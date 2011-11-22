#ifndef Menu_h
#define Menu_h

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
*******************************************************************************/

#include "Debug.h"
#include "Counter.h"
#include "File.h"

#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <cassert>
#include <map>

class RecentFilesMenu;
class TextMacroMenu;

//! menu
class Menu:public QMenuBar, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! creator
    Menu( QWidget* parent );

    //! destructor
    ~Menu( void );

    //! recent files menu
    RecentFilesMenu& recentFilesMenu( void ) const
    {
        assert( recentFilesMenu_ );
        return *recentFilesMenu_;
    }

    //! document class Menu
    QMenu& documentClassMenu( void ) const
    {
        assert( documentClassMenu_ );
        return *documentClassMenu_;
    }

    //! macro menu
    TextMacroMenu& macroMenu( void ) const
    {
        assert( macroMenu_ );
        return *macroMenu_;
    }

    private slots:

    //! update recent files menu
    /*! this is needed so that the current file appears checked in the menu */
    void _updateRecentFilesMenu( void );

    //! update edit menu
    void _updateEditMenu( void );

    //! update search menu
    void _updateSearchMenu( void );

    //! update preference menu
    void _updatePreferenceMenu( void );

    //! update tools menu
    void _updateToolsMenu( void );

    //! update macro menu
    void _updateMacroMenu( void );

    //! update windows menu
    void _updateWindowsMenu( void );

    //! select macro from menu
    void _selectMacro( QString );

    //! select file from windows menu
    void _selectFile( QAction* );

    private:

    //! local action group
    /*! used to track memory leaks */
    class ActionGroup: public QActionGroup, public Counter
    {

        public:

        //! constructor
        ActionGroup( QObject* parent ):
            QActionGroup( parent ),
            Counter( "Menu::ActionGroup" )
        {}

    };

    //!@name children
    //@{

    //! recent files menu
    RecentFilesMenu* recentFilesMenu_;

    //! document class menu
    QMenu* documentClassMenu_;

    //! edit menu
    QMenu* editMenu_;

    //! search menu
    QMenu* searchMenu_;

    //! preference menu
    QMenu* preferenceMenu_;

    //! toold menu
    QMenu* toolsMenu_;

    //! macro menu
    TextMacroMenu* macroMenu_;

    //! windows menu
    QMenu* windowsMenu_;

    //@}

    //!@name action groups
    //@{

    QActionGroup* windowsActionGroup_;

    //@}


    //!@name actions
    //@{

    //! map windows menu ID to file name
    std::map< QAction*, File > fileActions_;

    //@}

};

#endif
