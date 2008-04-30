#ifndef DocumentClass_h
#define DocumentClass_h

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
  \file DocumentClass.h
  \brief Highlight and indentation patterns for a given document class
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDomDocument>
#include <QDomElement>
#include <QRegExp>

#include <set>
#include <list>

#include "BlockDelimiter.h"
#include "Counter.h"
#include "Debug.h"
#include "IndentPattern.h"
#include "TextParenthesis.h"
#include "TextMacro.h"
#include "File.h"
#include "HighlightPattern.h"
#include "HighlightStyle.h"



//! Highlight and indentation patterns for a given document class
class DocumentClass: public Counter
{
  
  public:
      
  //! constructor
  DocumentClass( const QDomElement& element = QDomElement() );
    
  //! destructor
  ~DocumentClass( void )
  { 
    Debug::Throw( "DocumentClass::~DocumentClass.\n" );
    clear(); 
  }

  //! write to DomElement
  QDomElement domElement( QDomDocument& parent ) const;

  //! lower than operator
  bool operator < (const DocumentClass& document_class ) const
  { 
    if( isDefault() )  return true;
    if( document_class.isDefault() ) return false;
    return name() < document_class.name(); 
  }
  
  //! clear patterns and styles
  void clear( void );
  
  //! name
  const std::string& name ( void ) const
  { return name_; }
  
  //! file
  const File& file( void ) const
  { return file_; }
  
  //! set file
  void setFile( const File& file )
  { file_ = file; }
  
  //! default
  const bool& isDefault( void ) const
  { return default_; }
  
  //! filename matching pattern
  const QRegExp& fileMatchingPattern( void ) const 
  { return file_pattern_; }
  
  //! first line matching pattern
  const QRegExp& firstLineMatchingPattern( void ) const
  { return firstline_pattern_; }
  
  //! icon name
  const std::string& icon( void ) const
  { return icon_; }
  
  //! return true if document class match filename
  bool match( const File& file ) const;

  //! returns true if document class enables wrapping by default
  const bool& wrap( void ) const
  { return wrap_; }
  
  //! list of hightlight styles
  const HighlightStyle::Set& highlightStyles() const
  { return highlight_styles_; }
  
  //! list of highligh patterns
  const HighlightPattern::List& highlightPatterns() const
  { return highlight_patterns_; }
  
  //! list of indentation patterns
  const IndentPattern::List& indentPatterns() const
  { return indent_patterns_; }

  //! base indentation
  /*! 
  it is used which classes for which
  a certain amount of leading space character are 
  not to be considered when indenting. This is the case
  for the first 6 space characters in fortran files
  */
  const int& baseIndentation( void ) const
  { return base_indentation_; }
  
  //! list of text parenthesis
  const TextParenthesis::List& parenthesis() const
  { return text_parenthesis_; }

  //! list of block delimiters
  const BlockDelimiter::List& blockDelimiters() const
  { return block_delimiters_; }

  //! list of text macros
  const TextMacro::List& textMacros() const
  { return text_macros_; }
  
  //! used to match pointers to DocumentClass with same name
  class SameNameFTor
  {
    public:
    
    //! constructor
    SameNameFTor( const std::string& name ):
      name_( name )
    {}
    
    //! predicate
    bool operator() (const DocumentClass& document_class ) const
    { return document_class.name() == name_; }
    
    private:
    
    //! predicted name
    std::string name_; 
  };
  
  //! used to get non default document class matching a file
  class MatchFileFTor
  {
    public:
    
    //! constructor
    MatchFileFTor( const File& file ):
      file_( file )
    {}
    
    //! predicate
    bool operator () (const DocumentClass& document_class ) const
    { return document_class.match( file_ ) && !document_class.isDefault(); }
    
    private:
    
    //! predicted file
    const File file_;
    
  };
  
  //! used to counts number of default patterns registered
  class IsDefaultFTor
  {
    public:
    
    //! predicate
    bool operator() (const DocumentClass& document_class ) const
    { return document_class.isDefault(); }
    
  };
    
  private:
  
  //! base indentation
  void _setBaseIndentation( const int& value )
  { base_indentation_ = value; }
    
  //! name
  std::string name_;
  
  //! parent file
  File file_;
  
  //! file pattern
  QRegExp file_pattern_;
  
  //! first line pattern
  /*! it is used to see if a file match based on its first line rather than its extension */
  QRegExp firstline_pattern_;
  
  //! is class default
  bool default_;
  
  //! wrap flag
  bool wrap_;
  
  //! icon name
  std::string icon_;
  
  //! set of highlight styles
  HighlightStyle::Set highlight_styles_;
  
  //! list of highlight patterns
  HighlightPattern::List highlight_patterns_;  
  
  //! list of indentation patterns
  IndentPattern::List indent_patterns_;
  
  //! list of text parenthesis
  TextParenthesis::List text_parenthesis_;

  //! block delimiter
  BlockDelimiter::List block_delimiters_;
  
  //! list of text parenthesis
  TextMacro::List text_macros_;
  
  //! base indentation
  /*! 
    this is the number of space characters to add prior to any text indentation
  */
  int base_indentation_;
  
};


#endif
