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

#include "SessionFilesView.h"

#include "XmlFileRecord.h"

#include <QDomDocument>
#include <QDrag>
#include <QMimeData>

//____________________________________________________________________
void SessionFilesView::startDrag( Qt::DropActions supportedActions )
{

    Debug::Throw( "SessionFilesView::startDrag.\n" );

    // get list of dragable indexes
    QModelIndexList indexes;
    foreach( const QModelIndex& index, selectionModel()->selectedIndexes() )
    { if( model()->flags( index ) & Qt::ItemIsDragEnabled ) indexes << index; }
    if( indexes.isEmpty() ) return;

    // get mime data
    QMimeData* data = model()->mimeData( indexes );
    if( !data ) return;

    // create drag
    QDrag *drag = new QDrag(this);
    drag->setMimeData(data);
    if( !drag->exec( supportedActions, defaultDropAction() ) )
    {

        // get dropped file record (use XML)
        // dom document
        QDomDocument document;
        if( !document.setContent( data->data( FileRecord::MimeType ), false ) ) return;
        const XmlFileRecord::List records( document.documentElement() );

        if( records.empty() ) return;
        const File target( records.front().file() );
        bool first( true );
        foreach( const XmlFileRecord& record, records )
        {
            if( first )
            {
                first = false;
                emit detach( record.file() );
            } else emit reparentFilesToMain( record.file(), target );
        }

    }

}
