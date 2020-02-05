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
    static const QString Patterns( QStringLiteral("patterns") );
    static const QString DocumentClass( QStringLiteral("class") );
    static const QString Pattern( QStringLiteral("pattern") );
    static const QString FirstLinePattern( QStringLiteral("first_line_pattern") );
    static const QString Icon( QStringLiteral("icon") );
    static const QString Options( QStringLiteral("options") );
    static const QString OptionWrap( QStringLiteral("wrap") );
    static const QString OptionEmulateTabs( QStringLiteral("emulate_tabs") );
    static const QString OptionDefault( QStringLiteral("default") );
    static const QString TabSize( QStringLiteral("tab_size") );

    //@}

    //*@name highlight style
    //@{
    static const QString Style( QStringLiteral("style") );
    static const QString Name( QStringLiteral("name") );
    static const QString Format( QStringLiteral("format") );
    static const QString Color( QStringLiteral("color") );
    //@}

    //*@name highlight pattern
    //@{
    static const QString KeywordPattern( QStringLiteral("keyword_pattern") );
    static const QString RangePattern( QStringLiteral("range_pattern") );
    static const QString Parent( QStringLiteral("parent") );
    static const QString Keyword( QStringLiteral("keyword") );
    static const QString Begin( QStringLiteral("begin") );
    static const QString End( QStringLiteral("end") );
    static const QString Comments( QStringLiteral("comments") );

    //* used to tell that lines matching pattern are to be ignored when indenting the text
    static const QString OptionNoIndent( QStringLiteral("no_indent") );

    //* used to tell that Range can span across paragraphs
    static const QString OptionSpan( QStringLiteral("span") );

    //* used to tell that highlight pattern is not case sensitive
    static const QString OptionNoCase( QStringLiteral("no_case") );

    //* used to tell that highlight pattern corresponds to commented text
    static const QString OptionComment( QStringLiteral("comment") );

    //@}

    //*@name indentation pattern
    //@{
    static const QString IndentPattern( QStringLiteral("indent_pattern") );

    static const QString IndentNothing( QStringLiteral("unchanged") );
    static const QString IndentIncrement( QStringLiteral("increment") );
    static const QString IndentDecrement( QStringLiteral("decrement") );
    static const QString IndentDecrementAll( QStringLiteral("decrement all") );

    //* indentation pattern rule
    static const QString Rule( QStringLiteral("rule") );
    static const QString Type( QStringLiteral("type") );
    static const QString Scale( QStringLiteral("scale") );
    static const QString Par( QStringLiteral("par") );
    static const QString RegExp( QStringLiteral("regexp") );

    //* base number of characters to skip before applying the pattern
    static const QString BaseIndentation( QStringLiteral("base_indentation") );
    static const QString Value( QStringLiteral("value") );

    //@}

    //*@name text macro
    //@{
    static const QString Macro( QStringLiteral("macro") );
    static const QString Accelerator( QStringLiteral("accelerator") );
    static const QString Replacement( QStringLiteral("replacement") );

    //* do not split selection into single lines
    static const QString OptionNoSplit( QStringLiteral("no_split") );

    //* macro is a separator to be placed in the menu
    static const QString OptionSeparator( QStringLiteral("separator") );

    //* macro is automatic
    static const QString OptionAutomatic( QStringLiteral("automatic") );
    //@}

    //* text parenthesis
    static const QString Parenthesis( QStringLiteral("parenthesis") );

    //* block delimiters
    static const QString BlockDelimiter( QStringLiteral("block_delimiter") );

};

#endif
