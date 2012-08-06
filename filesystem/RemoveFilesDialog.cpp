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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "RemoveFilesDialog.h"

#include "BaseIcons.h"
#include "IconEngine.h"
#include "TreeView.h"
#include "XmlOptions.h"

#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <cassert>

//____________________________________________________________________________
RemoveFilesDialog::RemoveFilesDialog( QWidget* parent, const FileSystemModel::List& files ):
    CustomDialog( parent )
{

    Debug::Throw( "RemoveFilesDialog::RemoveFilesDialog.\n" );
    assert( !files.empty() );

    // options
    setOptionName( "REMOVE_FILES_DIALOG" );

    // customize buttons
    okButton().setText( "Remove" );
    okButton().setIcon( IconEngine::get( ICONS::DELETE ) );

    // label
    QString buffer;
    QTextStream what( &buffer );
    what << "Permanently remove ";
    if( files.size() == 1 ) what << "this item ?";
    else what << "these " << files.size() << " items ?";
    QLabel* textLabel( new QLabel( buffer, this ) );

    //! try load Question icon
    QPixmap questionPixmap( PixmapEngine::get( ICONS::WARNING ) );
    QHBoxLayout *hLayout( new QHBoxLayout() );
    hLayout->setSpacing(10);
    hLayout->setMargin(0);
    mainLayout().addLayout( hLayout );
    QLabel* label = new QLabel( this );
    label->setPixmap( questionPixmap );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( textLabel, 1, Qt::AlignLeft );

    // file list
    mainLayout().addWidget( list_ = new TreeView( this ), 1 );
    _list().setSelectionMode( QAbstractItemView::NoSelection );

    model_.setShowIcons( false );
    model_.setUseLocalNames( false );
    model_.add( files );
    model_.sort( FileSystemModel::FILE, Qt::DescendingOrder );

    _list().setModel( &model_ );
    _list().toggleShowHeader( false );
    _list().setMask( XmlOptions::get().get<int>( "FILE_SYSTEM_LIST_MASK" ) );
    _list().header()->setSortIndicator(
        XmlOptions::get().get<int>( "FILE_SYSTEM_LIST_SORT_COLUMN" ),
        (Qt::SortOrder) XmlOptions::get().get<int>( "FILE_SYSTEM_LIST_SORT_ORDER" ) );
    _list().resizeColumnToContents( FileSystemModel::FILE );

}
