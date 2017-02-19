#ifndef Application_h
#define Application_h

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

#include "BaseApplication.h"
#include "CommandLineArguments.h"
#include "CommandLineParser.h"
#include "Config.h"
#include "Counter.h"
#include "IconEngine.h"

#include <QBasicTimer>
#include <QTimerEvent>

class AutoSave;
class DocumentClassManager;
class FileCheck;
class FileList;
class WindowServer;
class Sync;

//* Application singleton
class Application: public BaseApplication, public Counter
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    Application( CommandLineArguments );

    //* destructor
    ~Application( void );

    //* initialize application manager
    virtual bool initApplicationManager( void );

    //* create all widgets
    bool realizeWidget( void );

    //* file list
    FileList& recentFiles( void ) const
    { return *recentFiles_; }

    //* Window server
    WindowServer& windowServer( void ) const
    { return *windowServer_; }

    //* DocumentClassManager
    DocumentClassManager& classManager( void ) const
    { return *classManager_; }

    //* retrieve AutoSave
    AutoSave& autoSave( void ) const
    { return *autosave_; }

    //* file check
    FileCheck& fileCheck( void ) const
    { return *fileCheck_; }

    //*@name actions
    //@{

    //* spellcheck configuration
    QAction& spellCheckConfigurationAction( void ) const
    { return *spellCheckConfigurationAction_; }

    //* document classes configuration
    QAction& documentClassesConfigurationAction( void ) const
    { return *documentClassesConfigurationAction_; }

    //* save session
    QAction& saveSessionAction( void ) const
    { return *saveSessionAction_; }

    //* print session
    QAction& printSessionAction( void ) const
    { return *printSessionAction_; }

    //* restore session
    QAction& restoreSessionAction( void ) const
    { return *restoreSessionAction_; }

    //* discard session
    QAction& discardSessionAction( void ) const
    { return *discardSessionAction_; }

    //* monitored files
    QAction& monitoredFilesAction( void ) const
    { return *monitoredFilesAction_; }


    //@}

    //*@name application information
    //@{

    //* command line parser
    CommandLineParser commandLineParser( CommandLineArguments = CommandLineArguments(), bool ignoreWarnings = true ) const;

    //* command line help
    void usage( void ) const;

    //* application name
    virtual QString applicationName( void ) const
    { return "Qedit"; }

    //* application icon
    virtual QIcon applicationIcon( void ) const
    { return IconEngine::get( ":/qedit.png" ); }

    // application version
    virtual QString applicationVersion( void ) const
    { return VERSION; }

    //@}

    Q_SIGNALS:

    //* spellcheck configuration modified
    void spellCheckConfigurationChanged( void );

    //* document classes have been modified
    void documentClassesChanged( void );

    protected Q_SLOTS:

    //* Update Document Classes from options
    void _updateDocumentClasses( void );

    //* configuration
    void _configuration( void );

    //* spellcheck configuration
    void _spellCheckConfiguration( void );

    //* document classes configuration
    void _documentClassesConfiguration( void );

    //* save session
    void _saveSession( void );

    //* save session
    void _printSession( void );

    //* restore session
    void _restoreSession( void );

    //* discard
    void _discardSession( void );

    //* update session actions
    void _updateSessionActions( void );

    //* monitored files
    void _showMonitoredFiles( void );

    //* exit safely
    void _exit( void );

    //* process request from application manager
    virtual bool _processCommand( Server::ServerCommand );

    protected:

    //* timer events
    /**
    this allows to read files from arguments after the call
    to "exec()" in the main routine,
    */
    void timerEvent( QTimerEvent* );

    private Q_SLOTS:

    //* update configuration
    void _updateConfiguration( void );

    private:

    //* install actions
    void _installActions( void );

    //* recent files list
    FileList* recentFiles_ = nullptr;

    //* session files
    FileList* sessionFiles_ = nullptr;

    //* window server
    WindowServer* windowServer_ = nullptr;

    //* document class manager singleton
    DocumentClassManager* classManager_ = nullptr;

    //* file autoSave manager
    AutoSave* autosave_ = nullptr;

    //* file check
    FileCheck* fileCheck_ = nullptr;

    //* startup single shot timer
    /**
    it allows to call startup methods after the exec() function
    is called from the main routine
    */
    QBasicTimer startupTimer_;

    //*@name actions
    //@{

    //* spellcheck configuration
    QAction* spellCheckConfigurationAction_ = nullptr;

    //* document classes
    QAction* documentClassesConfigurationAction_ = nullptr;

    //* save session
    QAction* saveSessionAction_ = nullptr;

    //* print session
    QAction* printSessionAction_ = nullptr;

    //* restore session
    QAction* restoreSessionAction_ = nullptr;

    //* discard session
    QAction* discardSessionAction_ = nullptr;

    //* show monitored files
    QAction* monitoredFilesAction_ = nullptr;

    //@}

};

#endif
