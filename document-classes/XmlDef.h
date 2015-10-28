#ifndef XmlDef_h
#define XmlDef_h

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

//* Some Xml definitions
namespace Xml
{

    //*@name document class
    //@{
    static const QString Patterns( "patterns" );
    static const QString DocumentClass( "class" );
    static const QString Pattern( "pattern" );
    static const QString FirstLinePattern( "first_line_pattern" );
    static const QString Icon( "icon" );
    static const QString Options( "options" );
    static const QString OptionWrap( "wrap" );
    static const QString OptionEmulateTabs( "emulate_tabs" );
    static const QString OptionDefault( "default" );
    static const QString TabSize( "tab_size" );

    //@}

    //*@name highlight style
    //@{
    static const QString Style( "style" );
    static const QString Name( "name" );
    static const QString Format( "format" );
    static const QString Color( "color" );
    //@}

    //*@name highlight pattern
    //@{
    static const QString KeywordPattern( "keyword_pattern" );
    static const QString RangePattern( "range_pattern" );
    static const QString Parent( "parent" );
    static const QString Keyword( "keyword" );
    static const QString Begin( "begin" );
    static const QString End( "end" );
    static const QString Comments( "comments" );

    //* used to tell that lines matching pattern are to be ignored when indenting the text
    static const QString OptionNoIndent( "no_indent" );

    //* used to tell that Range can span across paragraphs
    static const QString OptionSpan( "span" );

    //* used to tell that highlight pattern is not case sensitive
    static const QString OptionNoCase( "no_case" );

    //* used to tell that highlight pattern corresponds to commented text
    static const QString OptionComment( "comment" );

    //@}

    //*@name indentation pattern
    //@{
    static const QString IndentPattern( "indent_pattern" );

    static const QString IndentNothing( "unchanged" );
    static const QString IndentIncrement( "increment" );
    static const QString IndentDecrement( "decrement" );
    static const QString IndentDecrementAll( "decrement all" );

    //* indentation pattern rule
    static const QString Rule( "rule" );
    static const QString Type( "type" );
    static const QString Scale( "scale" );
    static const QString Par( "par" );
    static const QString RegExp( "regexp" );

    //* base number of characters to skip before applying the pattern
    static const QString BaseIndentation( "base_indentation" );
    static const QString Value( "value" );

    //@}

    //*@name text macro
    //@{
    static const QString Macro( "macro" );
    static const QString Accelerator( "accelerator" );
    static const QString Replacement( "replacement" );

    //* do not split selection into single lines
    static const QString OptionNoSplit( "no_split" );

    //* macro is a separator to be placed in the menu
    static const QString OptionSeparator( "separator" );

    //* macro is automatic
    static const QString OptionAutomatic( "automatic" );
    //@}

    //* text parenthesis
    static const QString Parenthesis( "parenthesis" );

    //* block delimiters
    static const QString BlockDelimiter( "block_delimiter" );

};

#endif
