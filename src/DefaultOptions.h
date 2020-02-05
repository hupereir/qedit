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
#include "CppUtil.h"
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
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/bibtex_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/makefile_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/perl_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/shell_patterns.xml", Option::Flag::None) );
    XmlOptions::get().add( "DEFAULT_PATTERN_FILENAME", Option( ":/patterns/xml_patterns.xml", Option::Flag::None) );

    XmlOptions::get().set<bool>( QStringLiteral("CUSTOM_BLOCK_DELIMITERS"), true );

    // diff configuration
    XmlOptions::get().set( "DIFF_ADDED_COLOR", Base::Color( "#acdde5" ) );
    XmlOptions::get().set( "DIFF_CONFLICT_COLOR", Base::Color( "#9addaa" ) );

    XmlOptions::get().set( "PARENTHESIS_COLOR", Base::Color( Qt::red ) );
    XmlOptions::get().set( "TAGGED_BLOCK_COLOR", Base::Color( "#fffdd4" ) );

    XmlOptions::get().set<bool>( QStringLiteral("BACKUP"), false );
    XmlOptions::get().set<int>( QStringLiteral("DB_SIZE"), 30 );

    XmlOptions::get().set<bool>( QStringLiteral("IGNORE_AUTOMATIC_MACROS"), false );
    XmlOptions::get().set<bool>( QStringLiteral("SHOW_BLOCK_DELIMITERS"), true );
    XmlOptions::get().set<bool>( QStringLiteral("TEXT_INDENT"), true );
    XmlOptions::get().set<bool>( QStringLiteral("TEXT_HIGHLIGHT"), true );
    XmlOptions::get().set<bool>( QStringLiteral("TEXT_PARENTHESIS"), true );
    XmlOptions::get().set<bool>( QStringLiteral("WRAP_FROM_CLASS"), true );
    XmlOptions::get().set<bool>( QStringLiteral("EMULATE_TABS_FROM_CLASS"), true );

    XmlOptions::get().set<int>( QStringLiteral("WINDOW_HEIGHT"), 500 );
    XmlOptions::get().set<int>( QStringLiteral("WINDOW_WIDTH"), 700 );
    XmlOptions::get().set<int>( QStringLiteral("SERVER_TIMEOUT_DELAY"), 3000 );

    XmlOptions::get().set<bool>( QStringLiteral("SHOW_SIDE_PANEL_WIDGET"), true );
    XmlOptions::get().set<bool>( QStringLiteral("SHOW_TOOLTIPS"), true );

    // splitters
    XmlOptions::get().set<int>( QStringLiteral("SIDE_PANEL_WIDTH"), 200 );

    // file lists mask and sorting
    XmlOptions::get().set<int>( QStringLiteral("SESSION_FILES_MASK"), 1<<FileRecordModel::FileName );
    XmlOptions::get().set<int>( QStringLiteral("SESSION_FILES_SORT_COLUMN"), FileRecordModel::Time );
    XmlOptions::get().set<int>( QStringLiteral("SESSION_FILES_SORT_ORDER"), Qt::AscendingOrder );

    XmlOptions::get().set<int>( QStringLiteral("RECENT_FILES_MASK"), 1<<FileRecordModel::FileName );
    XmlOptions::get().set<int>( QStringLiteral("RECENT_FILES_SORT_COLUMN"), FileRecordModel::Time );
    XmlOptions::get().set<int>( QStringLiteral("RECENT_FILES_SORT_ORDER"), Qt::AscendingOrder );

    // toolbars
    XmlOptions::get().set<bool>( QStringLiteral("DOCUMENT_CLASS_MANAGER_TOOLBAR"), true );
    XmlOptions::get().set<bool>( QStringLiteral("EDITION_TOOLBAR"), true );
    XmlOptions::get().set<bool>( QStringLiteral("EXTRA_TOOLBAR"), true );
    XmlOptions::get().set<bool>( QStringLiteral("FILE_TOOLBAR"), true );
    XmlOptions::get().set<bool>( QStringLiteral("NAVIGATION_TOOLBAR"), true );
    XmlOptions::get().set<bool>( QStringLiteral("SPLIT_TOOLBAR"), true );

    XmlOptions::get().set<int>( QStringLiteral("HIGHLIGHT_PARAGRAPH"), 1 );

    /* multiple views */
    XmlOptions::get().set<int>( QStringLiteral("OPEN_MODE"), Base::toIntegralType( WindowServer::OpenMode::ActiveWindow ) );
    XmlOptions::get().set<int>( QStringLiteral("ORIENTATION"), Qt::Horizontal );
    XmlOptions::get().set<int>( QStringLiteral("DIFF_ORIENTATION"), Qt::Horizontal );

    #ifdef WITH_ASPELL
    XmlOptions::get().set( "DICTIONARY", "en" );
    XmlOptions::get().set( "DICTIONARY_FILTER", "none" );
    XmlOptions::get().set( "AUTOSPELL_COLOR", "red" );
    XmlOptions::get().set<int>( QStringLiteral("AUTOSPELL_FONT_FORMAT"), 0 );
    XmlOptions::get().set<int>( QStringLiteral("MAX_SUGGESTIONS"), 0 );
    #endif

    XmlOptions::get().set<int>( QStringLiteral("SIDE_PANEL_TOOLBAR_TEXT_POSITION"), Qt::ToolButtonFollowStyle );
    XmlOptions::get().set<int>( QStringLiteral("SIDE_PANEL_TOOLBAR_ICON_SIZE"), 16 );

    XmlOptions::get().setRaw( QStringLiteral("TEXT_ENCODING"), QString( "UTF-8" ) );

    // toolbars default configuration

    // lists
    XmlOptions::get().set<int>( QStringLiteral("FILE_SELECTION_LIST_MASK"), (1<<FileRecordModel::FileName)|(1<<FileRecordModel::Time) );
    XmlOptions::get().set<int>( QStringLiteral("CLOSE_FILES_LIST_MASK"), (1<<FileRecordModel::FileName)|(1<<FileRecordModel::Time) );

    // autosave
    XmlOptions::get().setRaw( QStringLiteral("AUTOSAVE_PATH"), Util::tmp() );
    XmlOptions::get().set<bool>( QStringLiteral("AUTOSAVE"), true );
    XmlOptions::get().set<int>( QStringLiteral("AUTOSAVE_INTERVAL"), 20 );

    // resource file
    XmlOptions::get().set( "OLD_RC_FILE", Option( File(".qeditrc").addPath(Util::home()), Option::Flag::None ) );
    XmlOptions::get().set( "RC_FILE", Option( File("qeditrc").addPath(Util::config()), Option::Flag::None ) );

    XmlOptions::get().setAutoDefault( false );

};

#endif
