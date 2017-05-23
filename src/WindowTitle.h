#ifndef WindowTitle_h
#define WindowTitle_h

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
#include "File.h"

#include <QString>

class WindowTitle: private Base::Counter<WindowTitle>
{

    public:

    //* Flags
    enum Flags
    {
        None,
        Modified,
        ReadOnly
    };

    //* constructor
    WindowTitle( const File& file = File(), const Flags& flag = None ):
        Counter( "WindowTitle" ),
        file_( file ),
        flag_( flag )
    {}

    //* change flag
    WindowTitle& setFlag( const Flags& flag, bool value = true )
    {
        if( value ) flag_ = flag;
        return *this;
    }

    //* change flag
    WindowTitle& setModified( bool value = true )
    { return setFlag( Modified, value ); }

    //* change flag
    WindowTitle& setReadOnly( bool value = true )
    { return setFlag( ReadOnly, value ); }

    //* cast to string
    operator const QString& (void)
    {
        QString out;
        QTextStream what( &out );
        if( file_.size() ) what << file_.localName();
        else what << "QEdit";
        if( flag_ == Modified ) what << QObject::tr( " (modified)" );
        if( flag_ == ReadOnly ) what << QObject::tr( " (read-only)" );
        if( file_.size() ) what << " - " << file_.path();
        return title_ = out;
    }

    private:

    //* filename
    File file_;

    //* flags
    Flags flag_;

    //* stored string
    QString title_;

};

#endif
