
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
\file TextMacroRuleList.h
\brief List box for TextMacroRules
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
#include "TextMacroRuleDialog.h"
#include "TextMacroRuleList.h"
#include "TreeView.h"



//____________________________________________________
TextMacroRuleList::TextMacroRuleList( QWidget* parent ):
QGroupBox( "Rules", parent ),
Counter( "TextMacroRuleList" ),
modified_( false )
{
    Debug::Throw( "TextMacroRuleList::TextMacroRuleList.\n" );

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
    vLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::ADD ), "&Add", this ) );
    button->setToolTip( "Add a new rule to the list" );
    connect( button, SIGNAL( clicked() ), SLOT( _add() ) );

    vLayout->addWidget( remove_button_ = new QPushButton( IconEngine::get( ICONS::REMOVE ), "&Remove", this ) );
    remove_button_->setShortcut( Qt::Key_Delete );
    remove_button_->setToolTip( "Remove selected rule" );
    connect( remove_button_, SIGNAL( clicked() ), SLOT( _remove() ) );

    vLayout->addWidget( edit_button_ = new QPushButton( IconEngine::get( ICONS::EDIT ),  "&Edit", this ) );
    edit_button_->setToolTip( "Edit selected rule" );
    connect( edit_button_, SIGNAL( clicked() ), SLOT( _edit() ) );

    vLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::UP ), "Move &Up", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _up() ) );
    button->setToolTip( "Move up selected items" );
    move_up_button_ = button;

    vLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DOWN ), "Move &Down", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _down() ) );
    button->setToolTip( "Move down selected items" );
    move_down_button_ = button;

    vLayout->addStretch();

    _updateButtons();

}

//____________________________________________________
void TextMacroRuleList::setRules( const TextMacro::Rule::List& rule )
{

    Debug::Throw( "TextMacroRuleList::setRule.\n" );
    model_.set( rule );
    list_->resizeColumns();
    modified_ = false;

}

//____________________________________________________
TextMacro::Rule::List TextMacroRuleList::rules( void )
{

    Debug::Throw( "TextMacroRuleList::rule.\n" );
    return model_.get();

}

//____________________________________________________
void TextMacroRuleList::_updateButtons( void )
{
    Debug::Throw( "TextMacroRuleList::_updateButtons.\n" );
    bool has_selection( !list_->selectionModel()->selectedRows().empty() );
    edit_button_->setEnabled( has_selection );
    remove_button_->setEnabled( has_selection );
    move_up_button_->setEnabled( has_selection );
    move_down_button_->setEnabled( has_selection );
}

//____________________________________________________
void TextMacroRuleList::_add( void )
{
    Debug::Throw( "TextMacroRuleList::_add.\n" );

    TextMacroRuleDialog dialog( this );
    if( dialog.exec() == QDialog::Rejected ) return;
    model_.add( dialog.rule() );

}

//____________________________________________________
void TextMacroRuleList::_edit( void )
{
    Debug::Throw( "TextMacroRuleList::_edit.\n" );

    // retrieve selected items
    QModelIndexList selection( list_->selectionModel()->selectedRows() );
    if( selection.empty() ) {
        InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
        return;
    }

    TextMacroRuleModel::List rule( model_.get() );
    for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    {

        TextMacro::Rule old_rule( model_.get( *iter ) );

        TextMacroRuleDialog dialog( this );
        dialog.setRule( old_rule );
        if( dialog.exec() == QDialog::Rejected ) continue;

        TextMacro::Rule rule( dialog.rule() );
        if( rule == old_rule ) continue;

        model_.replace( *iter, rule );
        modified_ = true;

    }

}

//____________________________________________________
void TextMacroRuleList::_remove( void )
{
    Debug::Throw( "TextMacroRuleList::_remove.\n" );

    // retrieve selected items; make sure they do not include the navigator
    TextMacroRuleModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
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
void TextMacroRuleList::_storeSelection( void )
{
    // clear
    model_.clearSelectedIndexes();

    // retrieve selected indexes in list
    QModelIndexList selected_indexes( list_->selectionModel()->selectedRows() );
    for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); ++iter )
    {
        // check column
        if( !iter->column() == 0 ) continue;
        model_.setIndexSelected( *iter, true );
    }

}

//________________________________________
void TextMacroRuleList::_restoreSelection( void )
{

    // retrieve indexes
    QModelIndexList selected_indexes( model_.selectedIndexes() );
    if( selected_indexes.empty() ) list_->selectionModel()->clear();
    else {

        list_->selectionModel()->select( selected_indexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); ++iter )
        { list_->selectionModel()->select( *iter, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    }

    return;
}


//_________________________________________________________
void TextMacroRuleList::_up( void )
{

    Debug::Throw( "TextMacroRuleList::_up.\n" );
    TextMacro::Rule::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no item selected. <Move up> canceled" ).exec();
        return;
    }

    // retrieve selected indexes in list and store in model
    QModelIndexList selected_indexes( list_->selectionModel()->selectedRows() );
    TextMacro::Rule::List selectedAttributes( model_.get( selected_indexes ) );

    TextMacro::Rule::List currentAttributes( rules() );
    TextMacro::Rule::List newAttributes;

    for( TextMacro::Rule::List::const_iterator iter = currentAttributes.begin(); iter != currentAttributes.end(); ++iter )
    {

        // check if new list is not empty, current index is selected and last index is not.
        // if yes, move.
        if(
            !( newAttributes.empty() ||
            selected_indexes.indexOf( model_.index( *iter ) ) == -1 ||
            selected_indexes.indexOf( model_.index( newAttributes.back() ) ) != -1
            ) )
        {
            TextMacro::Rule last( newAttributes.back() );
            newAttributes.pop_back();
            newAttributes.push_back( *iter );
            newAttributes.push_back( last );
        } else newAttributes.push_back( *iter );

    }

    model_.set( newAttributes );

    // restore selection
    list_->selectionModel()->select( model_.index( selectedAttributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( TextMacro::Rule::List::const_iterator iter = selectedAttributes.begin(); iter != selectedAttributes.end(); ++iter )
    { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;

}

//_________________________________________________________
void TextMacroRuleList::_down( void )
{

    Debug::Throw( "TextMacroRuleList::_down.\n" );
    TextMacro::Rule::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no item selected. <Move down> canceled" ).exec();
        return;
    }

    // retrieve selected indexes in list and store in model
    QModelIndexList selected_indexes( list_->selectionModel()->selectedIndexes() );
    TextMacro::Rule::List selectedAttributes( model_.get( selected_indexes ) );

    TextMacro::Rule::List currentAttributes( rules() );
    TextMacro::Rule::List newAttributes;
    TextMacro::Rule::ListIterator iter( currentAttributes );
    iter.toBack();
    while( iter.hasPrevious() )
    {
        const TextMacro::Rule& current( iter.previous() );

        // check if new list is not empty, current index is selected and last index is not.
        // if yes, move.
        if(
            !( newAttributes.empty() ||
            selected_indexes.indexOf( model_.index( current ) ) == -1 ||
            selected_indexes.indexOf( model_.index( newAttributes.front() ) ) != -1
            ) )
        {

            TextMacro::Rule first( newAttributes.front() );
            newAttributes.pop_front();
            newAttributes.push_front( current );
            newAttributes.push_front( first );

        } else newAttributes.push_front( current );
    }

    model_.set( newAttributes );

    // restore selection
    list_->selectionModel()->select( model_.index( selectedAttributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( TextMacro::Rule::List::const_iterator iter = selectedAttributes.begin(); iter != selectedAttributes.end(); ++iter )
    { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;

}
