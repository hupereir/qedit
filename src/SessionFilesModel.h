#ifndef SessionFilesModel_h
#define SessionFilesModel_h

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

#include "Debug.h"
#include "FileRecordModel.h"

#include <QHash>

//* qlistview for object counters
class SessionFilesModel: public FileRecordModel
{

    Q_OBJECT

    public:

    //* constructor
    explicit SessionFilesModel( QObject* parent = nullptr );

    //*@name accessors
    //@{

    //* flags
    Qt::ItemFlags flags(const QModelIndex& ) const override;

    //* return data for a given index
    QVariant data(const QModelIndex &, int ) const override;

    //* mime type
    QStringList mimeTypes() const override;

    //* mime data
    QMimeData* mimeData(const QModelIndexList &indexes) const override;

    //* drop mine data
    bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex&) override;

    //@}

    Q_SIGNALS:

    //* file reorganization
    void reparentFiles( File, File );

    //* file reorganization
    void reparentFilesToMain( File, File );

    //* file open
    void requestOpen( FileRecord );

    private:

    //* configuration
    void _updateConfiguration();

    //* icon
    static const QIcon& _icon( int );

    //* icon cache
    using IconCache = QHash<int, QIcon>;

    //* type icon cache
    static IconCache& _icons();

};

#endif
