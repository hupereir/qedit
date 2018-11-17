#ifndef FileRemovedWidget_h
#define FileRemovedWidget_h

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

#include "Counter.h"
#include "CppUtil.h"
#include "File.h"
#include "Key.h"

#include <QWidget>

//* QDialog used when a file has been removed from disk
class FileRemovedWidget: public QWidget, public Base::Key, private Base::Counter<FileRemovedWidget>
{

    //* Qt macro
    Q_OBJECT

    public:

    //* return codes
    enum class ReturnCode
    {

        SaveAgain,
        SaveAs,
        Close,
        Ignore

    };

    //* constructor
    explicit FileRemovedWidget( QWidget* parent, const File& file );

    Q_SIGNALS:

    //* emitted when a given action is selected
    void actionSelected( FileRemovedWidget::ReturnCode );

    private Q_SLOTS:

    //* re-saved removed file
    void _reSave()
    {
        emit actionSelected( ReturnCode::SaveAgain );
        deleteLater();
    }

    //* save file with new name
    void _saveAs()
    {
        emit actionSelected( ReturnCode::SaveAs );
        deleteLater();
    }

    //* close current editor
    void _close()
    {
        emit actionSelected( ReturnCode::Close );
        deleteLater();
    }

    //* ignore warning
    void _ignore()
    {
        emit actionSelected( ReturnCode::Ignore );
        deleteLater();
    }

};

#endif
