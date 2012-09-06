
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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "BlockDelimiterDialog.h"
#include "BlockDelimiterList.h"
#include "DocumentClassIcons.h"
#include "IconEngine.h"
#include "InformationDialog.h"
#include "QuestionDialog.h"
#include "TreeView.h"

#include <QtGui/QHeaderView>
#include <QtGui/QLayout>

//____________________________________________________
BlockDelimiterList::BlockDelimiterList( QWidget* parent ):
QGroupBox( "Block delimiters", parent ),
Counter( "BlockDelimiterList" ),
modified_( false )
{
    Debug::Throw( "BlockDelimiterList::BlockDelimiterList.\n" );

    QHBoxLayout* hLayout;
    hLayout = new QHBoxLayout();
    hLayout->setSpacing(5);
    hLayout->setMargin(5);
    setLayout( hLayout );


    hLayout->addWidget( list_ = new TreeView( this ), 1 );
    list_->setModel( &model_ );
    list_->setSortingEnabled( false );
    list_->setAllColumnsShowFocus( true );

    connect( list_->selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateButtons() ) );
    connect( list_, SIGNAL( activated( const QModelIndex& ) ), SLOT( _edit() ) );

    connect( &model_, SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelection() ) );
    connect( &model_, SIGNAL( layoutChanged() ), SLOT( _restoreSelection() ) );

    QVBoxLayout* v_layout = new QVBoxLayout();
    v_layout->setSpacing(5);
    v_layout->setMargin(0);
    hLayout->addLayout( v_layout );

    QPushButton* button;
    v_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::ADD ), "Add", this ) );
    button->setToolTip( "Add a new delimiter to the list" );
    connect( button, SIGNAL( clicked() ), SLOT( _add() ) );

    v_layout->addWidget( remove_button_ = new QPushButton( IconEngine::get( ICONS::REMOVE ), "Remove", this ) );
    remove_button_->setToolTip( "Remove selected delimiter" );
    remove_button_->setShortcut( QKeySequence::Delete );
    connect( remove_button_, SIGNAL( clicked() ), SLOT( _remove() ) );

    v_layout->addWidget( edit_button_ = new QPushButton( IconEngine::get( ICONS::EDIT ), "Edit", this ) );
    edit_button_->setToolTip( "Edit selected delimiter" );
    connect( edit_button_, SIGNAL( clicked() ), SLOT( _edit() ) );

    v_layout->addStretch();

    _updateButtons();

}

//____________________________________________________
void BlockDelimiterList::setDelimiters( const BlockDelimiter::List& delimiter )
{

    Debug::Throw( "BlockDelimiterList::setDelimiter.\n" );
    model_.set( delimiter );
    list_->resizeColumns();
    modified_ = false;

}

//____________________________________________________
BlockDelimiter::List BlockDelimiterList::delimiters( void )
{

    Debug::Throw( "BlockDelimiterList::delimiter.\n" );
    return model_.get();

}

//____________________________________________________
void BlockDelimiterList::_updateButtons( void )
{
    Debug::Throw( "BlockDelimiterList::_updateButtons.\n" );
    bool hasSelection( !list_->selectionModel()->selectedRows().empty() );
    edit_button_->setEnabled( hasSelection );
    remove_button_->setEnabled( hasSelection );
}

//____________________________________________________
void BlockDelimiterList::_add( void )
{
    Debug::Throw( "BlockDelimiterList::_add.\n" );

    BlockDelimiterDialog dialog( this );
    if( dialog.exec() == QDialog::Rejected ) return;
    model_.add( dialog.delimiter() );

}

//____________________________________________________
void BlockDelimiterList::_edit( void )
{
    Debug::Throw( "BlockDelimiterList::_edit.\n" );

    // retrieve selected items
    QModelIndexList selection( list_->selectionModel()->selectedRows() );
    if( selection.empty() ) {
        InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
        return;
    }

    BlockDelimiterModel::List delimiter( model_.get() );
    for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    {

        BlockDelimiter old_delimiter( model_.get( *iter ) );

        BlockDelimiterDialog dialog( this );
        dialog.setDelimiter( old_delimiter );
        if( dialog.exec() == QDialog::Rejected ) continue;

        BlockDelimiter delimiter( dialog.delimiter() );
        if( delimiter == old_delimiter ) continue;

        model_.replace( *iter, delimiter );
        modified_ = true;

    }

}

//____________________________________________________
void BlockDelimiterList::_remove( void )
{
    Debug::Throw( "BlockDelimiterList::_remove.\n" );

    // retrieve selected items; make sure they do not include the navigator
    BlockDelimiterModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    if( selection.empty() ) {
        InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
        return;
    }

    // ask for confirmation
    QString buffer;
    QTextStream what( &buffer );
    what << "Remove selected item";
    if( selection.size()>1 ) what << "S";
    what << " ?";
    if( !QuestionDialog( this, buffer ).exec() ) return;

    // remove items
    model_.remove( selection );
    modified_ = true;

}

//________________________________________
void BlockDelimiterList::_storeSelection( void )
{

    // clear
    model_.clearSelectedIndexes();

    // retrieve selected indexes in list
    foreach( const QModelIndex& index, list_->selectionModel()->selectedRows() )
    { model_.setIndexSelected( index, true ); }

}

//________________________________________
void BlockDelimiterList::_restoreSelection( void )
{

    QModelIndexList selection( model_.selectedIndexes() );
    list_->selectionModel()->clearSelection();

    foreach( const QModelIndex& index, selection )
    { list_->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;
}
