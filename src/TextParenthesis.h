#ifndef _TextParenthesis_h_
#define _TextParenthesis_h_

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
  \file TextParenthesis.h
  \brief Text parenthesis (for highlighting)
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDomElement>
#include <QDomDocument>
#include <QRegExp>
#include <QChar>

#include <list>
#include <map>
#include <set>

#include "Counter.h"
#include "Debug.h"
#include "Str.h"

//! text parenthesis (for highlighting)
class TextParenthesis: public std::pair<QChar,QChar>, public Counter
{

  public:

  //! list of parenthesis
  typedef std::list<TextParenthesis> List;
  
  //! set of parenthesis
  typedef std::set<QChar> Set;
  
  //! constructor from DomElement
  TextParenthesis( const QDomElement& element = QDomElement() );

  //! dom element
  QDomElement domElement( QDomDocument& parent ) const;
  
  //! valid (begin and end brace must be different)
  bool isValid( void ) const
  { return first != second; }
  
  //! regExp that match either of the two parenthesis
  const QRegExp& regexp() const
  { return regexp_; }
  
  //! used to find parenthesis for which first character match
  class FirstElementFTor
  {
    public:
    
    //! constructor
    FirstElementFTor( const QChar& c ):
      c_( c )
    {}
    
    //! predicate
    bool operator() ( const TextParenthesis& parenthesis ) const
    { return parenthesis.first == c_; }
    
    private:

    //! predicted character
    QChar c_;
  
  };
  
  //! used to find parenthesis for which second character match
  class SecondElementFTor
  {
    public:
    
    //! constructor
    SecondElementFTor( const QChar& c ):
      c_( c )
    {}
    
    //! predicate
    bool operator() ( const TextParenthesis& parenthesis ) const
    { return parenthesis.second == c_; }
    
    private:

    //! predicted character
    QChar c_;
  
  };
  
  private:
  
  //! regular expression that match either of both characters
  QRegExp regexp_;
  
};
#endif
