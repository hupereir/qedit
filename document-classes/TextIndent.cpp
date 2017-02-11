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

/*!
\file TextIndent.cpp
\brief text indentation
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "TextEditor.h"
#include "TextIndent.h"

#include <QRegExp>
#include <QProgressDialog>

//______________________________________________
TextIndent::TextIndent( TextEditor* editor ):
    QObject( editor ),
    Counter( "TextIndent" ),
    editor_( editor )
{ Debug::Throw( "TextIndent::TextIndent.\n" ); }

//______________________________________________
void TextIndent::indent( QTextBlock first, QTextBlock last )
{

    Debug::Throw( "TextIndent::indent (multi-block).\n" );
    if( !isEnabled() || patterns_.empty() ) return;

    // store all blocks prior to starting modifications
    QList<QTextBlock> blocks;
    for( QTextBlock block( first ); block.isValid() && block != last; block = block.next() )
    { blocks << block; }
    blocks << last;

    QProgressDialog progress( tr( "Indenting selected paragraphs..." ), tr( "Abort" ), 0, blocks.size(), editor_);
    progress.show();

    // retrieve current cursor
    currentCursor_ = editor_->textCursor();

    // retrieve the first valid block prior to the first
    QTextBlock previousBlock( blocks.front().previous() );
    while( previousBlock.isValid() &&  editor_->ignoreBlock( previousBlock ) )
    { previousBlock = previousBlock.previous(); }

    // get tabs in previous block
    int previous_tabs( previousBlock.isValid() ? _tabCount( previousBlock ):0 );
    int i(0);
    for( const auto& block:blocks )
    {

        // update progress
        progress.setValue(i);
        qApp->processEvents();
        if (progress.wasCanceled()) break;

        int newTabs( previous_tabs );
        if( !previousBlock.isValid() ) _decrement( block );
        else {

            for( const auto& pattern:patterns_ )
            {

                if( _acceptPattern( block, pattern ) )
                {
                    Debug::Throw() << "TextIndent::indent - accepted pattern: " << pattern.name() << endl;
                    if( pattern.type() == IndentPattern::Increment ) newTabs += pattern.scale();
                    else if( pattern.type() == IndentPattern::Decrement ) newTabs -= pattern.scale();
                    else if( pattern.type() == IndentPattern::DecrementAll ) newTabs = 0;
                    break;
                }
            }

            // make sure newTabs is not negative
            newTabs = qMax( newTabs, 0 );
            _decrement( block );
            _increment( block, newTabs );

        }

        if( !editor_->ignoreBlock( block ) )
        {
            previousBlock = block;
            previous_tabs = newTabs;
        }

        ++i;

    }

    editor_->setTextCursor( currentCursor_ );
    return;

}

//______________________________________________
void TextIndent::indent( QTextBlock block, bool newLine )
{

    if( !isEnabled() || patterns_.empty() ) return;

    // ignore "empty" blocks
    // if( editor_->isEmptyBlock( block ) ) return;

    // store block and cursor
    currentCursor_ = editor_->textCursor();

    // retrieve previous valid block to
    // determine the base indentation
    QTextBlock previousBlock( block.previous() );
    while( previousBlock.isValid() &&  editor_->ignoreBlock( previousBlock ) )
    { previousBlock = previousBlock.previous(); }

    // add base indentation if needed
    if( newLine && baseIndentation() ) _addBaseIndentation( block );

    // _decrement if first paragraph of text
    if( !previousBlock.isValid() ) _decrement( block );
    else {

        // get previous paragraph tabs
        int previous_tabs( _tabCount( previousBlock ) );
        int newTabs = previous_tabs;
        for( IndentPattern::List::iterator iter = patterns_.begin(); iter != patterns_.end(); ++iter )
        {
            if( _acceptPattern( block, *iter ) )
            {

                Debug::Throw() << "TextIndent::indent - accepted pattern: " << iter->name() << endl;
                if( iter->type() == IndentPattern::Increment ) newTabs += iter->scale();
                else if( iter->type() == IndentPattern::Decrement ) newTabs -= iter->scale();
                else if( iter->type() == IndentPattern::DecrementAll ) newTabs = 0;
                break;
            }
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
bool TextIndent::_acceptPattern( QTextBlock block, const IndentPattern& pattern ) const
{

    // retrieve rules associated to pattern
    bool accepted( true );
    const IndentPattern::Rule::List& rules( pattern.rules() );
    int rule_id( 0 );
    for( IndentPattern::Rule::List::const_iterator iter = rules.begin(); iter != rules.end() && accepted; ++iter, rule_id++ )
    {

        QTextBlock local( block );

        // if working on current paragraph
        if( iter->paragraph() == 0 )
        {

            if( !iter->accept( local.text() ) )
            {
                Debug::Throw() << "TextIndent::_acceptPattern - [" << pattern.name() << "," << rule_id << "] rejected" << endl;
                accepted = false;
            }

        } else {

            // decrepent paragraph, skipping ignored lines
            int decrement = 0;
            int true_decrement = 0;
            do {
                local = local.previous();
                decrement--;
                true_decrement--;
                while( local.isValid() && editor_->ignoreBlock( local ) )
                {
                    local = local.previous();
                    true_decrement--;
                }

            } while( local.isValid() && decrement > iter->paragraph() );

            Debug::Throw() << "TextIndent::_acceptPattern - [" << pattern.name() << "," << rule_id << "]"
                << " decrement: " << decrement << " true: " << true_decrement
                << endl;

            // check paragraph and regexp
            // here one could have a flag on the indentation pattern rules
            // to check what is to be done when there is no valid paragraph
            // matching the request. This would allow to enable some patterns
            // that cannot otherwise.
            if( !local.isValid() || !iter->accept( local.text() ) )
            {
                Debug::Throw() << "TextIndent::_acceptPattern - [" << pattern.name() << "," << rule_id << "] rejected" << endl;
                accepted = false;
            }

        }

    }

    return accepted;
}

//____________________________________________
int TextIndent::_tabCount( const QTextBlock& block )
{

    QString text( block.text() );
    int count = 0;

    // skip the characters matching baseIndentation_
    int index = baseIndentation_;

    // loop over next characters to identify tabs
    // both normal and emulated tabs are counted
    while( index < text.size() )
    {
        if( text.mid( index, editor_->normalTabCharacter().size() ) == editor_->normalTabCharacter() )
        { index += editor_->normalTabCharacter().size(); count++; }

        else if( text.mid( index, editor_->emulatedTabCharacter().size() ) == editor_->emulatedTabCharacter() )
        { index += editor_->emulatedTabCharacter().size(); count++; }

        else break;
    }

    return count;
}

//____________________________________________
void TextIndent::_addBaseIndentation( QTextBlock block )
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
void TextIndent::_decrement( QTextBlock block )
{

    // set a cursor at beginning of block
    QTextCursor cursor( block );
    cursor.setPosition( block.position() + baseIndentation(), QTextCursor::MoveAnchor );

    // leading space characters regexp
    static const QRegExp regexp( "^\\s+" );

    // search text and remove characters
    if( regexp.indexIn( block.text().mid( baseIndentation() ) ) >= 0 )
    {
        int position( currentCursor_.position() );
        int anchor( currentCursor_.anchor() );
        int length( regexp.matchedLength() );
        Debug::Throw() << "TextIndent::_decrement - matched length: " << length << endl;

        cursor.setPosition( cursor.position() + length, QTextCursor::KeepAnchor );
        cursor.removeSelectedText();
        if( currentCursor_.block() == block && currentCursor_.position() - block.position() > baseIndentation() )
        {
            currentCursor_.setPosition( qMax( 0, anchor - length ), QTextCursor::MoveAnchor );
            currentCursor_.setPosition( qMax( 0, position - length ), QTextCursor::KeepAnchor );
        }
    } else { Debug::Throw() << "TextIndent::_decrement - no match" << endl; }
}

//____________________________________________
void TextIndent::_increment( QTextBlock block, int count )
{

    Debug::Throw() << "TextIndent::_increment - count " << count << endl;

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
