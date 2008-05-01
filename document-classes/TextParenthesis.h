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

#include <vector>
#include <map>
#include <set>

#include "Counter.h"
#include "Debug.h"
#include "Str.h"

//! text parenthesis (for highlighting)
class TextParenthesis: public Counter
{

  public:

  //! list of parenthesis
  typedef std::vector<TextParenthesis> List;
  
  //! set of parenthesis
  typedef std::set<TextParenthesis> Set;
  
  //! constructor from DomElement
  TextParenthesis( const QDomElement& element = QDomElement() );

  //! dom element
  QDomElement domElement( QDomDocument& parent ) const;

  //! regExp that match either of the two parenthesis
  const QRegExp& first() const
  { return first_; }

  //! regExp that match either of the two parenthesis
  const QRegExp& second() const
  { return second_; }

  //! regExp that match either of the two parenthesis
  const QRegExp& regexp() const
  { return regexp_; }
  
  //! used to find parenthesis for which first character match
  class MatchFTor
  {
    public:
    
    //! constructor
    MatchFTor( const QString& text, const int& position ):
      text_( text ),
      position_( position )
    {}
    
    //! predicate
    bool operator() ( const TextParenthesis& parenthesis ) const
    { return text_.indexOf( parenthesis.regexp(), position_ ) == position_; }
    
    private:

    //! predicted character
    const QString& text_;
    
    //! predicted position
    const int& position_;
  
  };
  
  //! used to find parenthesis for which first character match
  class FirstElementFTor
  {
    public:
    
    //! constructor
    FirstElementFTor( const QString& text, const int& position ):
      text_( text ),
      position_( position )
    {}
    
    //! predicate
    bool operator() ( const TextParenthesis& parenthesis ) const
    { return text_.indexOf( parenthesis.first(), position_ ) == position_; }
    
    private:

    //! predicted character
    const QString& text_;
    
    //! predicted position
    const int& position_;
  
  };
  
  //! used to find parenthesis for which first character match
  class SecondElementFTor
  {
    public:
    
    //! constructor
    SecondElementFTor( const QString& text, const int& position ):
      text_( text ),
      position_( position )
    {}
    
    //! predicate
    bool operator() ( const TextParenthesis& parenthesis ) const
    { return text_.indexOf( parenthesis.second(), position_ ) == position_; }
    
    private:

    //! predicted character
    const QString& text_;
    
    //! predicted position
    const int& position_;
  
  };
  
  private:
  
  //! regular expression that match first character
  QRegExp first_;
  
  //! regular expression that match second character
  QRegExp second_;
  
  //! regular expression that match either of both characters
  QRegExp regexp_;
  
};
#endif
