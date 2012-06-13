
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
\file HighlightPatternList.h
\brief List box for HighlightPatterns
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <QHeaderView>
#include <QLayout>

#include "DocumentClassIcons.h"
#include "HighlightPatternDialog.h"
#include "HighlightPatternList.h"
#include "IconEngine.h"
#include "InformationDialog.h"
#include "QuestionDialog.h"
#include "TreeView.h"



//____________________________________________________
HighlightPatternList::HighlightPatternList( QWidget* parent ):
QGroupBox( "Highlight patterns", parent ),
Counter( "HighlightPatternList" )
{
    Debug::Throw( "HighlightPatternList::HighlightPatternList.\n" );

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
    button->setToolTip( "Add a new highlight pattern to the list" );
    connect( button, SIGNAL( clicked() ), SLOT( _add() ) );

    vLayout->addWidget( removeButton_ = new QPushButton( IconEngine::get( ICONS::REMOVE ), "Remove", this ) );
    removeButton_->setShortcut( QKeySequence::Delete );
    removeButton_->setToolTip( "Remove selected highlight pattern" );
    connect( removeButton_, SIGNAL( clicked() ), SLOT( _remove() ) );

    vLayout->addWidget( editButton_ = new QPushButton( IconEngine::get( ICONS::EDIT ), "Edit", this ) );
    editButton_->setToolTip( "Edit selected highlight pattern" );
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
void HighlightPatternList::setPatterns( const HighlightPattern::List& patterns )
{
    Debug::Throw( "HighlightPatternList::setPatterns.\n" );
    model_.set( patterns );
    list_->resizeColumns();
}

//____________________________________________________
HighlightPattern::List HighlightPatternList::patterns( void )
{

    Debug::Throw( "HighlightPatternList::patterns.\n" );
    return model_.get();

}

//____________________________________________________
void HighlightPatternList::_updateButtons( void )
{
    Debug::Throw( "HighlightPatternList::_updateButtons.\n" );
    bool has_selection( !list_->selectionModel()->selectedRows().empty() );
    editButton_->setEnabled( has_selection );
    removeButton_->setEnabled( has_selection );
    moveUpButton_->setEnabled( has_selection );
    moveDownButton_->setEnabled( has_selection );
}

//____________________________________________________
void HighlightPatternList::_add( void )
{
    Debug::Throw( "HighlightPatternList::_add.\n" );

    // get set of highlight patterns to ensure name unicity
    HighlightPatternModel::List patterns( model_.get() );

    HighlightPatternDialog dialog( this );
    dialog.setStyles( styles_ );
    dialog.setPatterns( patterns );
    while( 1 )
    {
        if( dialog.exec() == QDialog::Rejected ) return;
        HighlightPattern pattern( dialog.pattern() );
        if( pattern.name().isEmpty() || std::find( patterns.begin(), patterns.end(), pattern ) != patterns.end() )
        {
            InformationDialog( this, "Invalid pattern name" ).exec();
        } else {
            model_.add( pattern );
            emit modified();
            break;
        }
    }

}

//____________________________________________________
void HighlightPatternList::_edit( void )
{
    Debug::Throw( "HighlightPatternList::_edit.\n" );

    // retrieve selected items
    QModelIndexList selection( list_->selectionModel()->selectedRows() );
    if( selection.empty() ) {
        InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
        return;
    }

    HighlightPatternModel::List patterns( model_.get() );
    for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    {

        HighlightPattern old_pattern( model_.get( *iter ) );

        HighlightPatternDialog dialog( this );
        dialog.setStyles( styles_ );
        dialog.setPatterns( patterns );
        dialog.setPattern( old_pattern );
        if( dialog.exec() == QDialog::Rejected ) continue;

        HighlightPattern pattern( dialog.pattern() );
        if( !( pattern == old_pattern ) )
        {
            model_.replace( *iter, pattern );
            emit modified();
        }

    }

}

//____________________________________________________
void HighlightPatternList::_remove( void )
{
    Debug::Throw( "HighlightPatternList::_remove.\n" );

    // retrieve selected items; make sure they do not include the navigator
    HighlightPatternModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
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
    emit modified();

}

//________________________________________
void HighlightPatternList::_storeSelection( void )
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
void HighlightPatternList::_restoreSelection( void )
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
void HighlightPatternList::_up( void )
{

    Debug::Throw( "HighlightPatternList::_up.\n" );
    HighlightPattern::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no item selected. <Move up> canceled" ).exec();
        return;
    }

    // retrieve selected indexes in list and store in model
    QModelIndexList selectedIndexes( list_->selectionModel()->selectedRows() );
    HighlightPattern::List selectedAttributes( model_.get( selectedIndexes ) );

    HighlightPattern::List currentAttributes( patterns() );
    HighlightPattern::List newAttributes;

    for( HighlightPattern::List::const_iterator iter = currentAttributes.begin(); iter != currentAttributes.end(); ++iter )
    {

        // check if new list is not empty, current index is selected and last index is not.
        // if yes, move.
        if(
            !( newAttributes.empty() ||
            selectedIndexes.indexOf( model_.index( *iter ) ) == -1 ||
            selectedIndexes.indexOf( model_.index( newAttributes.back() ) ) != -1
            ) )
        {
            HighlightPattern last( newAttributes.back() );
            newAttributes.pop_back();
            newAttributes.push_back( *iter );
            newAttributes.push_back( last );
        } else newAttributes.push_back( *iter );

    }

    model_.set( newAttributes );

    // restore selection
    list_->selectionModel()->select( model_.index( selectedAttributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( HighlightPattern::List::const_iterator iter = selectedAttributes.begin(); iter != selectedAttributes.end(); ++iter )
    { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;

}

//_________________________________________________________
void HighlightPatternList::_down( void )
{

    Debug::Throw( "HighlightPatternList::_down.\n" );
    HighlightPattern::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no item selected. <Move down> canceled" ).exec();
        return;
    }

    // retrieve selected indexes in list and store in model
    QModelIndexList selectedIndexes( list_->selectionModel()->selectedIndexes() );
    HighlightPattern::List selectedAttributes( model_.get( selectedIndexes ) );

    HighlightPattern::List currentAttributes( patterns() );
    HighlightPattern::List newAttributes;
    HighlightPattern::ListIterator iter( currentAttributes );
    iter.toBack();
    while( iter.hasPrevious() )
    {
        const HighlightPattern& current( iter.previous() );

        // check if new list is not empty, current index is selected and last index is not.
        // if yes, move.
        if(
            !( newAttributes.empty() ||
            selectedIndexes.indexOf( model_.index( current ) ) == -1 ||
            selectedIndexes.indexOf( model_.index( newAttributes.front() ) ) != -1
            ) )
        {

            HighlightPattern first( newAttributes.front() );
            newAttributes.pop_front();
            newAttributes.push_front( current );
            newAttributes.push_front( first );

        } else newAttributes.push_front( current );
    }

    model_.set( newAttributes );

    // restore selection
    list_->selectionModel()->select( model_.index( selectedAttributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( HighlightPattern::List::const_iterator iter = selectedAttributes.begin(); iter != selectedAttributes.end(); ++iter )
    { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;

}
