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

#include "PatternLocation.h"

//_____________________________________________________
PatternLocation::PatternLocation():
    Counter( QStringLiteral("PatternLocation") )
{}

//_____________________________________________________
PatternLocation::PatternLocation( const HighlightPattern& parent, int position, int length ):
    Counter( QStringLiteral("PatternLocation") ),
    id_( parent.id() ),
    parentId_( parent.parentId() ),
    flags_( parent.flags() ),
    format_( parent.style().fontFormat() ),
    color_( parent.style().color() ),
    backgroundColor_( parent.style().backgroundColor() ),
    position_( position ),
    length_( length )
{}

//_____________________________________________________
QTextCharFormat PatternLocation::format() const
{

    QTextCharFormat out;
    out.setFontWeight( (format_&TextFormat::Bold) ? QFont::Bold : QFont::Normal );
    out.setFontItalic( format_&TextFormat::Italic );
    out.setFontUnderline( format_&TextFormat::Underline );
    out.setFontOverline( format_&TextFormat::Overline );
    out.setFontStrikeOut( format_&TextFormat::Strike );
    if( color_.isValid() ) out.setForeground( color_ );
    if( backgroundColor_.isValid() ) out.setBackground( backgroundColor_ );

    return out;
}
