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

#include <qlayout.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassDialog.h"
#include "DocumentClassManager.h"
#include "CustomFileDialog.h"
#include "MainFrame.h"
#include "Options.h" 
#include "QtUtil.h"

using namespace std;

//______________________________________________________________
const char* DocumentClassDialog::column_titles_[ DocumentClassDialog::n_columns ] =
{
  "name",
  "file",
  ""
};

//______________________________________________________________
DocumentClassDialog::DocumentClassDialog( QWidget* parent, const string& name ):
  CustomDialog( parent, name, EXCLUSIVE )
{
  Debug::Throw( "DocumentClassDialog::DocumentClassDialog.\n" );
  setCaption( "Document Classes" );
  
  QVBox* box( &GetVBox() );
  QHBox* hbox( new QHBox( box ) );
  hbox->setSpacing( 5 );
  
  // create list
  list_ = new CustomListView( hbox );
  
  // add columns
  for( unsigned int i=0; i<n_columns; i++ )
  { list_->addColumn( column_titles_[i] ); }
  list_->header()->setStretchEnabled( true, EMPTY ); 
  
  // set connections
  connect( list_, SIGNAL( doubleClicked( QListViewItem* ) ), this, SLOT( _Select( QListViewItem* ) ) );
  connect( list_, SIGNAL( returnPressed( QListViewItem* ) ), this, SLOT( _Select( QListViewItem* ) ) );

  // add classes
  _LoadClasses();
  
  // buttons
  QVBox *vbox( new QVBox( hbox ) );
  vbox->setSpacing( 5 );
  QPushButton *button;
  button = new QPushButton( "&Select", vbox );
  connect( button, SIGNAL( clicked() ), this, SLOT( _Select() ) );
  QToolTip::add( button, "Apply selected document class to current file" );

  button = new QPushButton( "&Remove", vbox );
  connect( button, SIGNAL( clicked() ), this, SLOT( _Remove() ) ); 
  QToolTip::add( button, "Remove selected document class from list" );

  button = new QPushButton( "&Edit", vbox );
  connect( button, SIGNAL( clicked() ), this, SLOT( _Edit() ) ); 
  QToolTip::add( button, "Edit file from which selected document class is read" );

  button = new QPushButton( "Save &As", vbox ); 
  connect( button, SIGNAL( clicked() ), this, SLOT( _Save() ) );  
  QToolTip::add( button, "Save selected document classe to a file" );

  button = new QPushButton( "&Load File", vbox ); 
  connect( button, SIGNAL( clicked() ), this, SLOT( _LoadFile() ) );  
  QToolTip::add( button, "Load additional classes from file" );

  button = new QPushButton( "Rel&oad", vbox ); 
  connect( button, SIGNAL( clicked() ), this, SLOT( _Reload() ) );
  QToolTip::add( button, "Reload all classes" );

  new QVBox( vbox );
}

//___________________________________________________
void DocumentClassDialog::_Select( void )
{
  Debug::Throw( "DocumentClassDialog::_Select.\n" );
  QListViewItem *item( list_->selectedItem() );
  if( !item )
  {
    QtUtil::InfoDialogExclusive( this, "No item selected. <Select> canceled." );
    return;
  }
  
  emit ClassSelected( (const char*) item->text( NAME ) );
  return;
}

//___________________________________________________
void DocumentClassDialog::_Remove( void )
{
  Debug::Throw( "DocumentClassDialog::_Remove.\n" );
  QListViewItem *item( list_->selectedItem() );
  if( !item )
  {
    QtUtil::InfoDialogExclusive( this, "No item selected. <Remove> canceled." );
    return;
  }
  
  MainFrame &main_frame( *static_cast<MainFrame*>(qApp) );
  DocumentClassManager &manager( main_frame.GetClassManager() );
  if( manager.Remove( (const char*)item->text( NAME ) ) )
  {
    main_frame.UpdateEditFrames();
    _LoadClasses();
  }
}

//___________________________________________________
void DocumentClassDialog::_Edit( void )
{
  Debug::Throw( "DocumentClassDialog::_Edit.\n" );
  QListViewItem *item( list_->selectedItem() );
  if( !item )
  {
    QtUtil::InfoDialogExclusive( this, "No item selected. <Edit> canceled." );
    return;
  }
  
  static_cast<MainFrame*>(qApp)->Open( File( (const char*) item->text( FILE ) ) );
  close();
  return;
}
    
//___________________________________________________
void DocumentClassDialog::_LoadFile( void )
{
  Debug::Throw( "DocumentClassDialog::_LoadFile.\n" ); 

  // get file from dialog
  CustomFileDialog dialog( this, "file dialog", TRUE );
  dialog.setMode( QFileDialog::ExistingFile );
  if( dialog.exec() != QDialog::Accepted ) return;
  File file( (const char*) dialog.selectedFile() );
  
  // try load from file manager and add to options
  DocumentClassManager &manager( static_cast<MainFrame*>(qApp)->GetClassManager() ); 
  if( manager.Read( file ) ) {
    Options::Get().Add( Option( "PATTERN_FILENAME", file ) );
    _LoadClasses();
  }
  return; 
    
}
    
//___________________________________________________
void DocumentClassDialog::_Save( void )
{
  Debug::Throw( "DocumentClassDialog::_Save.\n" );
  
  QListViewItem *item( list_->selectedItem() );
  if( !item )
  {
    QtUtil::InfoDialogExclusive( this, "No item selected. <Save As> canceled." );
    return;
  }
  
  // retrieve file from dialog
  CustomFileDialog dialog( this, "file dialog", TRUE );
  dialog.setMode( QFileDialog::AnyFile );
  if( dialog.exec() != QDialog::Accepted ) return;
  File file( (const char*) dialog.selectedFile() );
  
  // check if file is directory
  if( file.IsDirectory() )
  {
    ostringstream what;
    what << "file \"" << file << "\" is a directory. <Save As> canceled.";
    QtUtil::InfoDialogExclusive( this, what.str() );
    return;
  }

  // check if file exist
  if( file.Exist() )
  {
    if( !file.IsWritable() )
    {
      ostringstream what;
      what << "file \"" << file << "\" is read-only. <Save As> canceled.";
      QtUtil::InfoDialogExclusive( this, what.str() );
      return;
    } else if( !QtUtil::QuestionDialogExclusive( this, "selected file already exist. Overwrite ?" ) )
    return;
  }
  
  static_cast<MainFrame*>(qApp)->GetClassManager().Write( (const char*) item->text( NAME ), file );
  return;
}

//___________________________________________________
void DocumentClassDialog::_Reload( void )
{
  Debug::Throw( "DocumentClassDialog::_Reload.\n" );  
  static_cast<MainFrame*>(qApp)->UpdateDocumentClasses();
  _LoadClasses();
}

//___________________________________________________ 
void DocumentClassDialog::_LoadClasses()
{
  Debug::Throw( "DocumentClassDialog::_LoadClasses.\n" );
  
  // clear list
  list_->clear();
  
  // retrieve classes from DocumentClass manager
  const DocumentClassManager::ClassList& classes( static_cast<MainFrame*>(qApp)->GetClassManager().GetList() );
  
  // add to list
  for( DocumentClassManager::ClassList::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { _AddClass( **iter ); }
  list_->SetItemParity();
}

//_________________________________________________________________
void DocumentClassDialog::_AddClass( const DocumentClass& document_class )
{
  Debug::Throw( "DocumentClassDialog::_AddDocumentClass.\n" );
  CustomListView::Item* item( new CustomListView::Item( list_ ) );
  item->setText( NAME, document_class.Name().c_str() );
  item->setText( FILE, document_class.GetFile().c_str() );
  return;
}
