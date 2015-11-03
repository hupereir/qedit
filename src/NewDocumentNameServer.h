#ifndef NewDocumentNameServer_h
#define NewDocumentNameServer_h

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

#include <QString>
#include <QList>

class NewDocumentNameServer
{

    public:

    //* constructor
    NewDocumentNameServer( void )
    {}

    //* get name
    QString get( void );

    //* remove (this is used to avoid that version number is incremented too quickly)
    void remove( QString );

    private:

    //* generate file name matching a given version
    static QString _get( int );

    class SameVersionFTor
    {
        public:

        //* constructor
        SameVersionFTor( const QString& name ):
            name_( name )
         {}

        //* predicate
        bool operator() (int version ) const
        { return NewDocumentNameServer::_get( version ) == name_; }

        private:

        //* prediction
        QString name_;

    };

    //* default name
    static const QString defaultName_;

    // version (appended)
    QList<int> versions_;

};

#endif
