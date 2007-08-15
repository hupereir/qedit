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

/*!
   \file QEdit.cc
   \brief main 
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <string>

#include "Debug.h"
#include "DefaultOptions.h"
#include "ErrorHandler.h"
#include "MainFrame.h"
#include "XmlOptions.h"
#include "Util.h"

using namespace std;

//_______________________________
//! handles keyboard interuptions
void interrupt( int sig ); 
 
//__________________________________________
//! main function
int main (int argc, char *argv[])
{
  try {
    
              
    // Ensure proper cleaning at exit
    signal(SIGINT,  interrupt);
    signal(SIGTERM, interrupt);
    
    // install error handler
    ErrorHandler::disableMessage( "QServerSocket: failed to bind or listen to the socket" );
    ErrorHandler::disableMessage( "QPixmap::resize: TODO: resize alpha data" );
    qInstallMsgHandler( ErrorHandler::Throw );

    // load possible command file
    ArgList args( argc, argv );
    if( args.find( "--help" ) )
    {
      MainFrame::usage();
      return 0;
    }

    // add user name
    Option option( "USER_NAME", Util::user() );
    option.setRecordable( false );
    XmlOptions::get().add( option );
    
    // add application name
    option = Option( "APP_NAME", "QEDIT" );
    option.setRecordable( false );
    XmlOptions::get().add( option );
    
    // add HelpFile
    option = Option( "HELP_FILE", Util::env( "HOME", "." ) + "/.qedit_help" );
    option.setRecordable( false );
    XmlOptions::get().add( option );

    // add DB file
    option = Option( "DB_FILE", Util::env( "HOME", "." ) + "/.qedit_db" );
    option.setRecordable( false );
    XmlOptions::get().add( option );
    
    // load default options
    installDefaultOptions();
  
    // load user resource file
    string rcfile = Util::env( "HOME", "." ) + "/.qeditrc";
    XmlOptions::read( rcfile ); 
      
    // set debug level
    int debug_level( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    Debug::setLevel( debug_level );
    if( debug_level ) XmlOptions::get().dump();

    // initialize main frame and run loop
    // QApplication::setStyle(new FlatStyle() );
    MainFrame main_frame( argc, argv );
    main_frame.initApplicationManager();
    main_frame.exec();
  } catch ( exception& e ) { cout << e.what() << endl; }
  return 0;
}

//_____________________________________________
void interrupt( int sig )
{ 
  Debug::Throw() << "interrupt - Recieved signal " << sig << endl;
  qApp->quit();
}
