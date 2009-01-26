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

#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "BaseIcons.h"
#include "CustomFileDialog.h"
#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassManagerDialog.h"
#include "DocumentClassManager.h"
#include "DocumentClassDialog.h"
#include "HighlightStyleList.h"
#include "Options.h" 
#include "PixmapEngine.h"
#include "QuestionDialog.h"
#include "InformationDialog.h"
#include "TreeView.h"

using namespace std;

//______________________________________________________________
DocumentClassManagerDialog::DocumentClassManagerDialog( QWidget* parent, DocumentClassManager* manager ):
  CustomDialog( parent ),
  document_class_manager_( manager )
{
  Debug::Throw( "DocumentClassManagerDialog::DocumentClassManagerDialog.\n" );
 
  setOptionName( "DOCUMENT_CLASS_MANAGER_DIALOG" );
  
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
    "To display and edit some of the document classes components, but \n"
    "Changes made to these are not stored.", this ), 1 );
  
  layout = new QHBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(0);
  mainLayout().addLayout( layout );
  
  // create list
  layout->addWidget( list_ = new TreeView( this ) );
  list_->setModel( &model_ );

  // set connections
  connect( &_list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _edit() ) );
  connect( _list().selectionModel(), SIGNAL( selectionChanged( const QItemSelection& , const QItemSelection& ) ), SLOT( _updateButtons() ) );

  // add classes
  _loadClasses();
  
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
  bool has_selection( !_list().selectionModel()->selectedRows().empty() );  
  edit_button_->setEnabled( has_selection );
  remove_button_->setEnabled( has_selection );
  save_button_->setEnabled( has_selection );

}

//___________________________________________________
void DocumentClassManagerDialog::_edit( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_edit.\n" );
  QModelIndex current( _list().selectionModel()->currentIndex() );
  if( !current.isValid() ) 
  {
    InformationDialog( this, "No item selected. <Edit> canceled." ).exec();
    return;
  }

  // retrieve DocumentClass matching name
  const DocumentClass& document_class( model_.get( current ) );
  
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
  QModelIndex current( _list().selectionModel()->currentIndex() );
  if( !current.isValid() ) 
  {
    InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
    return;
  }
  
  if( document_class_manager_->remove( model_.get( current ).name() ) )
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
    XmlOptions::get().add( "PATTERN_FILENAME", Option( file ) );
    _loadClasses();
  }
  return; 
    
}
    
//___________________________________________________
void DocumentClassManagerDialog::_save( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_save.\n" );
  
  QModelIndex current( _list().selectionModel()->currentIndex() );
  if( !current.isValid() ) 
  {
    InformationDialog( this, "No item selected. <Save As> canceled." ).exec();
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
    InformationDialog( this, what.str().c_str() ).exec();
    return;
  }

  // check if file exist
  if( file.exists() )
  {
    if( !file.isWritable() )
    {
      ostringstream what;
      what << "file \"" << file << "\" is read-only. <Save As> canceled.";
      InformationDialog( this, what.str().c_str() ).exec();
      return;
    } else if( !QuestionDialog( this, "Selected file already exist. Overwrite ?" ).exec() )
    return;
  }
  
  document_class_manager_->write( qPrintable( model_.get( current ).name() ), file );
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
  
  // retrieve classes from DocumentClass manager
  const DocumentClassManager::List& classes( document_class_manager_->list() );
  
  // add to list
  model_.update( DocumentClassModel::List( classes.begin(), classes.end() ) );

}
