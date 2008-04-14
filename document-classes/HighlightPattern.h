#ifndef _HighlightPattern_h_
#define _HighlightPattern_h_

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
  \file HighlightPattern.h
  \brief Base class for syntax highlighting
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDomElement>
#include <QDomDocument>
#include <QRegExp>

#include <set>
#include <vector>
#include <string>

#include "HighlightStyle.h"
#include "Counter.h"
#include "Debug.h"

class PatternLocationSet;

//! Base class for syntax highlighting
class HighlightPattern: public Counter
{

  public: 
    
  //! pattern flags
  enum Flag
  {
    
    //! no flag
    NONE = 0,
    
    //! pattern spans over several blocks
    SPAN = 1<<0,
    
    //! pattern exclude line from indentations
    /*! this is typically the case for comments */
    NO_INDENT = 1<<1,
    
    //! pattern matching should not be case sensitive
    CASE_INSENSITIVE = 1<<2
    
  };
  
  //! typedef for list of patterns
  typedef std::vector< HighlightPattern > List;
  
  //! constructor from DomElement
  HighlightPattern( const QDomElement& element = QDomElement() );
  
  //! dom element
  QDomElement domElement( QDomDocument& parent ) const;

  //! set id  
  /*!
  The unique ID has a single bit set to 1, to use
  faster active pattern masks, with no shift operators
  */
  void setId( const int& id )
  { id_ = (1<<id); }
    
  //! unique id
  const int& id( void ) const
  { return id_; }
  
  //! equal to operator
  bool operator == (const HighlightPattern& pattern ) const
  { return id() == pattern.id(); }
  
  //! less than operator
  bool operator < (const HighlightPattern& pattern ) const
  { return id() < pattern.id(); }
  
  //! true if any attributes is different from argument
  /*! this is a stricter comparison than the != operator */
  bool differs( const HighlightPattern& pattern ) const;
    
  //! name
  const std::string& name( void ) const
  { return name_; }
             
  //! name
  void setName( const std::string& name ) 
  { name_ = name; }
  
  //! pattern type
  enum Type
  {
    //! undefined
    UNDEFINED,
      
    //! single keyword
    KEYWORD_PATTERN,
    
    //! range pattern
    RANGE_PATTERN
    
  };   
 
  //!type
  const Type& type( void ) const
  { return type_; }

  //!type
  void setType( const Type& type )
  { type_ = type; }
   
  //! type
  std::string typeName( void ) const
  { return typeName( type() ); }
  
  //! type
  static std::string typeName( const Type& type );

  //! parent name
  const std::string& parent( void ) const
  { return parent_; }

  //! parent name
  void setParent( const std::string& parent )
  { parent_ = parent; }
  
  //! parent id
  const int& parentId( void ) const
  { return parent_id_; }
    
  //! parent id
  void setParentId( const int& id )
  { parent_id_ = id; }
    
  //! text style
  const HighlightStyle& style( void ) const
  { return style_; }
    
  //! text style 
  void setStyle( const HighlightStyle& style )
  { style_ = style; }
  
  //! child patterns
  const List& children( void ) const
  { return children_; }

  //! add child
  void addChild( HighlightPattern child )
  { children_.push_back( child ); }
  
  //! clear children
  void clearChildren( void )
  { children_.clear(); }
  
  //! keyword regexp
  const QRegExp& keyword( void ) const
  { return keyword_; }
  
  //! keyword
  virtual void setKeyword( const std::string& keyword )
  { keyword_.setPattern( keyword.c_str() ); }

  //! begin regexp
  const QRegExp& begin( void ) const
  { return keyword(); }

  //! keyword
  virtual void setBegin( const std::string& keyword )
  { setKeyword( keyword ); }
    
  //! end regexp
  const QRegExp& end( void ) const
  { return end_; }
  
  //! range end pattern
  virtual void setEnd( const std::string& keyword )
  { end_.setPattern( keyword.c_str() ); }

  //! comments
  const std::string& comments( void ) const
  { return comments_; }
  
  //! comments
  void setComments( const std::string& comments )
  { comments_ = comments; }
    
  //!@name flags
  //@{
  
  //! flags
  const unsigned int& flags( void ) const
  { return flags_; }
  
  //! flags
  void setFlags( const unsigned int& flags )
  { flags_ = flags; }

  //! flags
  bool flag( const Flag& flag ) const
  { return flags_ & flag; }
  
  //! flags
  void setFlag( const Flag& flag, const bool& value )
  { 
    if( value ) flags_ |= flag; 
    else flags_ &= (~flag);
  } 
  
  //@}
  
  //! validity
  bool isValid( void ) const
  { 
    switch( type() )
    {
      case KEYWORD_PATTERN: return keyword_.isValid();
      case RANGE_PATTERN: return keyword_.isValid() && end_.isValid();
      default: return false;
    }
  }
  
  //! process text and update the matching locations.
  /*! 
  Returns true if at least one match is found. 
  Locations and active parameters are changed 
  */
  bool processText( PatternLocationSet& locations, const QString& text, bool& active ) const
  {
    switch( type() )
    {
      case KEYWORD_PATTERN: return _findKeyword( locations, text, active );
      case RANGE_PATTERN: return _findRange( locations, text, active );
      default: return false;
    }
  }
  
  //! used to get patterns by name
  class SameNameFTor
  {
    
    public:
    
    //! constructor    
    SameNameFTor( const std::string& name ):
      name_( name )
    {}
    
    //! predicate
    bool operator() (const HighlightPattern& pattern ) const
    { return pattern.name() == name_; }
        
    private:
        
    //! predicate
    const std::string name_;
    
  };
  
  //! used to pattern by id
  class SameIdFTor
  {
    
    public:
    
    //! constructor
    SameIdFTor( const int& id ):
      id_( id )
    {}
      
    //! predicate
    bool operator() ( const HighlightPattern& pattern ) const
    { return pattern.id() == id_; }
    
    private:
    
    //! predicted id
    int id_;
    
  };
  
  protected:

  //! find keyword pattern
  bool _findKeyword( PatternLocationSet&, const QString&, bool& ) const;
  
  //! find range pattern
  bool _findRange( PatternLocationSet&, const QString&, bool& ) const;

  private:
        
  //! unique id
  /*!
    The unique ID has a single bit set to 1, to use
    faster active pattern masks, with no shift operators
  */
  int id_; 
  
  //! type
  Type type_;
      
  //! pattern name
  std::string name_;
  
  //! parent pattern name
  std::string parent_;
  
  //! parent pattern id
  int parent_id_;
  
  //! style
  HighlightStyle style_;
  
  //! child patterns
  List children_;
  
  //! comments
  std::string comments_;
  
  //! flags
  unsigned int flags_;
  
  //!@name patterns
  //@{
  
  //! keyword regexp (or begin in case of range pattern)
  QRegExp keyword_;
  
  //! range end regexp
  QRegExp end_;
  
  //@}
  
  //!@name dumpers
  //@{  
  //! dump
  friend std::ostream& operator << (std::ostream& out, const HighlightPattern& pattern )
  {  
    out << "id: " << pattern.id() << " name: " << pattern.name() << " parent name:" << pattern.parent();
    return out;
  }
  
  //@}  
};
#endif
