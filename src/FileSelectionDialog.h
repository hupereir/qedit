#ifndef _FileSelectionDialog_h_
#define _FileSelectionDialog_h_
// $Id$

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public license as published by the Free Software
* Foundation; either version 2 of the license, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public license
* for more details.
*
* You should have received a copy of the GNU General Public license along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "CustomDialog.h"
#include "FileRecordModel.h"
#include "TextSelection.h"

#include <list>

class TreeView;

//! QDialog used to select opened files
class FileSelectionDialog: public CustomDialog
{

    //! Qt macro
    Q_OBJECT

    public:

    //! constructor
    FileSelectionDialog( QWidget* parent, const TextSelection& );

    //! selected files
    typedef std::list<File> FileList;

    //! selected files
    FileList selectedFiles( void ) const;

    private slots:

    //! update buttons
    void _updateButtons( void );

    private:

    //! list
    TreeView& _list( void ) const
    { return *list_; }

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
