#ifndef RenameFileDialog_h
#define RenameFileDialog_h

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

#include "CustomDialog.h"
#include "FileRecord.h"

class LineEditor;

//* Edit/create keyword popup dialog
class RenameFileDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //* constructor
    explicit RenameFileDialog( QWidget* parent, FileRecord );

    //* new file name
    File file() const;

    private Q_SLOTS:

    //* button update
    void _updateButtons();

    private:

    //* line editor
    LineEditor& _editor() const
    { return *editor_; }

    //* editor
    LineEditor *editor_ = nullptr;

};

#endif
