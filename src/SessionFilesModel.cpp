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

#include "SessionFilesModel.h"
#include "Application.h"
#include "FileRecordProperties.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "Pixmap.h"
#include "QOrderedSet.h"
#include "Singleton.h"
#include "XmlFileRecord.h"
#include "XmlOptions.h"

#include <QMimeData>
#include <QPalette>
#include <QSet>
#include <QUrl>

//__________________________________________________________________
SessionFilesModel::IconCache& SessionFilesModel::_icons()
{
    static IconCache cache;
    return cache;
}

//__________________________________________________________________
SessionFilesModel::SessionFilesModel( QObject* parent ):
    FileRecordModel( parent )
{
    Debug::Throw(QStringLiteral("SessionFilesModel::SessionFilesModel.\n") );
    setShowIcons( false );
    connect( Base::Singleton::get().application<Application>(), &Application::configurationChanged, this, &SessionFilesModel::_updateConfiguration );
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
    const auto& record( get(index) );
    if( role == Qt::DecorationRole && index.column() == FileName )
    {

        if( record.hasFlag( FileRecordProperties::Modified ) ) return _icon( FileRecordProperties::Modified );
        else if( record.hasFlag( FileRecordProperties::Altered ) ) return _icon( FileRecordProperties::Altered );
        else return _icon( FileRecordProperties::None );

    } else if( role == Qt::ForegroundRole ) {

        return record.hasFlag( FileRecordProperties::Active ) ?
            QPalette().color( QPalette::Text ):
            QPalette().color( QPalette::Disabled, QPalette::Text );

    } else return FileRecordModel::data( index, role );

    return QVariant();

}

//____________________________________________________________
void SessionFilesModel::_updateConfiguration()
{
    Debug::Throw( QStringLiteral("SessionFilesModel::_updateConfiguration.\n") );
    _icons().clear();
}

//________________________________________________________
const QIcon& SessionFilesModel::_icon( int type )
{

    IconCache::const_iterator iter( _icons().find( type ) );
    if( iter != _icons().end() ) return iter.value();

    QIcon icon;
    if( type == FileRecordProperties::Modified ) icon = IconEngine::get( IconNames::Save );
    else if( type == FileRecordProperties::Altered ) icon = IconEngine::get( IconNames::DialogWarning );
    else if( type == FileRecordProperties::None )
    {

        const int iconSize( XmlOptions::get().get<int>( QStringLiteral("LIST_ICON_SIZE") ) );
        QSize size( iconSize, iconSize );

        // update size to match real icon
        QIcon prototype( IconEngine::get( IconNames::Save ) );
        if( !prototype.isNull() ) size = prototype.actualSize( size );

        // create empty pixmap
        icon = Pixmap( size, Pixmap::Flag::Transparent );

    }

    // store in map and return
    return _icons().insert( type, icon ).value();

}

//______________________________________________________________________
QStringList SessionFilesModel::mimeTypes() const
{ return { FileRecord::MimeType, "text/uri-list" }; }

//______________________________________________________________________
QMimeData* SessionFilesModel::mimeData(const QModelIndexList &indexes) const
{

    // base class
    auto mimeData = FileRecordModel::mimeData( indexes );

    // get selected filenames
    QOrderedSet<QString> filenames;
    XmlFileRecord::List records;
    for( const auto& index:indexes )
    {

        if( !index.isValid() ) continue;
        XmlFileRecord record( get(index) );
        records.append( record );
        filenames.insert( record.file() );

    }

    if( filenames.empty() ) return mimeData;
    else {

        if( !mimeData ) mimeData = new QMimeData;

        // fill text data
        {
            QString fullText;
            QTextStream buffer( &fullText );
            for( const auto& filename:filenames )
            { buffer << QStringLiteral( "file://%1" ).arg(filename) << Qt::endl; }
            mimeData->setText( fullText );
        }

        // fill url list
        {
            QList<QUrl> urlList;
            std::transform( filenames.begin(), filenames.end(), std::back_inserter( urlList ),
                []( const QString& filename ) { return QUrl( QStringLiteral( "file://%1" ).arg(filename) ); } );
            mimeData->setUrls( urlList );
        }

        // fill drag data. Use XML
        QDomDocument document;
        XmlFileRecord::Helper::domElement( records, document );
        mimeData->setData( FileRecord::MimeType, document.toByteArray() );
        return mimeData;

    }

}

//__________________________________________________________________
bool SessionFilesModel::dropMimeData(const QMimeData* data , Qt::DropAction action, int, int, const QModelIndex& parent)
{

    // check action
    if( action == Qt::IgnoreAction) return true;

    if( data->hasFormat( FileRecord::MimeType ) )
    {

        // get dropped file record (use XML)
        // dom document
        QDomDocument document;
        if( !document.setContent( data->data( FileRecord::MimeType ), false ) ) return false;
        const XmlFileRecord::List records( XmlFileRecord::Helper::list( document.documentElement() ) );

        // get current record
        if( parent.isValid() )
        {
            const auto target( get( parent ) );

            // loop over sources and emit proper signal
            for( const auto& record:records )
            { 
                // see if source record is in model
                if( index( record ).isValid() ) emit reparentFiles( record.file(), target.file() ); 
                else emit requestOpen( record, target.file() );
            }

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
                    const auto record( get( index ) );
                    if( record.hasFlag( FileRecordProperties::Active ) )
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
            for( const auto& record:records )
            { 
                if( index( record ).isValid() ) emit reparentFilesToMain( record.file(), target.file() ); 
                else emit requestOpen( record );
            }

            return true;

        }

    } else if( data->hasUrls() ) {

        // get current record
        if( parent.isValid() )
        {
            const auto target( get( parent ) );
            
            // TODO: should check number of files
            const auto urls( data->urls() );
            for( const auto& url:urls )
            {
                // check that local file
                if( !url.isLocalFile() ) continue;
                
                // get file and check existence
                const File file( url.path() );
                if( file.exists() && !file.isDirectory() )
                { emit requestOpen( FileRecord( file ), target.file() ); }
            }
        } else {
            const auto urls( data->urls() );
            for( const auto& url:urls )
            {
                
                // check that local file
                if( !url.isLocalFile() ) continue;
                
                // get file and check existence
                const File file( url.path() );
                if( file.exists() && !file.isDirectory() )
                { emit requestOpen( FileRecord( file ) ); }
                
            }
        }
        
        return true;

    } else return false;

}
