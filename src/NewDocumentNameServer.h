#ifndef NewDocumentNameServer_h
#define NewDocumentNameServer_h

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
* software; if not, write to the Free Software , Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include <QtCore/QString>
#include <QtCore/QList>

class NewDocumentNameServer
{

    public:

    //! constructor
    NewDocumentNameServer( void )
    {}

    //! destructor
    virtual ~NewDocumentNameServer( void )
    {}


    //! get name
    QString get( void );

    //! remove (this is used to avoid that version number is incremented too quickly)
    void remove( QString );

    private:

    //! generate file name matching a given version
    static QString _get( const unsigned int& version );

    class SameVersionFTor
    {
        public:

        //! constructor
        SameVersionFTor( const QString& name ):
            name_( name )
         {}

        //! predicate
        bool operator() (const unsigned int& version ) const
        { return NewDocumentNameServer::_get( version ) == name_; }

        private:

        //! prediction
        QString name_;

    };

    //! default name
    static const QString defaultName_;

    // version (appended)
    QList<unsigned int> versions_;

};

    #endif
