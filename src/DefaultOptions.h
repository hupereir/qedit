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
#include "Color.h"
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

    XmlOptions::get().set( "ICON_PIXMAP", Option( ":/qedit.png" , "application icon") );
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

    XmlOptions::get().set<bool>( "CUSTOM_BLOCK_DELIMITERS", true );

    // diff configuration
    XmlOptions::get().set<BASE::Color>( "DIFF_ADDED_COLOR", QColor( "#d0eaff" ) );
    XmlOptions::get().set<BASE::Color>( "DIFF_CONFLICT_COLOR", QColor( "#ecffec" ) );

    XmlOptions::get().set<BASE::Color>( "PARENTHESIS_COLOR", QColor( Qt::red ) );
    XmlOptions::get().set<BASE::Color>( "TAGGED_BLOCK_COLOR", QColor( "#fffdd4" ) );

    XmlOptions::get().set<bool>( "BACKUP", false );
    XmlOptions::get().set<int>( "DB_SIZE", 30 );

    XmlOptions::get().set<bool>( "IGNORE_AUTOMATIC_MACROS", false );
    XmlOptions::get().set<bool>( "SHOW_HIDDEN_FILES", true );
    XmlOptions::get().set<bool>( "SHOW_BLOCK_DELIMITERS", true );
    XmlOptions::get().set<bool>( "TEXT_INDENT", true );
    XmlOptions::get().set<bool>( "TEXT_HIGHLIGHT", true );
    XmlOptions::get().set<bool>( "TEXT_PARENTHESIS", true );
    XmlOptions::get().set<bool>( "WRAP_FROM_CLASS", true );
    XmlOptions::get().set<bool>( "EMULATE_TABS_FROM_CLASS", true );

    XmlOptions::get().set<int>( "WINDOW_HEIGHT", 500 );
    XmlOptions::get().set<int>( "WINDOW_WIDTH", 700 );
    XmlOptions::get().set<int>( "SERVER_TIMEOUT_DELAY", 3000 );

    XmlOptions::get().set<bool>( "SHOW_NAVIGATION_FRAME", true );
    XmlOptions::get().set<bool>( "SHOW_NAVIGATOR", false );
    XmlOptions::get().set<bool>( "SHOW_TOOLTIPS", true );

    // splitters
    XmlOptions::get().set<int>( "NAVIGATION_FRAME_WIDTH", 200 );

    // file lists mask and sorting
    XmlOptions::get().set<int>( "SESSION_FILES_MASK", 1<<FileRecordModel::FILE );
    XmlOptions::get().set<int>( "RECENT_FILES_MASK", 1<<FileRecordModel::FILE );

    XmlOptions::get().set<int>( "FILE_SYSTEM_LIST_MASK", 1<<FileSystemModel::FILE );
    XmlOptions::get().set<int>( "FILE_SYSTEM_LIST_SORT_COLUMN", FileSystemModel::TIME );

    // toolbars
    XmlOptions::get().set<bool>( "DOCUMENT_CLASS_MANAGER_TOOLBAR", true );
    XmlOptions::get().set<bool>( "EDITION_TOOLBAR", true );
    XmlOptions::get().set<bool>( "EXTRA_TOOLBAR", true );
    XmlOptions::get().set<bool>( "FILE_TOOLBAR", true );
    XmlOptions::get().set<bool>( "NAVIGATION_TOOLBAR", true );
    XmlOptions::get().set<bool>( "SPLIT_TOOLBAR", true );

    XmlOptions::get().set<int>( "HIGHLIGHT_PARAGRAPH", 1 );
    XmlOptions::get().set<BASE::Color>( "HIGHLIGHT_COLOR", QColor( "#FFFDD4" ) );

    /* multiple views */
    XmlOptions::get().set<int>( "OPEN_MODE", WindowServer::ACTIVE_WINDOW );
    XmlOptions::get().set<int>( "ORIENTATION", Qt::Horizontal );
    XmlOptions::get().set<int>( "DIFF_ORIENTATION", Qt::Horizontal );

    #ifdef WITH_ASPELL
    XmlOptions::get().set( "DICTIONARY", "en" );
    XmlOptions::get().set( "DICTIONARY_FILTER", "none" );
    XmlOptions::get().set( "AUTOSPELL_COLOR", "red" );
    XmlOptions::get().set<int>( "AUTOSPELL_FONT_FORMAT", 0 );
    XmlOptions::get().set<int>( "MAX_SUGGESTIONS", 0 );
    #endif

    XmlOptions::get().set<bool>( "NAVIGATION_SIDEBAR", true );
    XmlOptions::get().set<int>( "NAVIGATION_SIDEBAR_LOCATION", Qt::LeftToolBarArea );

    // toolbars default configuration
    XmlOptions::get().set<int>( "NAVIGATION_SIDEBAR_ICON_SIZE", 16 );
    XmlOptions::get().set<int>( "NAVIGATION_SIDEBAR_TEXT_POSITION", 2 );

    // lists
    XmlOptions::get().set<int>( "FILE_SELECTION_LIST_MASK", (1<<FileRecordModel::FILE)|(1<<FileRecordModel::TIME) );
    XmlOptions::get().set<int>( "CLOSE_FILES_LIST_MASK", (1<<FileRecordModel::FILE)|(1<<FileRecordModel::TIME) );

    // autosave
    XmlOptions::get().setRaw( "AUTOSAVE_PATH", Util::tmp() );
    XmlOptions::get().set<bool>( "AUTOSAVE", true );
    XmlOptions::get().set<int>( "AUTOSAVE_INTERVAL", 20 );

    // run-time non recordable options
    XmlOptions::get().set( "USER_NAME", Option( Util::user(), Option::None ) );
    XmlOptions::get().set( "APP_NAME", Option( "QEDIT", Option::None ) );
    XmlOptions::get().set( "RC_FILE", Option( File(".qeditrc").addPath(Util::home()), Option::None ) );

    XmlOptions::get().setAutoDefault( false );

};

#endif
