#ifndef DocumentClassToolBar_h
#define DocumentClassToolBar_h
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

#include "CustomToolBar.h"

#include <QComboBox>

//* display available document classes
class DocumentClassToolBar: public CustomToolBar
{

    Q_OBJECT

    public:

    //* constructor
    DocumentClassToolBar( QWidget* );

    //* destructor
    virtual ~DocumentClassToolBar( void ) = default;

    //* update selection
    void update( QString );

    Q_SIGNALS:

    //* emmited every time a document class is selected
    void documentClassSelected( QString );

    private Q_SLOTS:

    //* update list
    void _update( void );

    //* current index changed
    void _currentIndexChanged( int );

    private:

    //* document class selection combo box
    QComboBox& _comboBox( void ) const
    { return *combobox_; }

    //* document class selection combo box
    QComboBox* combobox_ = nullptr;

    //* current class
    QString currentClass_ = nullptr;

};

#endif
