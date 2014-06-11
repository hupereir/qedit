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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "Application.h"
#include "Application.moc"
#include "AutoSave.h"
#include "ConfigurationDialog.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
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
#include "AutoSpellConfiguration.h"
#include "SpellCheckConfiguration.h"
#include "SpellInterface.h"
#endif

#include <QMessageBox>

//____________________________________________
Application::Application( CommandLineArguments arguments ):
    BaseApplication( 0, arguments ),
    Counter( "Application" ),
    recentFiles_( 0 ),
    sessionFiles_( 0 ),
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

}

//____________________________________________
bool Application::initApplicationManager( void )
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
bool Application::realizeWidget( void )
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
    recentFiles_ = new XmlFileList( this );
    recentFiles_->setCheck( true );

    // session file list
    sessionFiles_ = new XmlFileList( this );
    static_cast<XmlFileList*>( sessionFiles_ )->setTagName( Base::Xml::SessionFileList );

    // class manager
    classManager_ = new DocumentClassManager();

    // autosave
    autosave_ = new AutoSave();

    // file check
    fileCheck_ = new FileCheck();

    // window server
    windowServer_ = new WindowServer();
    connect( windowServer_, SIGNAL(sessionFilesChanged()), SLOT(_updateSessionActions()) );

    // create first window and show
    windowServer_->newMainWindow().centerOnDesktop();

    // update configuration
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
    out.registerOption( "--filter", tr( "string" ), tr( "select  for spell checking") );
    out.registerOption( "--dictionary", tr( "string" ), tr( "select dictionary for spell checking") );
    out.registerFlag( "--list-filters", tr( "list all filters available to spell checking") );
    out.registerFlag( "--list-dictionaries", tr( "list all dictionaries available to spell checking") );
    #endif

    if( !arguments.isEmpty() ) out.parse( arguments, ignoreWarnings );
    return out;

}

//____________________________________________
void Application::usage( void ) const
{
    _usage( "qedit", tr("[options] [files]") );
    commandLineParser().usage();
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
    connect( &dialog, SIGNAL(configurationChanged()), SIGNAL(configurationChanged()) );
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

    SpellCheck::SpellCheckConfiguration* spellConfig = new SpellCheck::SpellCheckConfiguration( &dialog );
    dialog.mainLayout().addWidget( spellConfig );

    SpellCheck::AutoSpellConfiguration* autospellConfig = new SpellCheck::AutoSpellConfiguration( &dialog );
    dialog.mainLayout().addWidget( autospellConfig );
    dialog.centerOnWidget( qApp->activeWindow() );

    if( dialog.exec() == QDialog::Rejected ) return;
    spellConfig->write( XmlOptions::get() );
    autospellConfig->write( XmlOptions::get() );
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
    if( !dialog.exec() ) return;

    XmlOptions::get().clearSpecialOptions( "PATTERN_FILENAME" );
    foreach( const File& file, dialog.userFiles() )
    { XmlOptions::get().add( "PATTERN_FILENAME", file ); }

    _updateDocumentClasses();

}

//___________________________________________________________
void Application::_saveSession( void )
{
    Debug::Throw( "Application::_saveSession.\n" );
    sessionFiles_->set( windowServer_->records( WindowServer::ExistingOnly ) );
    static_cast<XmlFileList*>(sessionFiles_)->write();
    _updateSessionActions();
}

//___________________________________________________________
void Application::_printSession( void )
{
    Debug::Throw( "Application::_printSession.\n" );

    // get session records
    const FileRecord::List records( windowServer_->records( WindowServer::ExistingOnly ) );
    if( records.empty() ) return;

    // create dialog
    CustomDialog dialog( 0, CustomDialog::CloseButton );
    dialog.setOptionName( "PRINT_SESSION_DIALOG" );
    dialog.setWindowTitle( "Session files - Qedit" );
    LogWidget* logWidget = new LogWidget( &dialog );
    dialog.mainLayout().addWidget( logWidget );

    // populate log widget
    foreach( const FileRecord& record, records )
    { logWidget->append( record.file() + "\n" ); }

    dialog.exec();

}

//___________________________________________________________
void Application::_restoreSession( void )
{
    Debug::Throw( "Application::_restoreSession.\n" );
    const FileRecord::List records( sessionFiles_->records() );
    if( !( records.isEmpty() || !RestoreSessionDialog( qApp->activeWindow(), records ).exec() ) )
    { windowServer_->open( records ); }
}

//___________________________________________________________
void Application::_discardSession( void )
{
    Debug::Throw( "Application::_discardSession.\n" );
    sessionFiles_->clear();
    static_cast<XmlFileList*>(sessionFiles_)->write();
    _updateSessionActions();
}

//___________________________________________________________
void Application::_updateSessionActions( void )
{

    Debug::Throw( "Application::_updateSessionActions.\n" );
    const bool empty( sessionFiles_->isEmpty() );
    restoreSessionAction_->setEnabled( !empty );
    discardSessionAction_->setEnabled( !empty );
    saveSessionAction_->setEnabled( !windowServer_->records( WindowServer::ExistingOnly ).isEmpty() );

}

//_____________________________________________
void Application::_showMonitoredFiles( void )
{

    Debug::Throw( "Application::_showMonitoredFiles.\n" );
    FileCheckDialog dialog( qApp->activeWindow() );
    dialog.setFiles( fileCheck().fileSystemWatcher().files() );
    dialog.exec();

}

//_______________________________________________
void Application::_exit( void )
{

    Debug::Throw( "Application::_exit.\n" );
    if( !windowServer_->closeAll() ) return;
    qApp->quit();

}

//________________________________________________
bool Application::_processCommand( Server::ServerCommand command )
{

    if( BaseApplication::_processCommand( command ) ) return true;
    if( command.command() == Server::ServerCommand::Raise )
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
        connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()), Qt::UniqueConnection );

    } else return QObject::timerEvent( event );
}

//___________________________________________________________
void Application::_installActions( void )
{

    Debug::Throw( "Application::_installActions.\n" );

    // need to modify closeAction signal for proper exit
    closeAction().disconnect();
    connect( &closeAction(), SIGNAL(triggered()), SLOT(_exit()) );

    // spell check
    spellCheckConfigurationAction_ = new QAction( IconEngine::get( IconNames::SpellCheck ), "Configure Spell Checking...", this );
    connect( spellCheckConfigurationAction_, SIGNAL(triggered()), SLOT(_spellCheckConfiguration()) );

    // document classes
    documentClassesConfigurationAction_ = new QAction( IconEngine::get( IconNames::Configure ), "Configure Document Types...", this );
    connect( documentClassesConfigurationAction_, SIGNAL(triggered()), SLOT(_documentClassesConfiguration()) );

    // save session
    saveSessionAction_ = new QAction( IconEngine::get( IconNames::Save ), "Save Current Session", this );
    connect( saveSessionAction_, SIGNAL(triggered()), SLOT(_saveSession()) );

    // print session
    printSessionAction_ = new QAction( IconEngine::get( IconNames::Print ), "Print Current Session", this );
    connect( printSessionAction_, SIGNAL(triggered()), SLOT(_printSession()) );

    // restore session
    restoreSessionAction_ = new QAction( IconEngine::get( IconNames::Open ), "Restore Saved Session", this );
    connect( restoreSessionAction_, SIGNAL(triggered()), SLOT(_restoreSession()) );

    // discard session
    discardSessionAction_ = new QAction( IconEngine::get( IconNames::Delete ), "Discard Saved Session", this );
    connect( discardSessionAction_, SIGNAL(triggered()), SLOT(_discardSession()) );

    // monitored files
    monitoredFilesAction_ = new QAction( "Show Monitored Files", this );
    monitoredFilesAction_->setToolTip( "Show monitored files" );
    connect( monitoredFilesAction_, SIGNAL(triggered()), SLOT(_showMonitoredFiles()) );

}
