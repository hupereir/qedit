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

#include "TextIndent.h"

#include "TextBlockRange.h"
#include "TextEditor.h"

#include <QRegularExpression>
#include <QProgressDialog>

//______________________________________________
TextIndent::TextIndent( TextEditor* editor ):
    QObject( editor ),
    Counter( QStringLiteral("TextIndent") ),
    editor_( editor )
{ Debug::Throw( QStringLiteral("TextIndent::TextIndent.\n") ); }

//______________________________________________
void TextIndent::indent( const QTextBlock &first, const QTextBlock &last )
{

    Debug::Throw( QStringLiteral("TextIndent::indent (multi-block).\n") );
    if( !isEnabled() || patterns_.empty() ) return;

    // store all blocks prior to starting modifications
    QList<QTextBlock> blocks;
    const TextBlockRange range( first, last.next() );
    std::copy( range.begin(), range.end(), std::back_inserter( blocks ) );

    QProgressDialog progress( tr( "Indenting selected paragraphs..." ), tr( "Abort" ), 0, blocks.size(), editor_);
    progress.show();

    // retrieve current cursor
    currentCursor_ = editor_->textCursor();

    // retrieve the first valid block prior to the first
    QTextBlock previousBlock( blocks.front().previous() );
    for( ; previousBlock.isValid() &&  editor_->ignoreBlock( previousBlock ); previousBlock = previousBlock.previous() )
    {}

    // get tabs in previous block
    int previousTabs( previousBlock.isValid() ? _tabCount( previousBlock ):0 );
    int i(0);
    for( const auto& block:blocks )
    {

        // update progress
        progress.setValue(i);
        qApp->processEvents();
        if (progress.wasCanceled()) break;

        int newTabs( previousTabs );
        if( !previousBlock.isValid() ) _decrement( block );
        else {

            const auto iter = std::find_if( patterns_.begin(), patterns_.end(),
                [this, &block]( const IndentPattern& pattern ) { return _acceptPattern( block, pattern ); } );

            if( iter != patterns_.end() )
            {
                const auto& pattern = *iter;
                Debug::Throw() << "TextIndent::indent - accepted pattern: " << pattern.name() << Qt::endl;
                if( pattern.type() == IndentPattern::Type::Increment ) newTabs += pattern.scale();
                else if( pattern.type() == IndentPattern::Type::Decrement ) newTabs -= pattern.scale();
                else if( pattern.type() == IndentPattern::Type::DecrementAll ) newTabs = 0;
            }

            // make sure newTabs is not negative
            newTabs = qMax( newTabs, 0 );
            _decrement( block );
            _increment( block, newTabs );

        }

        if( !editor_->ignoreBlock( block ) )
        {
            previousBlock = block;
            previousTabs = newTabs;
        }

        ++i;

    }

    editor_->setTextCursor( currentCursor_ );
    return;

}

//______________________________________________
void TextIndent::indent( const QTextBlock &block, bool newLine )
{

    if( !isEnabled() || patterns_.empty() ) return;

    // store block and cursor
    currentCursor_ = editor_->textCursor();

    // retrieve previous valid block to
    // determine the base indentation
    auto previousBlock( block.previous() );
    for(; previousBlock.isValid() &&  editor_->ignoreBlock( previousBlock ); previousBlock = previousBlock.previous() )
    {}

    // add base indentation if needed
    if( newLine && baseIndentation() ) _addBaseIndentation( block );

    // _decrement if first paragraph of text
    if( !previousBlock.isValid() ) _decrement( block );
    else {

        // get previous paragraph tabs
        int previousTabs( _tabCount( previousBlock ) );
        int newTabs = previousTabs;

        const auto iter = std::find_if( patterns_.begin(), patterns_.end(),
            [this, &block]( const IndentPattern& pattern ) { return _acceptPattern( block, pattern ); } );

        if( iter != patterns_.end() )
        {
            const auto& pattern( *iter );
            Debug::Throw() << "TextIndent::indent - accepted pattern: " << pattern.name() << Qt::endl;
            if( pattern.type() == IndentPattern::Type::Increment ) newTabs += pattern.scale();
            else if( pattern.type() == IndentPattern::Type::Decrement ) newTabs -= pattern.scale();
            else if( pattern.type() == IndentPattern::Type::DecrementAll ) newTabs = 0;
        }

        // make sure newTabs is not negative
        newTabs = qMax( newTabs, 0 );

        // remove all leading tabs
        _decrement( block );

        // add new tabs
        _increment( block, newTabs );

    }

    // restore cursor
    editor_->setTextCursor( currentCursor_ );
    return;

}

//____________________________________________
bool TextIndent::_acceptPattern( const QTextBlock &block, const IndentPattern& pattern ) const
{

    // retrieve rules associated to pattern
    bool accepted( true );
    int ruleId( 0 );
    for( const auto& rule:pattern.rules() )
    {

        auto copy( block );

        // if working on current paragraph
        if( rule.paragraph() == 0 )
        {

            if( !rule.accept( copy.text() ) )
            {
                Debug::Throw() << "TextIndent::_acceptPattern - [" << pattern.name() << "," << ruleId << "] rejected" << Qt::endl;
                accepted = false;
            }

        } else {

            // decrepent paragraph, skipping ignored lines
            int decrement = 0;
            int trueDecrement = 0;
            do {
                copy = copy.previous();
                decrement--;
                trueDecrement--;
                while( copy.isValid() && editor_->ignoreBlock( copy ) )
                {
                    copy = copy.previous();
                    trueDecrement--;
                }

            } while( copy.isValid() && decrement > rule.paragraph() );

            Debug::Throw() << "TextIndent::_acceptPattern - [" << pattern.name() << "," << ruleId << "]"
                << " decrement: " << decrement << " true: " << trueDecrement
                << Qt::endl;

            // check paragraph and regexp
            // here one could have a flag on the indentation pattern rules
            // to check what is to be done when there is no valid paragraph
            // matching the request. This would allow to enable some patterns
            // that cannot otherwise.
            if( !copy.isValid() || !rule.accept( copy.text() ) )
            {
                Debug::Throw() << "TextIndent::_acceptPattern - [" << pattern.name() << "," << ruleId << "] rejected" << Qt::endl;
                accepted = false;
            }

        }

    }

    return accepted;
}

//____________________________________________
int TextIndent::_tabCount( const QTextBlock& block )
{

    auto text( block.text() );
    int count = 0;

    // skip the characters matching baseIndentation_
    int index = baseIndentation_;

    // loop over next characters to identify tabs
    // both normal and emulated tabs are counted
    while( index < text.size() )
    {
        if( text.midRef( index, editor_->normalTabCharacter().size() ) == editor_->normalTabCharacter() )
        { index += editor_->normalTabCharacter().size(); count++; }

        else if( text.midRef( index, editor_->emulatedTabCharacter().size() ) == editor_->emulatedTabCharacter() )
        { index += editor_->emulatedTabCharacter().size(); count++; }

        else break;
    }

    return count;
}

//____________________________________________
void TextIndent::_addBaseIndentation( const QTextBlock &block )
{

    Q_ASSERT( baseIndentation() );

    // set a cursor at beginning of block
    QTextCursor cursor( block );
    cursor.joinPreviousEditBlock();
    cursor.setPosition( block.position(), QTextCursor::MoveAnchor );

    int position( currentCursor_.position() );
    int anchor( currentCursor_.anchor() );

    // insert tab characters
    cursor.insertText( QString( baseIndentation(), ' ') );
    if( block == currentCursor_.block() )
    {
        currentCursor_.setPosition( anchor + baseIndentation(), QTextCursor::MoveAnchor );
        currentCursor_.setPosition( position + baseIndentation(), QTextCursor::KeepAnchor );
    }

    cursor.endEditBlock();

}

//____________________________________________
void TextIndent::_decrement( const QTextBlock &block )
{

    // set a cursor at beginning of block
    QTextCursor cursor( block );
    cursor.setPosition( block.position() + baseIndentation(), QTextCursor::MoveAnchor );

    // leading space characters regexp
    static QRegularExpression regexp( QStringLiteral("^\\s+") );
    const auto match( regexp.match( block.text() ) );
    if( match.hasMatch() )
    {
        const int position( currentCursor_.position() );
        const int anchor( currentCursor_.anchor() );
        const int length( match.capturedLength() );

        cursor.setPosition( cursor.position() + length, QTextCursor::KeepAnchor );
        cursor.removeSelectedText();
        if( currentCursor_.block() == block && currentCursor_.position() - block.position() > baseIndentation() )
        {
            currentCursor_.setPosition( qMax( 0, anchor - length ), QTextCursor::MoveAnchor );
            currentCursor_.setPosition( qMax( 0, position - length ), QTextCursor::KeepAnchor );
        }
    } else { Debug::Throw() << "TextIndent::_decrement - no match" << Qt::endl; }
}

//____________________________________________
void TextIndent::_increment( const QTextBlock &block, int count )
{

    Debug::Throw() << "TextIndent::_increment - count " << count << Qt::endl;

    // first make sure that the line has at least baseIndentation_ characters
    QTextCursor cursor( block );
    cursor.movePosition( QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );

    if( baseIndentation() && editor_->isEmptyBlock( block ) )
    {
        int position( currentCursor_.position() );
        int anchor( currentCursor_.anchor() );

        cursor.insertText( QString( baseIndentation(), ' ' ) );
        if( block == currentCursor_.block() )
        {
            currentCursor_.setPosition( anchor + baseIndentation(), QTextCursor::MoveAnchor );
            currentCursor_.setPosition( position + baseIndentation(), QTextCursor::KeepAnchor );
        }
    }

    // locate cursor at beginning of the paragraph + baseIndentation
    cursor.joinPreviousEditBlock();
    cursor.setPosition( block.position() + baseIndentation(), QTextCursor::MoveAnchor );
    for( int i=0; i < count; i++ )
    {

        int position( currentCursor_.position() );
        int anchor( currentCursor_.anchor() );

        // insert tab characters
        cursor.insertText( editor_->tabCharacter() );
        if( block == currentCursor_.block() )
        {
            currentCursor_.setPosition( anchor + editor_->tabCharacter().size(), QTextCursor::MoveAnchor );
            currentCursor_.setPosition( position + editor_->tabCharacter().size(), QTextCursor::KeepAnchor );
        }

    }
    cursor.endEditBlock();

}
