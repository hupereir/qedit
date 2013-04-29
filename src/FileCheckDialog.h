// $Id$
#ifndef _FileCheckDialog_h_
#define _FileCheckDialog_h_

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

#include <QStringList>

class TreeView;

//! QDialog used to select opened files
class FileCheckDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //! constructor
    FileCheckDialog( QWidget* parent );

    //! destructor
    virtual ~FileCheckDialog( void )
    {}

    //! set files
    void setFiles( const QStringList& );

    private:

    //! list
    TreeView& _list( void ) const
    { return *list_; }

    //! model
    FileRecordModel model_;

    //! list of files
    TreeView* list_;

};

#endif
