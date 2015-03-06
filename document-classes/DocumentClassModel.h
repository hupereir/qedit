#ifndef DocumentClassModel_h
#define DocumentClassModel_h

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
#include "ListModel.h"
#include "DocumentClass.h"

#include <QIcon>
#include <QHash>

//! DocumentClass model. Stores file information for display in lists
class DocumentClassModel : public ListModel<DocumentClass, DocumentClass::WeakEqualFTor, DocumentClass::WeakLessThanFTor>, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT;

    public:

    //! constructor
    DocumentClassModel(QObject *parent = 0);

    //! destructor
    virtual ~DocumentClassModel()
    {}

    //! column type enumeration
    enum ColumnType
    {
        Name,
        Filename,
        nColumns
    };

    //!@name methods reimplemented from base class
    //@{

    //! flags
    virtual Qt::ItemFlags flags( const QModelIndex& ) const;

    // return data for a given index
    virtual QVariant data( const QModelIndex&, int ) const;

    //! header data
    virtual QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole) const;

    //! number of columns for a given index
    virtual int columnCount(const QModelIndex& = QModelIndex() ) const
    { return nColumns; }

    //@}

    protected:

    //! sort
    virtual void _sort( int, Qt::SortOrder = Qt::AscendingOrder );

    private Q_SLOTS:

    //! configuration
    void _updateConfiguration( void );

    private:

    //! list column names
    static const QString columnTitles_[nColumns];

    //! used to sort IconCaches
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //! constructor
        SortFTor( const int& type, Qt::SortOrder order = Qt::AscendingOrder ):
            ItemModel::SortFTor( type, order )
        {}

        //! prediction
        bool operator() ( const DocumentClass&, const DocumentClass& ) const;

    };

    //! icon
    static const QIcon& _icon( const QString& );

    //! icon cache
    using IconCache = QHash<QString, QIcon>;

    //! type icon cache
    static IconCache& _icons( void );

};

#endif
