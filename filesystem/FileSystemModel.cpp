
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

#include "FileSystemModel.h"

#include "CustomPixmap.h"
#include "FileSystemIcons.h"
#include "FileRecordProperties.h"
#include "IconEngine.h"
#include "Singleton.h"
#include "XmlOptions.h"

#include <algorithm>
#include <cassert>

//_______________________________________________
FileSystemModel::IconCache& FileSystemModel::_icons( void )
{
    static IconCache cache;
    return cache;
}

//__________________________________________________________________
FileSystemModel::FileSystemModel( QObject* parent ):
    ListModel<FileRecord>( parent ),
    Counter( "FileSystemModel" ),
    useLocalNames_( true ),
    showIcons_( true ),
    sizePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::SIZE ) )
{
    Debug::Throw("FileSystemModel::FileSystemModel.\n" );

    columnTitles_
        << "File"
        << "Size"
        << "Time";

    connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );

}

//__________________________________________________________________
Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{

    // default flags
    Qt::ItemFlags flags;
    if( index.isValid() )
    {

        // check associated record validity
        const FileRecord& record( get(index) );
        if( record.isValid() ) flags |=  Qt::ItemIsEnabled |  Qt::ItemIsSelectable;

    }

    return flags;

}

//__________________________________________________________________
QVariant FileSystemModel::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    // retrieve associated file info
    const FileRecord& record( get(index) );

    // return text associated to file and column
    if( role == Qt::DisplayRole ) {

        switch( index.column() )
        {

            case FILE:
            {
                // store local nmae
                const QString localName( useLocalNames_ ? record.file().localName(): record.file() );

                // loop over previous rows to find a match and increment version number
                unsigned int version( 0 );
                for( int row = 0; row < index.row(); row++ )
                {
                    const QString rowName( useLocalNames_ ?
                        get( this->index( row, FILE ) ).file().localName() :
                        get( this->index( row, FILE ) ).file() );
                    if( localName == rowName ) version++;
                }

                // form output string.
                QString buffer;
                QTextStream what( &buffer );
                what << localName;
                if( version ) what << " (" << version+1 << ")";
                return buffer;
            }

            case SIZE:
            {
                if( record.hasFlag( Document ) ) return QString( record.property( sizePropertyId_ ) );
                else return QVariant();
            }

            case TIME:
            {
                if( record.hasFlag( Document ) ) return QString( TimeStamp( record.time() ).toString() );
                else return QVariant();
            }

            default:
            return QVariant();

        }

    } else if( showIcons_ && role == Qt::DecorationRole && index.column() == FILE ) {

        if( record.flags()&Navigator ) return _icons()[Navigator];
        else return _icons()[record.flags()&Any];

    }

    return QVariant();

}

//__________________________________________________________________
QVariant FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if(
        orientation == Qt::Horizontal &&
        role == Qt::DisplayRole &&
        section >= 0 &&
        section < (int) columnTitles_.size() )
    { return columnTitles_[section]; }

    // return empty
    return QVariant();

}


//_________________________________________________________
void FileSystemModel::_updateConfiguration( void )
{

    Debug::Throw( "FileSystemModel::_updateConfiguration.\n" );

    // install pixmaps
    _icons().clear();
    _installIcons();
    reset();

}

//____________________________________________________________
void FileSystemModel::_sort( int column, Qt::SortOrder order )
{ std::sort( _get().begin(), _get().end(), SortFTor( column, order, columnTitles_ ) ); }

//________________________________________________________
FileSystemModel::SortFTor::SortFTor( const int& type, Qt::SortOrder order, const QStringList& column_titles ):
    ItemModel::SortFTor( type, order ),
    sizePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::SIZE ) ),
    columnTitles_( column_titles )
{}

//________________________________________________________
bool FileSystemModel::SortFTor::operator () ( FileRecord first, FileRecord second ) const
{

    if( first.hasFlag( Navigator ) ) return true;
    if( second.hasFlag( Navigator ) ) return false;
    if( first.hasFlag( Folder ) && second.hasFlag( Document ) ) return true;
    if( second.hasFlag( Folder ) && first.hasFlag( Document ) ) return false;

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );

    switch( type_ )
    {

        case FILE:
        {
            if( first.hasFlag( Navigator ) ) return true;
            if( second.hasFlag( Navigator ) ) return false;
            if( first.hasFlag( Folder ) && second.hasFlag( Document ) ) return true;
            if( second.hasFlag( Folder ) && first.hasFlag( Document ) ) return false;
            return first.file().localName().compare( second.file().localName(), Qt::CaseInsensitive ) < 0;

        }

        case TIME: return (first.time() != second.time() ) ? first.time() < second.time() : first.file().localName() < second.file().localName();
        case SIZE:
        {
            long first_size( first.property( sizePropertyId_ ).toInt() );
            long second_size( second.property( sizePropertyId_ ).toInt() );
            return (first_size != second_size ) ? first_size < second_size : first.file().localName() < second.file().localName();
        }

        default: return true;

    }

}

//_____________________________________________________________________
void FileSystemModel::_installIcons( void ) const
{

    Debug::Throw( "FileSystemModel::_installIcons.\n" );

    if( !_icons().empty() ) return;

    // type icons
    typedef QHash< int, QString > IconNames;
    IconNames typeNames;
    typeNames[Document] = ICONS::DOCUMENT;
    typeNames[Folder] = ICONS::FOLDER;
    typeNames[Navigator] = ICONS::PARENT;

    // standard icons
    for( IconNames::iterator iter = typeNames.begin(); iter != typeNames.end(); ++iter )
    { _icons().insert( iter.key(), IconEngine::get( iter.value() ) ); }

    // link icons
    QIcon linkOverlay = IconEngine::get( ICONS::LINK );
    IconCache linkIcons;
    for( IconCache::const_iterator iter = _icons().constBegin(); iter != _icons().end(); ++iter )
    {

        QIcon icon;
        const QIcon& base( iter.value() );
        foreach( const QSize& size, base.availableSizes() )
        {

            QSize overlaySize;
            if( size.width() <= 16 ) overlaySize = QSize( 8, 8 );
            else if( size.width() <= 22 ) overlaySize = QSize( 8, 8 );
            else if( size.width() <= 32 ) overlaySize = QSize( 12, 12 );
            else if( size.width() <= 48 ) overlaySize = QSize( 16, 16 );
            else if( size.width() <= 64 ) overlaySize = QSize( 22, 22 );
            else if( size.width() <= 128 ) overlaySize = QSize( 48, 48 );
            else overlaySize = QSize( 64, 64 );

            icon.addPixmap( CustomPixmap( base.pixmap( size ) ).merge( linkOverlay.pixmap( overlaySize ), CustomPixmap::BOTTOM_RIGHT ) );
        }

        linkIcons.insert( iter.key() | Link, icon );
    }

    _icons().unite( linkIcons );

}
