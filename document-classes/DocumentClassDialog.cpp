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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "BlockDelimiterList.h"
#include "DocumentClassConfiguration.h"
#include "DocumentClassDialog.h"
#include "DocumentClassPreview.h"
#include "DocumentClassTextEditor.h"
#include "HighlightStyleList.h"
#include "HighlightPatternList.h"
#include "BaseIcons.h"
#include "IconEngine.h"
#include "IndentPatternList.h"
#include "InformationDialog.h"
#include "TextMacroList.h"
#include "TextMacroMenu.h"
#include "TextParenthesisList.h"
#include "TreeView.h"

#include <QtGui/QPushButton>

//__________________________________________________________
DocumentClassDialog::DocumentClassDialog( QWidget* parent ):
TabbedDialog( parent )
{

    Debug::Throw( "DocumentClassDialog::DocumentClassDialog.\n" );
    setOptionName( "DOCUMENT_CLASS_WINDOW" );

    _list().setMaximumWidth( 120 );

    // general
    QWidget* page = &addPage( "General", "Document class general settings" );
    page->layout()->addWidget( documentClassConfiguration_ = new DocumentClassConfiguration( page ) );

    // highlight styles
    page = &addPage( "Highlight", "Syntax highlighting styles and patterns", true );

    page->layout()->addWidget( highlightStyleList_ = new HighlightStyleList(page) );
    page->layout()->addWidget( highlightPatternList_ = new HighlightPatternList(page) );

    // delimiters
    page = &addPage( "Delimiters", "Parenthesis and block delimiters", true );
    page->layout()->addWidget( textParenthesisList_ = new TextParenthesisList( page ) );
    page->layout()->addWidget( blockDelimiterList_ = new BlockDelimiterList( page ) );

    // indentation
    page = &addPage( "Indentation", "Indentation rules", true );
    page->layout()->addWidget( indentPatternList_ = new IndentPatternList( page ) );

    // macro
    page = &addPage( "Macros", "Text processing macros", true );
    page->layout()->addWidget( textMacroList_ = new TextMacroList() );

    // preview
    page = &addPage( "Preview", "Document class preview and testing", true );
    page->layout()->addWidget( preview_ = new DocumentClassPreview() );
    connect( &_preview().reloadButton(), SIGNAL( clicked( void ) ), SLOT( _updatePreview( void ) ) );

    // buttons
    _buttonLayout().addStretch( 1 );

    QPushButton* button;
    _buttonLayout().addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_ACCEPT ), "&OK", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( accept() ) );

    _buttonLayout().addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&Cancel", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( reject() ) );

    // connections
    connect( &_highlightStyleList(), SIGNAL( modified( void ) ), SLOT( _updateStyles( void ) ) );
    adjustSize();

}

//___________________________________________________________________________________
void DocumentClassDialog::setDocumentClass( const DocumentClass& document_class )
{

    Debug::Throw( "DocumentClassDialog::setDocumentClass.\n" );

    // set window title
    QString buffer;
    QTextStream( &buffer ) << "Document Class: " << document_class.name() << " - Qedit";
    setWindowTitle( buffer );

    // configuration
    documentClassConfiguration_->setDocumentClass( document_class );

    // highlight styles
    highlightStyleList_->setStyles( document_class.highlightStyles() );

    // highlight patterns
    highlightPatternList_->setStyles( document_class.highlightStyles() );
    highlightPatternList_->setPatterns( document_class.highlightPatterns() );

    // parenthesis
    textParenthesisList_->setParenthesis( document_class.parenthesis() );

    // delimiters
    blockDelimiterList_->setDelimiters( document_class.blockDelimiters() );

    //! indentation
    indentPatternList_->setPatterns( document_class.indentPatterns() );

    // text macros
    textMacroList_->setMacros( document_class.textMacros() );

    // update preview
    _updatePreview();

}

//___________________________________________________________________________________
DocumentClass DocumentClassDialog::documentClass( void )
{

    Debug::Throw( "DocumentClassDialog::documentClass.\n" );
    DocumentClass out( _documentClassConfiguration().documentClass() );
    out.setHighlightStyles( _highlightStyleList().styles() );
    out.setHighlightPatterns( _highlightPatternList().patterns() );
    QStringList warnings = out.associatePatterns();

    out.setIndentPatterns( _indentPatternList().patterns() );
    out.setParenthesis( _textParenthesisList().parenthesis() );
    out.setBlockDelimiters( _blockDelimiterList().delimiters() );
    out.setTextMacros( _textMacroList().macros() );

    if( !warnings.empty() )
    {
        QString message;
        QTextStream what( &message );
        what << "Document class named " << out.name() << " contains the following error";
        if( warnings.size() > 1 ) what << "s";
        what << ": " << endl;

        for( QStringList::const_iterator iter = warnings.begin(); iter != warnings.end(); ++iter )
        { what << "  " << *iter << endl; }

        InformationDialog( this, message ).centerOnParent().exec();
    }

    return out;
}

//___________________________________________________________________________________
void DocumentClassDialog::_updateStyles( void )
{

    Debug::Throw( "DocumentClassDialog::_updateStyles.\n" );

    // update styles associated to patterns
    HighlightStyle::Set styles(  _highlightStyleList().styles() );
    _highlightPatternList().setStyles( styles );

}

//___________________________________________________________________________________
void DocumentClassDialog::_updatePreview( void )
{
    Debug::Throw( "DocumentClassDialog::_updatePreview.\n" );

    // get document class
    DocumentClass document_class( documentClass() );
    _preview().macroMenu().setTextMacros( document_class.textMacros(), _preview().editor().textCursor().hasSelection() );
    _preview().editor().setDocumentClass( document_class );
    _preview().editor().rehighlight();
}
