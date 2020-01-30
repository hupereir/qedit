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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "Application.h"
#include "ConfigurationDialog.h"
#include "DocumentClassManagerDialog.h"
#include "FileCheck.h"
#include "FileCheckDialog.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "InformationDialog.h"
#include "LogWidget.h"
#include "MainWindow.h"
#include "RestoreSessionDialog.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlFileList.h"
#include "XmlFileRecord.h"
#include "XmlOptions.h"
#include "XmlDef.h"

#if WITH_ASPELL
#include "SpellCheckConfigurationDialog.h"
#include "SpellInterface.h"
#endif

#include <QMessageBox>

//____________________________________________
Application::Application( CommandLineArguments arguments ):
    BaseApplication( nullptr, arguments ),
    Counter( "Application" )
{}

//____________________________________________
bool Application::initApplicationManager()
{
    Debug::Throw( "Application::initApplicationManager.\n" );

    // retrieve files from arguments and expand if needed
    auto parser = commandLineParser( _arguments() );
    for( auto& orphan:parser.orphans() )
    { if( !orphan.isEmpty() ) File::expand( orphan ); }

    // replace arguments
    _setArguments( parser.arguments() );

    // base class initialization
    if( !BaseApplication::initApplicationManager() ) return false;
    else {

        #if WITH_ASPELL
        bool accepted( true );
        if( parser.hasFlag( "--list-dictionaries" ) )
        {
            SpellCheck::SpellInterface().listDictionaries();
            accepted = false;
        }

        if( parser.hasFlag( "--list-filters" ) )
        {
            SpellCheck::SpellInterface().listFilters();
            accepted = false;
        }

        return accepted;

        #else
        return true;
        #endif

    }

}

//____________________________________________
bool Application::realizeWidget()
{
    Debug::Throw( "Application::realizeWidget.\n" );

    // check if the method has already been called.
    if( !BaseApplication::realizeWidget() ) return false;

    // disable exit on last window
    // this is re-enabled manually after arguments are read
    qApp->setQuitOnLastWindowClosed( false );

    // actions
    _installActions();

    // file list
    recentFiles_.reset(new XmlFileList( this ));
    recentFiles_->setCheck( true );

    // session file list
    sessionFiles_.reset(new XmlFileList( this ));
    static_cast<XmlFileList*>( sessionFiles_.get() )->setTagName( Base::Xml::SessionFileList );

    // session file list
    lastSessionFiles_.reset(new XmlFileList( this ));
    static_cast<XmlFileList*>( lastSessionFiles_.get() )->setTagName( Base::Xml::LastSessionFileList );

    // class manager
    classManager_.reset(new DocumentClassManager);

    // autosave
    autosave_.reset( new AutoSave );

    // file check
    fileCheck_.reset( new FileCheck );

    // window server
    windowServer_.reset( new WindowServer );
    connect( windowServer_.get(), &WindowServer::sessionFilesChanged, this, &Application::_updateSessionActions );

    // create first window and show
    windowServer_->newMainWindow().centerOnDesktop();

    // update configuration
    connect( this, &Application::configurationChanged, this, &Application::_updateConfiguration );
    emit configurationChanged();

    _updateSessionActions();
    _updateDocumentClasses();

    // run startup timer to open files after the call to exec() is
    // performed in the main routine
    startupTimer_.start( 0, this );

    return true;

}

//____________________________________________
CommandLineParser Application::commandLineParser( CommandLineArguments arguments, bool ignoreWarnings ) const
{
    Debug::Throw( "Application::commandLineParser.\n" );
    CommandLineParser out( BaseApplication::commandLineParser() );

    out.setGroup( CommandLineParser::applicationGroupName );
    out.registerFlag( CommandLineParser::Tag( "--tabbed", "-t" ), tr( "opens files in same window") );
    out.registerFlag( "--same-window", tr( "open files in same window") );
    out.registerFlag( CommandLineParser::Tag( "--new-window", "-n" ), tr( "open files in a new window") );
    out.registerFlag( CommandLineParser::Tag( "--diff", "-d" ), tr( "open files in same window and perform diff") );
    // out.registerFlag( CommandLineParser::Tag( "--list", "-l" ), tr( "list all opened files" ) );
    out.registerFlag( "--close", tr( "close displays matching file names and exit") );
    out.registerOption( CommandLineParser::Tag( "--orientation", "-o" ), tr( "string" ), tr( "select view orientation for tabbed edition (vertical|horizontal)") );

    #if WITH_ASPELL
    out.registerFlag( "--autospell", tr( "switch autospell on for all files") );
    out.registerOption( "--filter", tr( "string" ), tr( "select filter for spell checking") );
    out.registerOption( "--dictionary", tr( "string" ), tr( "select dictionary for spell checking") );
    out.registerFlag( "--list-filters", tr( "list all filters available to spell checking") );
    out.registerFlag( "--list-dictionaries", tr( "list all dictionaries available to spell checking") );
    #endif

    if( !arguments.isEmpty() ) out.parse( arguments, ignoreWarnings );
    return out;
}

//____________________________________________
void Application::usage() const
{
    _usage( "qedit", tr("[options] [files]") );
    commandLineParser().usage();
}

//____________________________________________________________
void Application::_updateDocumentClasses()
{
    Debug::Throw( "Application::_updateDocumentClasses.\n" );

    // clear document classes
    classManager_->clear();

    // load files from options
    QString buffer;
    QTextStream what( &buffer );

    // read user specific patterns
    for( const auto& option:XmlOptions::get().specialOptions( "PATTERN_FILENAME" ) )
    {
        classManager_->read( File( option.raw() ) );
        what << classManager_->readError();
    }

    // read build-in patterns
    for( const auto& option:XmlOptions::get().specialOptions( "DEFAULT_PATTERN_FILENAME" ) )
    {
        classManager_->read( File( option.raw() ) );
        what << classManager_->readError();
    }

    if( !buffer.isEmpty() ) InformationDialog( 0, buffer ).exec();

    // load document classes icons into iconEngine cache, if any
    for( const auto& documentClass:classManager_->classes() )
    { if( !documentClass.icon().isEmpty() ) { IconEngine::get( documentClass.icon() ); } }

    // emit configuration changed to force displays to be updated
    emit documentClassesChanged();

    return;
}

//___________________________________________________________
void Application::_configuration()
{
    Debug::Throw( "Application::_configuration.\n" );
    emit saveConfiguration();
    ConfigurationDialog dialog;
    connect( &dialog, &BaseConfigurationDialog::configurationChanged, this, &BaseCoreApplication::configurationChanged );
    dialog.centerOnWidget( qApp->activeWindow() );
    dialog.exec();
}

//_______________________________________________
void Application::_spellCheckConfiguration()
{

    #if WITH_ASPELL

    Debug::Throw( "Application::_spellCheckConfiguration.\n" );

    // create dialog
    SpellCheck::ConfigurationDialog dialog;
    connect( &dialog, &BaseConfigurationDialog::configurationChanged, this, &Application::spellCheckConfigurationChanged );
    dialog.centerOnWidget( qApp->activeWindow() );
    dialog.exec();

    #endif

}

//_______________________________________________
void Application::_documentClassesConfiguration()
{
    Debug::Throw( "Application::_documentClassesConfiguration.\n" );
    DocumentClassManagerDialog dialog;
    dialog.setWindowTitle( tr( "Document Types Configuration" ) );
    if( !dialog.exec() ) return;

    XmlOptions::get().clearSpecialOptions( "PATTERN_FILENAME" );
    for( const auto& file:dialog.userFiles() )
    { XmlOptions::get().add( "PATTERN_FILENAME", file ); }

    _updateDocumentClasses();

}

//___________________________________________________________
void Application::_saveSession()
{
    Debug::Throw( "Application::_saveSession.\n" );
    sessionFiles_->set( windowServer_->records( WindowServer::ExistingOnly ) );
    static_cast<XmlFileList*>(sessionFiles_.get())->write();
    _updateSessionActions();
}

//___________________________________________________________
void Application::_printSession()
{
    Debug::Throw( "Application::_printSession.\n" );

    // get session records
    const FileRecord::List records( windowServer_->records( WindowServer::ExistingOnly ) );
    if( records.empty() ) return;

    // create dialog
    CustomDialog dialog( 0, CustomDialog::CloseButton );
    dialog.setOptionName( "PRINT_SESSION_DIALOG" );
    dialog.setWindowTitle( tr( "Session files" ) );
    LogWidget* logWidget = new LogWidget( &dialog );
    dialog.mainLayout().addWidget( logWidget );

    // populate log widget
    for( const auto& record:records )
    { logWidget->append( record.file() + "\n" ); }

    dialog.exec();

}

//___________________________________________________________
void Application::_restoreSession()
{
    Debug::Throw( "Application::_restoreSession.\n" );
    auto&& records( sessionFiles_->records() );
    if( !( records.isEmpty() || !RestoreSessionDialog( qApp->activeWindow(), records ).exec() ) )
    { windowServer_->open( records ); }
}

//___________________________________________________________
void Application::_restoreLastSession()
{
    Debug::Throw( "Application::_restoreLastSession.\n" );
    auto&& records( lastSessionFiles_->records() );
    if( !( records.isEmpty() || !RestoreSessionDialog( qApp->activeWindow(), records ).exec() ) )
    { windowServer_->open( records ); }
}

//___________________________________________________________
void Application::_discardSession()
{
    Debug::Throw( "Application::_discardSession.\n" );
    sessionFiles_->clear();
    static_cast<XmlFileList*>(sessionFiles_.get())->write();
    _updateSessionActions();
}

//___________________________________________________________
void Application::_updateLastSessionFiles()
{
    lastSessionFiles_->set( windowServer_->records( WindowServer::ExistingOnly ) );
    static_cast<XmlFileList*>(lastSessionFiles_.get())->write();
}

//___________________________________________________________
void Application::_updateSessionActions()
{

    Debug::Throw( "Application::_updateSessionActions.\n" );

    {
        const bool empty( sessionFiles_->isEmpty() );
        const bool recordsEmpty( windowServer_->records( WindowServer::ExistingOnly ).isEmpty() );
        restoreSessionAction_->setEnabled( !empty );
        discardSessionAction_->setEnabled( !empty );
        saveSessionAction_->setEnabled( !recordsEmpty );
        printSessionAction_->setEnabled( !recordsEmpty );
    }

    {
        const bool empty( lastSessionFiles_->isEmpty() );
        restoreLastSessionAction_->setEnabled( !empty );
    }

}

//_____________________________________________
void Application::_showMonitoredFiles()
{

    Debug::Throw( "Application::_showMonitoredFiles.\n" );
    FileCheckDialog dialog( qApp->activeWindow() );
    dialog.setFiles( fileCheck_->fileSystemWatcher().files() );
    dialog.exec();

}

//_______________________________________________
void Application::_exit()
{

    Debug::Throw( "Application::_exit.\n" );
    _updateLastSessionFiles();
    if( !windowServer_->closeAll() ) return;
    qApp->quit();

}

//________________________________________________
bool Application::_processCommand( Server::ServerCommand command )
{

    if( BaseApplication::_processCommand( command ) ) return true;
    if( command.command() == Server::ServerCommand::CommandType::Raise )
    {

        // copy arguments and try open
        _setArguments( command.arguments() );
        startupTimer_.start( 100, this );
        return true;
    }

    return false;

}

//___________________________________________________________
void Application::timerEvent( QTimerEvent* event )
{
    Debug::Throw( "Application::timerEvent.\n" );
    if( event->timerId() == startupTimer_.timerId() )
    {

        startupTimer_.stop();
        windowServer_->readFilesFromArguments( commandLineParser( _arguments() ) );
        connect( qApp, &QApplication::lastWindowClosed, qApp, &QApplication::quit, Qt::UniqueConnection );

    } else return QObject::timerEvent( event );
}

//_________________________________________________
void Application::_updateConfiguration()
{
    Debug::Throw( "Application::_updateConfiguration.\n" );
    static_cast<XmlFileList*>(recentFiles_.get())->setDBFile( File( XmlOptions::get().raw( "RC_FILE" ) ) );
    static_cast<XmlFileList*>(sessionFiles_.get())->setDBFile( File( XmlOptions::get().raw( "RC_FILE" ) ) );
    static_cast<XmlFileList*>(lastSessionFiles_.get())->setDBFile( File( XmlOptions::get().raw( "RC_FILE" ) ) );
    recentFiles_->setMaxSize( XmlOptions::get().get<int>( "DB_SIZE" ) );
}

//___________________________________________________________
void Application::_installActions()
{

    Debug::Throw( "Application::_installActions.\n" );

    // need to modify closeAction signal for proper exit
    closeAction().disconnect();
    connect( &closeAction(), &QAction::triggered, this, &Application::_exit );

    // spell check
    spellCheckConfigurationAction_ = new QAction( IconEngine::get( IconNames::SpellCheck ), tr( "Configure Spell Checking..." ), this );
    connect( spellCheckConfigurationAction_, &QAction::triggered, this, &Application::_spellCheckConfiguration );

    // document classes
    documentClassesConfigurationAction_ = new QAction( IconEngine::get( IconNames::Configure ), tr( "Configure Document Types..." ), this );
    connect( documentClassesConfigurationAction_, &QAction::triggered, this, &Application::_documentClassesConfiguration );

    // save session
    saveSessionAction_ = new QAction( IconEngine::get( IconNames::Save ), tr( "Save Current Session" ), this );
    connect( saveSessionAction_, &QAction::triggered, this, &Application::_saveSession );

    // print session
    printSessionAction_ = new QAction( IconEngine::get( IconNames::Print ), tr( "Print Current Session" ), this );
    connect( printSessionAction_, &QAction::triggered, this, &Application::_printSession );

    // restore session
    restoreSessionAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Restore Saved Session" ), this );
    connect( restoreSessionAction_, &QAction::triggered, this, &Application::_restoreSession );

    // restore session
    restoreLastSessionAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Restore Last Session" ), this );
    connect( restoreLastSessionAction_, &QAction::triggered, this, &Application::_restoreLastSession );

    // discard session
    discardSessionAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Discard Saved Session" ), this );
    connect( discardSessionAction_, &QAction::triggered, this, &Application::_discardSession );

    // monitored files
    monitoredFilesAction_ = new QAction( tr( "Show Monitored Files" ), this );
    monitoredFilesAction_->setToolTip( tr( "Show monitored files" ) );
    connect( monitoredFilesAction_, &QAction::triggered, this, &Application::_showMonitoredFiles );

}
