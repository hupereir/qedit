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

#include "DocumentClassManagerDialog.h"
#include "BaseIconNames.h"
#include "ContextMenu.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "FileDialog.h"
#include "IconEngine.h"
#include "QtUtil.h"
#include "QuestionDialog.h"
#include "TreeView.h"
#include "XmlOptions.h"


#include <QHeaderView>
#include <QLayout>

//______________________________________________________________________
DocumentClassManagerDialog::DocumentClassManagerDialog( QWidget* parent ):
    Dialog( parent, OkButton|CancelButton )
{

    Debug::Throw( QStringLiteral("DocumentClassManagerDialog::DocumentClassManagerDialog.\n") );
    setOptionName( QStringLiteral("DOCUMENT_CLASS_MANAGER_DIALOG") );

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(5);
    QtUtil::setMargin(hLayout, 0);
    mainLayout().addLayout( hLayout );

    // setup list
    hLayout->addWidget( list_ = new TreeView( this ) );
    list_->setModel( &model_ );
    list_->setSortingEnabled( false );
    list_->header()->hide();
    list_->setOptionName( QStringLiteral("DOCUMENT_CLASS_MANAGER_LIST") );

    // buttons
    auto vLayout = new QVBoxLayout;
    vLayout->setSpacing(5);
    QtUtil::setMargin(vLayout, 0);
    hLayout->addLayout( vLayout );

    vLayout->addWidget( addButton_ = new QPushButton( IconEngine::get( IconNames::Add ), tr( "Add" ), this ) );
    connect( addButton_, &QAbstractButton::clicked, this, &DocumentClassManagerDialog::_add );

    vLayout->addWidget( removeButton_ = new QPushButton( IconEngine::get( IconNames::Remove ), tr( "Remove" ), this ) );
    connect( removeButton_, &QAbstractButton::clicked, this, &DocumentClassManagerDialog::_remove );

    vLayout->addWidget( reloadButton_ = new QPushButton( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this ) );
    connect( reloadButton_, &QAbstractButton::clicked, this, &DocumentClassManagerDialog::_reload );

    vLayout->addStretch( 1 );

    // actions
    auto menu = new ContextMenu( list_ );
    addAction( addAction_ = new QAction( IconEngine::get( IconNames::Add ), tr( "Add" ), this ) );
    connect( addAction_, &QAction::triggered, this, &DocumentClassManagerDialog::_add );
    addAction_->setShortcut( QKeySequence::New );
    menu->addAction( addAction_ );

    addAction( removeAction_ = new QAction( IconEngine::get( IconNames::Remove ), tr( "Remove" ), this ) );
    connect( removeAction_, &QAction::triggered, this, &DocumentClassManagerDialog::_remove );
    removeAction_->setShortcut( QKeySequence::Delete );
    menu->addAction( removeAction_ );

    addAction( reloadAction_ = new QAction( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this ) );
    connect( reloadAction_, &QAction::triggered, this, &DocumentClassManagerDialog::_reload );
    reloadAction_->setShortcut( QKeySequence::Refresh );
    menu->addAction( reloadAction_ );

    // updates
    connect( list_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DocumentClassManagerDialog::_updateButtons );
    connect( list_->selectionModel(), &QItemSelectionModel::currentChanged, this, &DocumentClassManagerDialog::_updateButtons );

    _reload();
    _updateButtons();

}

//______________________________________________________________________
File::List DocumentClassManagerDialog::userFiles() const
{
    File::List files;
    for( const auto& documentClass:model_.get() )
    { if( !documentClass.isBuildIn() ) files.append( documentClass.file() ); }

    return files;
}

//______________________________________________________________________
void DocumentClassManagerDialog::_reload()
{

    Debug::Throw( QStringLiteral("DocumentClassManager::Dialog::_reload.\n") );

    DocumentClassManager classManager;
    for( const auto& option:XmlOptions::get().specialOptions( QStringLiteral("PATTERN_FILENAME") ) )
    { classManager.read( File( option.raw() ) ); }

    // read build-in patterns
    for( const auto& option:XmlOptions::get().specialOptions( QStringLiteral("DEFAULT_PATTERN_FILENAME") ) )
    { classManager.read( File( option.raw() ) ); }

    // update model
    model_.set( classManager.classes() );

}

//______________________________________________________________________
void DocumentClassManagerDialog::_add()
{

    Debug::Throw( QStringLiteral("DocumentClassManagerDialog::_add.\n") );
    FileDialog dialog( this );
    dialog.setAcceptMode( QFileDialog::AcceptOpen );
    dialog.setFileMode( QFileDialog::ExistingFile );

    File file( dialog.getFile() );
    if( file.isEmpty() ) return;

    DocumentClassManager classManager;
    classManager.read( file );
    model_.add( classManager.classes() );

}

//______________________________________________________________________
void DocumentClassManagerDialog::_remove()
{
    Debug::Throw( QStringLiteral("DocumentClassManagerDialog::_remove.\n") );

    // loop over selected items
    QSet<File> removedFiles;
    for( const auto& documentClass:model_.get( list_->selectionModel()->selectedRows() ) )
    { if( !documentClass.isBuildIn() ) removedFiles.insert( documentClass.file() ); }

    if( removedFiles.empty() ) return;

    // ask confirmation
    QString buffer = (removedFiles.size() == 1 ) ?
        tr( "Remove all items from the selected file ?" ):
        tr( "Remove all items from the selected %1 files ?" ).arg( removedFiles.size() );
    if( !QuestionDialog( this, buffer ).exec() ) return;

    DocumentClassManager::List removedItems;
    for( const auto& documentClass:model_.get() )
    { if( removedFiles.contains( documentClass.file() ) ) removedItems.append( documentClass ); }

    // remove
    model_.remove( removedItems );

    return;

}

//______________________________________________________________________
void DocumentClassManagerDialog::_updateButtons()
{
    // loop over selected items
    const auto selection( model_.get( list_->selectionModel()->selectedRows() ) );
    const bool removeEnabled = std::any_of( selection.begin(), selection.end(), Base::Functor::UnaryTrue<DocumentClass, &DocumentClass::isBuildIn>() );

    removeAction_->setEnabled( removeEnabled );
    removeButton_->setEnabled( removeEnabled );
}
