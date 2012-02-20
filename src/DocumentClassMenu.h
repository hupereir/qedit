#ifndef DocumentClassMenu_h
#define DocumentClassMenu_h

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
\file DocumentClassMenu.h
\brief display available document classes
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "Counter.h"

#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>

#include <QtCore/QHash>

//! display available document classes
class DocumentClassMenu: public QMenu, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    DocumentClassMenu( QWidget* );

    //! destructor
    virtual ~DocumentClassMenu( void )
    {}

    signals:

    //! emmited every time a document class is selected
    void documentClassSelected( QString );

    protected slots:

    //! update
    void _update( void );

    //! select document class from menu
    void _selectClassName( QAction* );

    private:

    //! actions
    QActionGroup* actionGroup_;

    //! map document class to macro names
    typedef QHash< QAction*, QString > ActionMap;
    ActionMap actions_;

};

#endif
