// $Id$

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public license as published by the Free Software
* Foundation; either version 2 of the license, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public license
* for more details.
*
* You should have received a copy of the GNU General Public license along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "Application.h"
#include "Debug.h"
#include "FileCheckDialog.h"
#include "FileList.h"
#include "Singleton.h"
#include "TreeView.h"

#include <QHeaderView>

//________________________________________________________
FileCheckDialog::FileCheckDialog( QWidget* parent ):
CustomDialog( parent, CloseButton )
{
    Debug::Throw( "FileCheckDialog::FileCheckDialog.\n" );
    setWindowTitle( tr( "Monitored Files - Qedit" ) );
    setOptionName( "FILE_CHECK_DIALOG" );

    layout()->setMargin(0);
    buttonLayout().setMargin(5);

    // custom list display
    mainLayout().addWidget( list_ = new TreeView( this ) );
    _list().setModel( &model_ );
    _list().setSelectionMode( QAbstractItemView::NoSelection );

    // add options
    _list().setOptionName( "FILE_CHECK_LIST" );

}

//________________________________________________________
void FileCheckDialog::setFiles( const QStringList& files )
{

    Debug::Throw( "FileCheckDialog::setFiles.\n" );
    FileRecordModel::List records;
    foreach( const QString& file, files )
    { records << Singleton::get().application<Application>()->recentFiles().get( file ); }

    model_.set( records );

    _list().updateMask();
    _list().resizeColumns();

}
