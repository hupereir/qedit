
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

#include "HighlightPatternDialog.h"
#include "HighlightPatternList.h"
#include "InformationDialog.h"
#include "QuestionDialog.h"
#include "TreeView.h"

using namespace std;

//____________________________________________________
HighlightPatternList::HighlightPatternList( QWidget* parent ):
  QGroupBox( "Highlight patterns", parent ),
  Counter( "HighlightPatternList" ),
  modified_( false )
{
  Debug::Throw( "HighlightPatternList::HighlightPatternList.\n" );

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
  button->setToolTip( "Add a new highlight pattern to the list" );
  connect( button, SIGNAL( clicked() ), SLOT( _add() ) );
  
  v_layout->addWidget( edit_button_ = new QPushButton( "&Edit", this ) );
  edit_button_->setToolTip( "Edit selected highlight pattern" );
  connect( edit_button_, SIGNAL( clicked() ), SLOT( _edit() ) );

  v_layout->addWidget( remove_button_ = new QPushButton( "&Remove", this ) );
  remove_button_->setToolTip( "Remove selected highlight pattern" );
  connect( remove_button_, SIGNAL( clicked() ), SLOT( _remove() ) );
  
  v_layout->addWidget( button = new QPushButton( "Move &up", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _up() ) );
  button->setToolTip( "move up selected items" );
  move_up_button_ = button;
  
  v_layout->addWidget( button = new QPushButton( "Move &down", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _down() ) );
  button->setToolTip( "move down selected items" );  
  move_down_button_ = button;
  
  v_layout->addStretch();
  
  _updateButtons();
  
}

//____________________________________________________
void HighlightPatternList::setPatterns( const HighlightPattern::List& patterns ) 
{

  Debug::Throw( "HighlightPatternList::setPatterns.\n" );
  model_.set( patterns );
  list_->resizeColumns();
  modified_ = false;
  
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
  edit_button_->setEnabled( has_selection );
  remove_button_->setEnabled( has_selection );
  move_up_button_->setEnabled( has_selection );
  move_down_button_->setEnabled( has_selection );
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
  for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
  
    HighlightPattern old_pattern( model_.get( *iter ) );

    HighlightPatternDialog dialog( this );
    dialog.setStyles( styles_ );
    dialog.setPatterns( patterns );
    dialog.setPattern( old_pattern );
    if( dialog.exec() == QDialog::Rejected ) continue;
    
    HighlightPattern pattern( dialog.pattern() );
    if( pattern.differs( old_pattern ) ) 
    { 
      model_.replace( *iter, pattern ); 
      modified_ = true;
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
  ostringstream what;
  what << "Remove selected item";
  if( selection.size()>1 ) what << "s";
  what << " ?";
  if( !QuestionDialog( this, what.str().c_str() ).exec() ) return;
  
  // remove items
  model_.remove( selection );
  modified_ = true;
  
}

//________________________________________
void HighlightPatternList::_storeSelection( void )
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
void HighlightPatternList::_restoreSelection( void )
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
  QModelIndexList selected_indexes( list_->selectionModel()->selectedRows() );
  HighlightPattern::List selected_attributes( model_.get( selected_indexes ) );
  
  HighlightPattern::List current_attributes( patterns() );
  HighlightPattern::List new_attributes;
  
  for( HighlightPattern::List::const_iterator iter = current_attributes.begin(); iter != current_attributes.end(); iter++ )
  {

    // check if new list is not empty, current index is selected and last index is not.
    // if yes, move.
    if( 
      !( new_attributes.empty() || 
      selected_indexes.indexOf( model_.index( *iter ) ) == -1 ||
      selected_indexes.indexOf( model_.index( new_attributes.back() ) ) != -1 
      ) )
    { 
      HighlightPattern last( new_attributes.back() );
      new_attributes.pop_back();
      new_attributes.push_back( *iter );
      new_attributes.push_back( last );
    } else new_attributes.push_back( *iter );
    
  }

  model_.set( new_attributes );
  
  // restore selection
  list_->selectionModel()->select( model_.index( selected_attributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
  for( HighlightPattern::List::const_iterator iter = selected_attributes.begin(); iter != selected_attributes.end(); iter++ )
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
  QModelIndexList selected_indexes( list_->selectionModel()->selectedIndexes() );
  HighlightPattern::List selected_attributes( model_.get( selected_indexes ) );
  
  HighlightPattern::List current_attributes( patterns() );
  HighlightPattern::List new_attributes;
  
  for( HighlightPattern::List::reverse_iterator iter = current_attributes.rbegin(); iter != current_attributes.rend(); iter++ )
  {
   
    // check if new list is not empty, current index is selected and last index is not.
    // if yes, move.
    if( 
      !( new_attributes.empty() || 
      selected_indexes.indexOf( model_.index( *iter ) ) == -1 ||
      selected_indexes.indexOf( model_.index( new_attributes.back() ) ) != -1 
      ) )
    { 
      
      HighlightPattern last( new_attributes.back() );
      new_attributes.pop_back();
      new_attributes.push_back( *iter );
      new_attributes.push_back( last );
      
    } else new_attributes.push_back( *iter );
  }
    
  model_.set( HighlightPattern::List( new_attributes.rbegin(), new_attributes.rend() ) );
  
  // restore selection
  list_->selectionModel()->select( model_.index( selected_attributes.front() ),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
  for( HighlightPattern::List::const_iterator iter = selected_attributes.begin(); iter != selected_attributes.end(); iter++ )
  { list_->selectionModel()->select( model_.index( *iter ), QItemSelectionModel::Select|QItemSelectionModel::Rows ); }
      
  return;
    
}
