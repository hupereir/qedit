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

#include "DocumentClassModel.h"

#include "Application.h"
#include "Pixmap.h"
#include "IconEngine.h"
#include "Singleton.h"
#include "XmlOptions.h"

//__________________________________________________________________
DocumentClassModel::IconCache& DocumentClassModel::_icons()
{
    static IconCache cache;
    return cache;
}

//__________________________________________________________________
DocumentClassModel::DocumentClassModel( QObject* parent ):
    ListModel(parent),
    Counter( QStringLiteral("DocumentClassModel") )
{ connect( Base::Singleton::get().application<Application>(), &Application::configurationChanged, this, &DocumentClassModel::_updateConfiguration ); }

//__________________________________________________________________
Qt::ItemFlags DocumentClassModel::flags( const QModelIndex& index ) const
{

    // default flags
    if( !contains( index ) ) return Qt::ItemFlags();

    // check associated record validity
    const DocumentClass& documentClass( get(index) );
    return documentClass.isBuildIn() ? Qt::ItemFlags() : Qt::ItemIsEnabled |  Qt::ItemIsSelectable;

}

//__________________________________________________________________
QVariant DocumentClassModel::data( const QModelIndex& index, int role ) const
{

    // check index
    if( !contains( index ) ) return QVariant();

    // retrieve associated file info
    const DocumentClass& documentClass( get()[index.row()] );

    // return text associated to file and column

    if( role == Qt::DecorationRole && index.column() == Name )
    {

        return _icon( documentClass.icon() );

    } else if( role == Qt::DisplayRole ) {

        switch( index.column() )
        {
            case Name: return documentClass.name();
            case FileName:
            {
                if( documentClass.isBuildIn() ) return tr( "Internal" );
                else return documentClass.file().get();
            }

            default: break;
        }
    }

    return QVariant();

}

//__________________________________________________________________
QVariant DocumentClassModel::headerData(int section, Qt::Orientation, int role) const
{

    if(
        role == Qt::DisplayRole &&
        section >= 0 &&
        section < nColumns )
    { return columnTitles_[section]; }

    // return empty
    return QVariant();

}


//____________________________________________________________
void DocumentClassModel::_sort( int column, Qt::SortOrder order )
{

    Debug::Throw() << "DocumentClassModel::sort - column: " << column << " order: " << order << endl;
    std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) );

}

//____________________________________________________________
void DocumentClassModel::_updateConfiguration()
{
    Debug::Throw( QStringLiteral("DocumentClassModel::_updateConfiguration.\n") );
    _icons().clear();
}

//________________________________________________________
bool DocumentClassModel::SortFTor::operator () ( const DocumentClass& constFirst, const DocumentClass& constSecond ) const
{

    DocumentClass first( constFirst );
    DocumentClass second( constSecond );
    if( order_ == Qt::DescendingOrder ) std::swap( first, second );
    switch( type_ )
    {
        case Name: return DocumentClass::WeakLessThanFTor()( first, second );
        case FileName: return first.file() < second.file();
        default: return true;
    }

}

//________________________________________________________
const QIcon& DocumentClassModel::_icon( const QString& name )
{

    Debug::Throw() << "DocumentClassModel::_icon - name: " << name << endl;

    IconCache::const_iterator iter( _icons().find( name ) );
    if( iter != _icons().end() ) return iter.value();

    QIcon icon( IconEngine::get( name ) );
    if( icon.isNull() )
    {
        const int iconSize( XmlOptions::get().get<int>( QStringLiteral("LIST_ICON_SIZE") ) );
        const QSize size( iconSize, iconSize );
        icon = Pixmap( size, Pixmap::Flag::Transparent );
    }

    return _icons().insert( name, icon ).value();

}
