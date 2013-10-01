#ifndef ReplaceDialog_h
#define ReplaceDialog_h

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

#include "BaseReplaceDialog.h"

//! replace_text dialog for text editor widgets
class ReplaceDialog: public BaseReplaceDialog
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! constructor
    ReplaceDialog( QWidget* = 0, Qt::WindowFlags = 0 );

    //! destructor
    virtual ~ReplaceDialog( void )
    {}

    signals:

    //! emmited when Replace is pressed
    void replaceInFiles();

    private:

    //! replace button
    QPushButton* replaceInFilesButton_;

};
#endif
