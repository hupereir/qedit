#ifndef MainFrame_h
#define MainFrame_h

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
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 *******************************************************************************/

/*!
  \file MainFrame.h
  \brief Application singleton
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <list>
#include <QApplication>
#include <QCursor>
#include <QTimer>

#include "ApplicationManager.h"
#include "ArgList.h"
#include "Counter.h"
#include "Exception.h"
#include "FileRecord.h"
#include "Key.h"
#include "TextSelection.h"

class AutoSave;
class DocumentClassManager;
class EditFrame;
class Sync;

//! Application singleton
class MainFrame: public QApplication, public Counter, public BASE::Key
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! command line help
  static void usage( void );
  
  //! constructor
  MainFrame( int argc, char*argv[] );

  //! destructor
  ~MainFrame( void );

  //! initialize application manager
  void initApplicationManager( void );

  //! create all widgets
  void realizeWidget( void );
  
  //! retrieve DocumentClassManager
  DocumentClassManager& classManager( void )
  { 
    Exception::checkPointer( class_manager_, DESCRIPTION( "class_manager_ not initialized" ));
    return *class_manager_;
  }
  
  //! retrieve AutoSave
  AutoSave& autoSave( void )
  { 
    Exception::checkPointer( autosave_, DESCRIPTION( "autosave_ not initialized" ));
    return *autosave_;
  }
  
  //! set application busy
  void busy( void ) 
  {
    setOverrideCursor( Qt::WaitCursor ); 
    processEvents(); 
  }
  
  //! set application idle
  void idle( void )
  { restoreOverrideCursor(); }

  //! create new empty editFrame
  EditFrame& newEditFrame( void );
  
  signals:
  
  //! configuration has changed
  void configurationChanged();
  
  //! spellcheck configuration modified
  void spellCheckConfigurationChanged();
  
  //! document classes have been modified
  void documentClassesChanged();
  
  signals:
  
  //! configuration changed
  void configurationChanged();
  
  public slots:

  //! configuration
  void configuration( void );
  
  //! spellcheck configuration
  void spellCheckConfiguration( void );
  
  //! open
  /*! returns false if no file is open (force application to quit) */
  EditFrame* open( FileRecord record = FileRecord() );

  //! exit safely
  void exit( void );

  //! opens MessageBox about Qt version
  void about( void );

  //! Update Configuration from options
  void updateConfiguration( void );
  
  //! Update Document Classes from options
  void updateDocumentClasses( void );

  //! multiple files replace
  void multipleFileReplace( std::list<File> file, TextSelection selection );
  
  private slots:
  
  //! read file from arguments. 
  /*!
    this is a slot because it must be called after the call
    to "exec()" in the main routine, by means of a single shot QTimer
  */
  void _readFilesFromArgs( void );
  
  //! actions called when application is about to quit
  void _aboutToQuit( void );

  //! process request from application manager
  void _processRequest( const ArgList& );

  //! application manager state is changed
  void _applicationManagerStateChanged( SERVER::ApplicationManager::State );

  private:

  //! open file status
  enum OpenStatus
  {
    
    //! file successfully opened
    OPEN,
    
    //! file is invalid
    INVALID,
    
    //! file invalid and application should be closed
    EXIT_APP
    
  };
    
  //! current open file status
  OpenStatus open_status_;
  
  //! pointer to application manager
  SERVER::ApplicationManager* application_manager_;

  //! document class manager singleton
  DocumentClassManager* class_manager_;
  
  //! file autoSave manager
  AutoSave* autosave_;
  
  //! command line arguments
  ArgList args_;

  //! true when Realized Widget has been called.
  bool realized_;

  //! startup single shot timer
  /*!
    it allows to call startup methods after the exec() function
    is called from the main routine
  */
  QTimer startup_timer_;

};

#endif

