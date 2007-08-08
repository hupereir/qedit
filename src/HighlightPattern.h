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
#include <list>
#include <string>

#include "HighlightStyle.h"
#include "Counter.h"
#include "Debug.h"

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
  typedef std::list< HighlightPattern* > List;

  
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
  virtual const int& id( void ) const
  { return id_; }
  
  //! name
  virtual const std::string& name( void ) const
  { return name_; }
  
  //! parent name
  virtual const std::string& parent( void ) const
  { return parent_; }
  
  //! parent id
  virtual const int& parentId( void ) const
  { return parent_id_; }
    
  //! parent id
  virtual void setParentId( const int& id )
  { parent_id_ = id; }
    
  //! text style
  virtual const HighlightStyle& style( void ) const
  { return style_; }
    
  //! text style 
  virtual void setStyle( const HighlightStyle& style )
  { style_ = style; }
  
  //! child patterns
  virtual const List& children( void ) const
  { return children_; }

  //! add child
  virtual void addChild( HighlightPattern* child )
  { children_.push_back( child ); }
  
  //! clear children
  virtual void clearChildren( void )
  { children_.clear(); }
  
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
  virtual bool isValid( void ) const
  { return true; }

  //! location of text to be formated
  class Location: public Counter
  {
    public:        
            
    //! constructor
    Location( 
      const HighlightPattern& parent,
      const int& position, 
      const unsigned int& length ):
        Counter( "HighlightPattern::Location" ),
        parent_( &parent ),
        position_( position ),
        length_( length )
    { }
    
    //! less than operator
    bool operator < (const Location& location ) const
    { 
      return 
        (position() < location.position()) ||
        (position() == location.position() && parentId() < location.parentId() ) ; 
      }
    
    //! position
    const int& position( void ) const
    { return position_; }
    
    //! length
    const unsigned int& length( void ) const
    { return length_; }
    
    //! parent
    const HighlightPattern& parent( void ) const
    { return *parent_; }
    
    //! pattern id
    const int& id( void ) const
    { return parent_->id(); }
    
    //! parent pattern id
    const int& parentId( void ) const
    { return parent_->parentId(); }
    
    //! style
    const HighlightStyle& style( void ) const
    { return parent_->style(); }
     
    //! used to find a location matching index
    class ContainsFTor 
    {
    
      public:
      
      //! constructor
      ContainsFTor( const int& index ):
        index_( index )
        {}
        
      //! prediction
      bool operator() (const Location& location )
      { 
        return 
          index_ >= location.position() && 
          index_ < location.position()+int(location.length()); 
      }
      
      private:
      
      //! predicted index
      int index_;
      
    };
    
    // overlaps
    class OverlapFTor
    {
      
      public:
      
      bool operator() (const Location& first, const Location& second ) 
      { return second.position() < first.position() + (int)first.length(); }
      
    };
    
    private:

    //! pointer to parent
    const HighlightPattern* parent_;
        
    //! position in text
    int position_;
    
    //! length of the pattern
    unsigned int length_;

    //! dump
    friend std::ostream& operator << (std::ostream& out, const Location& location )
    {  
      out << "id: " << location.id() << " parent id:" << location.parentId() << " position: " << location.position() << " length: " << location.length() ;
      return out;
    }
    
  };
  
  //! set of locations. 
  class LocationSet: public std::set<Location>
  {
    
    public:
    
    //! default constructor
    LocationSet():
      active_id_( std::make_pair( 0, 0 ) )
    {}
      
    //! active id
    const std::pair<int,int>& activeId( void ) const
    { return active_id_; }
        
    //! active id
    std::pair<int,int>& activeId( void )
    { return active_id_; }
    
    private:
    
    //! active patterns from previous and this paragraph
    std::pair<int, int> active_id_;
    
  };
  
  //! process text and returns the matching locations
  /*! locations and active parameters are changed */
  virtual void processText( LocationSet& locations, const QString& text, bool& active ) const
  {}
  
  //! used to get patterns by name
  class SameNameFTor
  {
    
    public:
    
    //! constructor    
    SameNameFTor( const std::string& name ):
      name_( name )
    {}
    
    //! predicate
    bool operator() (const HighlightPattern* pattern ) const
    { return (pattern && pattern->name() == name_); }
        
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
    bool operator() ( const HighlightPattern* pattern ) const
    { return pattern->id() == id_; }
    
    private:
    
    //! predicted id
    int id_;
    
  };
  
  protected:
          
  //! name
  virtual void _setName( const std::string& name ) 
  { name_ = name; }
  
  //! parent name
  virtual void _setParent( const std::string& parent )
  { parent_ = parent; }
      
  //!type
  const std::string& _type( void ) const
  { return type_; }
  
  //!type
  virtual void _setType( const std::string& type )
  { type_ = type; }
  
  private:
        
  //! unique id
  /*!
    The unique ID has a single bit set to 1, to use
    faster active pattern masks, with no shift operators
  */
  int id_; 
  
  //! type
  std::string type_;
      
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
  
  //!@name dumpers
  //@{  
  //! dump
  friend std::ostream& operator << (std::ostream& out, const HighlightPattern& pattern )
  {  
    out << "id: " << pattern.id() << " name: " << pattern.name() << " parent name:" << pattern.parent();
    return out;
  }
  
  //! dump
  friend std::ostream& operator << (std::ostream& out, const LocationSet& locations )
  {  
    out << "[" << locations.activeId().first << "," << locations.activeId().second << "] ";
    for( LocationSet::const_iterator iter = locations.begin(); iter != locations.end(); iter++ )
    out << *iter << std::endl;
    return out;
  }
  //@}  
};
#endif
