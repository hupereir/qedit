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

    protected:

    //! read
    virtual void _read( void );

    protected slots:

    //! add document class
    virtual void _add( void );

    private:

    //! user treeview
    TreeView* list_;

    //! model
    DocumentClassModel model_;

};

#endif
