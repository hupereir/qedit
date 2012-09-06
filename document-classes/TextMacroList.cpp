
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

/*!
\file TextMacroList.h
\brief List box for TextMacros
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <QHeaderView>
#include <QLayout>

#include "DocumentClassIcons.h"
#include "IconEngine.h"
#include "InformationDialog.h"
#include "QuestionDialog.h"
#include "TextMacroDialog.h"
#include "TextMacroList.h"
#include "TreeView.h"



//____________________________________________________
TextMacroList::TextMacroList( QWidget* parent ):
    QGroupBox( "Macros", parent ),
    Counter( "TextMacroList" ),
    modified_( false )
{
    Debug::Throw( "TextMacroList::TextMacroList.\n" );

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

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setSpacing(5);
    vLayout->setMargin(0);
    hLayout->addLayout( vLayout );

    QPushButton* button;
    vLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::ADD ), "Add", this ) );
    button->setToolTip( "Add a new macro to the list" );
    connect( button, SIGNAL( clicked() ), SLOT( _add() ) );

    vLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::ADD ), "Add Separator", this ) );
    button->setToolTip( "Add separator to the list" );
    connect( button, SIGNAL( clicked() ), SLOT( _addSeparator() ) );

    vLayout->addWidget( removeButton_ = new QPushButton( IconEngine::get( ICONS::REMOVE ), "Remove", this ) );
    removeButton_->setShortcut( QKeySequence::Delete );
    removeButton_->setToolTip( "Remove selected macro" );
    connect( removeButton_, SIGNAL( clicked() ), SLOT( _remove() ) );

    vLayout->addWidget( editButton_ = new QPushButton( IconEngine::get( ICONS::EDIT ), "Edit", this ) );
    editButton_->setToolTip( "Edit selected macro" );
    connect( editButton_, SIGNAL( clicked() ), SLOT( _edit() ) );

    vLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::UP ), "Move Up", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _up() ) );
    button->setToolTip( "Move up selected items" );
    moveUpButton_ = button;

    vLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DOWN ), "Move Down", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _down() ) );
    button->setToolTip( "Move down selected items" );
    moveDownButton_ = button;
    vLayout->addStretch();

    _updateButtons();

}

//____________________________________________________
void TextMacroList::setMacros( const TextMacro::List& macros )
{

    Debug::Throw( "TextMacroList::setMacros.\n" );
    model_.set( macros );
    list_->resizeColumns();
    modified_ = false;

}

//____________________________________________________
TextMacro::List TextMacroList::macros( void )
{

    Debug::Throw( "TextMacroList::macros.\n" );
    return model_.get();

}

//____________________________________________________
void TextMacroList::_updateButtons( void )
{
    Debug::Throw( "TextMacroList::_updateButtons.\n" );
    bool hasSelection( !list_->selectionModel()->selectedRows().empty() );
    editButton_->setEnabled( hasSelection );
    removeButton_->setEnabled( hasSelection );
    moveUpButton_->setEnabled( hasSelection );
    moveDownButton_->setEnabled( hasSelection );
}

//____________________________________________________
void TextMacroList::_add( void )
{
    Debug::Throw( "TextMacroList::_add.\n" );

    TextMacroDialog dialog( this );
    if( dialog.exec() == QDialog::Rejected ) return;
    model_.add( dialog.macro() );

}

//____________________________________________________
void TextMacroList::_addSeparator( void )
{
    Debug::Throw( "TextMacroList::_addSeparator.\n" );

    TextMacro macro;
    macro.setIsSeparator();
    model_.add( macro );

}

//____________________________________________________
void TextMacroList::_edit( void )
{
    Debug::Throw( "TextMacroList::_edit.\n" );

    // retrieve selected items
    QModelIndexList selection( list_->selectionModel()->selectedRows() );
    if( selection.empty() ) {
        InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
        return;
    }

    TextMacroModel::List macro( model_.get() );
    for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    {

        TextMacro old_macro( model_.get( *iter ) );

        TextMacroDialog dialog( this );
        dialog.setMacro( old_macro );
        if( dialog.exec() == QDialog::Rejected ) continue;

        TextMacro macro( dialog.macro() );
        if( macro == old_macro ) continue;

        model_.replace( *iter, macro );
        modified_ = true;

    }

}

//____________________________________________________
void TextMacroList::_remove( void )
{
    Debug::Throw( "TextMacroList::_remove.\n" );

    // retrieve selected items; make sure they do not include the navigator
    TextMacroModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
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
void TextMacroList::_storeSelection( void )
{

    // clear
    model_.clearSelectedIndexes();

    // retrieve selected indexes in list
    foreach( const QModelIndex& index, list_->selectionModel()->selectedRows() )
    { model_.setIndexSelected( index, true ); }

}

//________________________________________
void TextMacroList::_restoreSelection( void )
{

    QModelIndexList selection( model_.selectedIndexes() );
    list_->selectionModel()->clearSelection();

    foreach( const QModelIndex& index, selection )
    { list_->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;
}

//_________________________________________________________
void TextMacroList::_up( void )
{

    Debug::Throw( "TextMacroList::_up.\n" );
    TextMacro::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no item selected. <Move up> canceled" ).exec();
        return;
    }

    // retrieve selected indexes in list and store in model
    QModelIndexList selectedIndexes( list_->selectionModel()->selectedRows() );
    TextMacro::List selectedAttributes( model_.get( selectedIndexes ) );

    TextMacro::List currentAttributes( macros() );
    TextMacro::List newAttributes;

    for( TextMacro::List::const_iterator iter = currentAttributes.begin(); iter != currentAttributes.end(); ++iter )
    {

        // check if new list is not empty, current index is selected and last index is not.
        // if yes, move.
        if(
            !( newAttributes.empty() ||
            selectedIndexes.indexOf( model_.index( *iter ) ) == -1 ||
            selectedIndexes.indexOf( model_.index( newAttributes.back() ) ) != -1
            ) )
        {
            TextMacro last( newAttributes.back() );
            newAttributes.pop_back();
            newAttributes.push_back( *iter );
            newAttributes.push_back( last );
        } else newAttributes.push_back( *iter );

    }

    model_.set( newAttributes );

    // restore selection
    list_->selectionModel()->select( model_.index( selectedAttributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( TextMacro::List::const_iterator iter = selectedAttributes.begin(); iter != selectedAttributes.end(); ++iter )
    { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;

}

//_________________________________________________________
void TextMacroList::_down( void )
{

    Debug::Throw( "TextMacroList::_down.\n" );
    TextMacro::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no item selected. <Move down> canceled" ).exec();
        return;
    }

    // retrieve selected indexes in list and store in model
    QModelIndexList selectedIndexes( list_->selectionModel()->selectedIndexes() );
    TextMacro::List selectedAttributes( model_.get( selectedIndexes ) );

    TextMacro::List currentAttributes( macros() );
    TextMacro::List newAttributes;
    TextMacro::ListIterator iter( currentAttributes );
    iter.toBack();
    while( iter.hasPrevious() )
    {

        const TextMacro& current( iter.previous() );
        if(
            !( newAttributes.empty() ||
            selectedIndexes.indexOf( model_.index( current ) ) == -1 ||
            selectedIndexes.indexOf( model_.index( newAttributes.front() ) ) != -1
            ) )
        {

            TextMacro first( newAttributes.front() );
            newAttributes.pop_front();
            newAttributes.push_front( current );
            newAttributes.push_front( first );

        } else newAttributes.push_front( current );
    }

    model_.set( newAttributes );

    // restore selection
    list_->selectionModel()->select( model_.index( selectedAttributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( TextMacro::List::const_iterator iter = selectedAttributes.begin(); iter != selectedAttributes.end(); ++iter )
    { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;

}
