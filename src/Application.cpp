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

#include "Application.h"
#include "AutoSave.h"
#include "Config.h"
#include "ConfigurationDialog.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "DocumentClassManagerDialog.h"
#include "FileCheck.h"
#include "FileCheckDialog.h"
#include "IconEngine.h"
#include "Icons.h"
#include "InformationDialog.h"
#include "MainWindow.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlFileList.h"
#include "XmlOptions.h"
#include "XmlDef.h"

#if WITH_ASPELL
#include "SpellCheckConfiguration.h"
#include "AutoSpellConfiguration.h"
#endif

#include <QtGui/QMessageBox>

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
    recentFiles_( 0 ),
    windowServer_( 0 ),
    classManager_( 0 ),
    autosave_( 0 ),
    fileCheck_( 0 )
{}

//____________________________________________
Application::~Application( void )
{
    Debug::Throw( "Application::~Application.\n" );

    if( classManager_ ) delete classManager_;
    if( fileCheck_ ) delete fileCheck_;
    if( autosave_ ) delete autosave_;
    if( windowServer_ ) delete windowServer_;
    if( recentFiles_ ) delete recentFiles_;

}

//____________________________________________
void Application::initApplicationManager( void )
{
    Debug::Throw( "Application::initApplicationManager.\n" );

    // retrieve files from arguments and expand if needed
    CommandLineParser parser( commandLineParser( _arguments() ) );
    QStringList& orphans( parser.orphans() );
    for( QStringList::iterator iter = orphans.begin(); iter != orphans.end(); ++iter )
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

    // disable exit on last window
    // this is re-enabled manually after arguments are read
    qApp->setQuitOnLastWindowClosed( false );

    // rename about action
    aboutAction().setText( "About Qedit" );

    // need to modify closeAction signal for proper exit
    closeAction().disconnect();
    connect( &closeAction(), SIGNAL( triggered() ), SLOT( _exit() ) );

    spellCheckConfigurationAction_ = new QAction( IconEngine::get( ICONS::CONFIGURE ), "Configure Spell Checking...", this );
    connect( spellCheckConfigurationAction_, SIGNAL( triggered() ), SLOT( _spellCheckConfiguration() ) );

    documentClassesConfigurationAction_ = new QAction( IconEngine::get( ICONS::CONFIGURE ), "Configure Document Types...", this );
    connect( documentClassesConfigurationAction_, SIGNAL( triggered() ), SLOT( _documentClassesConfiguration() ) );


    monitoredFilesAction_ = new QAction( "Show Monitored Files", this );
    monitoredFilesAction_->setToolTip( "Show monitored files" );
    connect( monitoredFilesAction_, SIGNAL( triggered() ), SLOT( _showMonitoredFiles() ) );

    configurationAction().setText( "Configure Qedit..." );

    // file list
    recentFiles_ = new XmlFileList();
    recentFiles_->setCheck( true );

    // class manager
    classManager_ = new DocumentClassManager();

    // autosave
    autosave_ = new AutoSave();

    // file check
    fileCheck_ = new FileCheck();

    // window server
    windowServer_ = new WindowServer();

    // create first window and show
    windowServer().newMainWindow().centerOnDesktop();

    // update configuration
    emit configurationChanged();
    _updateDocumentClasses();

    // run startup timer to open files after the call to exec() is
    // performed in the main routine
    startupTimer_.start( 0, this );

    Debug::Throw( "Application::realizeWidget - done.\n" );
    return true;

}

//____________________________________________________________
void Application::_updateDocumentClasses( void )
{
    Debug::Throw( "Application::_updateDocumentClasses.\n" );

    // clear document classes
    classManager_->clear();

    // load files from options
    QString buffer;
    QTextStream what( &buffer );

    // read user specific patterns
    foreach( const Option& option, XmlOptions::get().specialOptions( "PATTERN_FILENAME" ) )
    {
        classManager_->read( QString( option.raw() ) );
        what << classManager_->readError();
    }

    // read build-in patterns
    foreach( const Option& option, XmlOptions::get().specialOptions( "DEFAULT_PATTERN_FILENAME" ) )
    {
        classManager_->read( QString( option.raw() ) );
        what << classManager_->readError();
    }

    if( !buffer.isEmpty() ) InformationDialog( 0, buffer ).exec();

    // load document classes icons into iconEngine cache, if any
    foreach( const DocumentClass& documentClass, classManager_->classes() )
    { if( !documentClass.icon().isEmpty() ) { IconEngine::get( documentClass.icon() ); } }

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

//_______________________________________________
void Application::_spellCheckConfiguration( void )
{

    #if WITH_ASPELL

    Debug::Throw( "Application::_spellCheckConfiguration.\n" );

    // create dialog
    CustomDialog dialog;
    dialog.setWindowTitle( "Spell Check Configuration - Qedit" );

    SPELLCHECK::SpellCheckConfiguration* spellConfig = new SPELLCHECK::SpellCheckConfiguration( &dialog );
    dialog.mainLayout().addWidget( spellConfig );

    SPELLCHECK::AutoSpellConfiguration* autospellConfig = new SPELLCHECK::AutoSpellConfiguration( &dialog );
    dialog.mainLayout().addWidget( autospellConfig );
    dialog.centerOnWidget( qApp->activeWindow() );

    if( dialog.exec() == QDialog::Rejected ) return;
    spellConfig->write();
    autospellConfig->write();
    XmlOptions::write();

    emit spellCheckConfigurationChanged();

    #endif

}

//_______________________________________________
void Application::_documentClassesConfiguration( void )
{
    Debug::Throw( "Application::_documentClassesConfiguration.\n" );
    DocumentClassManagerDialog dialog;
    dialog.setWindowTitle( "Document Types Configuration - Qedit" );
    dialog.exec();
}

//_______________________________________________
void Application::_exit( void )
{

    Debug::Throw( "Application::_exit.\n" );
    if( !windowServer().closeAll() ) return;
    qApp->quit();

}

//___________________________________________________________
void Application::timerEvent( QTimerEvent* event )
{
    Debug::Throw( "Application::timerEvent.\n" );
    if( event->timerId() == startupTimer_.timerId() )
    {

        startupTimer_.stop();
        windowServer().readFilesFromArguments( _arguments() );
        connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ), Qt::UniqueConnection );

    } else return QObject::timerEvent( event );
}


//________________________________________________
bool Application::_processCommand( SERVER::ServerCommand command )
{

    if( BaseApplication::_processCommand( command ) ) return true;
    if( command.command() == SERVER::ServerCommand::RAISE )
    {

        // copy arguments and try open
        _setArguments( command.arguments() );
        startupTimer_.start( 100, this );
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
