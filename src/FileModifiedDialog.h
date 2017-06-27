#ifndef FileModifiedDialog_h
#define FileModifiedDialog_h

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
#include "Counter.h"
#include "File.h"

//* QDialog used to ask if a new file should be created
class FileModifiedDialog: public BaseDialog, private Base::Counter<FileModifiedDialog>
{

    //* Qt macro
    Q_OBJECT

    public:

    //* return codes
    enum ReturnCode
    {

        SaveAgain,
        SaveAs,
        Reload,
        Ignore

    };

    //* constructor
    explicit FileModifiedDialog( QWidget* parent, const File& file );

    private Q_SLOTS:

    //* re-saved removed file
    void _reLoad()
    { done( Reload ); }

    //* re-saved removed file
    void _reSave()
    { done( SaveAgain ); }

    //* save file with new name
    void _saveAs()
    { done( SaveAs ); }

    //* save file with new name
    void _ignore()
    { done( Ignore ); }

};

#endif
