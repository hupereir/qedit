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

#include "HighlightBlockData.h"

#include <algorithm>

//____________________________________________________________
HighlightBlockData::HighlightBlockData( void ):
    TextBlockData(),
    parenthesis_( -1 ),
    parenthesisLength_(0)
{}

#if USE_ASPELL

//____________________________________________________________
SpellCheck::Word HighlightBlockData::misspelledWord( int position ) const
{

    Debug::Throw( "HighlightBlockData::misspelledWord.\n" );
    auto&& iter = std::find_if( words_.begin(), words_.end(), SpellCheck::Word::AtPositionFTor( position ) );
    return (iter == words_.end()) ? SpellCheck::Word():*iter;

}

#endif
