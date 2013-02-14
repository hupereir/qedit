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

#include "BaseApplication.h"
#include "CommandLineArguments.h"
#include "CommandLineParser.h"
#include "Config.h"
#include "Counter.h"

#include <QBasicTimer>

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
    { return *recentFiles_; }

    //! Window server
    WindowServer& windowServer( void ) const
    { return *windowServer_; }

    //! DocumentClassManager
    DocumentClassManager& classManager( void ) const
    { return *classManager_; }

    //! retrieve AutoSave
    AutoSave& autoSave( void ) const
    { return *autosave_; }

    //! file check
    FileCheck& fileCheck( void ) const
    { return *fileCheck_; }

    //!@name actions
    //@{

    //! spellcheck configuration
    QAction& spellCheckConfigurationAction( void ) const
    { return *spellCheckConfigurationAction_; }

    //! document classes configuration
    QAction& documentClassesConfigurationAction( void ) const
    { return *documentClassesConfigurationAction_; }

    //! monitored files
    QAction& monitoredFilesAction( void ) const
    { return *monitoredFilesAction_; }

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

    //! spellcheck configuration
    void _spellCheckConfiguration( void );

    //! document classes configuration
    void _documentClassesConfiguration( void );

    //! exit safely
    void _exit( void );

    //! process request from application manager
    virtual bool _processCommand( SERVER::ServerCommand );

    //! monitored files
    void _showMonitoredFiles( void );

    protected:

    //! timer events
    /*!
    this allows to read files from arguments after the call
    to "exec()" in the main routine,
    */
    void timerEvent( QTimerEvent* );

    private:

    //! recent files list
    FileList* recentFiles_;

    //! window server
    WindowServer* windowServer_;

    //! document class manager singleton
    DocumentClassManager* classManager_;

    //! file autoSave manager
    AutoSave* autosave_;

    //! file check
    FileCheck* fileCheck_;

    //! startup single shot timer
    /*!
    it allows to call startup methods after the exec() function
    is called from the main routine
    */
    QBasicTimer startupTimer_;

    //!@name actions
    //@{

    //! spellcheck configuration
    QAction* spellCheckConfigurationAction_;

    //! document classes
    QAction* documentClassesConfigurationAction_;

    //! show monitored files
    QAction* monitoredFilesAction_;

    //@}

};

#endif
