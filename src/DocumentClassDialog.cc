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
  \file DocumentClassDialog.cc
  \brief list document classes
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>
#include <QPushButton>

#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassDialog.h"
#include "DocumentClassManager.h"
#include "CustomFileDialog.h"
#include "MainFrame.h"
#include "XmlOptions.h" 
#include "QtUtil.h"

using namespace std;

//______________________________________________________________
const char* DocumentClassDialog::column_titles_[ DocumentClassDialog::n_columns ] =
{
  "name",
  "file"
};

//______________________________________________________________
DocumentClassDialog::DocumentClassDialog( QWidget* parent ):
  QDialog( parent ),
  Counter( "DocumentClassDialog" )
{
  Debug::Throw( "DocumentClassDialog::DocumentClassDialog.\n" );
  setWindowTitle( "QEdit - Document Classes" );
  
  // horizontal layout
  QHBoxLayout* h_layout( new QHBoxLayout() );
  h_layout->setMargin(0);
  h_layout->setSpacing(5);
  setLayout( h_layout );
  
  // create list
  h_layout->addWidget( list_ = new CustomListView( this ), 1 );
  list_->setColumnCount(n_columns);
  
  // add columns
  for( unsigned int i=0; i<n_columns; i++ )
  { list_->setColumnName( i, column_titles_[i] ); }
  
  // connections
  connect( list_, SIGNAL( itemActivated( QTreeWidgetItem* ) ), this, SLOT( _select( QTreeWidgetItem* ) ) );

  // add classes
  _load();
  
  // buttons
  QVBoxLayout* v_layout( new QVBoxLayout() );
  v_layout->setSpacing(5);
  v_layout->setMargin(0);
  h_layout->addLayout( v_layout );
  
  QPushButton *button;
  v_layout->addWidget( button = new QPushButton( "&Select", this ) );
  connect( button, SIGNAL( clicked() ), this, SLOT( _select() ) );
  button->setToolTip( "Apply selected document class to current file" );

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

  v_layout->addStretch();
  
}

//___________________________________________________
void DocumentClassDialog::_select( void )
{
  Debug::Throw( "DocumentClassDialog::_select.\n" );
  QTreeWidgetItem *item( list_->QTreeWidget::currentItem() );
  if( !item )
  {
    QtUtil::infoDialog( this, "No item selected. <Select> canceled." );
    return;
  }
  
  emit classSelected( qPrintable( item->text( NAME ) ) );
  return;
}

//___________________________________________________
void DocumentClassDialog::_remove( void )
{
  Debug::Throw( "DocumentClassDialog::_remove.\n" );

  QList<QTreeWidgetItem*> items( list_->QTreeWidget::selectedItems() );
  if( items.empty() ) return QtUtil::infoDialog( this, "No class selected. <Remove> canceled" );

  ostringstream what;
  if( items.size() > 1 ) what << "Remove selected classes ?";
  else what << "Remove the selected class ?";
  if( !QtUtil::questionDialog( this, what.str() ) ) return;
  
  MainFrame &main_frame( *static_cast<MainFrame*>(qApp) );
  DocumentClassManager &manager( main_frame.classManager() );
  
  bool removed( false );
  for( QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  { removed |= manager.remove( qPrintable( (*iter)->text( NAME ) ) ); }
  
  if( removed )
  {
    main_frame.updateEditFrames();
    _load();
  }
  
}

//___________________________________________________
void DocumentClassDialog::_edit( void )
{
  Debug::Throw( "DocumentClassDialog::_edit.\n" );

  QList<QTreeWidgetItem*> items( list_->QTreeWidget::selectedItems() );
  if( items.empty() ) return QtUtil::infoDialog( this, "No class selected. <Edit> canceled" );

  // loop over selected items and edit
  for( QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  { static_cast<MainFrame*>(qApp)->open( File( qPrintable( (*iter)->text( FILE ) ) ) ); }
  
  // close dialog
  close();
  
  return;
}
    
//___________________________________________________
void DocumentClassDialog::_loadFile( void )
{
  Debug::Throw( "DocumentClassDialog::_LoadFile.\n" ); 

  // get file from dialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::ExistingFile );
  if( dialog.exec() != QDialog::Accepted ) return;
  
  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return;
  
  File file = File( qPrintable( files.front() ) ).expand();
  
  // try load from file manager and add to options
  DocumentClassManager &manager( static_cast<MainFrame*>(qApp)->classManager() ); 
  if( manager.read( file ) ) {
    XmlOptions::get().add( Option( "PATTERN_FILENAME", file ) );
    _load();
  }
  return; 
    
}
    
//___________________________________________________
void DocumentClassDialog::_save( void )
{
  Debug::Throw( "DocumentClassDialog::_save.\n" );
  
  QTreeWidgetItem *item( list_->QTreeWidget::currentItem() );
  if( !item )
  {
    QtUtil::infoDialog( this, "No item selected. <Select> canceled." );
    return;
  }
  
  // retrieve file from dialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::AnyFile );
  if( dialog.exec() != QDialog::Accepted ) return;

  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return;
  
  File file = File( qPrintable( files.front() ) ).expand();
  
  // check if file is directory
  if( file.isDirectory() )
  {
    ostringstream what;
    what << "file \"" << file << "\" is a directory. <Save As> canceled.";
    QtUtil::infoDialog( this, what.str() );
    return;
  }

  // check if file exist
  if( file.exist() )
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
  
  static_cast<MainFrame*>(qApp)->classManager().write( qPrintable( item->text( NAME ) ), file );
  return;
}

//___________________________________________________
void DocumentClassDialog::_reload( void )
{
  Debug::Throw( "DocumentClassDialog::_reload.\n" );  
  static_cast<MainFrame*>(qApp)->updateDocumentClasses();
  _load();
}

//___________________________________________________ 
void DocumentClassDialog::_load()
{
  Debug::Throw( "DocumentClassDialog::_load.\n" );
  
  // clear list
  list_->clear();
  
  // retrieve classes from DocumentClass manager
  const DocumentClassManager::ClassList& classes( static_cast<MainFrame*>(qApp)->classManager().list() );
  
  // add to list
  for( DocumentClassManager::ClassList::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { _add( **iter ); }

  list_->resizeColumnToContents( NAME );
  list_->resizeColumnToContents( FILE );
  
}

//_________________________________________________________________
void DocumentClassDialog::_add( const DocumentClass& document_class )
{
  Debug::Throw( "DocumentClassDialog::_AddDocumentClass.\n" );
  CustomListView::Item* item( new CustomListView::Item( list_ ) );
  item->setText( NAME, document_class.name().c_str() );
  item->setText( FILE, document_class.file().c_str() );
  return;
}
