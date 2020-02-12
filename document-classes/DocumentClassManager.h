#ifndef DocumentClassManager_h
#define DocumentClassManager_h

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
#include "File.h"

#include <QString>
#include <QVector>

class DocumentClass;

//* Store list of Document Class
class DocumentClassManager final: private Base::Counter<DocumentClassManager>
{

    public:

    //* constructor
    explicit DocumentClassManager();

    //* destructor
    ~DocumentClassManager()
    { clear(); }

    //* clear document classes
    void clear();

    //* read classes from file
    bool read( const File& file );

    //* read errors
    const QString& readError() const
    { return readError_; }

    //* write all classes to file
    bool write( const File& path ) const;

    //* write classe to file
    bool write( const QString&, const File& ) const;

    //* write classe to file
    bool write( const DocumentClass&, const File& ) const;

    //* get default document class
    DocumentClass defaultClass() const;

    //* get class matching filename. Return 0 if not found
    DocumentClass find( const File& file ) const;

    //* get class matching name. Return 0 if none found
    DocumentClass get( const QString& name ) const;

    //* remove a class matching name.
    bool remove( const QString& name );

    //* shortcut to list of document classes
    using List = QVector<DocumentClass>;

    //* get all classes
    const List& classes() const
    { return documentClasses_; }

    //* set all classes
    void setClasses( const List& classes )
    { documentClasses_ = classes; }

    private:

    //* list of document classes
    List documentClasses_;

    //* read error
    QString readError_;
};

#endif
