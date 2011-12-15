#ifndef PrintHelper_h
#define PrintHelper_h

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
#include <QtGui/QPainter>
#include <QtGui/QPrinter>

class TextDisplay;

//! printing utility
class PrintHelper: public QObject, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    PrintHelper( QObject* parent, TextDisplay* editor ):
        QObject( parent ),
        Counter( "PrintHelper" ),
        editor_( editor )
    { Debug::Throw( "PrintHelper::PrintHelper.\n" ); }

    //! destructor
    virtual ~PrintHelper( void )
    {}

    public slots:

    //! print
    void print( QPrinter* );

    protected:

    //! print page header to a given printer
    void _printHeader( QPainter*, const QRect&, int ) const;

    //! print page footer to a given printer
    void _printFooter( QPainter*, const QRect&, int ) const;

    private:

    //! editor
    TextDisplay* editor_;

};

#endif
