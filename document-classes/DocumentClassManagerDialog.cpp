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
  \file DocumentClassManagerDialog.cc
  \brief list document classes
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <qlayout.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassManagerDialog.h"
#include "DocumentClassManager.h"
#include "DocumentClassDialog.h"
#include "CustomFileDialog.h"
#include "Options.h" 
#include "QtUtil.h"
#include "XmlOptions.h"

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
  
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(0);
  mainLayout().addLayout( layout );
  
  // create list
  layout->addWidget( list_ = new CustomListView( this ) );
  
  // add columns
  list_->setColumnCount( n_columns_ );
  for( unsigned int i=0; i<n_columns_; i++ )
  { list_->setColumnName( i, column_titles_[i] ); }

  // set connections
  connect( list_, SIGNAL( itemActivated( QTreeWidgetItem*, int ) ), this, SLOT( _edit() ) );

  // add classes
  _loadClasses();
  
  // buttons
  QVBoxLayout *v_layout( new QVBoxLayout() );
  v_layout->setSpacing(5);
  layout->setMargin(0);
  layout->addLayout( v_layout );
  
  QPushButton *button;
  v_layout->addWidget( button = new QPushButton( "&Remove", this ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( _remove() ) ); 
  button->setToolTip( "Remove selected document class from list" );

  v_layout->addWidget( button = new QPushButton( "&Edit", this ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( _edit() ) ); 
  button->setToolTip( "Edit file from which selected document class is read" );

  v_layout->addWidget( button = new QPushButton( "Save &As", this ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( _save() ) );  
  button->setToolTip( "Save selected document classe to a file" );

  v_layout->addWidget( button = new QPushButton( "&Load File", this ) ); 
  connect( button, SIGNAL( clicked() ), this, SLOT( _loadFile() ) );  
  button->setToolTip( "Load additional classes from file" );

  v_layout->addWidget( button = new QPushButton( "Rel&oad", this ) ); 
  connect( button, SIGNAL( clicked() ), this, SLOT( _reload() ) );
  button->setToolTip( "Reload all classes" );

  v_layout->addStretch(1);
  adjustSize();
  resize( 500, 250 );
}

//___________________________________________________
void DocumentClassManagerDialog::_remove( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_remove.\n" );
  QTreeWidgetItem *item( list_->QTreeWidget::currentItem() );
  if( !item )
  {
    QtUtil::infoDialog( this, "No item selected. <Remove> canceled." );
    return;
  }
  
  if( document_class_manager_->remove( qPrintable( item->text( NAME ) ) ) )
  {
    //main_frame.updateEditFrames();
    _loadClasses();
  }
}

//___________________________________________________
void DocumentClassManagerDialog::_edit( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_Edit.\n" );
  
  QTreeWidgetItem *item( list_->QTreeWidget::currentItem() );
  if( !item )
  {
    QtUtil::infoDialog( this, "No item selected. <Edit> canceled." );
    return;
  }

  // retrieve DocumentClass matching name
  string name( qPrintable( item->text(NAME) ) );
  const DocumentClass* document_class( document_class_manager_->get( name ) );
  if( !document_class )
  {
    ostringstream what;
    what << "No document class matching name " << name << ".\n<Edit> canceled";
    QtUtil::infoDialog( this, what.str() );
    return;
  }
  
  // create dialog
  DocumentClassDialog dialog( this );
  dialog.setDocumentClass( *document_class );
  dialog.exec();
  
  return;
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
  { _addClass( **iter ); }
}

//_________________________________________________________________
void DocumentClassManagerDialog::_addClass( const DocumentClass& document_class )
{
  Debug::Throw( "DocumentClassManagerDialog::_AddDocumentClass.\n" );
  CustomListView::Item* item( new CustomListView::Item( list_ ) );
  item->setText( NAME, document_class.name().c_str() );
  item->setText( FILE, document_class.file().c_str() );
  return;
}
