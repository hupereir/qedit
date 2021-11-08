#ifndef HighlightBlockData_h
#define HighlightBlockData_h

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

#include "HighlightBlockFlags.h"
#include "PatternLocationSet.h"
#include "TextBlockData.h"
#include "TextBlockDelimiter.h"
#include "Word.h"

#if WITH_ASPELL
#endif

//* TextBlock data for syntax highlighting
class HighlightBlockData: public TextBlockData
{

    public:

    //* constructor
    explicit HighlightBlockData();

    //* constructor
    explicit HighlightBlockData( const TextBlockData& reference ):
        TextBlockData( reference )
    {}

    //* constructor
    explicit HighlightBlockData( const TextBlockData* pointer ):
        TextBlockData( *pointer )
    {}

    //* syntax highlighting pattern locations
    const PatternLocationSet& locations() const
    { return locations_; }

    //* syntax highlighting pattern locations
    void setLocations( const PatternLocationSet& locations )
    { locations_ = locations; }

    //* return true if locations correspond to a commented block
    bool ignoreBlock() const
    { return (!locations().empty()) && locations().begin()->hasFlag( HighlightPattern::NoIndent ); }

    //*@name parenthesis
    //@{

    bool hasParenthesis() const
    { return parenthesis_ != -1; }

    //* highlighted parenthesis
    int parenthesis() const
    { return parenthesis_; }

    //* highlighted parenthesis
    int parenthesisLength() const
    { return parenthesisLength_; }

    //* set parenthesis
    void setParenthesis( int value, int length )
    {
        parenthesis_ = value;
        parenthesisLength_ = length;
    }

    //* clear parenthesis
    void clearParenthesis()
    {
        parenthesis_ = -1;
        parenthesisLength_= 0;
    }

    //@}

    //*@name block limits
    //@{

    //* delimiters
    const TextBlock::Delimiter::List& delimiters() const
    { return delimiters_; }

    //* delimiters
    bool setDelimiters( int id, const TextBlock::Delimiter& delimiter )
    { return delimiters_.set( id, delimiter ); }

    //@}

    #if WITH_ASPELL
    //*@name spelling
    //@{
    //* set of misspelled words
    const SpellCheck::Word::Set& misspelledWords() const
    { return words_; }

    //* set of misspelled words
    void setMisspelledWords( const SpellCheck::Word::Set& words )
    { words_ = words; }

    //* return misspelled word matching position, if any
    SpellCheck::Word misspelledWord( int ) const;
    //@}
    #endif

    private:

    //* locations and ids of matching syntax highlighting patterns
    PatternLocationSet locations_;

    //* highlighted parenthesis location
    /** local with respect to the block */
    int parenthesis_ = -1;

    //* parenthesis length
    int parenthesisLength_ = 0;

    //* block delimiters
    TextBlock::Delimiter::List delimiters_;

    //@}

    #if WITH_ASPELL
    //* set of misspelled words and position in associated block
    SpellCheck::Word::Set words_;
    #endif

};

#endif
