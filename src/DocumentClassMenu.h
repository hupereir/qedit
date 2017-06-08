#ifndef DocumentClassMenu_h
#define DocumentClassMenu_h
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

#include <QMenu>
#include <QAction>
#include <QActionGroup>

#include <QHash>

//* display available document classes
class DocumentClassMenu: public QMenu, private Base::Counter<DocumentClassMenu>
{

    Q_OBJECT

    public:

    //* constructor
    explicit DocumentClassMenu( QWidget* );

    Q_SIGNALS:

    //* emitted every time a document class is selected
    void documentClassSelected( QString );

    protected Q_SLOTS:

    //* update
    void _update( void );

    //* select document class from menu
    void _selectClassName( QAction* );

    private:

    //* actions
    QActionGroup* actionGroup_ = nullptr;

    //* map document class to macro names
    using ActionMap = QHash< QAction*, QString >;
    ActionMap actions_;

};

#endif
