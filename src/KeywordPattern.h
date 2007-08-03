#ifndef _KeywordPattern_h_
#define _KeywordPattern_h_
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
  \file KeywordPattern.h
  \brief single keyword syntax highlighting
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <set>
#include <string>

#include "Counter.h"
#include "HighlightPattern.h"

//! Base class for syntax highlighting
class KeywordPattern: public HighlightPattern
{
  
  public: 
  
  //! constructor from DomElement
  KeywordPattern( const QDomElement& element = QDomElement() );
  
  //! dom element
  virtual QDomElement domElement( QDomDocument& parent ) const;

  //! keyword
  virtual void setKeyword( const std::string& keyword )
  { keyword_.setPattern( keyword.c_str() ); }
  
  //! keyword
  virtual const QRegExp& keyword( void ) const
  { return keyword_; }
  
  //! validity
  virtual bool isValid( void ) const
  { return keyword_.isValid(); }

  //! process text and returns the matching locations
  virtual void processText( LocationSet& locations, const QString& text, bool& active ) const;
  
  private:
  
  QRegExp keyword_;
    
};
#endif
