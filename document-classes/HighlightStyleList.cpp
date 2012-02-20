
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
  \file HighlightStyleList.h
  \brief List box for HighlightStyles
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QHeaderView>
#include <QLayout>

#include "BaseIcons.h"
#include "HighlightStyleDialog.h"
#include "HighlightStyleList.h"
#include "IconEngine.h"
#include "InformationDialog.h"
#include "QuestionDialog.h"
#include "TreeView.h"



//____________________________________________________
HighlightStyleList::HighlightStyleList( QWidget* parent ):
  QGroupBox( "Highlight styles", parent ),
  Counter( "HighlightStyleList" )
{
  Debug::Throw( "HighlightStyleList::HighlightStyleList.\n" );

  QHBoxLayout* hLayout;
  hLayout = new QHBoxLayout();
  hLayout->setSpacing(5);
  hLayout->setMargin(5);
  setLayout( hLayout );


  hLayout->addWidget( list_ = new TreeView( this ), 1 );
  list_->setModel( &model_ );
  list_->setSortingEnabled( true );
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
  button->setToolTip( "Add a new highlight style to the list" );
  connect( button, SIGNAL( clicked() ), SLOT( _add() ) );

  vLayout->addWidget( remove_button_ = new QPushButton( IconEngine::get( ICONS::REMOVE ), "&Remove", this ) );
  remove_button_->setToolTip( "Remove selected highlight style" );
  remove_button_->setShortcut( Qt::Key_Delete );
  connect( remove_button_, SIGNAL( clicked() ), SLOT( _remove() ) );

  vLayout->addWidget( edit_button_ = new QPushButton( IconEngine::get( ICONS::EDIT ), "&Edit", this ) );
  edit_button_->setToolTip( "Edit selected highlight style" );
  connect( edit_button_, SIGNAL( clicked() ), SLOT( _edit() ) );

  vLayout->addStretch();

  _updateButtons();

}

//____________________________________________________
void HighlightStyleList::setStyles( const HighlightStyle::Set& styles )
{ model_.set( styles.toList() ); }

//____________________________________________________
HighlightStyle::Set HighlightStyleList::styles( void )
{ return HighlightStyle::Set::fromList( model_.get() ); }

//____________________________________________________
void HighlightStyleList::_updateButtons( void )
{
  Debug::Throw( "HighlightStyleList::_updateButtons.\n" );
  bool has_selection( !list_->selectionModel()->selectedRows().empty() );
  edit_button_->setEnabled( has_selection );
  remove_button_->setEnabled( has_selection );
}

//____________________________________________________
void HighlightStyleList::_add( void )
{
  Debug::Throw( "HighlightStyleList::_add.\n" );

  // get set of highlight styles to ensure name unicity
  HighlightStyleModel::List styles( model_.get() );

  HighlightStyleDialog dialog( this );
  while( 1 )
  {
    if( dialog.exec() == QDialog::Rejected ) return;
    HighlightStyle style( dialog.style() );
    if( style.name().isEmpty() || std::find( styles.begin(), styles.end(), style ) != styles.end() )
    {
      InformationDialog( this, "Invalid pattern name" ).exec();
    } else {
      model_.add( style );
      emit modified();
      break;
    }
  }

}

//____________________________________________________
void HighlightStyleList::_edit( void )
{
  Debug::Throw( "HighlightStyleList::_edit.\n" );

  // retrieve selected items;
  QModelIndexList selection( list_->selectionModel()->selectedRows() );
  if( selection.empty() ) {
    InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
    return;
  }

  for( QModelIndexList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
  {

    HighlightStyle old_style( model_.get( *iter ) );

    HighlightStyleDialog dialog( this );
    dialog.setStyle( old_style );
    if( dialog.exec() == QDialog::Rejected ) continue;

    HighlightStyle style( dialog.style() );
    if( !( style == old_style ) )
    {
      model_.replace( *iter, style );
      emit modified();
    }

  }

}

//____________________________________________________
void HighlightStyleList::_remove( void )
{
  Debug::Throw( "HighlightStyleList::_remove.\n" );

  // retrieve selected items; make sure they do not include the navigator
  HighlightStyleModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
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
void HighlightStyleList::_storeSelection( void )
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
void HighlightStyleList::_restoreSelection( void )
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
