#ifndef DocumentClassManagerDialog_h
#define DocumentClassManagerDialog_h

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
* ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA   02111-1307 USA
*
*******************************************************************************/

#include "CustomDialog.h"
#include "DocumentClassModel.h"

#include <QAction>
#include <QPushButton>

class TreeView;

//! handles list of connection attributes
class DocumentClassManagerDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //! constructor
    DocumentClassManagerDialog( QWidget* = 0x0 );

    //! destructor
    virtual ~DocumentClassManagerDialog( void )
    {}

    //! list of user files
    File::List userFiles( void ) const;

    protected slots:

    //! read
    virtual void _reload( void );

    //! add document class
    virtual void _add( void );

    //! remove document classes
    virtual void _remove( void );

    //! update buttons
    virtual void _updateButtons( void );

    private:

    //! user treeview
    TreeView* list_;

    //! model
    DocumentClassModel model_;

    //! add button
    QPushButton* addButton_;

    //! remove button
    QPushButton* removeButton_;

    //! reload button
    QPushButton* reloadButton_;

    //! add action
    QAction* addAction_;

    //! remove action
    QAction* removeAction_;

    //! remove action
    QAction* reloadAction_;

};

#endif
