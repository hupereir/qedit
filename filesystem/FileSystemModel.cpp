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

#include "BaseFileInfo.h"
#include "BaseFileInfoModel.h"
#include "CppUtil.h"
#include "CustomPixmap.h"
#include "FileIconProvider.h"
#include "FileSystemIconNames.h"
#include "FileRecordProperties.h"
#include "IconEngine.h"
#include "XmlOptions.h"

#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QUrl>

//__________________________________________________________________
FileSystemModel::FileSystemModel( QObject* parent ):
    ListModel( parent ),
    Counter( "FileSystemModel" ),
    columnTitles_( Base::makeT<QStringList>({ tr( "File" ), tr( "Size" ), tr( "Last Accessed" ) }) ),
    sizePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::Size ) )
{
    Debug::Throw("FileSystemModel::FileSystemModel.\n" );
    iconProvider_ = new FileIconProvider( this );
}

//__________________________________________________________________
Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{

    // default flags
    Qt::ItemFlags flags;
    if( contains( index ) )
    {

        const FileRecord& record( get(index) );
        if( record.isValid() ) flags |=  Qt::ItemIsEnabled |  Qt::ItemIsSelectable;
        if( record.isValid() && index.column() == Filename ) flags |= Qt::ItemIsDragEnabled;

    }

    return flags;

}

//__________________________________________________________________
QVariant FileSystemModel::data( const QModelIndex& index, int role ) const
{

    // check index
    if( !contains( index ) ) return QVariant();

    // return text associated to file and column
    switch( role )
    {

        case Qt::DisplayRole:
        {

            switch( index.column() )
            {

                case Filename:
                {
                    // store local nmae
                    const FileRecord& record( get(index) );
                    return useLocalNames_ ? record.file().localName().get(): record.file().get();
                }

                case Size:
                {
                    const FileRecord& record( get(index) );
                    if( record.hasFlag( BaseFileInfo::Document ) ) return QString( record.property( sizePropertyId_ ) );
                    else return QVariant();
                }

                case Time:
                {
                    const FileRecord& record( get(index) );
                    if( record.hasFlag( BaseFileInfo::Document ) ) return QString( TimeStamp( record.time() ).toString() );
                    else return QVariant();
                }

                default:
                return QVariant();

            }

            break;
        }

        case Qt::DecorationRole:
        if( showIcons_ && index.column() == Filename )
        {

            const FileRecord& record( get(index) );
            return iconProvider_->icon( record );

        } else break;

        case Qt::ForegroundRole:
        {
            const FileRecord& record( get(index) );
            if( record.hasFlag( BaseFileInfo::Hidden ) && ( this->flags( index )&Qt::ItemIsEnabled ) )
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
            if( record.hasFlag( BaseFileInfo::Link ) )
            {
                QFont font( QApplication::font() );
                font.setItalic( true );
                return font;
            }

            break;
        }

        case Base::FileTypeRole:
        {
            const FileRecord& record( get(index) );
            return record.flags();
        }

        default: break;

    }

    return QVariant();

}

//__________________________________________________________________
QVariant FileSystemModel::headerData( int section, Qt::Orientation orientation, int role ) const
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

//______________________________________________________________________
QMimeData* FileSystemModel::mimeData(const QModelIndexList &indexes) const
{

    // get selected filenames
    QOrderedSet<QString> filenames;
    for( const auto& index:indexes )
    {

        if( !index.isValid() ) continue;
        const auto record( get(index) );
        filenames.insert( record.file() );

    }

    if( filenames.empty() ) return nullptr;
    else {

        auto mimeData = new QMimeData;

        // fill text data
        {
            QString fullText;
            QTextStream buffer( &fullText );
            for( const auto& filename:filenames )
            { buffer << QString( "file://%1" ).arg(filename) << endl; }
            mimeData->setText( fullText );
        }

        // fill url list
        {
            QList<QUrl> urlList;
            std::transform( filenames.begin(), filenames.end(), std::back_inserter( urlList ),
                []( const QString& filename ) { return QUrl( QString( "file://%1" ).arg(filename) ); } );
            mimeData->setUrls( urlList );
        }

        return mimeData;

    }

}

//____________________________________________________________
void FileSystemModel::_sort( int column, Qt::SortOrder order )
{ std::sort( _get().begin(), _get().end(), SortFTor( column, order, columnTitles_ ) ); }

//________________________________________________________
FileSystemModel::SortFTor::SortFTor( int type, Qt::SortOrder order, const QStringList& columnTitles ):
    ItemModel::SortFTor( type, order ),
    sizePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::Size ) ),
    columnTitles_( columnTitles )
{}

//________________________________________________________
bool FileSystemModel::SortFTor::operator () ( FileRecord first, FileRecord second ) const
{

    if( first.hasFlag( BaseFileInfo::Navigator ) ) return true;
    if( second.hasFlag( BaseFileInfo::Navigator ) ) return false;
    if( first.hasFlag( BaseFileInfo::Folder ) && second.hasFlag( BaseFileInfo::Document ) ) return true;
    if( second.hasFlag( BaseFileInfo::Folder ) && first.hasFlag( BaseFileInfo::Document ) ) return false;

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );

    switch( type_ )
    {

        case Filename:
        {
            if( first.hasFlag( BaseFileInfo::Navigator ) ) return true;
            if( second.hasFlag( BaseFileInfo::Navigator ) ) return false;
            if( first.hasFlag( BaseFileInfo::Folder ) && second.hasFlag( BaseFileInfo::Document ) ) return true;
            if( second.hasFlag( BaseFileInfo::Folder ) && first.hasFlag( BaseFileInfo::Document ) ) return false;
            return first.file().localName().get().compare( second.file().localName(), Qt::CaseInsensitive ) < 0;

        }

        case Time: return (first.time() != second.time() ) ? first.time() < second.time() : first.file().localName() < second.file().localName();
        case Size:
        {
            long first_size( first.property( sizePropertyId_ ).toInt() );
            long second_size( second.property( sizePropertyId_ ).toInt() );
            return (first_size != second_size ) ? first_size < second_size : first.file().localName() < second.file().localName();
        }

        default: return true;

    }

}
