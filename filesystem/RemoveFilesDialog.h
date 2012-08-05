#ifndef _RemoveFilesDialog_h_
#define _RemoveFilesDialog_h_

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
*******************************************************************************/

#include "CustomDialog.h"
#include "FileSystemModel.h"

#include <QtCore/QList>
#include <QtGui/QPushButton>

#include <cassert>

class TreeView;
class TextEditor;

//! QDialog used to commit selected files
class RemoveFilesDialog: public CustomDialog
{

    public:

    //! constructor
    RemoveFilesDialog( QWidget*, const FileSystemModel::List& files = FileSystemModel::List() );

    protected:

    //! list
    TreeView& _list() const
    {
        assert( list_ );
        return *list_;
    }

    //! model
    const FileSystemModel& _model() const
    { return model_; }

    //! model
    FileSystemModel& _model()
    { return model_; }

    private:

    //! list of files
    TreeView* list_;

    //! model
    FileSystemModel model_;

};

#endif
