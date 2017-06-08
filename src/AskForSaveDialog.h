#ifndef AskForSaveDialog_h
#define AskForSaveDialog_h

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

//* QDialog used to ask if modifications of a file should be saved
class AskForSaveDialog: public BaseDialog, private Base::Counter<AskForSaveDialog>
{

    //* Qt macro
    Q_OBJECT

    public:

    //* return codes
    /** it is also used to decide which buttons are to be shown */
    enum ReturnCode
    {

        Unknown = 0,
        Yes = 1<<0,
        No = 1<<1,
        YesToAll = 1<<2,
        NoToAll = 1<<3,
        Cancel = 1<<4,
        Default = Yes|No|Cancel
    };

    Q_DECLARE_FLAGS( ReturnCodes, ReturnCode );

    //* constructor
    explicit AskForSaveDialog( QWidget*, const File&, ReturnCodes = Default );

    private Q_SLOTS:

    //* save changes
    void _yes( void )
    { done( Yes ); }

    //* discard changes
    void _no( void )
    { done( No ); }

    //* discard changes
    void _yesToAll( void )
    { done( YesToAll ); }

    //* discard changes
    void _noToAll( void )
    { done( NoToAll ); }

    //* cancel action
    void _cancel( void )
    { done( Cancel ); }

};

Q_DECLARE_OPERATORS_FOR_FLAGS( AskForSaveDialog::ReturnCodes );

#endif
