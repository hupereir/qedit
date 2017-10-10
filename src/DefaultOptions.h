#ifndef DefaultOptions_h
#define DefaultOptions_h

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

#include "Color.h"
#include "FileRecordModel.h"
#include "FileSystemModel.h"
#include "MainWindow.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlOptions.h"

//_____________________________________________________
//* default options installer
void installDefaultOptions()
{

    // set options default values
    XmlOptions::get().setAutoDefault( true );
    XmlOptions::get().keep( "PIXMAP_PATH" );
    XmlOptions::get().add( "PIXMAP_PATH", "/usr/share/icons/oxygen/32x32/mimetypes");

    // font
    XmlOptions::get().set( "FIXED_FONT_NAME", Option( "fixed" ) );

    // user specified document classes
    XmlOptions::get().keep( "PATTERN_FILENAME" );

    // built-in document classes
    XmlOptions::get().keep( "DEFAULT_PATTERN_FILENAME" );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/c_patterns.xml" , Option::Flag::None ) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/alice_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/default_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/diff_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/fortran_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/html_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/latex_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/makefile_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/perl_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/shell_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/xml_patterns.xml", Option::Flag::None) );

    XmlOptions::get().set<bool>( "CUSTOM_BLOCK_DELIMITERS", true );

    // diff configuration
    XmlOptions::get().set( "DIFF_ADDED_COLOR", Base::Color( "#acdde5" ) );
    XmlOptions::get().set( "DIFF_CONFLICT_COLOR", Base::Color( "#9addaa" ) );

    XmlOptions::get().set( "PARENTHESIS_COLOR", Base::Color( Qt::red ) );
    XmlOptions::get().set( "TAGGED_BLOCK_COLOR", Base::Color( "#fffdd4" ) );

    XmlOptions::get().set<bool>( "BACKUP", false );
    XmlOptions::get().set<int>( "DB_SIZE", 30 );

    XmlOptions::get().set<bool>( "IGNORE_AUTOMATIC_MACROS", false );
    XmlOptions::get().set<bool>( "SHOW_BLOCK_DELIMITERS", true );
    XmlOptions::get().set<bool>( "TEXT_INDENT", true );
    XmlOptions::get().set<bool>( "TEXT_HIGHLIGHT", true );
    XmlOptions::get().set<bool>( "TEXT_PARENTHESIS", true );
    XmlOptions::get().set<bool>( "WRAP_FROM_CLASS", true );
    XmlOptions::get().set<bool>( "EMULATE_TABS_FROM_CLASS", true );

    XmlOptions::get().set<int>( "WINDOW_HEIGHT", 500 );
    XmlOptions::get().set<int>( "WINDOW_WIDTH", 700 );
    XmlOptions::get().set<int>( "SERVER_TIMEOUT_DELAY", 3000 );

    XmlOptions::get().set<bool>( "SHOW_SIDE_PANEL_WIDGET", true );
    XmlOptions::get().set<bool>( "SHOW_TOOLTIPS", true );

    // splitters
    XmlOptions::get().set<int>( "SIDE_PANEL_WIDTH", 200 );

    // file lists mask and sorting
    XmlOptions::get().set<int>( "SESSION_FILES_MASK", 1<<FileRecordModel::Filename );
    XmlOptions::get().set<int>( "SESSION_FILES_SORT_COLUMN", FileRecordModel::Time );
    XmlOptions::get().set<int>( "SESSION_FILES_SORT_ORDER", Qt::AscendingOrder );

    XmlOptions::get().set<int>( "RECENT_FILES_MASK", 1<<FileRecordModel::Filename );
    XmlOptions::get().set<int>( "RECENT_FILES_SORT_COLUMN", FileRecordModel::Time );
    XmlOptions::get().set<int>( "RECENT_FILES_SORT_ORDER", Qt::AscendingOrder );

    // toolbars
    XmlOptions::get().set<bool>( "DOCUMENT_CLASS_MANAGER_TOOLBAR", true );
    XmlOptions::get().set<bool>( "EDITION_TOOLBAR", true );
    XmlOptions::get().set<bool>( "EXTRA_TOOLBAR", true );
    XmlOptions::get().set<bool>( "FILE_TOOLBAR", true );
    XmlOptions::get().set<bool>( "NAVIGATION_TOOLBAR", true );
    XmlOptions::get().set<bool>( "SPLIT_TOOLBAR", true );

    XmlOptions::get().set<int>( "HIGHLIGHT_PARAGRAPH", 1 );

    /* multiple views */
    XmlOptions::get().set<int>( "OPEN_MODE", WindowServer::ActiveWindow );
    XmlOptions::get().set<int>( "ORIENTATION", Qt::Horizontal );
    XmlOptions::get().set<int>( "DIFF_ORIENTATION", Qt::Horizontal );

    #ifdef WITH_ASPELL
    XmlOptions::get().set( "DICTIONARY", "en" );
    XmlOptions::get().set( "DICTIONARY_FILTER", "none" );
    XmlOptions::get().set( "AUTOSPELL_COLOR", "red" );
    XmlOptions::get().set<int>( "AUTOSPELL_FONT_FORMAT", 0 );
    XmlOptions::get().set<int>( "MAX_SUGGESTIONS", 0 );
    #endif

    XmlOptions::get().set<bool>( "SIDE_PANEL_TOOLBAR", true );
    XmlOptions::get().set<int>( "SIDE_PANEL_TOOLBAR_LOCATION", Qt::LeftToolBarArea );
    XmlOptions::get().set<int>( "SIDE_PANEL_TOOLBAR_ICON_SIZE", 16 );
    XmlOptions::get().set<int>( "SIDE_PANEL_TOOLBAR_TEXT_POSITION", 2 );

    XmlOptions::get().setRaw( "TEXT_ENCODING", QString( "UTF-8" ) );

    // toolbars default configuration

    // lists
    XmlOptions::get().set<int>( "FILE_SELECTION_LIST_MASK", (1<<FileRecordModel::Filename)|(1<<FileRecordModel::Time) );
    XmlOptions::get().set<int>( "CLOSE_FILES_LIST_MASK", (1<<FileRecordModel::Filename)|(1<<FileRecordModel::Time) );

    // autosave
    XmlOptions::get().setRaw( "AUTOSAVE_PATH", Util::tmp() );
    XmlOptions::get().set<bool>( "AUTOSAVE", true );
    XmlOptions::get().set<int>( "AUTOSAVE_INTERVAL", 20 );

    // resource file
    XmlOptions::get().set( "OLD_RC_FILE", Option( File(".qeditrc").addPath(Util::home()), Option::Flag::None ) );
    XmlOptions::get().set( "RC_FILE", Option( File("qeditrc").addPath(Util::config()), Option::Flag::None ) );

    XmlOptions::get().setAutoDefault( false );

};

#endif
