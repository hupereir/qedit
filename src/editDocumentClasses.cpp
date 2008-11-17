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

#include <QApplication>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <string>

#include "Debug.h"
#include "DefaultOptions.h"
#include "DocumentClassManager.h"
#include "DocumentClassManagerDialog.h"
#include "FlatStyle.h"
#include "SystemOptions.h"
#include "ErrorHandler.h"
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
  ErrorHandler::get().disableMessage( "Object::connect:" );
  qInstallMsgHandler( ErrorHandler::Throw );
  
  // load default options
  installDefaultOptions();
  installSystemOptions();
  XmlOptions::read( XmlOptions::get().raw( "RC_FILE" ) );     
  
  // set debug level
  int debug_level( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
  Debug::setLevel( debug_level );
  if( debug_level ) cout << XmlOptions::get();
  
  // initialize main frame and run loop
  // initialize main frame and run loop
  Q_INIT_RESOURCE( basePixmaps );
  Q_INIT_RESOURCE( patterns );
  Q_INIT_RESOURCE( pixmaps );
  QApplication application( argc, argv );
  
  // options
  if( XmlOptions::get().get<bool>( "USE_FLAT_THEME" ) ) application.setStyle( new FlatStyle() );
  // set fonts
  QFont font;
  font.fromString( XmlOptions::get().raw( "FONT_NAME" ).c_str() );
  application.setFont( font );
  
  font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ).c_str() );
  application.setFont( font, "QLineEdit" );
  application.setFont( font, "QTextEdit" );
  
  // read document classes
  DocumentClassManager manager;
  list<string> files( XmlOptions::get().specialOptions<string>( "PATTERN_FILENAME" ) );
  for( list<string>::const_iterator iter = files.begin(); iter != files.end(); iter++ )
  { 
    cout << "editDocumentClasses - reading " << *iter << endl;
    manager.read( *iter ); 
  }
  
  // prepare dialog
  DocumentClassManagerDialog dialog(0, &manager);
  dialog.centerOnDesktop();
  dialog.show();
  application.exec();
  
  return 0;
}

//_____________________________________________
void interrupt( int sig )
{ 
  Debug::Throw() << "interrupt - Recieved signal " << sig << endl;
  qApp->quit();
}
