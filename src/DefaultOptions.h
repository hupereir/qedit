// $Id$

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA XmlOptions::get().add( Option("mailto: hugo.pereira@free.fr>
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
#include "XmlOptions.h"

//_____________________________________________________
//! default options installer
void installDefaultOptions( void )
{

  // set options default values
  XmlOptions::get().keep( "PIXMAP_PATH" );
  XmlOptions::get().add( Option( "PIXMAP_PATH", ":/pixmaps"));
  XmlOptions::get().add( Option( "ICON_PIXMAP", ":/icon.png" , "application icon"));
  XmlOptions::get().add( Option( "DEFAULT_ICON_PATH", "/usr/share/icons" , "default path to look for icons"));

  XmlOptions::get().add( Option( "USE_PRINT_COMMAND", "0", "use print/edit command" ) );
  XmlOptions::get().add( Option( "PRINT_MODE", "PDF" , "print mode. Can be either PDF/HTML"));  
  XmlOptions::get().add( Option( "PRINT_LINE_SIZE", "0" , "Maximum line width. 0 means any."));  
  
  // input document_class pattern filenames options are special.
  // keep the full list
  XmlOptions::get().keep( "PATTERN_FILENAME" );
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/c_patterns.xml" , "default patterns for c/c++ files"));
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/default_patterns.xml" , "default patterns"));
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/diff_patterns.xml" , "default patterns for diff files"));
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/fortran_patterns.xml" , "default patterns for fortran files")); 
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/html_patterns.xml" , "default patterns for html files"));
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/latex_patterns.xml", "default patterns for latex files"));
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/makefile_patterns.xml", "default patterns for makefile files"));
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/perl_patterns.xml", "default patterns for perl files"));
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/shell_patterns.xml", "default patterns for shell files"));
  XmlOptions::get().add( Option( "PATTERN_FILENAME", ":/patterns/xml_patterns.xml", "default patterns for xml files"));

  // diff configuration
  XmlOptions::get().add( Option( "DIFF_ADDED_COLOR", "#d0eaff" ) );
  XmlOptions::get().add( Option( "DIFF_CONFLICT_COLOR", "#e0ffd9" ) );
  
  XmlOptions::get().add( Option( "PARENTHESIS_COLOR", "red", "color when highlighting matching parenthesis" ) );
  XmlOptions::get().add( Option( "TAGGED_BLOCK_COLOR", "#fffdd4", "color for tagged paragraphs" ) );
  XmlOptions::get().add( Option( "BACKUP", "0", "make backup when saving file" ) );
  XmlOptions::get().add( Option( "DB_SIZE", "30" ) );
  XmlOptions::get().add( Option( "DEBUG_LEVEL", "0" , "debug verbosity level"));

  XmlOptions::get().add( Option( "TEXT_INDENT", "1" ));
  XmlOptions::get().add( Option( "TEXT_HIGHLIGHT", "1" ));
  XmlOptions::get().add( Option( "TEXT_PARENTHESIS", "1" ));
  XmlOptions::get().add( Option( "WRAP_FROM_CLASS", "1", "set to 1 if you wand DocumentClass to enable/disable wrapping" ));
  XmlOptions::get().add( Option( "WINDOW_HEIGHT", "500" ));
  XmlOptions::get().add( Option( "WINDOW_WIDTH", "700" ));
  XmlOptions::get().add( Option( "SERVER_TIMEOUT_DELAY", "3000" , "timeout delay to wait for existing server to answer" ));

  // toolbars
  XmlOptions::get().add( Option( "FILE_TOOLBAR",   "1" , "toolbar visibility" ));
  XmlOptions::get().add( Option( "EDITION_TOOLBAR", "1" , "toolbar visibility" ));
  XmlOptions::get().add( Option( "EXTRA_TOOLBAR", "1" , "toolbar visibility" ));
  XmlOptions::get().add( Option( "SPLIT_TOOLBAR", "1" , "toolbar visibility" ));

  XmlOptions::get().add( Option( "FILE_TOOLBAR_LOCATION",   "top" , "toolbar location" ));
  XmlOptions::get().add( Option( "EDITION_TOOLBAR_LOCATION", "top" , "toolbar location" ));
  XmlOptions::get().add( Option( "EXTRA_TOOLBAR_LOCATION", "top" , "toolbar location" ));
  XmlOptions::get().add( Option( "SPLIT_TOOLBAR_LOCATION",   "top" , "toolbar location" ));
  XmlOptions::get().add( Option( "AUTOSAVE_INTERVAL", "20", "time interval between two automatic save of the file" ) );

  XmlOptions::get().add( Option( "HIGHLIGHT_PARAGRAPH", "1", "active paragraph highlighting" ) );
  XmlOptions::get().add( Option( "HIGHLIGHT_COLOR", "#FFFDD4", "paragraph highlight color" ) );
  
  /* multiple views */
  XmlOptions::get().add( Option( "OPEN_MODE", "open in new window", "default mode to open new file. either \"open in new window\" or \"open in new view\"" ) );
  XmlOptions::get().add( Option( "ORIENTATION", "top/bottom", "default orientation for splitters. either \"left/right\" or \"top/bottom\"" ) );
  
  XmlOptions::get().add( Option( "SHADE_INACTIVE_VIEWS", "1", "shade inactive views" ) );
  XmlOptions::get().add( Option( "INACTIVE_COLOR", "#f8f8f8", "inactive view color" ) );
  
  #ifdef WITH_ASPELL
  XmlOptions::get().add( Option( "DICTIONARY", "en" , "default dictionary"  ));
  XmlOptions::get().add( Option( "DICTIONARY_FILTER", "none" , "default filter"  ));
  XmlOptions::get().add( Option( "AUTOSPELL", "0", "true to set automatic spellcheck on by default" ));
  XmlOptions::get().add( Option( "AUTOSPELL_COLOR", "red", "highlight color for autospell" ));
  XmlOptions::get().add( Option( "AUTOSPELL_FONT_FORMAT", "0", "highlight font format for autospell" ));
  XmlOptions::get().add( Option( "MAX_SUGGESTIONS", "0", "maximum number of suggestions. 0 means all." ));
  #endif

};