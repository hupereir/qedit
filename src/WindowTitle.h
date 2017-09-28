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
#include "Util.h"

#include <QString>

class WindowTitle final: private Base::Counter<WindowTitle>
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
    explicit WindowTitle( const File& file = File(), const Flags& flag = None ):
        Counter( "WindowTitle" ),
        file_( file ),
        flag_( flag )
    {}

    //*@name accessors
    //@{

    //* cast to string
    QString get() const
    {
        QString out;
        QTextStream what( &out );
        if( !file_.isEmpty() ) what << file_.localName();
        else what << "QEdit";
        if( flag_ == Modified ) what << QObject::tr( " (modified)" );
        if( flag_ == ReadOnly ) what << QObject::tr( " (read-only)" );
        if( !file_.isEmpty() ) what << Util::windowTitleSeparator() << file_.path();
        return out;
    }

    //@}

    //*@name modifiers
    //@{

    //* change flag
    void setFlag( const Flags& flag, bool value = true )
    {
        if( value ) flag_ = flag;
    }

    //* change flag
    void setModified( bool value = true )
    { setFlag( Modified, value ); }

    //* change flag
    void setReadOnly( bool value = true )
    { setFlag( ReadOnly, value ); }

    //@}

    private:

    //* filename
    File file_;

    //* flags
    Flags flag_;

};

#endif
