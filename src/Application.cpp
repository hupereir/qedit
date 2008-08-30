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
#include "ErrorHandler.h"
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
void Application::usage( void )
{
  cout << "Usage : qedit [options] <file1> <file2> <...>" << endl;
  cout << endl;
  cout << "Options : " << endl;
  cout << "  --help\t\t displays this help and exit" << endl;
  cout << "  --tabbed\t\t opens files in same window" << endl;
  cout << "  --same-window\t\t open files in same window" << endl;
  cout << "  --new-window\t\t open files in a new window" << endl;
  cout << "  --diff\t\t opens files in same window and perform diff" << endl;
  cout << "  --autospell\t\t switch autospell on for all files" << endl;
  cout << "  --filter <filter>\t select filter for autospell" << endl;
  cout << "  --dictionary <dict>\t select dictionary for autospell" << endl;
  cout << "  --close\t\t close displays matching file names and exit" << endl;
  SERVER::ApplicationManager::usage();
  return;
}

//____________________________________________
Application::Application( int argc, char*argv[] ) :
  BaseApplication( argc, argv ),
  Counter( "Application" ),
  recent_files_( 0 ),
  window_server_( 0 ),
  class_manager_( 0 ),
  autosave_( 0 ),
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
  if( autosave_ ) delete autosave_;
  if( window_server_ ) delete window_server_; 
  if( recent_files_ ) delete recent_files_;

}

//____________________________________________
void Application::initApplicationManager( void )
{
  Debug::Throw( "Application::initApplicationManager.\n" );

  // retrieve files from arguments and expand if needed
  ArgList::Arg& last( _arguments().get().back() );
  for( list< string >::iterator iter = last.options().begin(); iter != last.options().end(); iter++ )
  { if( File( *iter ).size() ) (*iter) = File( *iter ).expand(); }

  // base class initialization
  BaseApplication::initApplicationManager();

}

//____________________________________________
bool Application::realizeWidget( void )
{
  Debug::Throw( "Application::realizeWidget.\n" );

  //! check if the method has already been called.
  if( !BaseApplication::realizeWidget() ) return false;
  
  // need to modify closeAction signal for proper exit
  closeAction().disconnect();
  connect( &closeAction(), SIGNAL( triggered() ), SLOT( _exit() ) );

  document_class_configuration_action_ = new QAction( IconEngine::get( ICONS::CONFIGURE ), "Document Class &Configuration", this );
  connect( document_class_configuration_action_, SIGNAL( triggered() ), SLOT( _documentClassConfiguration() ) );
  
  spellcheck_configuration_action_ = new QAction( IconEngine::get( ICONS::CONFIGURE ), "&Spell-check &Configuration", this );
  connect( spellcheck_configuration_action_, SIGNAL( triggered() ), SLOT( _spellCheckConfiguration() ) );
  
  // file list
  recent_files_ = new XmlFileList();
  recent_files_->setCheck( true );

  // class manager
  class_manager_ = new DocumentClassManager();
  
  // autosave
  autosave_ = new AutoSave();
  
  // window server
  window_server_ = new WindowServer();
  
  // create first window and show
  MainWindow& window( windowServer().newMainWindow() );
  window.centerOnDesktop();
  window.show();

  // make sure application ends when last window is closed.
  connect( this, SIGNAL( lastWindowClosed() ), SLOT( quit() ) );
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
  ostringstream what;
  list<string> files( XmlOptions::get().specialOptions<string>( "PATTERN_FILENAME" ) );
  for( list<string>::const_iterator iter = files.begin(); iter != files.end(); iter++ )
  { 
    class_manager_->read( *iter ); 
    what << qPrintable( class_manager_->readError() );
  }

  if( !what.str().empty() ) InformationDialog( 0, what.str().c_str() ).exec();
  
  // load document classes icons into iconEngine cache, if any
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  const DocumentClassManager::List& classes( class_manager_->list() );
  for( DocumentClassManager::List::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { if( !iter->icon().isEmpty() ) { IconEngine::get( qPrintable( iter->icon() ) ); } }
  
  // emit configuration changed to force displays to be updated
  emit documentClassesChanged();
  
  return;
}

//___________________________________________________________ 
void Application::_configuration( void )
{
  Debug::Throw( "Application::_configuration.\n" );
  emit saveConfiguration();
  ConfigurationDialog dialog( 0 );
  connect( &dialog, SIGNAL( configurationChanged() ), SIGNAL( configurationChanged() ) );
  dialog.centerOnWidget( activeWindow() );
  dialog.exec();
}

//___________________________________________________________ 
void Application::_documentClassConfiguration( void )
{
  Debug::Throw( "Application::_documentClassConfiguration.\n" );
  DocumentClassManagerDialog dialog( activeWindow(), &classManager() );
  connect( &dialog, SIGNAL( updateNeeded() ), SIGNAL( documentClassesChanged() ) );
  dialog.exec();
}

//_______________________________________________
void Application::_spellCheckConfiguration( void )
{
  
  #if WITH_ASPELL

  Debug::Throw( "Application::_spellCheckConfiguration.\n" );
  
  // create dialog
  CustomDialog dialog( activeWindow() );
  
  SpellCheckConfiguration* spell_config = new SpellCheckConfiguration( &dialog );
  dialog.mainLayout().addWidget( spell_config );
  spell_config->read();

  AutoSpellConfiguration* autospell_config = new AutoSpellConfiguration( &dialog );
  dialog.mainLayout().addWidget( autospell_config );
  autospell_config->read();
  dialog.centerOnWidget( activeWindow() );
  
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
  quit();

}

//___________________________________________________________ 
void Application::_readFilesFromArguments( void )
{
  Debug::Throw( "Application::_readFilesFromArgs.\n" );
  windowServer().readFilesFromArguments( _arguments() );
}

//________________________________________________
void Application::_processRequest( const ArgList& args )
{
  Debug::Throw() << "Application::_ProcessRequest - " << args << endl;

  // copy arguments and try open (via QTimer)
  _setArguments( args );
  startup_timer_.start( 100 );
  return;
  
}
