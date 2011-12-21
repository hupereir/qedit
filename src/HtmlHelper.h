#ifndef PrintHelper_h
#define HtmlHelper_h

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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
****************************************************************************/

#include "Counter.h"
#include "Debug.h"

#include <QtCore/QObject>
#include <QtCore/QIODevice>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class TextDisplay;

//! printing utility
class HtmlHelper: public QObject, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    HtmlHelper( QObject* parent, TextDisplay* editor ):
        QObject( parent ),
        Counter( "HtmlHelper" ),
        editor_( editor )
    { Debug::Throw( "HtmlHelper::HtmlHelper.\n" ); }

    //! destructor
    virtual ~HtmlHelper( void )
    {}

    public slots:

    //! print
    void print( QIODevice* );

    protected:

    //! get full html string
    QString _htmlString( void );

    //! get documents (editor) Html node
    QDomElement _htmlNode( QDomDocument& );

    private:

    //! editor
    TextDisplay* editor_;

};

#endif
