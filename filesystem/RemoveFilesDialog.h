#ifndef RemoveFilesDialog_h
#define RemoveFilesDialog_h

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
#include "FileSystemModel.h"

#include <QList>
#include <QPushButton>

class TreeView;
class TextEditor;

//* QDialog used to commit selected files
class RemoveFilesDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //* constructor
    explicit RemoveFilesDialog( QWidget*, const FileSystemModel::List& files = FileSystemModel::List() );

    private:

    //* list of files
    TreeView* list_ = nullptr;

    //* model
    FileSystemModel model_;

};

#endif
