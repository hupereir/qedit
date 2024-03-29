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

#include "CollapsedBlockData.h"
#include "HighlightBlockData.h"

#include <numeric>

//_____________________________________________________________
CollapsedBlockData::CollapsedBlockData( const QTextBlock& block ):
text_( block.text() )
{

    // would need to retrieve the "children" data from the block format rather that from the HighlightBlockData
    QTextBlockFormat format( block.blockFormat() );
    collapsed_ = format.boolProperty( TextBlock::Collapsed );
    if( collapsed_ && format.hasProperty( TextBlock::CollapsedData ) )
    {
        delimiters_ = format.property( TextBlock::CollapsedData ).value<CollapsedBlockData>().delimiters();
        setChildren( format.property( TextBlock::CollapsedData ).value<CollapsedBlockData>().children() );
    }

}

//_____________________________________________________________
int CollapsedBlockData::blockCount() const
{

    return std::accumulate( children_.begin(), children_.end(), 1,
        []( int out, const CollapsedBlockData& child )
        { return std::move(out) + child.blockCount(); } );
}

//_____________________________________________________________
QString CollapsedBlockData::toPlainText() const
{
    return std::accumulate( children_.begin(), children_.end(), text()+"\n",
        []( QString text, const CollapsedBlockData& child )
        { return std::move(text) + child.toPlainText(); } );
}
