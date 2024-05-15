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

#include "TextHighlight.h"
#include "Debug.h"
#include "HighlightBlockData.h"
#include "HighlightPattern.h"
#include "TextParenthesis.h"

#include <QTextDocument>

#include <numeric>

//_________________________________________________________
TextHighlight::TextHighlight( QTextDocument* document ):
    QSyntaxHighlighter( document ),
    Counter( QStringLiteral("TextHighlight") )
{
    textSelectionHighlightPattern_.setType( HighlightPattern::Type::KeywordPattern );
}

//_______________________________________________________
void TextHighlight::setParenthesis( const TextParenthesis::List& parenthesis )
{
    Debug::Throw( QStringLiteral("TextHighlight::setParenthesis.\n") );
    parenthesis_ = parenthesis;
}

//_________________________________________________________
void TextHighlight::highlightBlock( const QString& text )
{
    // check if syntax highlighting is enabled
    bool highlightEnabled( isHighlightEnabled()  && !patterns_.empty() );
    #if WITH_ASPELL
    highlightEnabled |= spellParser_.isEnabled();
    #endif

    // retrieve activeId from last block state
    int activeId( previousBlockState() );
    PatternLocationSet locations;

    // try retrieve HighlightBlockData
    bool needUpdate( true );

    // try retrieve block data
    auto data = dynamic_cast<HighlightBlockData*>( currentBlockUserData() );

    if( data )
    {
        // see if block needs update
        needUpdate =
            data->hasFlag( TextBlock::BlockModified ) ||
            (highlightEnabled && (locations = data->locations()).activeId().first != activeId );
    } else {
        // try retrieve data from parent type
        auto textData = static_cast<TextBlockData*>( currentBlockUserData() );
        data = textData ? new HighlightBlockData( textData ) : new HighlightBlockData;
        setCurrentBlockUserData( data );
    }
    
    // highlight patterns
    if( highlightEnabled && needUpdate )
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
        if( std::accumulate( blockDelimiters_.begin(), blockDelimiters_.end(), false,
            [this, data, &text]( bool value, const BlockDelimiter& delimiter )
            { return _updateDelimiter( data, delimiter, text ) ? true:std::move(value); } ) )
        { emit needSegmentUpdate(); }
    }

    // before try applying the found locations see if automatic spellcheck is on
    #if WITH_ASPELL
    if( spellParser_.isEnabled() )
    {

        // clear locations
        locations = _spellCheckLocationSet( text, data );
        data->setLocations( PatternLocationSet() );
        setCurrentBlockState( -1 );

    }
    #endif

    // text selection
    if( textSelectionHighlightPattern_.isValid() )
    {
        bool active = false;
        textSelectionHighlightPattern_.processText( locations, text, active );        
    }
    
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
    if( spellParser_.isEnabled() ) return _spellCheckLocationSet( text );
    else
    #endif

    if( isHighlightEnabled()  && !patterns_.empty() ) return _highlightLocationSet( text, activeId );
    else return PatternLocationSet();

}

//_________________________________________________________
void TextHighlight::setTextSelectionHighlightColor( const QColor& color )
{
    HighlightStyle style( QStringLiteral("textselection_style") );
    style.setBackgroundColor( color );
    textSelectionHighlightPattern_.setStyle( style ); 
}

//_________________________________________________________
bool TextHighlight::updateTextSelection(const TextSelection& textSelection )
{
    
    const bool changed = 
        (textSelection_.hasFlag( TextSelection::HighlightAll ) != textSelection.hasFlag( TextSelection::HighlightAll ) ) ||
        (textSelection_.hasFlag( TextSelection::CaseSensitive ) != textSelection.hasFlag( TextSelection::CaseSensitive ) ) ||
        (textSelection_.hasFlag( TextSelection::EntireWord ) != textSelection.hasFlag( TextSelection::EntireWord ) ) ||
        (textSelection_.hasFlag( TextSelection::RegExp ) != textSelection.hasFlag( TextSelection::RegExp ) ) ||
        (textSelection_.text() != textSelection.text());

    // check if changed
    if( !changed ) return false; 

    // if highlight all has not changed and is false, also do nothing
    if( !( textSelection_.hasFlag( TextSelection::HighlightAll ) || textSelection.hasFlag( TextSelection::HighlightAll ) ) )
    {
        textSelection_ = textSelection;
        return false;
    }
    
    // update stored selection
    textSelection_ = textSelection;
    
    // update text selection highlight pattern
    if( !textSelection.hasFlag( TextSelection::HighlightAll ) || textSelection.text().isEmpty() ) 
    { 
        textSelectionHighlightPattern_.setKeyword( QRegularExpression() );
    } else {    
        textSelectionHighlightPattern_.setFlag( HighlightPattern::CaseInsensitive, !textSelection.hasFlag( TextSelection::CaseSensitive ) );
        if( textSelection.hasFlag( TextSelection::RegExp ) ) textSelectionHighlightPattern_.setKeyword( textSelection.text() );
        else {
            auto escaped = QRegularExpression::escape( textSelection.text() );
            if( textSelection.hasFlag( TextSelection::EntireWord )) escaped = QStringLiteral( "\\b" ) + escaped + QStringLiteral( "\\b" );
            textSelectionHighlightPattern_.setKeyword( escaped );
        }
    }
    
    return true;
}

//_________________________________________________________
void TextHighlight::updateSpellPattern()
{ 
    HighlightStyle style( QStringLiteral("spellcheck_style") );
    style.setFontFormat( spellParser_.fontFormat() );
    style.setColor( spellParser_.color() );
    spellPattern_.setStyle( std::move( style ) );
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
        auto patternIter = std::find_if( patterns_.begin(), patterns_.end(), HighlightPattern::SameIdFTor( activeId ) );
        Q_ASSERT( patternIter != patterns_.end() );

        const HighlightPattern &pattern( *patternIter );
        bool active=true;
        pattern.processText( locations, text, active );

        // if not active, break the loop to process the other patterns
        if( active )
        {

            // if still active. look for child patterns
            for( const auto& childPattern:pattern.children() )
            { childPattern.processText( locations, text, active );}

            // remove patterns that overlap with others
            auto iter = locations.begin();
            auto prev = locations.begin();

            // first pattern is skipped because it must be the parent
            // so that prev is incremented once and current is incremented twice
            if( iter != locations.end() ) ++iter;
            if( iter != locations.end() ) ++iter;
            if( prev != locations.end() ) ++prev;
            while( iter != locations.end() )
            {

                // no need to compare prev and iter parent Ids because they are known to be the
                // active parrent
                if( iter->position() < prev->position()+(int)prev->length() )
                {

                    // current iterator overlaps with prev
                    auto current = iter++;
                    locations.erase( current );

                } else {

                    prev = iter++;

                }
            }

            return locations;
        }

    }

    // no active pattern
    // normal processing
    int activePatterns(0);
    for( const auto& pattern:patterns_ )
    {

        // do not reprocess active pattern (if any)
        // sincee it was already done
        if( (int)pattern.id() == activeId ) continue;

        // process pattern, store activity
        bool active = false;

        // here one could check if the pattern appears at least once (by checking return value of processText
        // and loop over children here (in place of main loop) if yes.
        pattern.processText( locations, text, active );
        if( active ) activePatterns |= pattern.id();

    }

    // check number of recorded locations
    if( locations.empty() ) return locations;

    // remove locations that are front and have parents
    while( !locations.empty() && locations.begin()->parentId() ) locations.erase(locations.begin());

    // remove patterns that overlap with others
    auto iter = locations.begin();
    auto prev = locations.begin();
    auto parent = locations.begin();

    if( iter != locations.end() ) ++iter;
    while( iter != locations.end() )
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
                auto current = iter;
                ++iter;

                // remove pattern from active list
                activePatterns &= (~current->id());
                locations.erase( current );
            }

        } else if( iter->position() < parent->position()+(int)parent->length()  ) {

            // no overlap with prev. Check against parent
            if( iter->parentId() == parent->id() )
            {
                prev = iter;
                ++iter;
            } else {

                auto current = iter;
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
    {
        locations.activeId().second = 0;
        auto iter = std::find_if( locations.begin(), locations.end(),
            [&activePatterns]( const PatternLocation& location )
            { return activePatterns & location.id(); } );

        if( iter != locations.end() ) locations.activeId().second = iter->id();
    }

    return locations;

}

//_________________________________________________________
PatternLocationSet TextHighlight::_spellCheckLocationSet( const QString& text, HighlightBlockData* data )
{
    PatternLocationSet locations;

    #if WITH_ASPELL

    // insert highlight
    const SpellCheck::Word::Set& words( spellParser_.parse( text ) );
    for( const auto& word:words )
    { locations.insert( PatternLocation( spellPattern_, word.position(), word.length() ) ); }

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
    for( const auto& location:locations )
    {
        if( patternId != location.id() )
        {
            patternId = location.id();
            currentFormat = location.format();
        }

        QTextCharFormat format( currentFormat );
        QTextCharFormat old( TextHighlight::format( location.position() ) );
        if( old.hasProperty( QTextFormat::BackgroundBrush ) && !format.hasProperty( QTextFormat::BackgroundBrush ) )
        { format.setBackground( old.background() ); }
        
        setFormat( location.position(), location.length(), format );

    }

    return;
}

//_________________________________________________________
bool TextHighlight::_updateDelimiter( HighlightBlockData* data, const BlockDelimiter& delimiter, const QString& text ) const
{

    TextBlock::Delimiter counter;
    auto iter( delimiter.regexp().globalMatch( text ) );
    while( iter.hasNext() )
    {
        const auto match( iter.next() );
        const auto position = match.capturedStart();
        const auto length = match.capturedLength();
        const bool isCommented( data->locations().isCommented( position ) );
        const auto matchedString( text.mid( position, length ) );
        if( matchedString.contains( delimiter.first() ) ) counter.increment( isCommented );
        else if( matchedString.contains( delimiter.second() ) ) counter.decrement( isCommented );
    }

    return data->setDelimiters( delimiter.id(), counter );
}
