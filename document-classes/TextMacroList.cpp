
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

using namespace std;

//____________________________________________________
TextMacroList::TextMacroList( QWidget* parent ):
    QGroupBox( "Macros", parent ),
    Counter( "TextMacroList" ),
    modified_( false )
{
    Debug::Throw( "TextMacroList::TextMacroList.\n" );

    QHBoxLayout* h_layout;
    h_layout = new QHBoxLayout();
    h_layout->setSpacing(5);
    h_layout->setMargin(5);
    setLayout( h_layout );


    h_layout->addWidget( list_ = new TreeView( this ), 1 );
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
    h_layout->addLayout( v_layout );

    QPushButton* button;
    v_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::ADD ), "&Add", this ) );
    button->setToolTip( "Add a new macro to the list" );
    connect( button, SIGNAL( clicked() ), SLOT( _add() ) );

    v_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::ADD ), "&Add Separator", this ) );
    button->setToolTip( "Add separator to the list" );
    connect( button, SIGNAL( clicked() ), SLOT( _addSeparator() ) );

    v_layout->addWidget( removeButton_ = new QPushButton( IconEngine::get( ICONS::REMOVE ), "&Remove", this ) );
    removeButton_->setShortcut( Qt::Key_Delete );
    removeButton_->setToolTip( "Remove selected macro" );
    connect( removeButton_, SIGNAL( clicked() ), SLOT( _remove() ) );

    v_layout->addWidget( editButton_ = new QPushButton( IconEngine::get( ICONS::EDIT ), "&Edit", this ) );
    editButton_->setToolTip( "Edit selected macro" );
    connect( editButton_, SIGNAL( clicked() ), SLOT( _edit() ) );

    v_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::UP ), "Move &Up", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _up() ) );
    button->setToolTip( "Move up selected items" );
    moveUpButton_ = button;

    v_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DOWN ), "Move &Down", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _down() ) );
    button->setToolTip( "Move down selected items" );
    moveDownButton_ = button;
    v_layout->addStretch();

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
    bool has_selection( !list_->selectionModel()->selectedRows().empty() );
    editButton_->setEnabled( has_selection );
    removeButton_->setEnabled( has_selection );
    moveUpButton_->setEnabled( has_selection );
    moveDownButton_->setEnabled( has_selection );
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
    for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); iter++ )
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
    QModelIndexList selected_indexes( list_->selectionModel()->selectedRows() );
    for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
    {
        // check column
        if( !iter->column() == 0 ) continue;
        model_.setIndexSelected( *iter, true );
    }

}

//________________________________________
void TextMacroList::_restoreSelection( void )
{

    // retrieve indexes
    QModelIndexList selected_indexes( model_.selectedIndexes() );
    if( selected_indexes.empty() ) list_->selectionModel()->clear();
    else {

        list_->selectionModel()->select( selected_indexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
        { list_->selectionModel()->select( *iter, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    }

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
    QModelIndexList selected_indexes( list_->selectionModel()->selectedRows() );
    TextMacro::List selected_attributes( model_.get( selected_indexes ) );

    TextMacro::List current_attributes( macros() );
    TextMacro::List new_attributes;

    for( TextMacro::List::const_iterator iter = current_attributes.begin(); iter != current_attributes.end(); iter++ )
    {

        // check if new list is not empty, current index is selected and last index is not.
        // if yes, move.
        if(
            !( new_attributes.empty() ||
            selected_indexes.indexOf( model_.index( *iter ) ) == -1 ||
            selected_indexes.indexOf( model_.index( new_attributes.back() ) ) != -1
            ) )
        {
            TextMacro last( new_attributes.back() );
            new_attributes.pop_back();
            new_attributes.push_back( *iter );
            new_attributes.push_back( last );
        } else new_attributes.push_back( *iter );

    }

    model_.set( new_attributes );

    // restore selection
    list_->selectionModel()->select( model_.index( selected_attributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( TextMacro::List::const_iterator iter = selected_attributes.begin(); iter != selected_attributes.end(); iter++ )
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
    QModelIndexList selected_indexes( list_->selectionModel()->selectedIndexes() );
    TextMacro::List selected_attributes( model_.get( selected_indexes ) );

    TextMacro::List current_attributes( macros() );
    TextMacro::List new_attributes;

    for( TextMacro::List::reverse_iterator iter = current_attributes.rbegin(); iter != current_attributes.rend(); iter++ )
    {

        // check if new list is not empty, current index is selected and last index is not.
        // if yes, move.
        if(
            !( new_attributes.empty() ||
            selected_indexes.indexOf( model_.index( *iter ) ) == -1 ||
            selected_indexes.indexOf( model_.index( new_attributes.back() ) ) != -1
            ) )
        {

            TextMacro last( new_attributes.back() );
            new_attributes.pop_back();
            new_attributes.push_back( *iter );
            new_attributes.push_back( last );

        } else new_attributes.push_back( *iter );
    }

    model_.set( TextMacroModel::List( new_attributes.rbegin(), new_attributes.rend() ) );

    // restore selection
    list_->selectionModel()->select( model_.index( selected_attributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( TextMacro::List::const_iterator iter = selected_attributes.begin(); iter != selected_attributes.end(); iter++ )
    { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;

}
