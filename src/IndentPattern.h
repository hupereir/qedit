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

#include <list>
#include <string>

#include "Counter.h"
#include "Debug.h"

//! Base class for syntax highlighting
class IndentPattern: public Counter
{

  public: 

  //! typedef for list of patterns
  typedef std::list< IndentPattern > List;
        
  //! constructor from DomElement
  IndentPattern( const QDomElement& element = QDomElement() );
  
  //! dom element
  QDomElement domElement( QDomDocument& parent ) const;
  
  //! Id
  const unsigned int& id( void ) const
  { return id_; }
  
  //! name
  const std::string& name() const
  { return name_; }
  
  //! reset counter
  static void resetCounter( void )
  { id_counter_ = 0; }
  
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
    
  //! pattern scale
  const unsigned int& scale( void ) const
  { return scale_; }
  
  //! indentation rule
  /*! used to check a regExp against a given paragraph */
  class Rule: public Counter
  {
    public:
    
    //! constructor
    Rule( const QDomElement& element = QDomElement() );
  
    //! dom element
    virtual QDomElement domElement( QDomDocument& parent ) const;
        
    //! paragraph
    const int& paragraph() const
    { return paragraph_; }
    
    //! true if valid
    bool isValid( void ) const
    { return regexp_.isValid(); }
    
    //! returns true if the text match the rule
    bool accept( const QString& text ) const;
    
    private:
    
    //! set paragraph
    void _setParagraph( const int& par )
    { paragraph_ = par; }
    
    //! set regExp
    void _setRegExp( const std::string& regexp )
    { regexp_ = QRegExp( regexp.c_str() ); }      
            
    //! paragraph id (vs current)
    int paragraph_;
    
    //! regexp
    QRegExp regexp_;
    
    //! streamer
    friend std::ostream& operator << ( std::ostream& out, const Rule& rule )
    {
      out << "Rule - par_id: " << rule.paragraph_ << " RegExp: \"" << qPrintable( rule.regexp_.pattern() ) << "\"";
      return out;
    }  
  };
  
  //! shortcut to Rules
  typedef std::list< Rule > RuleList;
  
  //! retrieve rules
  const RuleList& rules( void ) const
  { return rules_; }
  
  //! validity
  bool isValid( void ) const
  {
    for( RuleList::const_iterator iter =  rules_.begin(); iter != rules_.end(); iter++ )
    if( !iter->isValid() ) return false;
    return true;
  }
  
  protected:
  
  //! name
  void _setName( const std::string& name )
  { name_ = name; }
  
  //! type
  void _setType( const Type& type )
  { type_ = type; }

  //! scale
  void _setScale( const unsigned int& scale )
  { scale_ = scale; }
  
  //! add rule
  void _addRule( const Rule& rule )
  { rules_.push_back( rule ); }
  
  private:
  
  //! unique id counter
  static unsigned int id_counter_;
  
  //! unique id
  unsigned int id_; 

  //! pattern name
  std::string name_;
  
  //! type
  Type type_;
          
  //! scale
  /*! 
    decide how many increment/decrement operations 
    should be performed. Default is one
  */
  
  unsigned int scale_;
  
  //! list of rules to match
  RuleList rules_;
      
  //! dumper
  friend std::ostream& operator << ( std::ostream& out, const IndentPattern& pattern )
  {
    out << "IndentPattern - name: " << pattern.name_ << " type: " << pattern.type_ << std::endl;
    for( RuleList::const_iterator iter =  pattern.rules_.begin(); iter != pattern.rules_.end(); iter++ )
    out << "  " << *iter << std::endl;
    return out;
  }
  
};
#endif
