
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
\file IndentPatternList.h
\brief List box for IndentPatterns
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <QHeaderView>
#include <QLayout>

#include "DocumentClassIcons.h"
#include "IconEngine.h"
#include "IndentPatternDialog.h"
#include "IndentPatternList.h"
#include "InformationDialog.h"
#include "QuestionDialog.h"
#include "TreeView.h"

using namespace std;

//____________________________________________________
IndentPatternList::IndentPatternList( QWidget* parent ):
    QGroupBox( "Indentation patterns", parent ),
    Counter( "IndentPatternList" ),
    modified_( false )
{
    Debug::Throw( "IndentPatternList::IndentPatternList.\n" );

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
    button->setToolTip( "Add a new Indent pattern to the list" );
    connect( button, SIGNAL( clicked() ), SLOT( _add() ) );

    v_layout->addWidget( removeButton_ = new QPushButton( IconEngine::get( ICONS::REMOVE ), "&Remove", this ) );
    removeButton_->setToolTip( "Remove selected Indent pattern" );
    removeButton_->setShortcut( Qt::Key_Delete );
    connect( removeButton_, SIGNAL( clicked() ), SLOT( _remove() ) );

    v_layout->addWidget( editButton_ = new QPushButton( IconEngine::get( ICONS::EDIT ), "&Edit", this ) );
    editButton_->setToolTip( "Edit selected Indent pattern" );
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
void IndentPatternList::setPatterns( const IndentPattern::List& patterns )
{

    Debug::Throw( "IndentPatternList::setPatterns.\n" );
    model_.set( patterns );
    list_->resizeColumns();
    modified_ = false;

}

//____________________________________________________
IndentPattern::List IndentPatternList::patterns( void )
{

    Debug::Throw( "IndentPatternList::patterns.\n" );
    return model_.get();

}

//____________________________________________________
void IndentPatternList::_updateButtons( void )
{
    Debug::Throw( "IndentPatternList::_updateButtons.\n" );
    bool hasSelection( !list_->selectionModel()->selectedRows().empty() );
    editButton_->setEnabled( hasSelection );
    removeButton_->setEnabled( hasSelection );
    moveUpButton_->setEnabled( hasSelection );
    moveDownButton_->setEnabled( hasSelection );
}

//____________________________________________________
void IndentPatternList::_add( void )
{
    Debug::Throw( "IndentPatternList::_add.\n" );

    // get set of Indent patterns to ensure name unicity
    IndentPatternModel::List patterns( model_.get() );

    IndentPatternDialog dialog( this );
    while( 1 )
    {
        if( dialog.exec() == QDialog::Rejected ) return;
        IndentPattern pattern( dialog.pattern() );
        if( pattern.name().isEmpty() || std::find( patterns.begin(), patterns.end(), pattern ) != patterns.end() )
        {
            InformationDialog( this, "Invalid pattern name" ).exec();
        } else {
            model_.add( pattern );
            break;
        }
    }

}

//____________________________________________________
void IndentPatternList::_edit( void )
{
    Debug::Throw( "IndentPatternList::_edit.\n" );

    // retrieve selected items
    QModelIndexList selection( list_->selectionModel()->selectedRows() );
    if( selection.empty() ) {
        InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
        return;
    }

    IndentPatternModel::List patterns( model_.get() );
    for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    {

        IndentPattern old_pattern( model_.get( *iter ) );

        IndentPatternDialog dialog( this );
        dialog.setPattern( old_pattern );
        if( dialog.exec() == QDialog::Rejected ) continue;

        IndentPattern pattern( dialog.pattern() );
        if( !( pattern == old_pattern ) )
        {
            model_.replace( *iter, pattern );
            modified_ = true;
        }

    }

}

//____________________________________________________
void IndentPatternList::_remove( void )
{
    Debug::Throw( "IndentPatternList::_remove.\n" );

    // retrieve selected items; make sure they do not include the navigator
    IndentPatternModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
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
void IndentPatternList::_storeSelection( void )
{
    // clear
    model_.clearSelectedIndexes();

    // retrieve selected indexes in list
    QModelIndexList selectedIndexes( list_->selectionModel()->selectedRows() );
    for( QModelIndexList::iterator iter = selectedIndexes.begin(); iter != selectedIndexes.end(); ++iter )
    {
        // check column
        if( !iter->column() == 0 ) continue;
        model_.setIndexSelected( *iter, true );
    }

}

//________________________________________
void IndentPatternList::_restoreSelection( void )
{

    // retrieve indexes
    QModelIndexList selectedIndexes( model_.selectedIndexes() );
    if( selectedIndexes.empty() ) list_->selectionModel()->clear();
    else {

        list_->selectionModel()->select( selectedIndexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        for( QModelIndexList::const_iterator iter = selectedIndexes.begin(); iter != selectedIndexes.end(); ++iter )
        { list_->selectionModel()->select( *iter, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    }

    return;
}


//_________________________________________________________
void IndentPatternList::_up( void )
{

    Debug::Throw( "IndentPatternList::_up.\n" );

    // retrieve selected indexes in list and store in model
    QModelIndexList selectedIndexes( list_->selectionModel()->selectedRows() );
    if( selectedIndexes.empty() )
    {
        InformationDialog( this, "no item selected. <Move up> canceled" ).exec();
        return;
    }

    IndentPattern::List selected_attributes( model_.get( selectedIndexes ) );

    IndentPattern::List current_attributes( patterns() );
    IndentPattern::List new_attributes;

    for( IndentPattern::List::const_iterator iter = current_attributes.begin(); iter != current_attributes.end(); ++iter )
    {

        // check if new list is not empty, current index is selected and last index is not.
        // if yes, move.
        if(
            !( new_attributes.empty() ||
            selectedIndexes.indexOf( model_.index( *iter ) ) == -1 ||
            selectedIndexes.indexOf( model_.index( new_attributes.back() ) ) != -1
            ) )
        {
            IndentPattern last( new_attributes.back() );
            new_attributes.pop_back();
            new_attributes.push_back( *iter );
            new_attributes.push_back( last );
        } else new_attributes.push_back( *iter );

    }

    model_.set( new_attributes );

    // restore selection
    list_->selectionModel()->select( model_.index( selected_attributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( IndentPattern::List::const_iterator iter = selected_attributes.begin(); iter != selected_attributes.end(); ++iter )
    { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;

}

//_________________________________________________________
void IndentPatternList::_down( void )
{

    Debug::Throw( "IndentPatternList::_down.\n" );

    // retrieve selected indexes in list and store in model
    QModelIndexList selectedIndexes( list_->selectionModel()->selectedIndexes() );
    if( selectedIndexes.empty() )
    {
        InformationDialog( this, "no item selected. <Move up> canceled" ).exec();
        return;
    }
    IndentPattern::List selected_attributes( model_.get( selectedIndexes ) );

    IndentPattern::List current_attributes( patterns() );
    IndentPattern::List new_attributes;

    for( IndentPattern::List::reverse_iterator iter = current_attributes.rbegin(); iter != current_attributes.rend(); ++iter )
    {

        // check if new list is not empty, current index is selected and last index is not.
        // if yes, move.
        if(
            !( new_attributes.empty() ||
            selectedIndexes.indexOf( model_.index( *iter ) ) == -1 ||
            selectedIndexes.indexOf( model_.index( new_attributes.back() ) ) != -1
            ) )
        {

            IndentPattern last( new_attributes.back() );
            new_attributes.pop_back();
            new_attributes.push_back( *iter );
            new_attributes.push_back( last );

        } else new_attributes.push_back( *iter );
    }

    model_.set( IndentPatternModel::List( new_attributes.rbegin(), new_attributes.rend() ) );

    // restore selection
    list_->selectionModel()->select( model_.index( selected_attributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( IndentPattern::List::const_iterator iter = selected_attributes.begin(); iter != selected_attributes.end(); ++iter )
    { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;

}
