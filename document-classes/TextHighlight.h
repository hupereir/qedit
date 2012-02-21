#ifndef _TextHighlight_h_
#define _TextHighlight_h_

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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "BlockDelimiter.h"
#include "Config.h"
#include "Counter.h"
#include "Debug.h"
#include "HighlightPattern.h"
#include "HighlightBlockFlags.h"
#include "Key.h"
#include "TextParenthesis.h"

#if WITH_ASPELL
#include "SpellParser.h"
#endif

#include <QtGui/QSyntaxHighlighter>

class HighlightPattern;
class HighlightBlockData;

//! syntax highlighting based on text patterns
class TextHighlight: public QSyntaxHighlighter, public Counter
{

    //! Qt meta object
    Q_OBJECT

        public:

        //! constructor
        TextHighlight( QTextDocument* );

    //! destructor
    virtual ~TextHighlight( void )
    {}

    //! highlight paragraph
    virtual void highlightBlock( const QString& text );

    //! retrieve highlight location for given text
    PatternLocationSet locationSet( const QString& text, const int& active_id );

    //!@name highlight patterns
    //@{

    //! enable highlight. Returns true if changed
    bool setHighlightEnabled( const bool& state )
    {
        if( highlightEnabled_ == state ) return false;
        highlightEnabled_ = state;
        return true;
    }

    //! true if enabled
    const bool& isHighlightEnabled( void ) const
    { return highlightEnabled_; }

    //! patterns
    const HighlightPattern::List& patterns( void ) const
    { return patterns_; }

    //! patterns
    void setPatterns( const HighlightPattern::List& patterns )
    { patterns_ = patterns; }

    //@}

    //!@name parenthesis
    //@{

    //! parenthesis enabled
    const bool& isParenthesisEnabled( void ) const
    { return parenthesis_enabled_; }

    //! parenthesis enabled
    bool setParenthesisEnabled( const bool& state )
    {
        if( parenthesis_enabled_ == state ) return false;
        parenthesis_enabled_ = state;
        return true;
    }

    //! parenthesis highlight color
    void setParenthesisHighlightColor( const QColor& color )
    { parenthesisHighlightFormat_.setBackground(color); }

    //! parenthesis highlight color
    QColor parenthesisHighlightColor( void )
    { return parenthesisHighlightFormat_.background().color(); }

    //! parenthesis
    const TextParenthesis::List& parenthesis( void ) const
    { return parenthesis_; }

    //! set parenthesis
    void setParenthesis( const TextParenthesis::List& );

    //@}

    //!@name block delimiters
    //@{

    //! block delimiters enabled
    const bool& isBlockDelimitersEnabled( void ) const
    { return blockDelimitersEnabled_; }

    //! block delimiters enabled
    bool setBlockDelimitersEnabled( const bool& state )
    {
        if( blockDelimitersEnabled_ == state ) return false;
        blockDelimitersEnabled_ = state;
        return true;
    }

    //! block delimiters
    void setBlockDelimiters( const BlockDelimiter::List& delimiters )
    { blockDelimiters_ = delimiters; }

    //! block delimiters
    const BlockDelimiter::List& blockDelimiters( void ) const
    { return blockDelimiters_; }

    //@}

    //! patterns
    void clear( void )
    {
        Debug::Throw( "TextHighlight.clear.\n" );
        patterns_.clear();
    }

    #if WITH_ASPELL

    //! embedded spellcheck parser
    SPELLCHECK::SpellParser& spellParser( void )
    { return spellParser_; }

    //! highlight pattern associated to auto-spell
    const HighlightPattern& spellPattern( void ) const
    { return spellPattern_; }

    //! update highlight pattern associated to auto-spell
    void updateSpellPattern( void )
    { spellPattern_.setStyle( HighlightStyle( "spellcheck_style", spellParser().fontFormat(), spellParser().color() ) ); }

    #endif

    signals:

    //! emmited when block delimiters have changed
    void needSegmentUpdate( void );

    private:

    //!@name syntax highlighting
    //@{

    //! retrieve highlight location for given text
    PatternLocationSet _highlightLocationSet( const QString& text, const int& active_id ) const;

    //! retrieve highlight location for given text
    PatternLocationSet _spellCheckLocationSet( const QString& text, HighlightBlockData* data = 0 );

    //! apply locations to current block
    void _applyPatterns( const PatternLocationSet& locations );

    //! calculate delimiter object
    TextBlock::Delimiter _delimiter( const BlockDelimiter&, const QString& text ) const;

    //! true if highlight is enabled
    bool highlightEnabled_;

    //! list of highlight patterns
    HighlightPattern::List patterns_;

    //@}

    //!@name text parenthesis
    //@{

    //! parenthesis enabled
    bool parenthesis_enabled_;

    //! text parenthesis
    TextParenthesis::List parenthesis_;

    //! parenthesis highlight format
    QTextCharFormat parenthesisHighlightFormat_;

    //@}

    //!@name block delimiters
    //@{

    //! enabled
    bool blockDelimitersEnabled_;

    //! block delimiters
    BlockDelimiter::List blockDelimiters_;

    //@}

    //!@name spell checking
    //@{

    #if WITH_ASPELL

    //! spell check parser
    SPELLCHECK::SpellParser spellParser_;

    //! spellcheck highlight pattern
    HighlightPattern spellPattern_;

    #endif

    //@}

};

#endif
