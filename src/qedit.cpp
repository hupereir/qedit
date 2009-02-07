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
   \file qedit.cpp
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
#include "SystemOptions.h"
#include "ErrorHandler.h"
#include "Application.h"
#include "Singleton.h"
#include "XmlOptions.h"

using namespace std;

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
  
  // load possible command file
  CommandLineArguments arguments( argc, argv );
  if( Application::commandLineParser( arguments, false ).hasFlag( "--help" ) )
  {
    Application::usage();
    return 0;
  }
  
  // load default options
  installDefaultOptions();
  installSystemOptions();
  XmlOptions::read( XmlOptions::get().raw( "RC_FILE" ) );     
  
  // set debug level
  int debug_level( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
  Debug::setLevel( debug_level );
  if( debug_level ) XmlOptions::get().print();
  
  // initialize main frame and run loop
  Q_INIT_RESOURCE( basePixmaps );
  Q_INIT_RESOURCE( patterns );
  Q_INIT_RESOURCE( pixmaps );
  QApplication application( argc, argv );
  
  Application singleton( arguments );
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
