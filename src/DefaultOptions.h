#ifndef DefaultOptions_h
#define DefaultOptions_h

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

#include "Config.h"
#include "FileRecordModel.h"
#include "FileSystemModel.h"
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

    XmlOptions::get().set( "ICON_PIXMAP", Option( ":/icon.png" , "application icon") );
    XmlOptions::get().set( "FIXED_FONT_NAME", Option( "fixed" , "fixed font"  ) );

    // user specified document classes
    XmlOptions::get().keep( "PATTERN_FILENAME" );

    // built-in document classes
    XmlOptions::get().keep( "DEFAULT_PATTERN_FILENAME" );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/c_patterns.xml" , "default patterns for c/c++ files", Option::None ) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/alice_patterns.xml" , "Alice patterns for c/c++ files", Option::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/default_patterns.xml" , "default patterns", Option::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/diff_patterns.xml" , "default patterns for diff files", Option::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/fortran_patterns.xml" , "default patterns for fortran files", Option::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/html_patterns.xml" , "default patterns for html files", Option::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/latex_patterns.xml", "default patterns for latex files", Option::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/makefile_patterns.xml", "default patterns for makefile files", Option::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/perl_patterns.xml", "default patterns for perl files", Option::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/shell_patterns.xml", "default patterns for shell files", Option::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/xml_patterns.xml", "default patterns for xml files", Option::None) );

    XmlOptions::get().set( "CUSTOM_BLOCK_DELIMITERS", "1" );

    // diff configuration
    XmlOptions::get().set( "DIFF_ADDED_COLOR", "#d0eaff" );
    XmlOptions::get().set( "DIFF_CONFLICT_COLOR", "#ecffec" );

    XmlOptions::get().set( "PARENTHESIS_COLOR", "red" );
    XmlOptions::get().set( "TAGGED_BLOCK_COLOR", "#fffdd4" );
    XmlOptions::get().set( "BACKUP", "0" );
    XmlOptions::get().set( "DB_SIZE", "30" );

    XmlOptions::get().set( "IGNORE_AUTOMATIC_MACROS", "0" );
    XmlOptions::get().set( "SHOW_HIDDEN_FILES", "1" );
    XmlOptions::get().set( "SHOW_BLOCK_DELIMITERS", "1" );
    XmlOptions::get().set( "TEXT_INDENT", "1" );
    XmlOptions::get().set( "TEXT_HIGHLIGHT", "1" );
    XmlOptions::get().set( "TEXT_PARENTHESIS", "1" );
    XmlOptions::get().set( "WRAP_FROM_CLASS", "1" );
    XmlOptions::get().set( "EMULATE_TABS_FROM_CLASS", "1" );
    XmlOptions::get().set( "WINDOW_HEIGHT", "500" );
    XmlOptions::get().set( "WINDOW_WIDTH", "700" );
    XmlOptions::get().set( "SERVER_TIMEOUT_DELAY", "3000" );
    XmlOptions::get().set( "SHOW_NAVIGATION_FRAME", "1" );
    XmlOptions::get().set( "SHOW_NAVIGATOR", "0" );

    // splitters
    XmlOptions::get().set( "NAVIGATION_FRAME_WIDTH", "200" );

    // file lists mask and sorting
    XmlOptions::get().set<unsigned int>( "SESSION_FILES_MASK", 1<<FileRecordModel::FILE );
    XmlOptions::get().set<unsigned int>( "RECENT_FILES_MASK", 1<<FileRecordModel::FILE );

    XmlOptions::get().set<unsigned int>( "FILE_SYSTEM_LIST_MASK", 1<<FileSystemModel::FILE );
    XmlOptions::get().set<unsigned int>( "FILE_SYSTEM_LIST_SORT_COLUMN", FileSystemModel::TIME );

    // toolbars
    XmlOptions::get().set( "DOCUMENT_CLASS_MANAGER_TOOLBAR", "1" );
    XmlOptions::get().set( "EDITION_TOOLBAR", "1" );
    XmlOptions::get().set( "EXTRA_TOOLBAR", "1" );
    XmlOptions::get().set( "FILE_TOOLBAR", "1" );
    XmlOptions::get().set( "NAVIGATION_TOOLBAR", "1" );
    XmlOptions::get().set( "SPLIT_TOOLBAR", "1" );

    XmlOptions::get().set( "AUTOSAVE", "1" );
    XmlOptions::get().set( "AUTOSAVE_INTERVAL", "20" );

    XmlOptions::get().set( "HIGHLIGHT_PARAGRAPH", "1" );
    XmlOptions::get().set( "HIGHLIGHT_COLOR", "#FFFDD4" );

    /* multiple views */
    XmlOptions::get().set( "OPEN_MODE", WindowServer::SINGLE_WINDOW );
    XmlOptions::get().set( "ORIENTATION", MainWindow::TOP_BOTTOM );
    XmlOptions::get().set( "DIFF_ORIENTATION", MainWindow::LEFT_RIGHT );

    #ifdef WITH_ASPELL
    XmlOptions::get().set( "DICTIONARY", "en" );
    XmlOptions::get().set( "DICTIONARY_FILTER", "none" );
    XmlOptions::get().set( "AUTOSPELL_COLOR", "red" );
    XmlOptions::get().set( "AUTOSPELL_FONT_FORMAT", "0" );
    XmlOptions::get().set( "MAX_SUGGESTIONS", "0" );
    #endif

    XmlOptions::get().set( "NAVIGATION_SIDEBAR", "1" );
    XmlOptions::get().set( "NAVIGATION_SIDEBAR_LOCATION", "left" );

    // toolbars default configuration
    XmlOptions::get().set( "NAVIGATION_SIDEBAR_ICON_SIZE", "16" );
    XmlOptions::get().set( "NAVIGATION_SIDEBAR_TEXT_POSITION", "2" );

    // run-time non recordable options
    // default value for autosave directory
    XmlOptions::get().setRaw( "AUTOSAVE_PATH", Util::tmp() );

    XmlOptions::get().set( "USER_NAME", Option( Util::user(), Option::None ) );
    XmlOptions::get().set( "APP_NAME", Option( "QEDIT", Option::None ) );
    XmlOptions::get().set( "HELP_FILE", Option( File( ".qedit_help").addPath(Util::home()), Option::None ) );
    XmlOptions::get().set( "DB_FILE", Option( File(".qedit_db").addPath(Util::home()), Option::None ) );
    XmlOptions::get().set( "RC_FILE", Option( File(".qeditrc").addPath(Util::home()), Option::None ) );
    XmlOptions::get().setAutoDefault( false );

};

#endif
