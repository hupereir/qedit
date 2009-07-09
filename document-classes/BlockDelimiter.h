#ifndef _BlockDelimiter_h_
#define _BlockDelimiter_h_

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
  \file BlockDelimiter.h
  \brief Text delimiter (for highlighting)
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDomElement>
#include <QDomDocument>
#include <QRegExp>

#include <vector>

#include "Counter.h"
#include "Debug.h"
#include "Str.h"

//! text delimiter (for highlighting)
class BlockDelimiter: public Counter
{

  public:

  //! typedef for list of patterns
  typedef std::vector< BlockDelimiter > List;

  //! constructor from DomElement
  BlockDelimiter( const QDomElement& element = QDomElement(), const unsigned int& id = 0 );

  //! dom element
  QDomElement domElement( QDomDocument& parent ) const;
     
  //! Id
  const unsigned int& id( void ) const
  { return id_; }

  //! equal to operator
  bool operator == ( const BlockDelimiter& other ) const
  { 
    return 
      first() == other.first() &&
      second() == other.second() &&
      regexp() == other.regexp();
  }
  
  //! less than operator
  bool operator < ( const BlockDelimiter& other ) const
  { 
    if( first() != other.first() ) return first() < other.first();
    if( second() != other.second() ) return second() < other.second();
    if( regexp().pattern() != other.regexp().pattern() ) return regexp().pattern() < other.regexp().pattern();
    return false;
  }
  
  //! block start
  const QString& first() const
  { return first_; }

  //! first
  void setFirst( const QString& value )
  { first_ = value; }

  //! block end
  const QString& second() const
  { return second_; }
  
  //! second
  void setSecond( const QString& value )
  { second_ = value; }

  //! regExp that match either block start or end
  const QRegExp& regexp() const
  { return regexp_; }
  
  //! regext
  void setRegexp( const QString& value )
  { regexp_.setPattern( value ); }

    
  private:
    
  //! unique id
  unsigned int id_; 
  
  //! regular expression that match first character
  QString first_;
  
  //! regular expression that match second character
  QString second_;
  
  //! regular expression that match either of both characters
  QRegExp regexp_;
  
  //! streamer
  friend QTextStream& operator << ( QTextStream& out, const BlockDelimiter& delimiter )
  {
    out << " first: " << delimiter.first()
      << " second: " << delimiter.second() 
      << " regexp: " << delimiter.regexp().pattern();
    return out;
  }
  
};
#endif
