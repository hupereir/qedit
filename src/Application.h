#ifndef Application_h
#define Application_h

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
  \file Application.h
  \brief Application singleton
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <list>
#include <QTimer>

#include "BaseApplication.h"
#include "CommandLineArguments.h"
#include "CommandLineParser.h"
#include "Config.h"
#include "Counter.h"

class AutoSave;
class DocumentClassManager;
class FileCheck;
class FileList;
class WindowServer;
class Sync;

//! Application singleton
class Application: public BaseApplication, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! command line parser
  static CommandLineParser commandLineParser( CommandLineArguments arguments = CommandLineArguments(), bool ignore_warnings = true );
  
  //! command line help
  static void usage( void );
  
  //! constructor
  Application( CommandLineArguments );

  //! destructor
  ~Application( void );

  //! initialize application manager
  virtual void initApplicationManager( void );

  //! create all widgets
  bool realizeWidget( void );
  
  //! file list
  FileList& recentFiles( void ) const
  { 
    assert( recent_files_ );
    return *recent_files_;
  }
  
  //! Window server
  WindowServer& windowServer( void ) const
  {
    assert( window_server_ );
    return *window_server_;
  }

  //! DocumentClassManager
  DocumentClassManager& classManager( void ) const
  { 
    assert( class_manager_ );
    return *class_manager_;
  }
  
  //! retrieve AutoSave
  AutoSave& autoSave( void ) const
  { 
    assert( autosave_ );
    return *autosave_;
  }
  
  //! file check
  FileCheck& fileCheck( void ) const
  { 
    assert( file_check_ );
    return *file_check_; 
  }
  
  //!@name actions
  //@{

  //! configure
  QAction& documentClassConfigurationAction( void ) const
  { return *document_class_configuration_action_; }

  //! configure
  QAction& spellCheckConfigurationAction( void ) const
  { return *spellcheck_configuration_action_; }
  
  //! monitored files 
  QAction& monitoredFilesAction( void ) const
  { return *monitored_files_action_; }
  
  //@}
  
  signals:
  
  //! spellcheck configuration modified
  void spellCheckConfigurationChanged( void );
  
  //! document classes have been modified
  void documentClassesChanged( void );
 
  protected slots:

  //! Update Document Classes from options
  void _updateDocumentClasses( void );

  //! about
  void _about( void )
  { BaseApplication::_about( "qedit", VERSION, BUILD_TIMESTAMP ); }
  
  //! configuration
  void _configuration( void );
  
  //! document class configuration
  void _documentClassConfiguration( void );
  
  //! spellcheck configuration
  void _spellCheckConfiguration( void );

  //! exit safely
  void _exit( void );
  
  //! read file from arguments. 
  /*!
    this is a slot because it must be called after the call
    to "exec()" in the main routine, by means of a single shot QTimer
  */
  void _readFilesFromArguments( void );
  
  //! process request from application manager
  void _processRequest( const CommandLineArguments& );
    
  //! monitored files
  void _showMonitoredFiles( void );

  private:
  
  //! recent files list
  FileList* recent_files_;

  //! window server
  WindowServer* window_server_;
  
  //! document class manager singleton
  DocumentClassManager* class_manager_;
  
  //! file autoSave manager
  AutoSave* autosave_;
  
  //! file check
  FileCheck* file_check_;
  
  //! startup single shot timer
  /*!
    it allows to call startup methods after the exec() function
    is called from the main routine
  */
  QTimer startup_timer_;

  //!@name actions
  //@{
  
  //! configure
  QAction* document_class_configuration_action_;
  
  //! configure
  QAction* spellcheck_configuration_action_;
  
  //! show monitored files
  QAction* monitored_files_action_;
  
  //@}
  
};

#endif
