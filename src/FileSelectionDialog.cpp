// $Id$

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public license as published by the Free Software
* Foundation; either version 2 of the license, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public license
* for more details.
*
* You should have received a copy of the GNU General Public license along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file FileSelectionDialog.cpp
  \brief QDialog used to select opened files
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <algorithm>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "Application.h"
#include "Debug.h"
#include "MainWindow.h"
#include "RecentFilesMenu.h"
#include "FileSelectionDialog.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Singleton.h"
#include "TextDisplay.h"
#include "TreeView.h"
#include "WindowServer.h"
#include "XmlOptions.h"

using namespace std;

//________________________________________________________
FileSelectionDialog::FileSelectionDialog( QWidget* parent, const TextSelection& selection ):
  CustomDialog( parent ),
  selection_( selection )
{
  
  setWindowTitle( "file selection" );
  setOptionName( "FILE_SELECTION_DIALOG" );
  
  // custom list display
  list_ = new TreeView( this );
  _list().setModel( &model_ );
  _list().setSelectionMode( QAbstractItemView::MultiSelection );
  connect( _list().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateButtons() ) );

  // retrieve file records
  model_.set( Singleton::get().application<Application>()->windowServer().records() );
  
  // mask
  unsigned int mask( 
    (1<<FileRecordModel::ICON)|
    (1<<FileRecordModel::FILE)|
    (1<<FileRecordModel::PATH ));
  int class_column( model_.findColumn( "class_name" ) );
  if( class_column >= 0 ) mask |= (1<<class_column);
  _list().setMask( mask );
  _list().resizeColumns();  
  mainLayout().addWidget( list_ );

  // generic button
  QPushButton* button;  

  // deselect all
  buttonLayout().insertWidget( 0, button = new QPushButton( "&Clear Selection", this ) );
  button->setToolTip( "Deselect all files in list" );
  connect( button, SIGNAL( clicked() ), list_, SLOT( clearSelection() ) );
  clear_selection_button_ = button;

  // select all
  buttonLayout().insertWidget( 0, button = new QPushButton( "&Select All", this ) );
  button->setToolTip( "Select all files in list" );
  connect( button, SIGNAL( clicked() ), list_, SLOT( selectAll() ) );
  select_all_button_ = button;
    
  // replace
  okButton().setToolTip( "Replace in all selected files" );
  okButton().setText( "&Replace" );
  _updateButtons();

  // sort list and select all items
  if( XmlOptions::get().find( "SESSION_FILES_SORT_COLUMN" ) && XmlOptions::get().find( "SESSION_FILES_SORT_ORDER" ) )
  { 
    _list().sortByColumn( 
      XmlOptions::get().get<int>( "SESSION_FILES_SORT_COLUMN" ), 
      (Qt::SortOrder)(XmlOptions::get().get<int>( "SESSION_FILES_SORT_ORDER" ) ) ); 
  }

  _list().selectAll();

  
  adjustSize();
  
}

//________________________________________________________
void FileSelectionDialog::_updateButtons( void )
{

  Debug::Throw( "FileSelectionDialog::_updateButtons.\n" );
  QList<QModelIndex> selection( _list().selectionModel()->selectedRows() );
  
  clear_selection_button_->setEnabled( !selection.empty() );
  okButton().setEnabled( !selection.empty() );
  
}

//________________________________________________________
FileSelectionDialog::FileList FileSelectionDialog::selectedFiles( void ) const
{

  Debug::Throw( "FileSelectionDialog::_replace.\n" );

  // retrieve selection from the list
  FileRecordModel::List selection( model_.get( _list().selectionModel()->selectedRows() ) );
  FileList files;
  for( FileRecordModel::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { files.push_back( iter->file() ); }

  return files;
}
