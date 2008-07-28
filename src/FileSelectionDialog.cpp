
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

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "Debug.h"
#include "MainWindow.h"
#include "FileSelectionDialog.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Application.h"
#include "QtUtil.h"
#include "TextDisplay.h"
#include "TreeView.h"

using namespace std;

//________________________________________________________
FileSelectionDialog::FileSelectionDialog( QWidget* parent, const TextSelection& selection ):
  QDialog( parent ),
  Counter( "FileSelectionDialog" ),
  selection_( selection )
{

  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(10);
  setLayout( layout );

  // custom list display
  list_ = new TreeView( this );
  list_->setModel( &model_ );
  list_->setSelectionMode( QAbstractItemView::MultiSelection );

  // store set of found files to avoid duplication
  std::set< File > file_set;

  // retrieve MainWindows
  BASE::KeySet<MainWindow> frames( static_cast< Application*>( qApp ) );
  for( BASE::KeySet<MainWindow>::const_iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
  {

    // retrieve associated TextDisplays
    BASE::KeySet<TextDisplay> displays( *frame_iter );
    for( BASE::KeySet<TextDisplay>::const_iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      
      // retrieve filename
      const File& file( (*iter)->file() );
      if( file.empty() ) continue;

      // try add file, skipp if already inserted
      if( !file_set.insert( file ).second ) continue;

      // retrieve document class
      const QString& class_name( (*iter)->className() );
      model_.add( std::make_pair( file.expand(), class_name ) );

    }
  }

  list_->resizeColumnToContents( Model::FILE );
  list_->selectAll();
  layout->addWidget( list_ );

  // button layout
  QHBoxLayout *button_layout( new QHBoxLayout() );
  button_layout->setSpacing(5);
  button_layout->setMargin(0);
  layout->addLayout( button_layout );

  // generic button
  QPushButton* button;  

  // select all
  button_layout->addWidget( button = new QPushButton( "&Select All", this ) );
  button->setToolTip( "Select all files in list" );
  connect( button, SIGNAL( clicked() ), list_, SLOT( selectAll() ) );

  // deselect all
  button_layout->addWidget( button = new QPushButton( "&Clear Selection", this ) );
  button->setToolTip( "deselect all files in list" );
  connect( button, SIGNAL( clicked() ), list_, SLOT( clearSelection() ) );

  // replace
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_ACCEPT ), "&Replace", this ) );
  button->setToolTip( "Replace in all selected files" );
  connect( button, SIGNAL( clicked() ), this, SLOT( _replace() ) );

  // cancel
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&Cancel", this ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( _cancel() ) );

  adjustSize();
  
}

//________________________________________________________
void FileSelectionDialog::_replace( void )
{

  Debug::Throw( "FileSelectionDialog::_replace.\n" );

  // retrieve selection from the list
  Model::List selection( model_.get( list_->selectionModel()->selectedRows() ) );
  list<File> files;
  for( Model::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { files.push_back( iter->first ); }

  emit fileSelected( files, selection_ );
  done( QDialog::Accepted );

}

//_______________________________________________
const char* FileSelectionDialog::Model::column_titles_[ FileSelectionDialog::Model::n_columns ] =
{ 
  "file name",
  "path",
  "class"
};

//__________________________________________________________________
QVariant FileSelectionDialog::Model::data( const QModelIndex& index, int role ) const
{
  Debug::Throw( "FileSelectionDialog::Model::data.\n" );
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  
  // retrieve associated file info
  const FilePair& file( get(index) );
  
  // return text associated to file and column
  if( role == Qt::DisplayRole ) {
    
    switch( index.column() )
    {
      case FILE:
      return QString( file.first.localName().c_str() );

      case PATH:
      return QString( file.first.path().c_str() );
      
      case CLASS:
      return QString( file.second );
      
      default:
      return QVariant();
    }
  }
 
  return QVariant();
  
}

//__________________________________________________________________
QVariant FileSelectionDialog::Model::headerData(int section, Qt::Orientation orientation, int role) const
{

  if( 
    orientation == Qt::Horizontal && 
    role == Qt::DisplayRole && 
    section >= 0 && 
    section < n_columns )
  { return QString( column_titles_[section] ); }
  
  // return empty
  return QVariant(); 

}
