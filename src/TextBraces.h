#ifndef _TextBraces_h_
#define _TextBraces_h_

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
  \file TextBraces.h
  \brief Text braces (for highlighting)
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

//! text braces (for highlighting)
class TextBraces: public std::pair<QChar,QChar>, public Counter
{

  public:

  //! list of braces
  typedef std::list<TextBraces> List;
  
  //! set of braces
  typedef std::set<QChar> Set;
  
  //! constructor from DomElement
  TextBraces( const QDomElement& element = QDomElement() );

  //! dom element
  QDomElement domElement( QDomDocument& parent ) const;
  
  //! valid (begin and end brace must be different)
  bool isValid( void ) const
  { return first != second; }
  
  //! regExp that match either of the two braces
  const QRegExp& regexp() const
  { return regexp_; }
  
  //! used to find braces for which first character match
  class FirstElementFTor
  {
    public:
    
    //! constructor
    FirstElementFTor( const QChar& c ):
      c_( c )
    {}
    
    //! predicate
    bool operator() ( const TextBraces& braces ) const
    { return braces.first == c_; }
    
    private:

    //! predicted character
    QChar c_;
  
  };
  
  //! used to find braces for which second character match
  class LastElementFTor
  {
    public:
    
    //! constructor
    LastElementFTor( const QChar& c ):
      c_( c )
    {}
    
    //! predicate
    bool operator() ( const TextBraces& braces ) const
    { return braces.second == c_; }
    
    private:

    //! predicted character
    QChar c_;
  
  };
  
  private:
  
  //! regular expression that match either of both characters
  QRegExp regexp_;
  
};
#endif
