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

#include "SessionFilesModel.h"

#include "CustomPixmap.h"
#include "FileRecordProperties.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Singleton.h"
#include "XmlFileList.h"
#include "XmlFileRecord.h"
#include "XmlOptions.h"
#include "QOrderedSet.h"

#include <QtCore/QMimeData>
#include <QtCore/QSet>
#include <QtGui/QPalette>
#include <algorithm>
#include <cassert>

//______________________________________________________________
const QString SessionFilesModel::DRAG = "base/sessionfilesmodel/drag";

//__________________________________________________________________
SessionFilesModel::IconCache& SessionFilesModel::_icons( void )
{
    static IconCache cache;
    return cache;
}

//__________________________________________________________________
SessionFilesModel::SessionFilesModel( QObject* parent ):
    FileRecordModel( parent )
{

    Debug::Throw("SessionFilesModel::SessionFilesModel.\n" );
    setShowIcons( false );
    connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );

}

//__________________________________________________________________
Qt::ItemFlags SessionFilesModel::flags(const QModelIndex &index) const
{

    // get flags from parent class
    Qt::ItemFlags flags( FileRecordModel::flags( index ) );
    return flags | Qt::ItemIsDropEnabled;

}

//__________________________________________________________________
QVariant SessionFilesModel::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    // retrieve associated file info
    const FileRecord& record( get(index) );

    if( role == Qt::DecorationRole && index.column() == ICON )
    {

        if( record.hasFlag( FileRecordProperties::MODIFIED ) ) return _icon( FileRecordProperties::MODIFIED );
        else if( record.hasFlag( FileRecordProperties::ALTERED ) ) return _icon( FileRecordProperties::ALTERED );
        else return _icon( FileRecordProperties::NONE );

    } else if( role == Qt::ForegroundRole ) {

        return record.hasFlag( FileRecordProperties::ACTIVE ) ?
            QPalette().color( QPalette::Text ):
            QPalette().color( QPalette::Disabled, QPalette::Text );

    } else return FileRecordModel::data( index, role );

    return QVariant();

}

//____________________________________________________________
void SessionFilesModel::_updateConfiguration( void )
{
    Debug::Throw( "SessionFilesModel::_updateConfiguration.\n" );
    _icons().clear();
}

//________________________________________________________
const QIcon& SessionFilesModel::_icon( unsigned int type )
{

    IconCache::const_iterator iter( _icons().find( type ) );
    if( iter != _icons().end() ) return iter.value();

    QIcon icon;
    if( type == FileRecordProperties::MODIFIED ) icon = IconEngine::get( ICONS::SAVE );
    else if( type == FileRecordProperties::ALTERED ) icon = IconEngine::get( ICONS::WARNING );
    else if( type == FileRecordProperties::NONE )
    {

        const int iconSize( XmlOptions::get().get<unsigned int>( "LIST_ICON_SIZE" ) );
        QSize size( iconSize, iconSize );

        // update size to match real icon
        QIcon prototype( IconEngine::get( ICONS::SAVE ) );
        if( !prototype.isNull() ) size = prototype.actualSize( size );

        // create empty pixmap
        icon = CustomPixmap().empty( size );

    }

    // store in map and return
    return _icons().insert( type, icon ).value();

}

//______________________________________________________________________
QStringList SessionFilesModel::mimeTypes( void ) const
{
    QStringList types;
    types << DRAG;
    return types;
}

//______________________________________________________________________
QMimeData* SessionFilesModel::mimeData(const QModelIndexList &indexes) const
{

    // return FileRecordModel::mimeData( indexes );
    QOrderedSet<QString> filenames;
    QOrderedSet<FileRecord> records;
    foreach( const QModelIndex& index, indexes )
    {

        if( !index.isValid() ) continue;
        FileRecord record( get(index) );
        records.insert( record );
        filenames.insert( record.file() );

    }

    if( filenames.empty() ) return 0;
    else {

        QMimeData *mime = new QMimeData();

        // fill text data
        QString full_text;
        QTextStream buffer( &full_text );
        foreach( const QString& filename, filenames )
        { buffer << filename << endl; }

        mime->setText( full_text );

        // fill DRAG data. Use XML
        QDomDocument document;
        QDomElement top = document.appendChild( document.createElement( XmlFileRecord::XML_FILE_LIST ) ).toElement();

        foreach( const FileRecord& record, records )
        {

            if( record.file().isEmpty() ) continue;
            top.appendChild( XmlFileRecord( record ).domElement( document ) );

        }
        mime->setData( DRAG, document.toByteArray() );
        return mime;

    }

}

//__________________________________________________________________
bool SessionFilesModel::dropMimeData(const QMimeData* data , Qt::DropAction action, int, int, const QModelIndex& parent)
{

    // check action
    if( action == Qt::IgnoreAction) return true;

    // Drag from Keyword model
    if( !data->hasFormat( DRAG ) ) return false;

    FileRecordModel::List records;

    // get dropped file record (use XML)
    // dom document
    QDomDocument document;
    if( !document.setContent( data->data( DRAG ), false ) ) return false;

    QDomElement doc_element = document.documentElement();
    QDomNode node = doc_element.firstChild();
    for(QDomNode node = doc_element.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        QDomElement element = node.toElement();
        if( element.isNull() ) continue;

        // special options
        if( element.tagName() == XmlFileRecord::XML_RECORD )
        {

            XmlFileRecord record( element );
            if( !record.file().isEmpty() ) records << record;

        }
    }

    // get current record
    if( parent.isValid() )
    {

        FileRecord target( get( parent ) );

        // loop over sources and emit proper signal
        foreach( const FileRecord& record, records )
        { emit reparentFiles( record.file(), target.file() ); }

        return true;

    } else {

        // look for first active file in this window
        FileRecord target;
        QModelIndex targetIndex;
        for( int row = 0; row < rowCount(); row++ )
        {

            QModelIndex index( SessionFilesModel::index( row, 0 ) );
            if( flags( index ) & Qt::ItemIsEnabled )
            {
                FileRecord record( get( index ) );
                if( record.hasFlag( FileRecordProperties::ACTIVE ) )
                {
                    targetIndex = index;
                    target = record;
                    break;
                }

            }
        }

        // check that targetIndex is valid
        if( !targetIndex.isValid() ) return false;

        // emit relevant reparent signal
        foreach( const FileRecord& record, records )
        { emit reparentFilesToMain( record.file(), target.file() ); }

        return true;

    }

}
