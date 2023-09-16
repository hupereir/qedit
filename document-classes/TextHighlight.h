#ifndef TextHighlight_h
#define TextHighlight_h

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

#include "BlockDelimiter.h"
#include "Counter.h"
#include "Debug.h"
#include "HighlightBlockFlags.h"
#include "HighlightPattern.h"
#include "TextParenthesis.h"
#include "TextSelection.h"

#if WITH_ASPELL
#include "SpellParser.h"
#endif

#include <QSyntaxHighlighter>

class HighlightPattern;
class HighlightBlockData;

//* syntax highlighting based on text patterns
class TextHighlight: public QSyntaxHighlighter, private Base::Counter<TextHighlight>
{

    //* Qt meta object
    Q_OBJECT

    public:

    //* constructor
    explicit TextHighlight( QTextDocument* );

    //* highlight paragraph
    void highlightBlock( const QString& ) override;

    //* retrieve highlight location for given text
    PatternLocationSet locationSet( const QString& text, int activeId );

    //*@name highlight patterns
    //@{

    //* enable highlight. Returns true if changed
    bool setHighlightEnabled( bool state )
    {
        if( highlightEnabled_ == state ) return false;
        highlightEnabled_ = state;
        return true;
    }

    //* true if enabled
    bool isHighlightEnabled() const
    { return highlightEnabled_; }

    //* patterns
    const HighlightPattern::List& patterns() const
    { return patterns_; }

    //* patterns
    void setPatterns( const HighlightPattern::List& patterns )
    { patterns_ = patterns; }

    //@}

    //*@name parenthesis
    //@{

    //* parenthesis enabled
    bool isParenthesisEnabled() const
    { return parenthesisEnabled_; }

    //* parenthesis enabled
    bool setParenthesisEnabled( bool state )
    {
        if( parenthesisEnabled_ == state ) return false;
        parenthesisEnabled_ = state;
        return true;
    }

    //* parenthesis highlight color
    void setParenthesisHighlightColor( const QColor& color )
    { parenthesisHighlightFormat_.setBackground(color); }

    //* parenthesis highlight color
    QColor parenthesisHighlightColor()
    { return parenthesisHighlightFormat_.background().color(); }

    //* parenthesis
    const TextParenthesis::List& parenthesis() const
    { return parenthesis_; }

    //* set parenthesis
    void setParenthesis( const TextParenthesis::List& );

    //@}

    //*@name block delimiters
    //@{

    //* block delimiters enabled
    bool isBlockDelimitersEnabled() const
    { return blockDelimitersEnabled_; }

    //* block delimiters enabled
    bool setBlockDelimitersEnabled( bool state )
    {
        if( blockDelimitersEnabled_ == state ) return false;
        blockDelimitersEnabled_ = state;
        return true;
    }

    //* block delimiters
    void setBlockDelimiters( const BlockDelimiter::List& delimiters )
    { blockDelimiters_ = delimiters; }

    //* block delimiters
    const BlockDelimiter::List& blockDelimiters() const
    { return blockDelimiters_; }

    //@}

    //*@name text selection highlight
    //@{
    //* highghlight color
    void setTextSelectionHighlightColor( const QColor& color )
    { textSelectionHighlightFormat_.setBackground( color ); }
    
    //* highghlight color
    QColor textSelectionHighlightColor() const
    { return textSelectionHighlightFormat_.background().color(); }
    
    //* find text selection
    void findAll( const TextSelection& textSelection )
    { textSelection_ = textSelection; }
    
    //@}
    
    //* patterns
    void clear()
    {
        Debug::Throw( QStringLiteral("TextHighlight.clear.\n") );
        patterns_.clear();
    }

    #if WITH_ASPELL

    //* embedded spellcheck parser
    const SpellCheck::SpellParser& spellParser() const
    { return spellParser_; }

    //* embedded spellcheck parser
    SpellCheck::SpellParser& spellParser()
    { return spellParser_; }

    //* highlight pattern associated to auto-spell
    const HighlightPattern& spellPattern() const
    { return spellPattern_; }

    //* update highlight pattern associated to auto-spell
    void updateSpellPattern()
    { spellPattern_.setStyle( HighlightStyle( QStringLiteral("spellcheck_style"), spellParser_.fontFormat(), spellParser_.color() ) ); }

    #endif

    Q_SIGNALS:

    //* emitted when block delimiters have changed
    void needSegmentUpdate();

    private:

    //*@name syntax highlighting
    //@{

    //* retrieve highlight location for given text
    PatternLocationSet _highlightLocationSet( const QString&, int activeId ) const;

    //* retrieve highlight location for given text
    PatternLocationSet _spellCheckLocationSet( const QString& text, HighlightBlockData* data = 0 );

    //* retrieve text selection location sets 
    PatternLocationSet _textSelectionLocationSet( const QString& text );
    
    //* apply locations to current block
    void _applyPatterns( const PatternLocationSet& locations );

    //* calculate delimiter object
    bool _updateDelimiter( HighlightBlockData*, const BlockDelimiter&, const QString& ) const;

    //* true if highlight is enabled
    bool highlightEnabled_ = false;

    //* list of highlight patterns
    HighlightPattern::List patterns_;

    //* text selection highlight pattern
    QTextCharFormat textSelectionHighlightFormat_;
    
    //* current text selection
    TextSelection textSelection_;
    
    //@}

    //*@name text parenthesis
    //@{

    //* parenthesis enabled
    bool parenthesisEnabled_ = false;

    //* text parenthesis
    TextParenthesis::List parenthesis_;

    //* parenthesis highlight format
    QTextCharFormat parenthesisHighlightFormat_;

    //@}

    //*@name block delimiters
    //@{

    //* enabled
    bool blockDelimitersEnabled_ = true;

    //* block delimiters
    BlockDelimiter::List blockDelimiters_;

    //@}

    #if WITH_ASPELL
    //*@name spell checking
    //@{

    //* spell check parser
    SpellCheck::SpellParser spellParser_;

    //* spellcheck highlight pattern
    HighlightPattern spellPattern_;
    //@}

    #endif


};

#endif
