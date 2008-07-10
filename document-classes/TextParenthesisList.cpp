
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
  \file TextParenthesisList.h
  \brief List box for TextParenthesiss
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QHeaderView>
#include <QLayout>

#include "TextParenthesisDialog.h"
#include "TextParenthesisList.h"
#include "QtUtil.h"
#include "TreeView.h"

using namespace std;

//____________________________________________________
TextParenthesisList::TextParenthesisList( QWidget* parent ):
  QGroupBox( "Parenthesis", parent ),
  Counter( "TextParenthesisList" ),
  modified_( false )
{
  Debug::Throw( "TextParenthesisList::TextParenthesisList.\n" );

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
  v_layout->addWidget( button = new QPushButton( "&Add", this ) );
  button->setToolTip( "Add a new parenthesis to the list" );
  connect( button, SIGNAL( clicked() ), SLOT( _add() ) );
  
  v_layout->addWidget( edit_button_ = new QPushButton( "&Edit", this ) );
  edit_button_->setToolTip( "Edit selected parenthesis" );
  connect( edit_button_, SIGNAL( clicked() ), SLOT( _edit() ) );

  v_layout->addWidget( remove_button_ = new QPushButton( "&Remove", this ) );
  remove_button_->setToolTip( "Remove selected parenthesis" );
  connect( remove_button_, SIGNAL( clicked() ), SLOT( _remove() ) );
  
  v_layout->addStretch();
  
  _updateButtons();
  
}

//____________________________________________________
void TextParenthesisList::setParenthesis( const TextParenthesis::List& parenthesis ) 
{

  Debug::Throw( "TextParenthesisList::setParenthesis.\n" );
  model_.set( parenthesis );
  list_->resizeColumns();
  modified_ = false;
  
}

//____________________________________________________
TextParenthesis::List TextParenthesisList::parenthesis( void ) 
{
  
  Debug::Throw( "TextParenthesisList::parenthesis.\n" );
  return model_.get();
  
}

//____________________________________________________
void TextParenthesisList::_updateButtons( void )
{
  Debug::Throw( "TextParenthesisList::_updateButtons.\n" );
  bool has_selection( !list_->selectionModel()->selectedRows().empty() );
  edit_button_->setEnabled( has_selection );
  remove_button_->setEnabled( has_selection );
}

//____________________________________________________
void TextParenthesisList::_add( void )
{
  Debug::Throw( "TextParenthesisList::_add.\n" );
   
  TextParenthesisDialog dialog( this );
  if( dialog.exec() == QDialog::Rejected ) return;
  model_.add( dialog.parenthesis() );
   
}

//____________________________________________________
void TextParenthesisList::_edit( void )
{
  Debug::Throw( "TextParenthesisList::_edit.\n" );
 
  // retrieve selected items
  QModelIndexList selection( list_->selectionModel()->selectedRows() );
  if( selection.empty() ) {
    QtUtil::infoDialog( this, "No item selected. <Remove> canceled." );
    return;
  }

  TextParenthesisModel::List parenthesis( model_.get() );
  for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
  
    TextParenthesis old_parenthesis( model_.get( *iter ) );

    TextParenthesisDialog dialog( this );
    dialog.setParenthesis( old_parenthesis );
    if( dialog.exec() == QDialog::Rejected ) continue;
    
    TextParenthesis parenthesis( dialog.parenthesis() );
    if( parenthesis == old_parenthesis ) continue; 
    
    model_.replace( *iter, parenthesis ); 
    modified_ = true;

  }
  
}

//____________________________________________________
void TextParenthesisList::_remove( void )
{
  Debug::Throw( "TextParenthesisList::_remove.\n" );

  // retrieve selected items; make sure they do not include the navigator
  TextParenthesisModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
  if( selection.empty() ) {
    QtUtil::infoDialog( this, "No item selected. <Remove> canceled." );
    return;
  }
  
  // ask for confirmation
  ostringstream what;
  what << "Remove selected item";
  if( selection.size()>1 ) what << "s";
  what << " ?";
  if( !QtUtil::questionDialog( this, what.str() ) ) return;
  
  // remove items
  model_.remove( selection );
  modified_ = true;
  
}

//________________________________________
void TextParenthesisList::_storeSelection( void )
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
void TextParenthesisList::_restoreSelection( void )
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