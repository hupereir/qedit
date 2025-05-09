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

#include "FileCheckDialog.h"
#include "Application.h"
#include "Debug.h"
#include "FileList.h"
#include "QtUtil.h"
#include "Singleton.h"
#include "TreeView.h"

#include <QHeaderView>

//________________________________________________________
FileCheckDialog::FileCheckDialog( QWidget* parent ):
    Dialog( parent, CloseButton )
{
    Debug::Throw( QStringLiteral("FileCheckDialog::FileCheckDialog.\n") );
    setWindowTitle( tr( "Monitored Files" ) );
    setOptionName( QStringLiteral("FILE_CHECK_DIALOG") );

    QtUtil::setMargin(layout(), 0);
    QtUtil::setMargin(&buttonLayout(), defaultMargin());

    // custom list display
    mainLayout().addWidget( list_ = new TreeView( this ) );
    list_->setModel( &model_ );
    list_->setSelectionMode( QAbstractItemView::NoSelection );

    // add options
    list_->setOptionName( QStringLiteral("FILE_CHECK_LIST") );

}

//________________________________________________________
void FileCheckDialog::setFiles( const QStringList& files )
{

    Debug::Throw( QStringLiteral("FileCheckDialog::setFiles.\n") );
    FileRecordModel::List records;

    std::transform( files.begin(), files.end(), std::back_inserter( records ),
        []( const QString& file )
        { return Base::Singleton::get().application<Application>()->recentFiles().get( File( file ) ); } );

    model_.set( records );

    list_->updateMask();
    list_->resizeColumns();

}
