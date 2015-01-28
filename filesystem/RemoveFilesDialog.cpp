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

#include "RemoveFilesDialog.h"

#include "BaseIconNames.h"
#include "IconEngine.h"
#include "TreeView.h"

#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QScrollBar>

//____________________________________________________________________________
RemoveFilesDialog::RemoveFilesDialog( QWidget* parent, const FileSystemModel::List& files ):
    CustomDialog( parent )
{

    Debug::Throw( "RemoveFilesDialog::RemoveFilesDialog.\n" );
    Q_ASSERT( !files.empty() );

    // options
    setOptionName( "REMOVE_FILES_DIALOG" );

    // customize buttons
    okButton().setText( "Delete" );
    okButton().setIcon( IconEngine::get( IconNames::Delete ) );

    // label
    QString buffer = (files.size() == 1) ?
        tr( "Permanently delete this item ?" ):
        QString( tr( "Permanently delete these %1 items ?" ) ).arg( files.size() );
    QLabel* textLabel( new QLabel( buffer, this ) );

    //! try load Question icon
    QHBoxLayout *hLayout( new QHBoxLayout() );
    hLayout->setSpacing(10);
    hLayout->setMargin(0);
    mainLayout().addLayout( hLayout );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( IconNames::DialogWarning ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( textLabel, 1, Qt::AlignLeft );

    // file list
    mainLayout().addWidget( list_ = new TreeView( this ), 1 );
    _list().setSelectionMode( QAbstractItemView::NoSelection );

    model_.setShowIcons( false );
    model_.setUseLocalNames( false );
    model_.add( files );

    // setup list
    _list().setModel( &model_ );
    _list().toggleShowHeader( false );
    _list().setMask( 1<<FileSystemModel::Filename );
    _list().setSortingEnabled( true );
    _list().header()->setSortIndicator( FileSystemModel::Filename, Qt::AscendingOrder );

    // resize list to accomodate longest item
    int maxWidth( 0 );
    foreach( const FileRecord& record, files )
    { maxWidth = qMax( maxWidth, _list().fontMetrics().width( record.file() ) ); }

    _list().verticalScrollBar()->adjustSize();
    _list().setMinimumSize( QSize(
        maxWidth + _list().verticalScrollBar()->width() + 10,
        _list().fontMetrics().height() + 10 ) );


}
