// $Id$
#ifndef DocumentClassTextEditor_h
#define DocumentClassTextEditor_h

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

/*!
  \file DocumentClassTextEditor.h
  \brief text display window
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/


#include <QRegExp>

#include "AnimatedTextEditor.h"
#include "BlockDelimiter.h"
#include "Config.h"
#include "Debug.h"
#include "HighlightBlockFlags.h"
#include "HighlightPattern.h"
#include "ParenthesisHighlight.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "TimeStamp.h"

#if WITH_ASPELL
#include "DictionaryMenu.h"
#include "FilterMenu.h"
#endif

// forward declaration
class BlockDelimiterDisplay;
class DocumentClass;
class HighlightBlockData;
class TextHighlight;

//! text display window
class DocumentClassTextEditor: public AnimatedTextEditor
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! constructor
  DocumentClassTextEditor( QWidget* parent );

  //! destructor
  virtual ~DocumentClassTextEditor();

  //! number of block associated to argument
  /*! reimplemented from base class to account for collapsed blocks */
  virtual int blockCount( const QTextBlock& ) const;

  //! draw margins
  virtual void paintMargin( QPainter& );

  //! document class
  void setDocumentClass( const DocumentClass& );

  //!@name macro
  //@{

  //! list of macros
  const TextMacro::List& macros( void ) const
  { return macros_; }

  //@}

  //! returns true if paragraph is to be ignored when identing/parsing parenthesis
  bool ignoreParagraph( const QTextBlock& paragraph );

  //! text highlight
  bool hasTextHighlight( void )
  { return text_highlight_; }

  //! text highlight
  TextHighlight& textHighlight( void )
  { return *text_highlight_; }

  //! text highlight
  const TextHighlight& textHighlight( void ) const
  { return *text_highlight_; }

  //! text indent
  TextIndent& textIndent( void ) const
  {
    assert( indent_ );
    return *indent_;
  }

  //! block delimiter display
  bool hasBlockDelimiterDisplay( void ) const
  { return block_delimiter_display_; }

  //! block delimiter display
  BlockDelimiterDisplay& blockDelimiterDisplay( void ) const
  { return *block_delimiter_display_; }

  //!@name block interface
  //@{

  // return true if block is an empty line
  virtual bool isEmptyBlock( const QTextBlock& block ) const
  { return _emptyLineRegExp().indexIn( block.text() ) >= 0; }

  //! return true is block is to be ignored from indentation scheme
  virtual bool ignoreBlock( const QTextBlock& block ) const;

  //@}

  //! return parenthesis highlight object
  ParenthesisHighlight& parenthesisHighlight( void ) const
  { return *parenthesis_highlight_; }

  signals:

  //! emmited when indentation several blocks is required
  void indent( QTextBlock, QTextBlock );

  //! emmited when indentation of one block is required
  void indent( QTextBlock, bool  );

  public slots:

  //! process macro by name
  void processMacro( QString );

  //! rehighlight
  void rehighlight( void );

  protected:

  //!@name event handlers
  //@{

  //! generic event
  virtual bool event( QEvent* );

  //! keypress event [overloaded]
  virtual void keyPressEvent( QKeyEvent* );

  //! paint event
  virtual void paintEvent( QPaintEvent* );

  //@}

  //! clear macros
  void _clearMacros( void )
  { macros_.clear(); }

  //! macros
  void _setMacros( const TextMacro::List& macros)
  { macros_ = macros; }

  //! track text modifications for syntax highlighting
  void _setBlockModified( const QTextBlock& );

  //! update margins
  virtual bool _updateMargin( void );

  private slots:

  //! update configuration
  void _updateConfiguration( void );

  //! indent paragraph (when return or tab is pressed)
  void _indentCurrentParagraph( void );

  //! track text modifications for syntax highlighting
  void _setBlockModified( int, int, int );

  //! highlight parenthesis
  void _highlightParenthesis( void );

  private:

  //! empty line
  static QRegExp& _emptyLineRegExp( void );

  //!@name document classes specific members
  //@{

  //! text indent
  TextIndent* indent_;

  //! text macro
  TextMacro::List macros_;

  //@}

  //! syntax highlighter
  TextHighlight* text_highlight_;

  //! parenthesis highlight object
  ParenthesisHighlight* parenthesis_highlight_;

  //! block delimiter
  BlockDelimiterDisplay* block_delimiter_display_;

};

#endif
