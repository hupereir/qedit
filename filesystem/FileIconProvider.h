#ifndef FileIconProvider_h
#define FileIconProvider_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "Counter.h"
#include "FileRecord.h"
#include "MimeTypeIconProvider.h"

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtGui/QIcon>

//! icon provider for file records
class FileIconProvider : public QObject, public Counter
{

    public:

    //! constructor
    FileIconProvider( QObject* parent = 0x0 ):
        QObject( parent ),
        Counter( "FileIconProvider" )
    {}

    //! destructor
    virtual ~FileIconProvider()
    {}

    //! icon matching given model index
    virtual const QIcon& icon( const FileRecord& );

    //! clear
    virtual void clear( void )
    { icons_.clear(); }

    class Key
    {
        public:

        //! constructor
        Key( const QString& extension = QString(), int type = 0 ):
            extension_( extension ),
            type_( type )
        {}

        //! equal to operator
        bool operator == (const Key& other ) const
        { return type_ == other.type_ && extension_ == other.extension_; }

        //! extension
        const QString& extension( void ) const
        { return extension_; }

        //! type
        int type( void ) const
        { return type_; }

        private:

        //! extension
        QString extension_;

        //! type
        int type_;

    };

    private:

    //! mime type icon provider
    MimeTypeIconProvider mimeTypeIconProvider_;

    //! icon cache
    typedef QHash<Key, QIcon> IconCache;

    //! icon cache
    IconCache icons_;

};

//! hash
inline unsigned int qHash( const FileIconProvider::Key& key )
{ return key.type() | (qHash( key.extension() ) << 4 ); }

#endif