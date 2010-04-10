#ifndef _SessionFilesModel_h_
#define _SessionFilesModel_h_

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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file SessionFilesModel.h
  \brief model for object counters
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <string.h>


#include "FileRecordModel.h"
#include "Debug.h"

//! qlistview for object counters
class SessionFilesModel: public FileRecordModel
{

    Q_OBJECT

    public:

    //! constructor
    SessionFilesModel( QObject* parent = 0 );

    //! destructor
    virtual ~SessionFilesModel( void )
    {}

    //! drag mime type
    static const QString DRAG;

    //!@name methods reimplemented from base class
    //@{

    //! flags
    virtual Qt::ItemFlags flags(const QModelIndex& ) const;

    //! return data for a given index
    virtual QVariant data(const QModelIndex &, int ) const;

    //! mime type
    virtual QStringList mimeTypes( void ) const;

    //! mime data
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;

    //! drop mine data
    virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex&);

    //@}

    signals:

    //! file reorganization
    void reparentFiles( const File&, const File& );

    //! file reorganization
    void reparentFilesToMain( const File&, const File& );

    private slots:

    //! configuration
    void _updateConfiguration( void );

    private:

    //! icon
    static QIcon _icon( unsigned int );

    //! icon cache
    typedef std::map<unsigned int, QIcon> IconCache;

    //! type icon cache
    static IconCache& _icons( void );

};

#endif
