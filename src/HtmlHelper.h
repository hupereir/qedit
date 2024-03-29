#ifndef HtmlHelper_h
#define HtmlHelper_h
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
#include "Debug.h"

#include <QObject>
#include <QIODevice>

#include <QDomDocument>
#include <QDomElement>

class TextDisplay;

//* printing utility
class HtmlHelper: public QObject, private Base::Counter<HtmlHelper>
{

    Q_OBJECT

    public:

    //* constructor
    explicit HtmlHelper( QObject* parent, TextDisplay* editor ):
        QObject( parent ),
        Counter( QStringLiteral("HtmlHelper") ),
        editor_( editor )
    { Debug::Throw( QStringLiteral("HtmlHelper::HtmlHelper.\n") ); }

    //* print
    void print( QIODevice* );

    protected:

    //* get full html string
    QString _htmlString();

    //* get documents (editor) Html node
    QDomElement _htmlNode( QDomDocument& );

    private:

    //* editor
    TextDisplay* editor_ = nullptr;

};

#endif
