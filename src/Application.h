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

#include "AutoSave.h"
#include "BaseApplication.h"
#include "CommandLineArguments.h"
#include "CommandLineParser.h"
#include "Config.h"
#include "Counter.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "IconEngine.h"

#include <QBasicTimer>
#include <QTimerEvent>

#include <memory>

class AutoSave;
class DocumentClassManager;
class FileCheck;
class FileList;
class WindowServer;
class Sync;

//* Application singleton
class Application: public BaseApplication, private Base::Counter<Application>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit Application( CommandLineArguments );

    //* initialize application manager
    bool initApplicationManager() override;

    //* create all widgets
    bool realizeWidget() override;

    //* file list
    FileList& recentFiles() const
    { return *recentFiles_; }

    //* Window server
    WindowServer& windowServer() const
    { return *windowServer_; }

    //* DocumentClassManager
    DocumentClassManager& classManager() const
    { return *classManager_; }

    //* retrieve AutoSave
    AutoSave& autoSave() const
    { return *autosave_; }

    //* file check
    FileCheck& fileCheck() const
    { return *fileCheck_; }

    //*@name actions
    //@{

    //* spellcheck configuration
    QAction& spellCheckConfigurationAction() const
    { return *spellCheckConfigurationAction_; }

    //* document classes configuration
    QAction& documentClassesConfigurationAction() const
    { return *documentClassesConfigurationAction_; }

    //* save session
    QAction& saveSessionAction() const
    { return *saveSessionAction_; }

    //* print session
    QAction& printSessionAction() const
    { return *printSessionAction_; }

    //* restore session
    QAction& restoreSessionAction() const
    { return *restoreSessionAction_; }

    //* restore session
    QAction& restoreLastSessionAction() const
    { return *restoreLastSessionAction_; }

    //* discard session
    QAction& discardSessionAction() const
    { return *discardSessionAction_; }

    //* monitored files
    QAction& monitoredFilesAction() const
    { return *monitoredFilesAction_; }


    //@}

    //*@name application information
    //@{

    //* command line parser
    CommandLineParser commandLineParser( CommandLineArguments = CommandLineArguments(), bool ignoreWarnings = true ) const override;

    //* command line help
    void usage() const override;

    //* application name
    QString applicationName() const override
    { return "Qedit"; }

    //* application icon
    QIcon applicationIcon() const override
    { return IconEngine::get( ":/qedit.png" ); }

    // application version
    QString applicationVersion() const override
    { return VERSION; }

    //@}

    Q_SIGNALS:

    //* spellcheck configuration modified
    void spellCheckConfigurationChanged();

    //* document classes have been modified
    void documentClassesChanged();

    protected Q_SLOTS:

    //* Update Document Classes from options
    void _updateDocumentClasses();

    //* configuration
    void _configuration();

    //* spellcheck configuration
    void _spellCheckConfiguration();

    //* document classes configuration
    void _documentClassesConfiguration();

    //* save session
    void _saveSession();

    //* save session
    void _printSession();

    //* restore session
    void _restoreSession();

    //* restore session
    void _restoreLastSession();

    //* discard
    void _discardSession();

    //* update session files
    void _updateLastSessionFiles();

    //* update session actions
    void _updateSessionActions();

    //* monitored files
    void _showMonitoredFiles();

    //* exit safely
    void _exit();

    //* process request from application manager
    bool _processCommand( Server::ServerCommand ) override;

    protected:

    //* timer events
    /**
    this allows to read files from arguments after the call
    to "exec()" in the main routine,
    */
    void timerEvent( QTimerEvent* ) override;

    private Q_SLOTS:

    //* update configuration
    void _updateConfiguration();

    private:

    //* install actions
    void _installActions();

    //* recent files list
    std::unique_ptr<FileList> recentFiles_;

    //* session files
    std::unique_ptr<FileList> sessionFiles_;

    //* session files
    std::unique_ptr<FileList> lastSessionFiles_;

    //* window server
    std::unique_ptr<WindowServer> windowServer_;

    //* document class manager singleton
    std::unique_ptr<DocumentClassManager> classManager_;

    //* file autoSave manager
    std::unique_ptr<AutoSave> autosave_;

    //* file check
    std::unique_ptr<FileCheck> fileCheck_;

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

    //* restore session
    QAction* restoreLastSessionAction_ = nullptr;

    //* discard session
    QAction* discardSessionAction_ = nullptr;

    //* show monitored files
    QAction* monitoredFilesAction_ = nullptr;

    //@}

};

#endif
