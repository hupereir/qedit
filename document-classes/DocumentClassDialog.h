#ifndef _DocumentClassDialog_h_
#define _DocumentClassDialog_h_

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

#include "TabbedDialog.h"
#include "DocumentClass.h"

class DocumentClassConfiguration;
class DocumentClassPreview;
class HighlightStyleList;
class HighlightPatternList;
class TextParenthesisList;
class BlockDelimiterList;
class TextMacroList;
class IndentPatternList;

//! Syntax highlighting style editing dialog
class DocumentClassDialog: public TabbedDialog
{

    Q_OBJECT

    public:

    //! constructor
    DocumentClassDialog( QWidget* parent );

    //! style
    void setDocumentClass( const DocumentClass& );

    //! return document class
    DocumentClass documentClass( void );

    //! true if any document class attribute has been modified
    bool modified( void )
    { return modified_; }

    private slots:

    //! update styles
    void _updateStyles( void );

    //! update preview
    void _updatePreview( void );

    private:

    //!@name ref accessors
    //@{

    DocumentClassConfiguration& _documentClassConfiguration( void ) const
    { return *documentClassConfiguration_; }

    HighlightStyleList& _highlightStyleList( void ) const
    { return *highlightStyleList_; }

    //! highlight pattern list
    HighlightPatternList& _highlightPatternList( void ) const
    { return *highlightPatternList_; }

    //! text parenthesis list
    TextParenthesisList& _textParenthesisList( void ) const
    { return *textParenthesisList_; }

    //! block delimiter list
    BlockDelimiterList& _blockDelimiterList( void ) const
    { return *blockDelimiterList_; }

    //! block delimiter list
    IndentPatternList& _indentPatternList( void ) const
    { return *indentPatternList_; }

    //! block delimiter list
    TextMacroList& _textMacroList( void ) const
    { return *textMacroList_; }

    //! document class preview
    DocumentClassPreview& _preview( void ) const
    { return *preview_; }

    //@}

    //! document class configuration
    DocumentClassConfiguration* documentClassConfiguration_;

    //! highlight style list
    HighlightStyleList* highlightStyleList_;

    //! highlight pattern list
    HighlightPatternList* highlightPatternList_;

    //! text parenthesis list
    TextParenthesisList* textParenthesisList_;

    //! block delimiter list
    BlockDelimiterList* blockDelimiterList_;

    //! block delimiter list
    IndentPatternList* indentPatternList_;

    //! block delimiter list
    TextMacroList* textMacroList_;

    //! preview
    DocumentClassPreview* preview_;

    //! modified
    bool modified_;

};

#endif
