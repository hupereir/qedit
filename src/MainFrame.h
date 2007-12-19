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
#include <QAction>
#include <QCursor>
#include <QTimer>

#include "ApplicationManager.h"
#include "ArgList.h"
#include "Counter.h"


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
    assert( class_manager_
);
    return *class_manager_;
  }
  
  //! retrieve AutoSave
  AutoSave& autoSave( void )
  { 
    assert( autosave_
);
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
  
  //!@name actions
  //@{
  
  //! about
  QAction& aboutAction( void ) const
  { return *about_action_; }
   
  //! about
  QAction& aboutQtAction( void ) const
  { return *aboutqt_action_; }
 
  //! close
  QAction& closeAction( void ) const
  { return *close_action_; }
 
  //! close
  QAction& saveAllAction( void ) const
  { return *save_all_action_; }

  //! configure
  QAction& configurationAction( void ) const
  { return *configuration_action_; }

  //! configure
  QAction& documentClassConfigurationAction( void ) const
  { return *document_class_configuration_action_; }

  //! configure
  QAction& spellCheckConfigurationAction( void ) const
  { return *spellcheck_configuration_action_; }
  
  //@}
  
  signals:
  
  //! request widget to save their current configuration
  void saveConfiguration( void );
  
  //! configuration has changed
  void configurationChanged( void );
  
  //! spellcheck configuration modified
  void spellCheckConfigurationChanged( void );
  
  //! document classes have been modified
  void documentClassesChanged( void );
  
  public slots:
  
  //! open
  /*! returns false if no file is open (force application to quit) */
  EditFrame* open( FileRecord record = FileRecord(), ArgList args = ArgList() );
  
  //! Update Document Classes from options
  void updateDocumentClasses( void );

  //! multiple files replace
  void multipleFileReplace( std::list<File> file, TextSelection selection );
  
  private slots:

  //! opens MessageBox about QEdit version
  void _about( void );
  
  //! configuration
  void _configuration( void );
  
  //! document class configuration
  void _documentClassConfiguration( void );
  
  //! spellcheck configuration
  void _spellCheckConfiguration( void );

  //! exit safely
  void _exit( void );

  //! save all edited files
  void _saveAll( void );

  //! Update Configuration from options
  void _updateConfiguration( void );
  
  //! read file from arguments. 
  /*!
    this is a slot because it must be called after the call
    to "exec()" in the main routine, by means of a single shot QTimer
  */
  void _readFilesFromArgs( void );
  
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

  //!@name actions
  //@{
  
  //! about
  QAction* about_action_;
  
  //! about Qt
  QAction* aboutqt_action_;
  
  //! close
  QAction* close_action_;
  
  //! save all modified files
  QAction* save_all_action_;
  
  //! configure
  QAction* configuration_action_;
  
  //! configure
  QAction* document_class_configuration_action_;
  
  //! configure
  QAction* spellcheck_configuration_action_;
  
  //@}
  
};

#endif

