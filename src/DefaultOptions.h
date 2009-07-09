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
* Place, Suite 330, Boston, MA	02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file DefaultOptions.h
  \brief default options
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "Config.h"
#include "FileRecordModel.h"
#include "MainWindow.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlOptions.h"

//_____________________________________________________
//! default options installer
void installDefaultOptions( void )
{

  // set options default values
  XmlOptions::get().setAutoDefault( true );
  XmlOptions::get().keep( "PIXMAP_PATH" );
  XmlOptions::get().add( "PIXMAP_PATH", "/usr/share/icons/oxygen/32x32/mimetypes");
  XmlOptions::get().add( "PIXMAP_PATH", "/usr/share/icons/crystalsvg/32x32/mimetypes");
  
  XmlOptions::get().set( "ICON_PIXMAP", Option( ":/icon.png" , "application icon") );
  
  XmlOptions::get().set( "FIXED_FONT_NAME", Option( "fixed" , "fixed font"  ) ); 

  XmlOptions::get().set( "USE_PRINT_COMMAND", Option( "0", "use print/edit command" ) );
  XmlOptions::get().set( "PRINT_MODE", Option( "PDF" , "print mode. Can be either PDF/HTML") );  
  XmlOptions::get().set( "PRINT_LINE_SIZE", Option( "0" , "Maximum line width. 0 means any.") );  
  
  // input document_class pattern filenames options are special.
  // keep the full list
  XmlOptions::get().keep( "PATTERN_FILENAME" );
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/c_patterns.xml" , "default patterns for c/c++ files") );
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/default_patterns.xml" , "default patterns") );
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/diff_patterns.xml" , "default patterns for diff files") );
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/fortran_patterns.xml" , "default patterns for fortran files") ); 
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/html_patterns.xml" , "default patterns for html files") );
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/latex_patterns.xml", "default patterns for latex files") );
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/makefile_patterns.xml", "default patterns for makefile files") );
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/perl_patterns.xml", "default patterns for perl files") );
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/shell_patterns.xml", "default patterns for shell files") );
  XmlOptions::get().add( "PATTERN_FILENAME", Option( ":/patterns/xml_patterns.xml", "default patterns for xml files") );

  XmlOptions::get().set( "CUSTOM_BLOCK_DELIMITERS", Option( "1", "use custom symbols for block delimiters") );

  // diff configuration
  XmlOptions::get().set( "DIFF_ADDED_COLOR", "#d0eaff" );
  XmlOptions::get().set( "DIFF_CONFLICT_COLOR", "#ecffec" );
  
  XmlOptions::get().set( "PARENTHESIS_COLOR", Option( "red", "color when highlighting matching parenthesis" ) );
  XmlOptions::get().set( "TAGGED_BLOCK_COLOR", Option( "#fffdd4", "color for tagged paragraphs" ) );
  XmlOptions::get().set( "BACKUP", Option( "0", "make backup when saving file" ) );
  XmlOptions::get().set( "DB_SIZE", "30" );

  XmlOptions::get().set( "SHOW_HIDDEN_FILES", "1" );
  XmlOptions::get().set( "SHOW_BLOCK_DELIMITERS", "1" );
  XmlOptions::get().set( "TEXT_INDENT", "1" );
  XmlOptions::get().set( "TEXT_HIGHLIGHT", "1" );
  XmlOptions::get().set( "TEXT_PARENTHESIS", "1" );
  XmlOptions::get().set( "WRAP_FROM_CLASS", Option( "1", "set to 1 if you wand DocumentClass to enable/disable wrapping" ) );
  XmlOptions::get().set( "EMULATE_TABS_FROM_CLASS", Option( "1", "set to 1 if you wand DocumentClass to enable/disable tab emulation" ) );
  XmlOptions::get().set( "WINDOW_HEIGHT", "500" );
  XmlOptions::get().set( "WINDOW_WIDTH", "700" );
  XmlOptions::get().set( "SERVER_TIMEOUT_DELAY", Option( "3000" , "timeout delay to wait for existing server to answer" ) );
  XmlOptions::get().set( "SHOW_NAVIGATION_FRAME", Option( "1", "show/hide navigation window" ) );

  // splitters
  XmlOptions::get().set( "NAVIGATION_FRAME_WIDTH", Option( "200" , "navigation frame width" ) );
  XmlOptions::get().set<unsigned int>( "SESSION_FILES_MASK", (1<<FileRecordModel::ICON)|(1<<FileRecordModel::FILE) );
  XmlOptions::get().set<unsigned int>( "RECENT_FILES_MASK", (1<<FileRecordModel::ICON)|(1<<FileRecordModel::FILE) );
  
  // toolbars
  XmlOptions::get().set( "DOCUMENT_CLASS_MANAGER_TOOLBAR", Option( "1" , "toolbar visibility" ) );
  XmlOptions::get().set( "EDITION_TOOLBAR", Option( "1" , "toolbar visibility" ) );
  XmlOptions::get().set( "EXTRA_TOOLBAR", Option( "1" , "toolbar visibility" ) );
  XmlOptions::get().set( "FILE_TOOLBAR", Option( "1" , "toolbar visibility" ) );
  XmlOptions::get().set( "NAVIGATION_TOOLBAR", Option( "1" , "toolbar visibility" ) );
  XmlOptions::get().set( "SPLIT_TOOLBAR", Option( "1" , "toolbar visibility" ) );

  XmlOptions::get().set( "DOCUMENT_CLASS_MANAGER_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
  XmlOptions::get().set( "EDITION_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
  XmlOptions::get().set( "EXTRA_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
  XmlOptions::get().set( "FILE_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
  XmlOptions::get().set( "SPLIT_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );

  XmlOptions::get().set( "AUTOSAVE", "1" );
  XmlOptions::get().set( "AUTOSAVE_INTERVAL", Option( "20", "time interval between two automatic save of the file" ) );

  XmlOptions::get().set( "HIGHLIGHT_PARAGRAPH", Option( "1", "active paragraph highlighting" ) );
  XmlOptions::get().set( "HIGHLIGHT_COLOR", Option( "#FFFDD4", "paragraph highlight color" ) );
  
  /* multiple views */
  XmlOptions::get().set( "OPEN_MODE", Option( WindowServer::SINGLE_WINDOW, "define how new files should be opened when running in server mode" ) );
  XmlOptions::get().set( "ORIENTATION", Option( MainWindow::TOP_BOTTOM, "default orientation for splitters. either \"left/right\" or \"top/bottom\"" ) );
  XmlOptions::get().set( "DIFF_ORIENTATION", Option( MainWindow::LEFT_RIGHT, "default orientation for splitters. either \"left/right\" or \"top/bottom\"" ) );
  
  #ifdef WITH_ASPELL
  XmlOptions::get().set( "DICTIONARY", Option( "en" , "default dictionary"  ) );
  XmlOptions::get().set( "DICTIONARY_FILTER", Option( "none" , "default filter"  ) );
  XmlOptions::get().set( "AUTOSPELL_COLOR", Option( "red", "highlight color for autospell" ) );
  XmlOptions::get().set( "AUTOSPELL_FONT_FORMAT", Option( "0", "highlight font format for autospell" ) );
  XmlOptions::get().set( "MAX_SUGGESTIONS", Option( "0", "maximum number of suggestions. 0 means all." ) );
  #endif
  
  XmlOptions::get().set( "NAVIGATION_SIDEBAR", "1" );
  XmlOptions::get().set( "NAVIGATION_SIDEBAR_LOCATION", "left" );
  
  // toolbars default configuration
  XmlOptions::get().set( "NAVIGATION_SIDEBAR_ICON_SIZE", "16" , "text label in tool buttons" ); 
  XmlOptions::get().set( "NAVIGATION_SIDEBAR_TEXT_POSITION", "2" , "text label in tool buttons" ); 

  // run-time non recordable options
  // default value for autosave directory
  XmlOptions::get().setRaw( "AUTOSAVE_PATH", Util::tmp() );

  XmlOptions::get().set( "USER_NAME", Option( Util::user(), Option::NONE ) );
  XmlOptions::get().set( "APP_NAME", Option( "QEDIT", Option::NONE ) );
  XmlOptions::get().set( "HELP_FILE", Option( File( ".qedit_help").addPath(Util::home()), Option::NONE ) );
  XmlOptions::get().set( "DB_FILE", Option( File(".qedit_db").addPath(Util::home()), Option::NONE ) );
  XmlOptions::get().set( "RC_FILE", Option(  File(".qeditrc").addPath(Util::home()), Option::NONE ) );
  XmlOptions::get().setAutoDefault( false );

};
