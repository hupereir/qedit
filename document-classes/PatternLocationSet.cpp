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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*******************************************************************************/

#include "PatternLocationSet.h"

//______________________________________________________________
bool PatternLocationSet::isCommented( int position ) const
{

    if( empty() ) return false;
    // if( size() == 1 && begin()->hasFlag( HighlightPattern::NO_INDENT ) ) return true;
    const const_iterator iter( std::find_if( constBegin(), constEnd(), PatternLocation::ContainsFTor( position ) ) );
    if( iter == constEnd() ) return false;
    else return iter->hasFlag( HighlightPattern::COMMENT );
}
