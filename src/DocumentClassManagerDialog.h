#ifndef DocumentClassManagerDialog_h
#define DocumentClassManagerDialog_h

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

#include "BaseDialog.h"
#include "DocumentClassModel.h"

#include <QAction>
#include <QPushButton>

class TreeView;

//* handles list of connection attributes
class DocumentClassManagerDialog: public BaseDialog
{

    Q_OBJECT

    public:

    //* constructor
    explicit DocumentClassManagerDialog( QWidget* = nullptr );

    //* list of user files
    File::List userFiles() const;

    private:

    //* read
    void _reload();

    //* add document class
    void _add();

    //* remove document classes
    void _remove();

    //* update buttons
    void _updateButtons();

    //* user treeview
    TreeView* list_ = nullptr;

    //* model
    DocumentClassModel model_;

    //* add button
    QPushButton* addButton_ = nullptr;

    //* remove button
    QPushButton* removeButton_ = nullptr;

    //* reload button
    QPushButton* reloadButton_ = nullptr;

    //* add action
    QAction* addAction_ = nullptr;

    //* remove action
    QAction* removeAction_ = nullptr;

    //* remove action
    QAction* reloadAction_ = nullptr;

};

#endif
