
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
\file FileSystemModel.cpp
\brief model for object records
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "CustomPixmap.h"
#include "FileSystemIcons.h"
#include "FileRecordProperties.h"
#include "FileSystemModel.h"
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
                if( record.hasFlag( DOCUMENT ) ) return QString( record.property( sizePropertyId_ ) );
                else return QVariant();
            }

            case TIME:
            {
                if( record.hasFlag( DOCUMENT ) ) return QString( TimeStamp( record.time() ).toString() );
                else return QVariant();
            }

            default:
            return QVariant();

        }

    } else if( showIcons_ && role == Qt::DecorationRole && index.column() == FILE ) {

        return _icons()[record.flags()&ANY];

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

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );

    if( first.hasFlag( NAVIGATOR ) ) return true;
    if( second.hasFlag( NAVIGATOR ) ) return false;
    if( first.hasFlag( FOLDER ) && second.hasFlag( DOCUMENT ) ) return true;
    if( second.hasFlag( FOLDER ) && first.hasFlag( DOCUMENT ) ) return false;

    switch( type_ )
    {

        case FILE:
        {
            if( first.hasFlag( NAVIGATOR ) ) return true;
            if( second.hasFlag( NAVIGATOR ) ) return false;
            if( first.hasFlag( FOLDER ) && second.hasFlag( DOCUMENT ) ) return true;
            if( second.hasFlag( FOLDER ) && first.hasFlag( DOCUMENT ) ) return false;
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

    // pixmap size
    unsigned int pixmap_size = XmlOptions::get().get<unsigned int>( "LIST_ICON_SIZE" );
    QSize size( pixmap_size, pixmap_size );
    QSize scale(size*0.9);

    // type icons
    typedef QHash< int, QString > IconNames;
    IconNames type_names;
    type_names[DOCUMENT] = ICONS::DOCUMENT;
    type_names[FOLDER] = ICONS::FOLDER;
    type_names[NAVIGATOR] = ICONS::PARENT_DIRECTORY;

    // load link pixmap
    CustomPixmap link = CustomPixmap().find( ICONS::LINK );

    for( IconNames::iterator iter = type_names.begin(); iter != type_names.end(); ++iter )
    {

        _icons()[iter.key()] = CustomPixmap()
            .empty( size )
            .merge( CustomPixmap().find( iter.value() )
            .scaled( scale, Qt::KeepAspectRatio, Qt::SmoothTransformation ), CustomPixmap::CENTER );

        _icons()[iter.key() | LINK] = CustomPixmap()
            .empty( size )
            .merge( CustomPixmap().find( iter.value() )
            .merge( link, CustomPixmap::BOTTOM_LEFT )
            .scaled( scale, Qt::KeepAspectRatio, Qt::SmoothTransformation ), CustomPixmap::CENTER );

    }

    return;
}
