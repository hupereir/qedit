#ifndef XmlDef_h
#define XmlDef_h

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

#include <QString>

//! Some Xml definitions
namespace Xml
{

    //!@name document class
    //@{
    static const QString PATTERNS( "patterns" );
    static const QString DOCUMENT_CLASS( "class" );
    static const QString PATTERN( "pattern" );
    static const QString FIRSTLINE_PATTERN( "first_line_pattern" );
    static const QString ICON( "icon" );
    static const QString OPTIONS( "options" );
    static const QString OPTION_WRAP( "wrap" );
    static const QString OPTION_EMULATE_TABS( "emulate_tabs" );
    static const QString OPTION_DEFAULT( "default" );
    static const QString TAB_SIZE( "tab_size" );

    //@}

    //!@name highlight style
    //@{
    static const QString STYLE( "style" );
    static const QString NAME( "name" );
    static const QString FORMAT( "format" );
    static const QString COLOR( "color" );
    //@}

    //!@name highlight pattern
    //@{
    static const QString KeywordPattern( "keyword_pattern" );
    static const QString RangePattern( "range_pattern" );
    static const QString PARENT( "parent" );
    static const QString KEYWORD( "keyword" );
    static const QString BEGIN( "begin" );
    static const QString END( "end" );
    static const QString COMMENTS( "comments" );

    //! used to tell that lines matching pattern are to be ignored when indenting the text
    static const QString OPTION_NO_INDENT( "no_indent" );

    //! used to tell that Range can span across paragraphs
    static const QString OPTION_SPAN( "span" );

    //! used to tell that highlight pattern is not case sensitive
    static const QString OPTION_NO_CASE( "no_case" );

    //! used to tell that highlight pattern corresponds to commented text
    static const QString OPTION_COMMENT( "comment" );

    //@}

    //!@name indentation pattern
    //@{
    static const QString INDENT_PATTERN( "indent_pattern" );

    static const QString INDENT_NOTHING( "unchanged" );
    static const QString INDENT_INCREMENT( "increment" );
    static const QString INDENT_DECREMENT( "decrement" );
    static const QString INDENT_DECREMENT_ALL( "decrement all" );

    //! indentation pattern rule
    static const QString RULE( "rule" );
    static const QString TYPE( "type" );
    static const QString SCALE( "scale" );
    static const QString PAR( "par" );
    static const QString REGEXP( "regexp" );

    //! base number of characters to skip before applying the pattern
    static const QString BASE_INDENTATION( "base_indentation" );
    static const QString VALUE( "value" );

    //@}

    //!@name text macro
    //@{
    static const QString MACRO( "macro" );
    static const QString ACCELERATOR( "accelerator" );
    static const QString REPLACEMENT( "replacement" );

    //! do not split selection into single lines
    static const QString OPTION_NO_SPLIT( "no_split" );

    //! macro is a separator to be placed in the menu
    static const QString OPTION_SEPARATOR( "separator" );

    //! macro is automatic
    static const QString OPTION_AUTOMATIC( "automatic" );
    //@}

    //! text parenthesis
    static const QString PARENTHESIS( "parenthesis" );

    //! block delimiters
    static const QString BLOCK_DELIMITER( "block_delimiter" );

};

#endif
