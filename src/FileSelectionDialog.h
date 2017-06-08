#ifndef FileSelectionDialog_h
#define FileSelectionDialog_h

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
#include "FileRecordModel.h"
#include "TextSelection.h"

class TreeView;

//* QDialog used to select opened files
class FileSelectionDialog: public CustomDialog
{

    //* Qt macro
    Q_OBJECT

    public:

    //* constructor
    explicit FileSelectionDialog( QWidget* parent, const TextSelection& );

    //* selected files
    using FileList = QList<File>;

    //* selected files
    FileList selectedFiles( void ) const;

    private Q_SLOTS:

    //* update buttons
    void _updateButtons( void );

    private:

    //* model
    FileRecordModel model_;

    //* list of files
    TreeView* list_ = nullptr;

    //* text selection
    TextSelection selection_;

    //*@name buttons
    //@{

    QPushButton* selectAllButton_ = nullptr;
    QPushButton* clearSelectionButton_ = nullptr;
    QPushButton* replaceButton_ = nullptr;

    //@}

};

#endif
