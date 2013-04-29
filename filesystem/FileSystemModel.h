#ifndef _FileSystemModel_h_
#define _FileSystemModel_h_

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
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "Counter.h"
#include "Debug.h"
#include "FileRecord.h"
#include "ListModel.h"

#include <QString>
#include <QStringList>
#include <QHash>

class FileIconProvider;

//! qlistview for object counters
class FileSystemModel: public ListModel<FileRecord>, public Counter
{

    Q_OBJECT

    public:

    //! file types
    enum FileType
    {
        Document = 1<<0,
        Folder = 1<<1,
        Link = 1<<2,
        Navigator = 1<<3,
        Hidden = 1<<4,
        Any = Document | Folder | Link | Navigator | Hidden
    };

    Q_DECLARE_FLAGS( FileTypes, FileType );

    //! number of columns
    enum { nColumns = 3 };

    //! column type enumeration
    enum ColumnType {
        FILE,
        SIZE,
        TIME
    };

    //! constructor
    FileSystemModel( QObject* parent = 0 );

    //! destructor
    virtual ~FileSystemModel( void )
    {}

    //! show icons
    void setShowIcons( const bool& value )
    { showIcons_ = value; }

    //! use local names
    void setUseLocalNames( const bool& value )
    { useLocalNames_ = value; }

    //!@name methods reimplemented from base class
    //@{

    //! flags
    virtual Qt::ItemFlags flags( const QModelIndex& ) const;

    // return data for a given index
    virtual QVariant data( const QModelIndex&, int ) const;

    //! header data
    virtual QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const;

    //! number of columns for a given index
    virtual int columnCount( const QModelIndex& = QModelIndex() ) const
    { return columnTitles_.size(); }

    //@}

    protected:

    //! sort
    virtual void _sort( int, Qt::SortOrder = Qt::AscendingOrder );

    private:

    //! icon provider
    FileIconProvider* iconProvider_;

    //! used to sort Counters
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //! constructor
        SortFTor( const int&, Qt::SortOrder, const QStringList& );

        //! prediction
        bool operator() ( FileRecord, FileRecord ) const;

        private:

        //! size property id
        FileRecord::PropertyId::Id sizePropertyId_;

        //! column titles
        QStringList columnTitles_;

    };

    //! local names
    bool useLocalNames_;

    //! true if icons are to be shown
    bool showIcons_;

    //! column titles
    QStringList columnTitles_;

    //! size property id
    FileRecord::PropertyId::Id sizePropertyId_;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( FileSystemModel::FileTypes );

#endif
