
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

#include "FileSystemModel.h"
#include "FileSystemModel.moc"

#include "CustomPixmap.h"
#include "FileIconProvider.h"
#include "FileSystemIcons.h"
#include "FileRecordProperties.h"
#include "IconEngine.h"
#include "XmlOptions.h"

#include <QApplication>
#include <QFont>
#include <QPalette>

//__________________________________________________________________
FileSystemModel::FileSystemModel( QObject* parent ):
    ListModel<FileRecord>( parent ),
    Counter( "FileSystemModel" ),
    useLocalNames_( true ),
    showIcons_( true ),
    sizePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::SIZE ) )
{
    Debug::Throw("FileSystemModel::FileSystemModel.\n" );
    columnTitles_ <<  tr( "File" ) << tr( "Size" ) << tr( "Last Accessed" );
    iconProvider_ = new FileIconProvider( this );

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

    // return text associated to file and column
    switch( role )
    {

        case Qt::DisplayRole:
        {

            switch( index.column() )
            {

                case FILE:
                {
                    // store local nmae
                    const FileRecord& record( get(index) );
                    return useLocalNames_ ? record.file().localName(): record.file();
                }

                case SIZE:
                {
                    const FileRecord& record( get(index) );
                    if( record.hasFlag( Document ) ) return QString( record.property( sizePropertyId_ ) );
                    else return QVariant();
                }

                case TIME:
                {
                    const FileRecord& record( get(index) );
                    if( record.hasFlag( Document ) ) return QString( TimeStamp( record.time() ).toString() );
                    else return QVariant();
                }

                default:
                return QVariant();

            }

            break;
        }

        case Qt::DecorationRole:
        if( showIcons_ && index.column() == FILE )
        {

            const FileRecord& record( get(index) );
            return iconProvider_->icon( record );
        } else break;

        case Qt::ForegroundRole:
        {
            const FileRecord& record( get(index) );
            if( record.hasFlag( Hidden ) && ( this->flags( index )&Qt::ItemIsEnabled ) )
            {
                QColor color( QPalette().color( QPalette::Text ) );
                color.setAlphaF( 0.7 );
                return color;
            }

            break;
        }

        case Qt::FontRole:
        {
            const FileRecord& record( get(index) );
            if( record.hasFlag( Link ) )
            {
                QFont font( QApplication::font() );
                font.setItalic( true );
                return font;
            }

            break;
        }

        default: break;

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
