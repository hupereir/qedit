#ifndef FileModifiedWidget_h
#define FileModifiedWidget_h

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

#include "File.h"
#include "MessageWidget.h"
#include "Key.h"

//* QDialog used to ask if a new file should be created
class FileModifiedWidget: public MessageWidget, public Base::Key
{

    //* Qt macro
    Q_OBJECT

    public:

    //* return codes
    enum class ReturnCode
    {

        SaveAgain,
        SaveAs,
        Reload,
        Ignore

    };

    //* constructor
    explicit FileModifiedWidget( QWidget* = nullptr, const File& = File() );

    //*@ modifiers
    //@{

    void setFile( const File& );

    //@}

    Q_SIGNALS:

    //* emitted when a given action is selected
    void actionSelected( FileModifiedWidget::ReturnCode );

    private:

    //* re-saved removed file
    void _reLoad();

    //* re-saved removed file
    void _reSave();

    //* save file with new name
    void _saveAs();

    //* save file with new name
    void _ignore();

};

#endif
