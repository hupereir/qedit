
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
  \file FileSelectionDialog.cc
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
#include "EditFrame.h"
#include "TextDisplay.h"
#include "FileSelectionDialog.h"
#include "QtUtil.h"

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

  // custom list view
  list_ = new CustomListView( this );
  layout->addWidget( list_ );
  list_->setColumnCount(2);
  list_->setColumnName( FILE, "file name" );
  list_->setColumnName( CLASS, "class" );
  list_->setSelectionMode( QAbstractItemView::ContiguousSelection );

  // store set of found files to avoid duplication
  std::set< File > file_set;

  // retrieve EditFrames
  BASE::KeySet<EditFrame> frames( dynamic_cast< BASE::Key*>( qApp ) );
  for( BASE::KeySet<EditFrame>::const_iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
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
      const string& class_name( (*iter)->className() );

      CustomListView::Item* item( new CustomListView::Item( list_ ) );
      item->setText( FILE, file.expand().c_str() );
      item->setText( CLASS, class_name.c_str() );

    }
  }

  list_->resizeColumnToContents( FILE );
  list_->selectAll();

  // button layout
  QHBoxLayout *button_layout( new QHBoxLayout() );
  button_layout->setSpacing(5);
  button_layout->setMargin(0);
  layout->addLayout( button_layout );

  QPushButton* button;

  // replace
  button_layout->addWidget( button = new QPushButton( "&Replace", this ) );
  button->setToolTip( "Replace in all selected files" );
  connect( button, SIGNAL( clicked() ), this, SLOT( _replace() ) );

  // select all
  button_layout->addWidget( button = new QPushButton( "&Select All", this ) );
  button->setToolTip( "Select all files in list" );
  connect( button, SIGNAL( clicked() ), list_, SLOT( selectAll() ) );

  // deselect all
  button_layout->addWidget( button = new QPushButton( "&Clear Selection", this ) );
  button->setToolTip( "deselect all files in list" );
  connect( button, SIGNAL( clicked() ), list_, SLOT( clearSelection() ) );

  // cancel
  button_layout->addWidget( button = new QPushButton( "&Cancel", this ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( _cancel() ) );

  adjustSize();
  
}

//________________________________________________________
void FileSelectionDialog::_replace( void )
{

  Debug::Throw( "FileSelectionDialog::_replace.\n" );

  // retrieve selection from the list
  QList< QTreeWidgetItem* > items( list_->QTreeWidget::selectedItems() );
  list<File> files;
  for( QList< QTreeWidgetItem* >::iterator iter = items.begin(); iter != items.end(); iter++ )
  { files.push_back( File( qPrintable( (*iter)->text( FILE ) ) ) ); }

  emit fileSelected( files, selection_ );
  done( QDialog::Accepted );

}
