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
#include "Debug.h"
#include "DefaultOptions.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "File.h"
#include "SystemOptions.h"
#include "Singleton.h"
#include "XmlFileRecord.h"
#include "XmlMigration.h"
#include "XmlOptions.h"

#include <QApplication>

//__________________________________________
//! main function
int main (int argc, char *argv[])
{

    // options
    installDefaultOptions();
    installSystemOptions();
    XmlOptions::setFile( XmlOptions::get().raw( "RC_FILE" ) );
    XmlOptions::read();

    // debug level
    Debug::setLevel( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    if( Debug::level() ) Debug::Throw() << XmlOptions::get() << endl;

    // migration
    XmlMigration( File(".qedit_db").addPath(Util::home() ), "DB_FILE", Base::Xml::FileList ).run();

    // resources
    Q_INIT_RESOURCE( basePixmaps );
    Q_INIT_RESOURCE( patterns );
    Q_INIT_RESOURCE( pixmaps );

    // application
    QApplication application( argc, argv );
    Application singleton( CommandLineArguments( argc, argv ) );
    singleton.setUseFixedFonts( true );
    Singleton::get().setApplication( &singleton );

    // initialize and run
    if( singleton.initApplicationManager() )
    {  application.exec(); }

    return 0;

}
