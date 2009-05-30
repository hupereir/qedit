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
  \file Application.cpp
  \brief application singleton object
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QMessageBox>

#include "Application.h"
#include "AutoSave.h"
#include "Config.h" 
#include "ConfigurationDialog.h"
#include "DocumentClassManager.h"
#include "DocumentClassManagerDialog.h"
#include "FileCheck.h"
#include "FileCheckDialog.h"
#include "IconEngine.h"
#include "Icons.h"
#include "XmlOptions.h"
#include "MainWindow.h"
#include "InformationDialog.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlFileList.h"
#include "XmlDef.h"

#if WITH_ASPELL
#include "SpellCheckConfiguration.h"
#include "AutoSpellConfiguration.h"
#endif

using namespace std;
using namespace Qt;


//____________________________________________
CommandLineParser Application::commandLineParser( CommandLineArguments arguments, bool ignore_warnings )
{
  Debug::Throw( "Application::commandLineParser.\n" );
  CommandLineParser out( SERVER::ApplicationManager::commandLineParser() );

  out.registerFlag( "--tabbed", "opens files in same window");
  out.registerFlag( "--same-window", "open files in same window");
  out.registerFlag( "--new-window", "open files in a new window");
  out.registerFlag( "--diff", "opens files in same window and perform diff");
  out.registerFlag( "--autospell", "switch autospell on for all files");
  out.registerFlag( "--close", "close displays matching file names and exit");
  out.registerOption( "--filter", "string", "select filter for autospell");
  out.registerOption( "--dictionary", "string", "select dictionary for autospell");
  out.registerOption( "--orientation", "string", "select view orientation for tabbed edition (vertical|horizontal)");
  if( !arguments.isEmpty() ) out.parse( arguments, ignore_warnings );
  return out;
  
}

//____________________________________________
void Application::usage( void )
{
  Debug::Throw(0) << "Usage : qedit [options] [files]" << endl;
  commandLineParser().usage();
}

//____________________________________________
Application::Application( CommandLineArguments arguments ):
  BaseApplication( 0, arguments ),
  Counter( "Application" ),
  recent_files_( 0 ),
  window_server_( 0 ),
  class_manager_( 0 ),
  autosave_( 0 ),
  file_check_( 0 ),
  startup_timer_( this )
{
  startup_timer_.setSingleShot( true );
  connect( &startup_timer_, SIGNAL( timeout() ), SLOT( _readFilesFromArguments() ) );
}

//____________________________________________
Application::~Application( void )
{ 
  Debug::Throw( "Application::~Application.\n" ); 
  
  if( class_manager_ ) delete class_manager_;
  if( file_check_ ) delete file_check_;
  if( autosave_ ) delete autosave_;
  if( window_server_ ) delete window_server_; 
  if( recent_files_ ) delete recent_files_;

}

//____________________________________________
void Application::initApplicationManager( void )
{
  Debug::Throw( "Application::initApplicationManager.\n" );

  // retrieve files from arguments and expand if needed
  CommandLineParser parser( commandLineParser( _arguments() ) );
  QStringList& orphans( parser.orphans() );
  for( QStringList::iterator iter = orphans.begin(); iter != orphans.end(); iter++ )
  { if( !iter->isEmpty() ) (*iter) = File( *iter ).expand(); }

  // replace arguments
  _setArguments( parser.arguments() );

  // base class initialization
  BaseApplication::initApplicationManager();

}

//____________________________________________
bool Application::realizeWidget( void )
{
  Debug::Throw( "Application::realizeWidget.\n" );

  // check if the method has already been called.
  if( !BaseApplication::realizeWidget() ) return false;
  
  // rename about action
  aboutAction().setText( "About &qedit" );

  // need to modify closeAction signal for proper exit
  closeAction().disconnect();
  connect( &closeAction(), SIGNAL( triggered() ), SLOT( _exit() ) );

  document_class_configuration_action_ = new QAction( IconEngine::get( ICONS::CONFIGURE ), "Configure Document Classes ...", this );
  connect( document_class_configuration_action_, SIGNAL( triggered() ), SLOT( _documentClassConfiguration() ) );
  
  spellcheck_configuration_action_ = new QAction( IconEngine::get( ICONS::CONFIGURE ), "Configure Spell Checking ...", this );
  connect( spellcheck_configuration_action_, SIGNAL( triggered() ), SLOT( _spellCheckConfiguration() ) );
  
  monitored_files_action_ = new QAction( "Show Monitored Files", this );
  monitored_files_action_->setToolTip( "Show monitored files" );
  connect( monitored_files_action_, SIGNAL( triggered() ), SLOT( _showMonitoredFiles() ) );
  
  configurationAction().setText( "Configure qedit ..." );
  
  // file list
  recent_files_ = new XmlFileList();
  recent_files_->setCheck( true );

  // class manager
  class_manager_ = new DocumentClassManager();
  
  // autosave
  autosave_ = new AutoSave();
  
  // file check
  file_check_ = new FileCheck();
  
  // window server
  window_server_ = new WindowServer();
  
  // create first window and show
  windowServer().newMainWindow().centerOnDesktop();

  connect( this, SIGNAL( configurationChanged() ), SLOT( _updateDocumentClasses() ) );

  // update configuration
  emit configurationChanged();
  
  // run startup timer to open files after the call to exec() is 
  // performed in the main routine
  startup_timer_.start(0);
  
  Debug::Throw( "Application::realizeWidget - done.\n" ); 
  return true;
  
}

//____________________________________________________________
void Application::_updateDocumentClasses( void )
{
  Debug::Throw( "Application::_updateDocumentClasses.\n" );
  
  // clear document classes
  class_manager_->clear();
  
  // load files from options
  QString buffer;
  QTextStream what( &buffer );
  Options::List files( XmlOptions::get().specialOptions( "PATTERN_FILENAME" ) );
  for( Options::List::const_iterator iter = files.begin(); iter != files.end(); iter++ )
  { 
    class_manager_->read( iter->raw() ); 
    what << class_manager_->readError();
  }

  if( !buffer.isEmpty() ) InformationDialog( 0, buffer ).exec();
  
  // load document classes icons into iconEngine cache, if any
  const DocumentClassManager::List& classes( class_manager_->list() );
  for( DocumentClassManager::List::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { if( !iter->icon().isEmpty() ) { IconEngine::get( iter->icon() ); } }
  
  // emit configuration changed to force displays to be updated
  emit documentClassesChanged();
  
  return;
}

//___________________________________________________________ 
void Application::_configuration( void )
{
  Debug::Throw( "Application::_configuration.\n" );
  emit saveConfiguration();
  ConfigurationDialog dialog;
  connect( &dialog, SIGNAL( configurationChanged() ), SIGNAL( configurationChanged() ) );
  dialog.centerOnWidget( qApp->activeWindow() );
  dialog.exec();
}

//___________________________________________________________ 
void Application::_documentClassConfiguration( void )
{
  Debug::Throw( "Application::_documentClassConfiguration.\n" );
  DocumentClassManagerDialog dialog( qApp->activeWindow(), &classManager() );
  connect( &dialog, SIGNAL( updateNeeded() ), SIGNAL( documentClassesChanged() ) );
  dialog.exec();
}

//_______________________________________________
void Application::_spellCheckConfiguration( void )
{
  
  #if WITH_ASPELL

  Debug::Throw( "Application::_spellCheckConfiguration.\n" );
  
  // create dialog
  CustomDialog dialog( qApp->activeWindow() );
  
  SpellCheckConfiguration* spell_config = new SpellCheckConfiguration( &dialog );
  dialog.mainLayout().addWidget( spell_config );
  spell_config->read();

  AutoSpellConfiguration* autospell_config = new AutoSpellConfiguration( &dialog );
  dialog.mainLayout().addWidget( autospell_config );
  autospell_config->read();
  dialog.centerOnWidget( qApp->activeWindow() );
  
  if( dialog.exec() == QDialog::Rejected ) return;
  spell_config->write();
  autospell_config->write();
  XmlOptions::write();
  
  emit spellCheckConfigurationChanged();
  
  #endif
  
}

//_______________________________________________
void Application::_exit( void )
{
  
  Debug::Throw( "Application::_exit.\n" );
  if( !windowServer().closeAll() ) return;
  qApp->quit();

}

//___________________________________________________________ 
void Application::_readFilesFromArguments( void )
{
  Debug::Throw( "Application::_readFilesFromArgs.\n" );
  windowServer().readFilesFromArguments( _arguments() );

  // make sure application ends when last window is closed.
  connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );

}

  
//________________________________________________
bool Application::_processCommand( SERVER::ServerCommand command )
{

  Debug::Throw( "Application::_processCommand.\n" );
  if( BaseApplication::_processCommand( command ) ) return true;
  if( command.command() == SERVER::ServerCommand::RAISE )
  {
    // copy arguments and try open (via QTimer)
    _setArguments( command.arguments() );
    startup_timer_.start( 100 );
    return true;
  }
  
  return false;
  
}

//_____________________________________________
void Application::_showMonitoredFiles( void )
{ 
  
  Debug::Throw( "Application::_showMonitoredFiles.\n" );
  FileCheckDialog dialog( qApp->activeWindow() ); 
  dialog.setFiles( fileCheck().fileSystemWatcher().files() );
  dialog.exec();
  
}
