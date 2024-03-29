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

#include "FileSelectionDialog.h"
#include "Application.h"
#include "Debug.h"
#include "MainWindow.h"
#include "RecentFilesMenu.h"
#include "Singleton.h"
#include "TextDisplay.h"
#include "TreeView.h"
#include "WindowServer.h"
#include "XmlOptions.h"

#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
FileSelectionDialog::FileSelectionDialog( QWidget* parent, const TextSelection& selection ):
Dialog( parent ),
selection_( selection )
{

    setWindowTitle( tr( "File Selection" ) );
    setOptionName( QStringLiteral("FILE_SELECTION_DIALOG") );

    // custom list display
    list_ = new TreeView( this );
    list_->setModel( &model_ );
    list_->setSelectionMode( QAbstractItemView::MultiSelection );
    connect( list_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileSelectionDialog::_updateButtons );

    // retrieve file records
    model_.set( Base::Singleton::get().application<Application>()->windowServer().records() );

    // mask
    int mask(
        (1<<FileRecordModel::FileName)|
        (1<<FileRecordModel::Path ));
    int classColumn( model_.findColumn( QStringLiteral("class_name") ) );
    if( classColumn >= 0 ) mask |= (1<<classColumn);
    list_->setMask( mask );
    list_->resizeColumns();
    list_->setOptionName( QStringLiteral("FILE_SELECTION_LIST") );
    mainLayout().addWidget( list_ );

    // generic button
    QPushButton* button;

    // deselect all
    buttonLayout().insertWidget( 0, button = new QPushButton( tr( "Clear Selection" ), this ) );
    button->setToolTip( tr( "Deselect all files in list" ) );
    connect( button, &QAbstractButton::clicked, list_, &QAbstractItemView::clearSelection );
    clearSelectionButton_ = button;

    // select all
    buttonLayout().insertWidget( 0, button = new QPushButton( tr( "Select All" ), this ) );
    button->setToolTip( tr( "Select all files in list" ) );
    connect( button, &QPushButton::clicked, list_, &TreeView::selectAll );
    selectAllButton_ = button;

    // replace
    okButton().setToolTip( tr( "Replace in all selected files" ) );
    okButton().setText( tr( "Replace" ) );
    _updateButtons();

    // sort list and select all items
    if( XmlOptions::get().contains( QStringLiteral("SESSION_FILES_SORT_COLUMN") ) && XmlOptions::get().contains( QStringLiteral("SESSION_FILES_SORT_ORDER") ) )
    {
        list_->sortByColumn(
            XmlOptions::get().get<int>( QStringLiteral("SESSION_FILES_SORT_COLUMN") ),
            (Qt::SortOrder)(XmlOptions::get().get<int>( QStringLiteral("SESSION_FILES_SORT_ORDER") ) ) );
    }

    list_->selectAll();


    adjustSize();

}

//________________________________________________________
void FileSelectionDialog::_updateButtons()
{

    Debug::Throw( QStringLiteral("FileSelectionDialog::_updateButtons.\n") );
    QList<QModelIndex> selection( list_->selectionModel()->selectedRows() );

    clearSelectionButton_->setEnabled( !selection.empty() );
    okButton().setEnabled( !selection.empty() );

}

//________________________________________________________
FileSelectionDialog::FileList FileSelectionDialog::selectedFiles() const
{

    Debug::Throw( QStringLiteral("FileSelectionDialog::_replace.\n") );

    // retrieve selection from the list
    auto selection( model_.get( list_->selectionModel()->selectedRows() ) );
    FileList files;
    std::transform( selection.begin(), selection.end(), std::back_inserter( files ), []( const FileRecord& record ) { return record.file(); } );

    return files;
}
