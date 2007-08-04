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

#include <qobject.h>

#include "Counter.h"
#include "Debug.h"
#include "IndentPattern.h"

class TextDisplay;

//! syntax highlighting based on text patterns
class TextIndent: public QObject, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:
  
  //! constructor
  TextIndent( TextDisplay* editor );

  //! enabled
  void setEnabled( bool state )
  { enabled_ = state; }

  //! enabled
  bool isEnabled( void ) const
  { return enabled_ && !patterns_.empty(); }
  
  //! base indentation
  const unsigned int& baseIndentation( void ) const
  { return base_indentation_; }

  //! base indentation
  void setBaseIndentation( const unsigned int& value )
  { base_indentation_ = value; }

  //! patterns
  typedef std::list< IndentPattern* > IndentPattern::List;
  
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
  void clearPatterns( void )
  {
    Debug::Throw( "TextIndent::ClearPatterns.\n" );
    patterns_.clear();
  }
  
  public slots:

  //! highlight paragraph
  virtual void indent( QTextBlock block );

  private:

  //! returns true if pattern match current paragraph
  bool _acceptPattern( QTextBlock block, const IndentPattern& pattern ) const;

  //! return number of tabs in given paragraph
  int _tabCount( const QTextBlock& block );

  //! number of tabs in previous valid paragraph
  // int _previousTabCount( QTextBlock block );

  //! increment paragraph with n tabs
  void _increment( QTextBlock block, const unsigned int& count = 1 );

  //! decrement paragraph
  //! \brief try remove leading tabs up to n
  void _decrement( QTextBlock block );

  //! enabled
  bool enabled_;

  //! destination editor
  TextDisplay* editor_;
  
  //! current cursor
  QTextCursor current_cursor_;
  
  //! base indentation
  /*! this is the number of space characters to add prior to any text indentation */
  unsigned int base_indentation_;
  
  //! list of highlight patterns
  IndentPattern::List patterns_;
  
};

#endif
