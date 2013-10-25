// $Id$
#ifndef _NewFileDialog_h_
#define _NewFileDialog_h_

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

/*!
\file NewFileDialog.h
\brief QDialog used to ask if a new file should be created
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "BaseDialog.h"
#include "Counter.h"
#include "File.h"

//! QDialog used to ask if a new file should be created
class NewFileDialog: public BaseDialog, public Counter
{

    //! Qt macro
    Q_OBJECT

    public:

    //! return codes
    /*! also used to decide which buttons are to be drawn */
    enum ReturnCode {

        Unknown = 0,
        Create = 1<<0,
        Cancel = 1<<1,
        Exit = 1<<2,
        Default = Create|Cancel|Exit
    };

    Q_DECLARE_FLAGS( ReturnCodes, ReturnCode );

    //! constructor
    NewFileDialog( QWidget*, const File&, ReturnCodes = Default );

    //! destructor
    virtual ~NewFileDialog( void )
    {}

    private Q_SLOTS:

    //! create new file
    void _create( void )
    { done( Create ); }

    //! cancel creation/exit editor
    void _cancel( void )
    { done( Cancel ); }

    //! exit application
    void _exit( void )
    { done( Exit ); }

};

Q_DECLARE_OPERATORS_FOR_FLAGS( NewFileDialog::ReturnCodes );

#endif
