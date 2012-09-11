
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
\file IndentPatternRuleList.h
\brief List box for IndentPatternRules
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <QHeaderView>
#include <QLayout>

#include "DocumentClassIcons.h"
#include "IconEngine.h"
#include "IndentPatternRuleDialog.h"
#include "IndentPatternRuleList.h"
#include "InformationDialog.h"
#include "QuestionDialog.h"
#include "TreeView.h"



//____________________________________________________
IndentPatternRuleList::IndentPatternRuleList( QWidget* parent ):
QGroupBox( "Rules", parent ),
Counter( "IndentPatternRuleList" ),
modified_( false )
{
    Debug::Throw( "IndentPatternRuleList::IndentPatternRuleList.\n" );

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
    v_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::ADD ), "Add", this ) );
    button->setToolTip( "Add a new rule to the list" );
    connect( button, SIGNAL( clicked() ), SLOT( _add() ) );

    v_layout->addWidget( remove_button_ = new QPushButton( IconEngine::get( ICONS::REMOVE ), "Remove", this ) );
    remove_button_->setShortcut( QKeySequence::Delete );
    remove_button_->setToolTip( "Remove selected rule" );
    connect( remove_button_, SIGNAL( clicked() ), SLOT( _remove() ) );

    v_layout->addWidget( edit_button_ = new QPushButton( IconEngine::get( ICONS::EDIT ), "Edit", this ) );
    edit_button_->setToolTip( "Edit selected rule" );
    connect( edit_button_, SIGNAL( clicked() ), SLOT( _edit() ) );

    v_layout->addStretch();

    _updateButtons();

}

//____________________________________________________
void IndentPatternRuleList::setRules( const IndentPattern::Rule::List& rule )
{

    Debug::Throw( "IndentPatternRuleList::setRule.\n" );
    model_.set( rule );
    list_->resizeColumns();
    modified_ = false;

}

//____________________________________________________
IndentPattern::Rule::List IndentPatternRuleList::rules( void )
{

    Debug::Throw( "IndentPatternRuleList::rule.\n" );
    return model_.get();

}

//____________________________________________________
void IndentPatternRuleList::_updateButtons( void )
{
    Debug::Throw( "IndentPatternRuleList::_updateButtons.\n" );
    bool hasSelection( !list_->selectionModel()->selectedRows().empty() );
    edit_button_->setEnabled( hasSelection );
    remove_button_->setEnabled( hasSelection );
}

//____________________________________________________
void IndentPatternRuleList::_add( void )
{
    Debug::Throw( "IndentPatternRuleList::_add.\n" );

    IndentPatternRuleDialog dialog( this );
    if( dialog.exec() == QDialog::Rejected ) return;
    model_.add( dialog.rule() );

}

//____________________________________________________
void IndentPatternRuleList::_edit( void )
{
    Debug::Throw( "IndentPatternRuleList::_edit.\n" );

    // retrieve selected items
    QModelIndexList selection( list_->selectionModel()->selectedRows() );
    if( selection.empty() ) {
        InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
        return;
    }

    IndentPatternRuleModel::List rule( model_.get() );
    for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    {

        IndentPattern::Rule old_rule( model_.get( *iter ) );

        IndentPatternRuleDialog dialog( this );
        dialog.setRule( old_rule );
        if( dialog.exec() == QDialog::Rejected ) continue;

        IndentPattern::Rule rule( dialog.rule() );
        if( rule == old_rule ) continue;

        model_.replace( *iter, rule );
        modified_ = true;

    }

}

//____________________________________________________
void IndentPatternRuleList::_remove( void )
{
    Debug::Throw( "IndentPatternRuleList::_remove.\n" );

    // retrieve selected items; make sure they do not include the navigator
    IndentPatternRuleModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
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
void IndentPatternRuleList::_storeSelection( void )
{ model_.setSelectedIndexes( list_->selectionModel()->selectedRows() ); }

//________________________________________
void IndentPatternRuleList::_restoreSelection( void )
{

    const QModelIndexList selection( model_.selectedIndexes() );
    list_->selectionModel()->clearSelection();

    foreach( const QModelIndex& index, selection )
    { list_->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    return;
}
