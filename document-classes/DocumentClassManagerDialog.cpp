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
  \file DocumentClassManagerDialog.cpp
  \brief list document classes
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <qlayout.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include "BaseIcons.h"
#include "CustomFileDialog.h"
#include "PixmapEngine.h"
#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassManagerDialog.h"
#include "DocumentClassManager.h"
#include "DocumentClassDialog.h"
#include "HighlightStyleList.h"

#include "Options.h" 
#include "QtUtil.h"

using namespace std;

//______________________________________________________________
const char* DocumentClassManagerDialog::column_titles_[ DocumentClassManagerDialog::n_columns_ ] =
{
  "name",
  "file"
};

//______________________________________________________________
DocumentClassManagerDialog::DocumentClassManagerDialog( QWidget* parent, DocumentClassManager* manager ):
  CustomDialog( parent ),
  document_class_manager_( manager )
{
  Debug::Throw( "DocumentClassManagerDialog::DocumentClassManagerDialog.\n" );
 
  _setSizeOptionName( "DOCUMENT_CLASS_MANAGER_DIALOG" );
  
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setSpacing(20);
  layout->setMargin(0);
  mainLayout().addLayout( layout );

  //! try load Question icon
  QPixmap question_pixmap( PixmapEngine::get( ICONS::WARNING ) );
  
  // insert main vertical box
  if( !question_pixmap.isNull() )
  {
    QLabel* label = new QLabel( this );
    label->setPixmap( question_pixmap );
    layout->addWidget( label );
  }
  layout->addWidget( new QLabel( 
    "This feature is work-in-progress. Right now this allows \n"
    "to display and edit some of the document classes components, but \n"
    "changes made to these are not stored.", this ), 1 );
  
  layout = new QHBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(0);
  mainLayout().addLayout( layout );
  
  // create list
  layout->addWidget( list_ = new TreeWidget( this ) );
  
  // add columns
  list_->setColumnCount( n_columns_ );
  for( unsigned int i=0; i<n_columns_; i++ )
  { list_->setColumnName( i, column_titles_[i] ); }

  list_->setAllColumnsShowFocus( true );

  // set connections
  connect( list_, SIGNAL( itemActivated( QTreeWidgetItem*, int ) ), this, SLOT( _edit() ) );
  connect( list_, SIGNAL( itemSelectionChanged() ), this, SLOT( _updateButtons() ) );

  // add classes
  _loadClasses();
  list_->sortItems( 0, Qt::AscendingOrder );
  //list_->sort();
  
  // buttons
  QVBoxLayout *v_layout( new QVBoxLayout() );
  v_layout->setSpacing(5);
  layout->setMargin(0);
  layout->addLayout( v_layout );
  
  // edit
  v_layout->addWidget( edit_button_ = new QPushButton( "&Edit", this ) );
  connect( edit_button_, SIGNAL( clicked() ), this, SLOT( _edit() ) ); 
  edit_button_->setToolTip( "Edit file from which selected document class is read" );

  // remove
  v_layout->addWidget( remove_button_ = new QPushButton( "&Remove", this ) );
  connect( remove_button_, SIGNAL( clicked() ), this, SLOT( _remove() ) ); 
  remove_button_->setToolTip( "Remove selected document class from list" );

  // save document class to file
  v_layout->addWidget( save_button_ = new QPushButton( "Save &As", this ) );
  connect( save_button_, SIGNAL( clicked() ), this, SLOT( _save() ) );  
  save_button_->setToolTip( "Save selected document classe to a file" );

  // load
  QPushButton *button;
  v_layout->addWidget( button = new QPushButton( "&Load File", this ) ); 
  connect( button, SIGNAL( clicked() ), this, SLOT( _loadFile() ) );  
  button->setToolTip( "Load additional classes from file" );

  // reload
  v_layout->addWidget( button = new QPushButton( "Rel&oad", this ) ); 
  connect( button, SIGNAL( clicked() ), this, SLOT( _reload() ) );
  button->setToolTip( "Reload all classes" );

  // update buttons
  _updateButtons();
  
  v_layout->addStretch(1);
  adjustSize();

}

//___________________________________________________
void DocumentClassManagerDialog::_updateButtons( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_ubdateButtons.\n" );
  bool has_selection( !list_->QTreeWidget::selectedItems().empty() );  
  edit_button_->setEnabled( has_selection );
  remove_button_->setEnabled( has_selection );
  save_button_->setEnabled( has_selection );

}

//___________________________________________________
void DocumentClassManagerDialog::_edit( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_edit.\n" );
 
  QTreeWidgetItem *item( list_->QTreeWidget::currentItem() );
  if( !item )
  {
    QtUtil::infoDialog( this, "No item selected. <Edit> canceled." );
    return;
  }

  // retrieve DocumentClass matching name
  DocumentClass document_class( document_class_manager_->get(  item->text(NAME) ) );
  
  // create dialog
  DocumentClassDialog dialog( this );
  dialog.setDocumentClass( document_class );
  if( dialog.exec() == QDialog::Accepted ) 
  { emit updateNeeded(); }
  
  return;
}
 
//___________________________________________________
void DocumentClassManagerDialog::_remove( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_remove.\n" );
  Item *item( list_->currentItem<Item>() );
  if( !item )
  {
    QtUtil::infoDialog( this, "No item selected. <Remove> canceled." );
    return;
  }
  
  if( document_class_manager_->remove( item->documentClass().name() ) )
  { 
    _loadClasses(); 
    emit updateNeeded();
  }
  
}

//___________________________________________________
void DocumentClassManagerDialog::_loadFile( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_loadFile.\n" ); 

  // get file from dialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::ExistingFile );
  if( dialog.exec() != QDialog::Accepted ) return;

  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return;
      
  File file = File( qPrintable( files.front() ) ).expand();
  
  // try load from file manager and add to options
  if( document_class_manager_->read( file ) ) 
  {
    XmlOptions::get().add( Option( "PATTERN_FILENAME", file ) );
    _loadClasses();
  }
  return; 
    
}
    
//___________________________________________________
void DocumentClassManagerDialog::_save( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_save.\n" );
  
  QTreeWidgetItem *item( list_->QTreeWidget::currentItem() );
  if( !item )
  {
    QtUtil::infoDialog( this, "No item selected. <Save As> canceled." );
    return;
  }
  
  // retrieve file from dialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::AnyFile );
  if( dialog.exec() != QDialog::Accepted ) return;
  
  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return;
  File file = File( qPrintable( files.front() ) );
  
  // check if file is directory
  if( file.isDirectory() )
  {
    ostringstream what;
    what << "file \"" << file << "\" is a directory. <Save As> canceled.";
    QtUtil::infoDialog( this, what.str() );
    return;
  }

  // check if file exist
  if( file.exists() )
  {
    if( !file.isWritable() )
    {
      ostringstream what;
      what << "file \"" << file << "\" is read-only. <Save As> canceled.";
      QtUtil::infoDialog( this, what.str() );
      return;
    } else if( !QtUtil::questionDialog( this, "selected file already exist. Overwrite ?" ) )
    return;
  }
  
  document_class_manager_->write( qPrintable( item->text( NAME ) ), file );
  return;
}

//___________________________________________________
void DocumentClassManagerDialog::_reload( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_Reload.\n" );  
  _loadClasses();
}

//___________________________________________________ 
void DocumentClassManagerDialog::_loadClasses()
{
  Debug::Throw( "DocumentClassManagerDialog::_loadClasses.\n" );
  
  // clear list
  list_->clear();
  
  // retrieve classes from DocumentClass manager
  const DocumentClassManager::ClassList& classes( document_class_manager_->list() );
  
  // add to list
  for( DocumentClassManager::ClassList::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { _addClass( *iter ); }
}

//_________________________________________________________________
void DocumentClassManagerDialog::_addClass( const DocumentClass& document_class )
{
  Debug::Throw( "DocumentClassManagerDialog::_AddDocumentClass.\n" );
  list_->addTopLevelItem( new Item( document_class ) );
  return;
}

//_________________________________________________________________
void DocumentClassManagerDialog::Item::update( void )
{
  setText( NAME, documentClass().name() );
  setText( FILE, documentClass().file().c_str() );
}
