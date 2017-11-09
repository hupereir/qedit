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

#include "Functors.h"

#include <QString>
#include <QList>

class NewDocumentNameServer
{

    public:

    //* get name
    QString get();

    //* remove (this is used to avoid that version number is incremented too quickly)
    void remove( QString );

    private:

    //* version
    class Version
    {

        public:

        using List = QList<Version>;

        //* constructor
        explicit Version( int value ):
            value_( value )
        {}

        //*@name accessors
        //@{

        //* name
        QString name() const;

        //* value
        int value() const
        { return value_; }

        //@}

        private:

        //* version number
        int value_ = 0;

    };

    //* generate file name matching a given version
    static QString _get( int );

    //* use to remove versions matching a given name
    using SameVersionFTor = Base::Functor::Unary<Version, QString, &Version::name>;

    //* default name
    static const QString defaultName_;

    // version (appended)
    Version::List versions_;

};

#endif
