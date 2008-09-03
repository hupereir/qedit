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
  \file RemoveFilesDialog.cpp
  \brief QDialog used to add selected files to the repository
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QLayout>

#include "BaseIcons.h"
#include "IconEngine.h"
#include "RemoveFilesDialog.h"
#include "TreeView.h"
#include "XmlOptions.h"

using namespace std;

//____________________________________________________________________________
RemoveFilesDialog::RemoveFilesDialog( QWidget* parent, const FileSystemModel::List& files ):
  BaseDialog( parent ),
  Counter( "RemoveFilesDialog" )
{
  
  Debug::Throw( "RemoveFilesDialog::RemoveFilesDialog.\n" );
  setSizeGripEnabled( true );
  
  QVBoxLayout* layout( new QVBoxLayout() );
  layout->setSpacing( 10 );
  layout->setMargin( 10 );
  setLayout( layout );
  
  layout->addWidget( new QLabel( "Selected files: ", this ) );
 
  // horizontal layout for list and comments
  QHBoxLayout* h_layout( new QHBoxLayout() );
  h_layout->setSpacing( 5 );
  h_layout->setMargin( 0 );
  layout->addLayout( h_layout, 1 );

  // file list
  QVBoxLayout* v_layout = new QVBoxLayout();
  v_layout->setSpacing( 5 );
  v_layout->setMargin( 0 );
  h_layout->addLayout( v_layout );

  v_layout->addWidget( list_ = new TreeView( this ), 1 );
  _list().setSelectionMode( QAbstractItemView::MultiSelection ); 
  
  model_.add( files );
  model_.sort( FileSystemModel::FILE, Qt::AscendingOrder );

  _list().setModel( &model_ );
  _list().setMask( XmlOptions::get().get<int>( "FILE_SYSTEM_LIST_MASK" ) );
  _list().resizeColumnToContents( FileSystemModel::FILE );
  _list().selectAll();
    
  // vertical layout for selection buttons and comments
  v_layout = new QVBoxLayout();
  v_layout->setSpacing( 5 );
  v_layout->setMargin( 0 );
  h_layout->addLayout( v_layout );
  
  // select all
  QPushButton* button;
  v_layout->addWidget( button = new QPushButton( "&Select All", this ) );
  button->setToolTip( "Select all files in list" );
  connect( button, SIGNAL( clicked() ), &_list(), SLOT( selectAll() ) );

  // deselect all
  v_layout->addWidget( clear_selection_button_ = new QPushButton( "&Clear Selection", this ) );
  clear_selection_button_->setToolTip( "deselect all files in list" );
  connect( clear_selection_button_, SIGNAL( clicked() ), _list().selectionModel(), SLOT( clear() ) );
  
  v_layout->addWidget( recursive_checkbox_ = new QCheckBox( "Remove recusively", this ) );
  recursive_checkbox_->setToolTip( "Remove directories recursively" );

  v_layout->addStretch(1);
  
  // buttons
  QHBoxLayout* button_layout( new QHBoxLayout() );
  button_layout->setSpacing( 5 );
  button_layout->setMargin( 0 );
  layout->addLayout( button_layout, 0 );
  
  // accept
  button_layout->addWidget( accept_button_ = new QPushButton( IconEngine::get( ICONS::DELETE ), "&Remove", this ) );
  connect( accept_button_, SIGNAL( clicked() ), SLOT( accept() ) );
  accept_button_->setDefault( true );
  
  // cancel
  button_layout->addWidget( cancel_button_ = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&Cancel", this ) );
  connect( cancel_button_, SIGNAL( clicked() ), this, SLOT( reject() ) );
  
  // connection
  connect( _list().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateButtons() ) );
  _updateButtons();

}  

//____________________________________________________________________________
FileSystemModel::List RemoveFilesDialog::selectedFiles( void ) const
{ return _model().get( _list().selectionModel()->selectedRows() ); }

//____________________________________________________________________  
void RemoveFilesDialog::_updateButtons( void )
{
  Debug::Throw( "RemoveFilesDialog::_updateButtons.\n" );
  bool has_selection( !_list().selectionModel()->selectedRows().empty() );
  clear_selection_button_->setEnabled( has_selection );
  accept_button_->setEnabled( has_selection );
}
