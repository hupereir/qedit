#ifndef _TextIndent_h_
#define _TextIndent_h_

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
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
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

//! syntax highlighting based on text patterns
class TextIndent: public QObject, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! constructor
  TextIndent( TextEditor* editor );

  //! enabled
  void setEnabled( bool state )
  { enabled_ = state; }

  //! enabled
  bool isEnabled( void ) const
  { return enabled_ && !patterns_.empty(); }

  //! base indentation
  const int& baseIndentation( void ) const
  { return base_indentation_; }

  //! base indentation
  void setBaseIndentation( const int& value )
  { base_indentation_ = value; }

  //! patterns
  const IndentPattern::List& patterns( void ) const
  { return patterns_; }

  //! patterns
  void setPatterns( const IndentPattern::List& patterns )
  {
    Debug::Throw( "TextIndent::SetPatterns.\n" );
    patterns_ = patterns;
  }

  //! patterns
  void clear( void )
  {
    Debug::Throw( "TextIndent::clear.\n" );
    patterns_.clear();
  }

  public slots:

  //! highlight blocks
  virtual void indent( QTextBlock first, QTextBlock last );

  //! highlight block
  /*!
  new_line argument is used in case there is a default base indentation,
  to properly indent paragraphs when return key is pressed
  */
  virtual void indent( QTextBlock block, bool new_line = false );

  private:

  //! returns true if pattern match current paragraph
  bool _acceptPattern( QTextBlock block, const IndentPattern& pattern ) const;

  //! return number of tabs in given paragraph
  int _tabCount( const QTextBlock& block );

  //! add base indentation
  void _addBaseIndentation( QTextBlock block );

  //! decrement paragraph
  //! \brief try remove leading tabs up to n
  void _decrement( QTextBlock block );

  //! increment paragraph with n tabs
  void _increment( QTextBlock block, const unsigned int& count = 1 );

  //! enabled
  bool enabled_;

  //! destination editor
  TextEditor* editor_;

  //! current cursor
  QTextCursor current_cursor_;

  //! base indentation
  /*! this is the number of space characters to add prior to any text indentation */
  int base_indentation_;

  //! list of highlight patterns
  IndentPattern::List patterns_;

};

#endif
