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

/*!
  \file DocumentClassDialog.h
  \brief Syntax highlighting style editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QPushButton>

#include "BlockDelimiterList.h"
#include "DocumentClassConfiguration.h"
#include "DocumentClassDialog.h"
#include "HighlightStyleList.h"
#include "HighlightPatternList.h"
#include "BaseIcons.h"
#include "IconEngine.h"
#include "IndentPatternList.h"
#include "InformationDialog.h"
#include "TextMacroList.h"
#include "TextParenthesisList.h"
#include "TreeView.h"

using namespace std;

//__________________________________________________________
DocumentClassDialog::DocumentClassDialog( QWidget* parent ):
  TabbedDialog( parent )
{
  
  Debug::Throw( "DocumentClassDialog::DocumentClassDialog.\n" );
  setOptionName( "DOCUMENT_CLASS_WINDOW" );

  _list().setMaximumWidth( 120 );
  
  // general
  QWidget* page = &addPage( "General", "Document class general settings" );
  page->layout()->addWidget( document_class_configuration_ = new DocumentClassConfiguration( page ) );
  
  // highlight styles
  page = &addPage( "Highlight", "Syntax highlighting styles and patterns", true );
  
  page->layout()->addWidget( highlight_style_list_ = new HighlightStyleList(page) );
  page->layout()->addWidget( highlight_pattern_list_ = new HighlightPatternList(page) );
  
  // delimiters
  page = &addPage( "Delimiters", "Parenthesis and block delimiters", true );
  page->layout()->addWidget( text_parenthesis_list_ = new TextParenthesisList( page ) );
  page->layout()->addWidget( block_delimiter_list_ = new BlockDelimiterList( page ) );
  
  // indentation
  page = &addPage( "Indentation", "Indentation rules", true );
  page->layout()->addWidget( indent_pattern_list_ = new IndentPatternList( page ) );
  
  // macro
  page = &addPage( "Macros", "Text processing macros", true );
  page->layout()->addWidget( text_macro_list_ = new TextMacroList() );

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
  QTextStream( &buffer ) << "Document class: " << document_class.name();
  setWindowTitle( buffer );
  
  // configuration
  document_class_configuration_->setDocumentClass( document_class );
      
  // highlight styles
  highlight_style_list_->setStyles( document_class.highlightStyles() );
  
  // highlight patterns
  highlight_pattern_list_->setStyles( document_class.highlightStyles() );
  highlight_pattern_list_->setPatterns( document_class.highlightPatterns() );
  
  // parenthesis
  text_parenthesis_list_->setParenthesis( document_class.parenthesis() );

  // delimiters
  block_delimiter_list_->setDelimiters( document_class.blockDelimiters() );
  
  //! indentation
  indent_pattern_list_->setPatterns( document_class.indentPatterns() );
  
  // text macros
  text_macro_list_->setMacros( document_class.textMacros() );
  
  
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
    
    for( QStringList::const_iterator iter = warnings.begin(); iter != warnings.end(); iter++ )
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
