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
#include "ExitDialog.h"
#include "FlatStyle.h"
#include "IconEngine.h"
#include "Icons.h"
#include "XmlOptions.h"
#include "MainWindow.h"
#include "QtUtil.h"
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
  QApplication( argc, argv ),
  Counter( "Application" ),
  application_manager_( 0 ),
  recent_files_( 0 ),
  window_server_( 0 ),
  class_manager_( 0 ),
  autosave_( 0 ),
  args_( argc, argv ),
  realized_( false ),
  startup_timer_( this )
{
  Debug::Throw( "Application::Application.\n" );
  if( XmlOptions::get().get<bool>( "USE_FLAT_THEME" ) ) setStyle( new FlatStyle() );

  startup_timer_.setSingleShot( true );
  connect( &startup_timer_, SIGNAL( timeout() ), SLOT( _readFilesFromArgs() ) );
  
}

//____________________________________________
Application::~Application( void )
{ 
  Debug::Throw( "Application::~Application.\n" ); 
  
  if( application_manager_ ) delete application_manager_;
  if( class_manager_ ) delete class_manager_;
  if( autosave_ ) delete autosave_;
  if( window_server_ ) delete window_server_; 
  if( recent_files_ ) delete recent_files_;

  XmlOptions::write();

  ErrorHandler::exit();

}

//____________________________________________
void Application::initApplicationManager( void )
{
  Debug::Throw( "Application::initApplicationManager.\n" );

  if( application_manager_ ) return;
  if( args_.find( "--no-server" ) ) 
  {
    realizeWidget();
    return;
  }

  // create application manager
  application_manager_ = new SERVER::ApplicationManager( this );
  application_manager_->setApplicationName( XmlOptions::get().get<string>( "APP_NAME" ) );
  connect( 
    application_manager_, SIGNAL( stateChanged( SERVER::ApplicationManager::State ) ),
    SLOT( _applicationManagerStateChanged( SERVER::ApplicationManager::State ) ) );
    
  connect( application_manager_, SIGNAL( serverRequest( const ArgList& ) ), SLOT( _processRequest( const ArgList& ) ) );
 
  // retrieve files from arguments and expand if needed
  ArgList::Arg& last( args_.get().back() );
  for( list< string >::iterator iter = last.options().begin(); iter != last.options().end(); iter++ )
  { if( File( *iter ).size() ) (*iter) = File( *iter ).expand(); }

  // pass args to application manager
  application_manager_->init( args_ );

}

//____________________________________________
void Application::realizeWidget( void )
{
  Debug::Throw( "Application::realizeWidget.\n" );

  //! check if the method has already been called.
  if( realized_ ) return;
  realized_ = true;
  
  // actions  
  about_action_ = new QAction( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).c_str() ), "About &QEdit", 0 );
  connect( about_action_, SIGNAL( triggered() ), SLOT( _about() ) );
   
  aboutqt_action_ = new QAction( IconEngine::get( ICONS::ABOUT_QT ), "About &Qt", this );
  connect( aboutqt_action_, SIGNAL( triggered() ), SLOT( aboutQt() ) ); 

  close_action_ = new QAction( IconEngine::get( ICONS::EXIT ), "E&xit", this );
  close_action_->setShortcut( CTRL+Key_Q );
  connect( close_action_, SIGNAL( triggered() ), SLOT( _exit() ) );

  configuration_action_ = new QAction( IconEngine::get( ICONS::CONFIGURE ), "Default &Configuration", this );
  connect( configuration_action_, SIGNAL( triggered() ), SLOT( _configuration() ) );

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
  window.show();

  // check number of valid files
  // this needs to be done here so that when calling qedit the first time
  // with no argument, a "new Document" is created (as opposed to an empty file display)
  // in contrast: this is not necessary via processRequest, since there should then already
  // be a window opened.
  {
    list<string> files( args_.get().back().options() );
    bool has_valid_file( false );
    for( list<string>::const_iterator iter( files.begin() ); iter != files.end(); iter++ )
    { 
      if( !iter->empty() ) 
      {
        has_valid_file = true;
        break;
      }
    }
    
    if( !has_valid_file ) window.activeDisplay().setIsNewDocument();
  }
  
  // update configuration
  _updateConfiguration();

  // make sure application ends when last window is closed.
  connect( this, SIGNAL( lastWindowClosed() ), SLOT( quit() ) );
  
  // run startup timer to open files after the call to exec() is 
  // performed in the main routine
  startup_timer_.start(0);
  
  Debug::Throw( "Application::realizeWidget - done.\n" ); 

}

//____________________________________________________________
void Application::updateDocumentClasses( void )
{
  Debug::Throw( "Application::updateDocumentClasses.\n" );
  
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

  if( !what.str().empty() ) QtUtil::infoDialog( 0, what.str() );
  
  // load document classes icons into iconEngine cache, if any
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  const DocumentClassManager::ClassList& classes( class_manager_->list() );
  for( DocumentClassManager::ClassList::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { if( !iter->icon().isEmpty() ) { IconEngine::get( qPrintable( iter->icon() ) ); } }
  
  // emit configuration changed to force displays to be updated
  emit documentClassesChanged();
  
  return;
}

//_______________________________________________
void Application::_about( void )
{

  Debug::Throw( "Application::_about.\n" );
  ostringstream what;
  what << "<b>QEdit</b> version " << VERSION << " (" << BUILD_TIMESTAMP << ")";
  what 
    << "<p>This application was written for personal use only. "
    << "It is not meant to be bug free, although all efforts "
    << "are made so that it remains/becomes so. "
    
    << "<p>Suggestions, comments and bug reports are welcome. "
    << "Please use the following e-mail address:"

    << "<p><a href=\"mailto:hugo.pereira@free.fr\">hugo.pereira@free.fr</a>";

  QMessageBox dialog;
  dialog.setWindowIcon( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).c_str() ) );
  dialog.setIconPixmap( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).c_str() ) );
  dialog.setText( what.str().c_str() );
  dialog.adjustSize();
  QtUtil::centerOnWidget( &dialog, activeWindow() );
  dialog.exec();

}

//___________________________________________________________ 
void Application::_configuration( void )
{
  Debug::Throw( "Application::_configuration.\n" );
  emit saveConfiguration();
  ConfigurationDialog dialog( 0 );
  connect( &dialog, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  QtUtil::centerOnWidget( &dialog, activeWindow() );
  dialog.exec();
  Debug::Throw( "Application::configuration - done.\n" );
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
  
  dialog.adjustSize();
  QtUtil::centerOnParent( &dialog );
  
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

  // retrieve opened files
  FileRecord::List files( windowServer().files() );
  
  // ask for confirmation if more than one file is opened.
  if( files.size() > 1 )
  {
    ExitDialog dialog( activeWindow(), files );
    QtUtil::centerOnParent( &dialog );
    if( !dialog.exec() ) return;
  }
  
  // try close all windows gracefully, and return if operation is canceled
  if( !windowServer().closeAllWindows() ) return;
  quit();

}

//_________________________________________________
void Application::_updateConfiguration( void )
{

  Debug::Throw( "Application::_updateConfiguration.\n" );
 
  // set fonts
  QFont font;
  font.fromString( XmlOptions::get().raw( "FONT_NAME" ).c_str() );
  setFont( font );

  font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ).c_str() );
  setFont( font, "QLineEdit" );
  setFont( font, "QTextEdit" );
  
  Debug::setLevel( XmlOptions::get().get<bool>("DEBUG_LEVEL") );
  setWindowIcon( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).expand().c_str() ) );
  
  // reload IconEngine cache (in case of icon_path_list that changed)
  IconEngine::get().reload();
  
  // configuration changed
  emit configurationChanged();
  
  // update document classes
  updateDocumentClasses();
  
  return;
}

//___________________________________________________________ 
void Application::_readFilesFromArgs( void )
{
  Debug::Throw( "Application::_readFilesFromArgs.\n" );
  windowServer().readFilesFromArguments( args_ );
}

//________________________________________________
void Application::_applicationManagerStateChanged( SERVER::ApplicationManager::State state )
{

  Debug::Throw() << "Application::_ApplicationManagerStateChanged - state=" << state << endl;

  switch ( state ) {
    case SERVER::ApplicationManager::ALIVE:
    realizeWidget();
    break;

    case SERVER::ApplicationManager::DEAD:
    quit();
    break;

    default:
    break;
  }

  return;

}

//________________________________________________
void Application::_processRequest( const ArgList& args )
{
  Debug::Throw() << "Application::_ProcessRequest - " << args << endl;

  // copy arguments and try open (via QTimer)
  args_ = args;
  startup_timer_.start( 100 );
  return;
  
}
