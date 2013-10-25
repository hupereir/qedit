#ifndef _FileSelectionDialog_h_
#define _FileSelectionDialog_h_
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

//! QDialog used to select opened files
class FileSelectionDialog: public CustomDialog
{

    //! Qt macro
    Q_OBJECT

    public:

    //! constructor
    FileSelectionDialog( QWidget* parent, const TextSelection& );

    //! destructor
    virtual ~FileSelectionDialog( void )
    {}

    //! selected files
    typedef QList<File> FileList;

    //! selected files
    FileList selectedFiles( void ) const;

    private Q_SLOTS:

    //! update buttons
    void _updateButtons( void );

    private:

    //! model
    FileRecordModel model_;

    //! list of files
    TreeView* list_;

    //! text selection
    TextSelection selection_;

    //!@name buttons
    //@{

    QPushButton* selectAllButton_;
    QPushButton* clearSelectionButton_;
    QPushButton* replaceButton_;

    //@}

};

#endif
