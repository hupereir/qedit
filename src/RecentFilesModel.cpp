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

#include "RecentFilesModel.h"
#include "MimeTypeIconProvider.h"

//__________________________________________________________________
RecentFilesModel::RecentFilesModel( QObject* parent ):
    FileRecordModel( parent ),
    mimeTypeIconProvider_( new MimeTypeIconProvider( this ) )
    {}


//__________________________________________________________________
QVariant RecentFilesModel::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    if( showIcons() && role == Qt::DecorationRole && index.column() == Filename )
    {
        // retrieve associated file info
        const FileRecord& record( get(index) );
        const QIcon icon( mimeTypeIconProvider_->icon( record.file().extension() ) );

        if( !icon.isNull() ) return icon;
        else return FileRecordModel::data( index, role );


    }

    return FileRecordModel::data( index, role );

}
