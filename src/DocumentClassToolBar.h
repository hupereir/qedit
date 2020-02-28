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

#include "ToolBar.h"

#include <QComboBox>

//* display available document classes
class DocumentClassToolBar: public ToolBar
{

    Q_OBJECT

    public:

    //* constructor
    explicit DocumentClassToolBar( QWidget* );

    //* update selection
    void update( const QString &);

    Q_SIGNALS:

    //* emitted every time a document class is selected
    void documentClassSelected( QString );

    private:

    //* update list
    void _update();

    //* current index changed
    void _currentIndexChanged( int );

    //* document class selection combo box
    QComboBox& _comboBox() const
    { return *combobox_; }

    //* document class selection combo box
    QComboBox* combobox_ = nullptr;

    //* current class
    QString currentClass_ = nullptr;

};

#endif
