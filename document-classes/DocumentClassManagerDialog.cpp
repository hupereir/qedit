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

#include "CustomToolBar.h"
#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassIcons.h"
#include "DocumentClassManagerDialog.h"
#include "DocumentClassManager.h"
#include "DocumentClassDialog.h"
#include "FileDialog.h"
#include "IconEngine.h"
#include "Options.h" 
#include "PixmapEngine.h"
#include "QuestionDialog.h"
#include "InformationDialog.h"
#include "TreeView.h"

using namespace std;

//______________________________________________________________
DocumentClassManagerDialog::DocumentClassManagerDialog( QWidget* parent, DocumentClassManager* manager ):
  BaseMainWindow( parent ),
  Counter( "DocumentClassManagerDialog" ),
  document_class_manager_( manager )
{
  Debug::Throw( "DocumentClassManagerDialog::DocumentClassManagerDialog.\n" );
 
  setOptionName( "DOCUMENT_CLASS_MANAGER_DIALOG" );
  
  // actions
  _installActions();
  
  // toolbar
  QToolBar* toolbar = new QToolBar( this );
  toolbar->addAction( &_newAction() );
  toolbar->addAction( &_openAction() ); 
  toolbar->addAction( &_saveAction() ); 
  toolbar->addAction( &_saveAsAction() ); 
  toolbar->addAction( &_saveAllAction() ); 
  toolbar->addAction( &_editAction() ); 
  toolbar->addAction( &_removeAction() ); 
  toolbar->addAction( &_reloadAction() ); 

  addToolBar( Qt::TopToolBarArea, toolbar );
  
  // main widget
  QWidget* main = new QWidget( this );
  main->setLayout( new QVBoxLayout() );
  main->layout()->setMargin(2);
  main->layout()->setSpacing(5);
  setCentralWidget( main );
  
  // create list
  main->layout()->addWidget( list_ = new TreeView( main ) );
  list_->setModel( &model_ );

  // set connections
  connect( &_list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _edit() ) );
  connect( _list().selectionModel(), SIGNAL( selectionChanged( const QItemSelection& , const QItemSelection& ) ), SLOT( _updateActions() ) );

  _loadClasses();
  _updateActions();
  
  adjustSize();

}

//___________________________________________________
void DocumentClassManagerDialog::_updateActions( void )
{
  
  Debug::Throw( "DocumentClassManagerDialog::_updateActions.\n" );
  bool has_selection( !_list().selectionModel()->selectedRows().empty() );  
  _editAction().setEnabled( has_selection );
  _removeAction().setEnabled( has_selection );
  _saveAction().setEnabled( has_selection );
  _saveAsAction().setEnabled( has_selection );

}

//___________________________________________________
void DocumentClassManagerDialog::_add( void )
{
  
  Debug::Throw( "DocumentClassManagerDialog::_add.\n" );

  // create dialog
  DocumentClassDialog dialog( this );
  if( dialog.exec() == QDialog::Accepted ) 
  { emit updateNeeded(); }
  
  return;
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
  { 
    DocumentClass new_document_class( dialog.documentClass() );
    if( new_document_class.differs( document_class ) ) 
    {
      // replace class in model
      model_.replace( current, new_document_class ); 
      emit updateNeeded(); 
    }
    
  }
  
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
  File file( FileDialog( this ).getFile() );
  if( file.isNull() ) return;
      
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
    InformationDialog( this, "No item selected. <Save> canceled." ).exec();
    return;
  }
  
  const DocumentClass& document_class( model_.get( current ) );

  // check filename
  File file( document_class.file() );
  if( file.indexOf( ":" ) == 0 )
  {
    QString what;
    QTextStream( &what ) << "Cannot write document class to file " << file;
    InformationDialog( this, what ).centerOnParent().exec();
    return;
  }
  
  // write
  if( !document_class_manager_->write( document_class.name(), file ) )
  {
    QString what;
    QTextStream( &what ) << "Cannot write document class to file " << file;
    InformationDialog( this, what ).centerOnParent().exec();
  };
  
  return;

}

//___________________________________________________
void DocumentClassManagerDialog::_saveAs( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_saveAs.\n" );
  
  QModelIndex current( _list().selectionModel()->currentIndex() );
  if( !current.isValid() ) 
  {
    InformationDialog( this, "No item selected. <Save As> canceled." ).exec();
    return;
  }
  
  // retrieve file from dialog
  FileDialog dialog( this );
  dialog.setFileMode( QFileDialog::AnyFile );
  dialog.setAcceptMode( QFileDialog::AcceptSave );
  File file( dialog.getFile() );
  if( file.isNull() ) return;
  
  // check if file is directory
  if( file.isDirectory() )
  {
    QString buffer;
    QTextStream( &buffer ) << "file \"" << file << "\" is a directory. <Save As> canceled.";
    InformationDialog( this, buffer ).exec();
    return;
  }

  // check if file exist
  if( file.exists() )
  {
    if( !file.isWritable() )
    {
      QString buffer;
      QTextStream( &buffer ) << "file \"" << file << "\" is read-only. <Save As> canceled.";
      InformationDialog( this, buffer ).exec();
      return;
    } else if( !QuestionDialog( this, "Selected file already exist. Overwrite ?" ).exec() )
    return;
  }
  
  document_class_manager_->write( model_.get( current ).name(), file );
  return;
}

//___________________________________________________
void DocumentClassManagerDialog::_saveAll( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_saveAll.\n" );
  
  // retrieve file from dialog
  FileDialog dialog( this );
  dialog.setFileMode( QFileDialog::Directory );
  dialog.setAcceptMode( QFileDialog::AcceptOpen );
  File file( dialog.getFile() );
  if( file.isNull() ) return;
  
  document_class_manager_->write( file );
  
}

//___________________________________________________
void DocumentClassManagerDialog::_reload( void )
{
  Debug::Throw( "DocumentClassManagerDialog::_Reload.\n" );  
  _loadClasses();
}

//________________________________________________________
void DocumentClassManagerDialog::_installActions( void )
{
  
  addAction( new_action_ = new QAction( IconEngine::get( ICONS::NEW ), "&New", this  ) ); 
  connect( new_action_, SIGNAL( triggered() ), SLOT( _add() ) );  
  new_action_->setToolTip( "Create new document class" );
  new_action_->setShortcut( Qt::CTRL + Qt::Key_N );

  //! open
  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open", this  ) ); 
  connect( open_action_, SIGNAL( triggered() ), SLOT( _loadFile() ) );  
  open_action_->setToolTip( "Load additional classes from file" );
  open_action_->setShortcut( Qt::CTRL + Qt::Key_O );

  // save document class to file
  addAction( save_action_ = new QAction( IconEngine::get( ICONS::SAVE ), "Save", this  ) );
  connect( save_action_, SIGNAL( triggered() ), SLOT( _save() ) );  
  save_action_->setToolTip( "Save selected document classe to file" );
  save_action_->setShortcut( Qt::CTRL + Qt::Key_S );

  // save document class to file
  addAction( save_as_action_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save &As", this  ) );
  connect( save_as_action_, SIGNAL( triggered() ), SLOT( _saveAs() ) );  
  save_as_action_->setToolTip( "Save selected document classe to file" );
  save_as_action_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_S );

  // save document class to file
  addAction( save_all_action_ = new QAction( IconEngine::get( ICONS::SAVE_ALL ), "Save &All", this  ) );
  connect( save_all_action_, SIGNAL( triggered() ), SLOT( _saveAll() ) );  
  save_all_action_->setToolTip( "Save all document classes" );

  // edit
  addAction( edit_action_ = new QAction( IconEngine::get( ICONS::EDIT ), "&Edit", this  ) );
  connect( edit_action_, SIGNAL( triggered() ), SLOT( _edit() ) ); 
  edit_action_->setToolTip( "Edit file from which selected document class is read" );

  // remove
  addAction( remove_action_ = new QAction( IconEngine::get( ICONS::REMOVE ), "&Remove", this  ) );
  connect( remove_action_, SIGNAL( triggered() ), SLOT( _remove() ) ); 
  remove_action_->setToolTip( "Remove selected document class from list" );
  
  // reload
  addAction( reload_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Rel&oad", this  ) ); 
  connect( reload_action_, SIGNAL( triggered() ), SLOT( _reload() ) );
  reload_action_->setToolTip( "Reload all classes" );
  reload_action_->setShortcut( Qt::Key_F5 );

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
