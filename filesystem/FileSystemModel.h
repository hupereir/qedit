#ifndef FileSystemModel_h
#define FileSystemModel_h

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

#include <QHash>
#include <QMimeData>
#include <QStringList>

class FileIconProvider;

//* qlistview for object counters
class FileSystemModel: public ListModel<FileRecord>, private Base::Counter<FileSystemModel>
{

    Q_OBJECT

    public:

    //* column type enumeration
    enum ColumnType
    {
        Filename,
        Size,
        Time,
        nColumns
    };

    //* constructor
    explicit FileSystemModel( QObject* = nullptr );

    //*@name accessors
    //@{

    //* flags
    Qt::ItemFlags flags( const QModelIndex& ) const override;

    // return data for a given index
    QVariant data( const QModelIndex&, int ) const override;

    //* header data
    QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const override;

    //* number of columns for a given index
    int columnCount( const QModelIndex& = QModelIndex() ) const override
    { return columnTitles_.size(); }

    //* mime data
    QMimeData* mimeData(const QModelIndexList &indexes) const override;

    //@}

    //*@name modifiers
    //@{

    //* show icons
    void setShowIcons( bool value )
    { showIcons_ = value; }

    //* use local names
    void setUseLocalNames( bool value )
    { useLocalNames_ = value; }

    //@}

    protected:

    //* sort
    void _sort( int column, Qt::SortOrder order ) override;

    private:

    //* icon provider
    FileIconProvider* iconProvider_ = nullptr;

    //* used to sort Counters
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //* constructor
        explicit SortFTor( int, Qt::SortOrder, const QStringList& );

        //* prediction
        bool operator() ( FileRecord, FileRecord ) const;

        private:

        //* size property id
        FileRecord::PropertyId::Id sizePropertyId_;

        //* column titles
        QStringList columnTitles_;

    };

    //* local names
    bool useLocalNames_ = true;

    //* true if icons are to be shown
    bool showIcons_ = true;

    //* column titles
    QStringList columnTitles_;

    //* size property id
    FileRecord::PropertyId::Id sizePropertyId_ = 0;

};

#endif
