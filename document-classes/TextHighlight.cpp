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

#include "Debug.h"
#include "HighlightPattern.h"
#include "HighlightBlockData.h"
#include "TextParenthesis.h"
#include "TextHighlight.h"
#include "TextHighlight.moc"

#include <QTextDocument>

//_________________________________________________________
TextHighlight::TextHighlight( QTextDocument* document ):
    QSyntaxHighlighter( document ),
    Counter( "TextHighlight" ),
    highlightEnabled_( false ),
    parenthesisEnabled_( false ),
    blockDelimitersEnabled_( true )
{ Debug::Throw( "TextHighlight::TextHighlight.\n" ); }

//_______________________________________________________
void TextHighlight::setParenthesis( const TextParenthesis::List& parenthesis )
{

    Debug::Throw( "TextHighlight::setParenthesis.\n" );
    parenthesis_ = parenthesis;

}

//_________________________________________________________
void TextHighlight::highlightBlock( const QString& text )
{

    // check if syntax highlighting is enabled
    bool highlightEnabled( isHighlightEnabled()  && !patterns_.empty() );
    #if WITH_ASPELL
    highlightEnabled |= spellParser().isEnabled();
    #endif

    // retrieve activeId from last block state
    int activeId( previousBlockState() );
    PatternLocationSet locations;

    // try retrieve HighlightBlockData
    bool needUpdate( true );

    // try retrieve block data
    HighlightBlockData* data = dynamic_cast<HighlightBlockData*>( currentBlockUserData() );

    if( data )
    {

        // see if block needs update
        needUpdate =
            data->hasFlag( TextBlock::BlockModified ) ||
            (highlightEnabled && (locations = data->locations()).activeId().first != activeId );

    } else {

        // try retrieve data from parent type
        TextBlockData* textData = static_cast<TextBlockData*>( currentBlockUserData() );
        data = textData ? new HighlightBlockData( textData ) : new HighlightBlockData();
        setCurrentBlockUserData( data );

    }

    // highlight patterns
    if( needUpdate && highlightEnabled )
    {

        // get new set of highlight locations
        locations = _highlightLocationSet( text, activeId );

        // update data modification state and highlight pattern locations
        data->setFlag( TextBlock::BlockModified, false );
        data->setLocations( locations );

        // store active id
        /* this is disabled when current block is collapsed */
        if( !data->hasFlag( TextBlock::BlockCollapsed ) ) setCurrentBlockState( locations.activeId().second );
        else setCurrentBlockState( 0 );

    }

    // block delimiters parsing
    if( isBlockDelimitersEnabled() && needUpdate )
    {
        bool segmentChanged( false );
        for( BlockDelimiter::List::const_iterator iter = blockDelimiters_.begin(); iter != blockDelimiters_.end(); ++iter )
        { segmentChanged |= _updateDelimiter( data, *iter, text ); }

        if( segmentChanged ) emit needSegmentUpdate();
    }

    // before try applying the found locations see if automatic spellcheck is on
    #if WITH_ASPELL
    if( spellParser().isEnabled() )
    {

        // clear locations
        locations = _spellCheckLocationSet( text, data );
        data->setLocations( PatternLocationSet() );
        setCurrentBlockState( -1 );

    }
    #endif

    // apply new location set
    if( !locations.empty() ) _applyPatterns( locations );

    // check if parenthesis need highlight
    if( isParenthesisEnabled() && data && data->hasParenthesis() )
    {
        QTextCharFormat old( TextHighlight::format( data->parenthesis() ) );
        old.setBackground( parenthesisHighlightFormat_.background() );
        setFormat( data->parenthesis(), data->parenthesisLength(), parenthesisHighlightFormat_ );
    }

    return;

}

//_________________________________________________________
PatternLocationSet TextHighlight::locationSet( const QString& text, int activeId )
{

    #if WITH_ASPELL
    if( spellParser().isEnabled() ) return _spellCheckLocationSet( text );
    else
    #endif

    if( isHighlightEnabled()  && !patterns_.empty() ) return _highlightLocationSet( text, activeId );
    else return PatternLocationSet();

}

//_________________________________________________________
PatternLocationSet TextHighlight::_highlightLocationSet( const QString& text, int activeId ) const
{

    // location list
    PatternLocationSet locations;
    locations.activeId().first = activeId;
    locations.activeId().second = activeId;

    // check if pattern activeId is still active
    if( activeId > 0 )
    {

        // look for matching pattern in list
        HighlightPattern::List::const_iterator patternIter = std::find_if( patterns_.begin(), patterns_.end(), HighlightPattern::SameIdFTor( activeId ) );
        Q_ASSERT( patternIter != patterns_.end() );

        const HighlightPattern &pattern( *patternIter );
        bool active=true;
        pattern.processText( locations, text, active );

        // if not active, break the loop to process the other patterns
        if( active )
        {

            // if still active. look for child patterns
            for( HighlightPattern::List::const_iterator childIter = pattern.children().begin(); childIter != pattern.children().end(); ++childIter )
            { childIter->processText( locations, text, active );}

            // remove patterns that overlap with others
            PatternLocationSet::iterator iter = locations.begin();
            PatternLocationSet::iterator prev = locations.begin();

            // first pattern is skipped because it must be the parent
            // so that prev is incremented once and current is incremented twice
            if( iter != locations.end() ) ++iter;
            if( iter != locations.end() ) ++iter;
            if( prev != locations.end() ) prev++;
            while(  iter != locations.end() )
            {

                // no need to compare prev and iter parent Ids because they are known to be the
                // active parrent
                if( iter->position() < prev->position()+(int)prev->length() )
                {

                    // current iterator overlaps with prev
                    PatternLocationSet::iterator current = iter;
                    ++iter;
                    locations.erase( current );

                } else {

                    prev = iter;
                    ++iter;

                }
            }

            return locations;
        }

    }

    // no active pattern
    // normal processing
    unsigned int active_patterns(0);
    for( HighlightPattern::List::const_iterator iter = patterns_.begin(); iter != patterns_.end(); ++iter )
    {

        const HighlightPattern &pattern( *iter );

        // do not reprocess active pattern (if any)
        // sincee it was already done
        if( (int)pattern.id() == activeId ) continue;

        // process pattern, store activity
        bool active = false;

        // here one could check if the pattern appears at least once (by checking return value of processText
        // and loop over children here (in place of main loop) if yes.
        pattern.processText( locations, text, active );
        if( active ) active_patterns |= pattern.id();

    }

    // check number of recorded locations
    if( locations.empty() ) return locations;

    // remove locations that are front and have parents
    while( locations.size() && locations.begin()->parentId() ) locations.erase(locations.begin());

    // remove patterns that overlap with others
    PatternLocationSet::iterator iter = locations.begin();
    PatternLocationSet::iterator prev = locations.begin();
    PatternLocationSet::iterator parent = locations.begin();

    if( iter != locations.end() ) ++iter;
    while(  iter != locations.end() )
    {

        // check if patterns overlap
        if( iter->position() < prev->position()+(int)prev->length() )
        {

            // check if iterator has parent that match
            if( iter->parentId() == prev->id() )
            {

                prev = iter;
                ++iter;

            } else {

                // remove current pattern
                PatternLocationSet::iterator current = iter;
                ++iter;

                // remove pattern from active list
                active_patterns &= (~current->id());
                locations.erase( current );
            }

            // no overlap with prev. Check against parent
        } else if( iter->position() < parent->position()+(int)parent->length()  ) {

            if( iter->parentId() == parent->id() )
            {
                prev = iter;
                ++iter;
            } else {

                PatternLocationSet::iterator current = iter;
                ++iter;

                // remove pattern from active list
                locations.erase( current );

            }

        } else {

            // no increment. Advance one.
            parent = iter;
            prev = iter;
            ++iter;

        }
    }

    // check activity
    // one loop over the remaining locations
    // stop at the first one that is found in the list of possibly active
    locations.activeId().second = 0;
    for( PatternLocationSet::iterator iter = locations.begin(); iter != locations.end(); ++iter )
    {
        if( active_patterns & iter->id() )
        {
            locations.activeId().second = iter->id();
            break;
        }
    }

    return locations;

}

//_________________________________________________________
PatternLocationSet TextHighlight::_spellCheckLocationSet( const QString& text, HighlightBlockData* data )
{

    PatternLocationSet locations;

    #if WITH_ASPELL

    // insert highlight
    const SpellCheck::Word::Set& words( spellParser().parse( text ) );
    for( SpellCheck::Word::Set::const_iterator iter = words.begin(); iter != words.end(); ++iter )
    { locations.insert( PatternLocation( spellPattern(), iter->position(), iter->size() ) ); }

    // store misspelled words
    if( data ) data->setMisspelledWords( words );

    #endif

    return locations;

}

//_________________________________________________________
void TextHighlight::_applyPatterns( const PatternLocationSet& locations )
{

    // initialize style
    int patternId(-1);
    QTextCharFormat currentFormat;
    foreach( const PatternLocation& location, locations )
    {
        if( patternId != location.id() )
        {
            patternId = location.id();
            currentFormat = location.format();
        }

        QTextCharFormat format( currentFormat );
        QTextCharFormat old( TextHighlight::format( location.position() ) );
        if( old.hasProperty( QTextFormat::BackgroundBrush ) ) format.setBackground( old.background() );
        setFormat( location.position(), location.length(), format );

    }

    return;
}

//_________________________________________________________
bool TextHighlight::_updateDelimiter( HighlightBlockData* data, const BlockDelimiter& delimiter, const QString& text ) const
{

    TextBlock::Delimiter counter;
    int position = 0;
    while( (position = delimiter.regexp().indexIn( text, position ) ) >= 0 )
    {

        const bool isCommented( data->locations().isCommented( position ) );
        const QString matchedString( text.mid( position, delimiter.regexp().matchedLength() ) );
        if( matchedString.contains( delimiter.first() ) ) counter.increment( isCommented );
        else if( matchedString.contains( delimiter.second() ) ) counter.decrement( isCommented );

        position++;

    }

    return data->setDelimiters( delimiter.id(), counter );
}
