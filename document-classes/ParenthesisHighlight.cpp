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

#include "ParenthesisHighlight.h"
#include "TextBlockRange.h"
#include "TextEditor.h"
#include "HighlightBlockData.h"

#include <QTextDocument>

//_______________________________________________________________________
ParenthesisHighlight::ParenthesisHighlight( TextEditor* parent ):
    QObject( parent ),
    Counter( "ParenthesisHighlight" ),
    parent_( parent )
{ Debug::Throw( "ParenthesisHighlight::ParenthesisHighlight.\n" ); }

//______________________________________________________________________
QList<QTextBlock> ParenthesisHighlight::clear()
{

    QList<QTextBlock> dirty;
    if( cleared_ ) return dirty;

    // loop over all blocks
    for( const auto& block:TextBlockRange( parent_->document() ) )
    {

        // retrieve block data
        auto data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
        if( !( data && data->hasParenthesis() ) ) continue;

        // do not clear current block if parenthesis is unchanged
        if(
            block.contains( location_ ) &&
            data->hasParenthesis() &&
            data->parenthesis() + block.position() == location_ &&
            isEnabled() ) continue;

        // clear parenthesis
        data->clearParenthesis();
        parent_->document()->markContentsDirty(block.position(), block.length()-1);
        dirty.append( block );
    }

    cleared_ = true;
    return dirty;

}

//______________________________________________________________________
void ParenthesisHighlight::synchronize( const ParenthesisHighlight& highlight )
{
    Debug::Throw( "ParenthesisHighlight::synchronized.\n" );
    enabled_ = highlight.enabled_;
    cleared_ = highlight.cleared_;
    location_ = highlight.location_;
    length_ = highlight.length_;
}

//______________________________________________________________________
void ParenthesisHighlight::highlight( int location, int length )
{

    if( !isEnabled() ) return;

    clear();
    location_ = location;
    length_ = length;
    _highlight();
}

//______________________________________________________________________
void ParenthesisHighlight::_highlight()
{

    // retrieve block matching location
    QTextBlock block( parent_->document()->findBlock( location_ ) );
    if( !block.isValid() ) return;

    HighlightBlockData* data = dynamic_cast<HighlightBlockData*>( block.userData() );
    if( !data )
    {
        // try retrieve data from parent type
        auto textData = dynamic_cast<TextBlockData*>( block.userData() );
        data = textData ? new HighlightBlockData( textData ) : new HighlightBlockData;
        block.setUserData( data );

    } else if( data->hasParenthesis() && data->parenthesis() + block.position() == location_ ) return;

    // update parenthesis
    data->setParenthesis( location_ - block.position(), length_ );
    parent_->document()->markContentsDirty( location_, length_ );
    cleared_ = false;

    // reset location
    location_ = -1;
}
