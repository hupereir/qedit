#ifndef TextIndent_h
#define TextIndent_h

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

/**
\file TextIndent.h
\brief text indentation
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <QObject>
#include <QTextBlock>
#include <QTextCursor>

#include "Counter.h"
#include "Debug.h"
#include "IndentPattern.h"

class TextEditor;

//* syntax highlighting based on text patterns
class TextIndent: public QObject, private Base::Counter<TextIndent>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit TextIndent( TextEditor* );

    //* enabled
    void setEnabled( bool state )
    { enabled_ = state; }

    //* enabled
    bool isEnabled( void ) const
    { return enabled_ && !patterns_.empty(); }

    //* base indentation
    int baseIndentation( void ) const
    { return baseIndentation_; }

    //* base indentation
    void setBaseIndentation( int value )
    { baseIndentation_ = value; }

    //* patterns
    const IndentPattern::List& patterns( void ) const
    { return patterns_; }

    //* patterns
    void setPatterns( const IndentPattern::List& patterns )
    {
        Debug::Throw( "TextIndent::SetPatterns.\n" );
        patterns_ = patterns;
    }

    //* patterns
    void clear( void )
    {
        Debug::Throw( "TextIndent::clear.\n" );
        patterns_.clear();
    }

    public Q_SLOTS:

    //* highlight blocks
    virtual void indent( QTextBlock first, QTextBlock last );

    //* highlight block
    /**
    newLine argument is used in case there is a default base indentation,
    to properly indent paragraphs when return key is pressed
    */
    virtual void indent( QTextBlock block, bool newLine = false );

    private:

    //* returns true if pattern match current paragraph
    bool _acceptPattern( QTextBlock block, const IndentPattern& pattern ) const;

    //* return number of tabs in given paragraph
    int _tabCount( const QTextBlock& block );

    //* add base indentation
    void _addBaseIndentation( QTextBlock block );

    //* decrement paragraph
    //* \brief try remove leading tabs up to n
    void _decrement( QTextBlock block );

    //* increment paragraph with n tabs
    void _increment( QTextBlock block, int count = 1 );

    //* enabled
    bool enabled_ = false;

    //* destination editor
    TextEditor* editor_ = nullptr;

    //* current cursor
    QTextCursor currentCursor_;

    //* base indentation
    /** this is the number of space characters to add prior to any text indentation */
    int baseIndentation_ = 0;

    //* list of highlight patterns
    IndentPattern::List patterns_;

};

#endif
