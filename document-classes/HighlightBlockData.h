#ifndef HighlightBlockData_h
#define HighlightBlockData_h

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
* Place, Suite 330, Boston, MA 02111-1307 USA
*
*
*******************************************************************************/

#include "CollapsedBlockData.h"
#include "HighlightBlockFlags.h"
#include "TextBlockData.h"
#include "PatternLocationSet.h"

#include "Config.h"

#if WITH_ASPELL
#include "Word.h"
#endif

//! TextBlock data for syntax highlighting
class HighlightBlockData: public TextBlockData
{

    public:

    //! constructor
    HighlightBlockData();

    //! constructor
    HighlightBlockData( const TextBlockData& reference ):
        TextBlockData( reference ),
        parenthesis_( -1 ),
        parenthesisLength_(0)
    {}

    //! constructor
    HighlightBlockData( const TextBlockData* pointer ):
        TextBlockData( *pointer ),
        parenthesis_( -1 ),
        parenthesisLength_(0)
    {}

    //! destructor
    virtual ~HighlightBlockData( void )
    {}

    //! syntax highlighting pattern locations
    const PatternLocationSet& locations( void ) const
    { return locations_; }

    //! syntax highlighting pattern locations
    void setLocations( const PatternLocationSet& locations )
    { locations_ = locations; }

    //! return true if locations correspond to a commented block
    bool ignoreBlock( void ) const
    { return (!locations().empty()) && locations().begin()->hasFlag( HighlightPattern::NO_INDENT ); }

    //!@name parenthesis
    //@{

    bool hasParenthesis( void ) const
    { return parenthesis_ != -1; }

    //! highlighted parenthesis
    const int& parenthesis( void ) const
    { return parenthesis_; }

    //! highlighted parenthesis
    const int& parenthesisLength( void ) const
    { return parenthesisLength_; }

    //! set parenthesis
    void setParenthesis( const int& value, const int& length )
    {
        parenthesis_ = value;
        parenthesisLength_ = length;
    }

    //! clear parenthesis
    void clearParenthesis( void )
    {
        parenthesis_ = -1;
        parenthesisLength_= 0;
    }

    //@}

    //!@name block limits
    //@{

    //! delimiters
    const TextBlock::Delimiter::List& delimiters( void ) const
    { return delimiters_; }

    //! delimiters
    bool setDelimiter( int id, TextBlock::Delimiter delimiter )
    { return delimiters_.set( id, delimiter ); }

    //@}

    #if WITH_ASPELL
    //!@name spelling
    //@{
    //! set of misspelled words
    const SPELLCHECK::Word::Set& misspelledWords( void ) const
    { return words_; }

    //! set of misspelled words
    void setMisspelledWords( const SPELLCHECK::Word::Set& words )
    { words_ = words; }

    //! return misspelled word matching position, if any
    SPELLCHECK::Word misspelledWord( const int& position ) const;
    //@}
    #endif

    private:

    //! locations and ids of matching syntax highlighting patterns
    PatternLocationSet locations_;

    //! highlighted parenthesis location
    /*! local with respect to the block */
    int parenthesis_;

    //! parenthesis length
    int parenthesisLength_;

    //! block delimiters
    TextBlock::Delimiter::List delimiters_;

    //@}

    #if WITH_ASPELL
    //! set of misspelled words and position in associated block
    SPELLCHECK::Word::Set words_;
    #endif

};

#endif
