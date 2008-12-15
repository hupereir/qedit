#ifndef _IndentPattern_h_
#define _IndentPattern_h_
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
  \file IndentPattern.h
  \brief Base class for indentation pattern
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDomElement>
#include <QDomDocument>
#include <QRegExp>

#include <vector>
#include <string>

#include "Counter.h"
#include "Debug.h"

//! Base class for syntax highlighting
class IndentPattern: public Counter
{

  public: 

  //! typedef for list of patterns
  typedef std::vector< IndentPattern > List;
        
  //! constructor from DomElement
  IndentPattern( void ):
    Counter( "IndentPattern" ),
    id_( 0 ),
    type_( NOTHING ),
    scale_( 0 )
  {}
    
  //! constructor from DomElement
  IndentPattern( const QDomElement& element );

  //! dom element
  QDomElement domElement( QDomDocument& parent ) const;
  
  //! Id
  const unsigned int& id( void ) const
  { return id_; }

  //! set id  
  void setId( const int& id )
  { id_ = id; }
 
  //! equal to operator
  bool operator == (const IndentPattern& pattern ) const
  { return id() == pattern.id(); }
  
  //! less than operator
  bool operator < (const IndentPattern& pattern ) const
  { return id() < pattern.id(); }
  
  //! true if any attributes is different from argument
  /*! this is a stricter comparison than the != operator */
  bool differs( const IndentPattern& pattern ) const;

  //! name
  const QString& name() const
  { return name_; }
  
    //! name
  void setName( const QString& name )
  { name_ = name; }

  //! reset counter
  static void resetCounter( void )
  { _counter() = 0; }
  
  //! pattern type enumeration
  enum Type
  {
    NOTHING,
    INCREMENT,
    DECREMENT,
    DECREMENT_ALL
  };
    
  //! pattern type
  const Type& type( void ) const
  { return type_; }
    
  //! type
  void setType( const Type& type )
  { type_ = type; }
  
  //! type
  QString typeName( void ) const
  { return typeName( type() ); }
  
  //! type
  static QString typeName( const Type& type );
  
  //! pattern scale
  const unsigned int& scale( void ) const
  { return scale_; }
    
  //! scale
  void setScale( const unsigned int& scale )
  { scale_ = scale; }
  
  //! indentation rule
  /*! used to check a regExp against a given paragraph */
  class Rule: public Counter
  {
    public:

    //! shortcut to Rules
    typedef std::vector< Rule > List;

    //! pattern flags
    enum Flag
    {
      
      //! no flag
      NONE = 0,
        
      //! pattern matching should not be case sensitive
      CASE_INSENSITIVE = 1<<2
    
    };   
    
    //! constructor
    Rule( const QDomElement& element = QDomElement() );
  
    //! dom element
    virtual QDomElement domElement( QDomDocument& parent ) const;
        
    //! equal to operator
    bool operator == ( const Rule& rule ) const
    { 
      return
        pattern().pattern() == rule.pattern().pattern() &&
        paragraph() == rule.paragraph() &&
        flags() == rule.flags();
    }
      
    //! less than operator
    bool operator < ( const Rule& rule ) const
    { 
      if( pattern().pattern() != rule.pattern().pattern() ) return pattern().pattern() < rule.pattern().pattern();
      if( paragraph() != rule.paragraph() ) return paragraph() < rule.paragraph();
      if( flags() != rule.flags() ) return flags() < rule.flags();
      return false;
    }

    //! paragraph
    const int& paragraph() const
    { return paragraph_; }

    //! set paragraph
    void setParagraph( const int& par )
    { paragraph_ = par; }
   
    //! regExp
    void setPattern( const QString& regexp )
    { regexp_.setPattern( regexp ); }      
    
    //! regexp
    const QRegExp& pattern( void ) const
    { return regexp_; }
    
    //! true if valid
    bool isValid( void ) const
    { return regexp_.isValid(); }
    
    //! returns true if the text match the rule
    bool accept( const QString& text ) const;
 
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
    
    
    private:
                
    //! paragraph id (vs current)
    int paragraph_;
    
    //! regexp
    QRegExp regexp_;
   
    //! flags
    unsigned int flags_;
   
    //! streamer
    friend std::ostream& operator << ( std::ostream& out, const Rule& rule )
    {
      out << "Rule - par_id: " << rule.paragraph_ << " RegExp: \"" << qPrintable( rule.regexp_.pattern() ) << "\"";
      return out;
    }  
  };
    
  //! retrieve rules
  const Rule::List& rules( void ) const
  { return rules_; }

  //! add rule
  void addRule( const Rule& rule )
  { rules_.push_back( rule ); }
    
  //! rules
  void setRules( const Rule::List& rules ) 
  { rules_ = rules; }
  
  //! validity
  bool isValid( void ) const
  {
    for( Rule::List::const_iterator iter =  rules_.begin(); iter != rules_.end(); iter++ )
    if( !iter->isValid() ) return false;
    return true;
  }
    
  private:
  
  //! unique id counter
  static unsigned int& _counter( void );
  
  //! unique id
  unsigned int id_; 

  //! pattern name
  QString name_;
  
  //! type
  Type type_;
          
  //! scale
  /*! 
    decide how many increment/decrement operations 
    should be performed. Default is one
  */
  
  unsigned int scale_;
  
  //! list of rules to match
  Rule::List rules_;
      
  //! dumper
  friend std::ostream& operator << ( std::ostream& out, const IndentPattern& pattern )
  {
    out << "IndentPattern - name: " << qPrintable( pattern.name() ) << " type: " << pattern.type_ << std::endl;
    for( Rule::List::const_iterator iter =  pattern.rules_.begin(); iter != pattern.rules_.end(); iter++ )
    out << "  " << *iter << std::endl;
    return out;
  }
  
};
#endif
