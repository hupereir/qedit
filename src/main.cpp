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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "Application.h"
#include "Debug.h"
#include "DefaultOptions.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "File.h"
#include "SystemOptions.h"
#include "ErrorHandler.h"
#include "Singleton.h"
#include "XmlFileRecord.h"
#include "XmlMigration.h"
#include "XmlOptions.h"

#include <QApplication>
#include <signal.h>

//_______________________________
//! handles keyboard interruptions
void interrupt( int sig );

//__________________________________________
//! main function
int main (int argc, char *argv[])
{

    // Ensure proper cleaning at exit
    signal(SIGINT,  interrupt);
    signal(SIGTERM, interrupt);

    // install error handler
    qInstallMsgHandler( ErrorHandler::Throw );

    // usage
    CommandLineArguments arguments( argc, argv );
    if( Application::commandLineParser( arguments, false ).hasFlag( "--help" ) )
    {
        Application::usage();
        return 0;
    }

    // options
    installDefaultOptions();
    installSystemOptions();
    XmlOptions::setFile( XmlOptions::get().raw( "RC_FILE" ) );
    XmlOptions::read();

    // debug level
    Debug::setLevel( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    if( Debug::level() ) Debug::Throw() << XmlOptions::get() << endl;

    // migration
    XmlMigration( File(".qedit_db").addPath(Util::home() ), "DB_FILE", FILERECORD::XML::FILE_LIST ).run();

    // resources
    Q_INIT_RESOURCE( basePixmaps );
    Q_INIT_RESOURCE( patterns );
    Q_INIT_RESOURCE( pixmaps );

    // application
    QApplication application( argc, argv );
    application.setApplicationName( "Qedit" );
    Application singleton( arguments );
    singleton.setUseFixedFonts( true );
    Singleton::get().setApplication( &singleton );
    singleton.initApplicationManager();

    application.exec();

    return 0;

}

//_____________________________________________
void interrupt( int sig )
{
    Debug::Throw() << "interrupt - Recieved signal " << sig << endl;
    qApp->quit();
}
