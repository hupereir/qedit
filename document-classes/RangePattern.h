#ifndef _RangePattern_h_
#define _RangePattern_h_
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
  \file RangePattern.h
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
class RangePattern: public HighlightPattern
{
  
  public: 
  
  //! constructor from DomElement
  RangePattern( const QDomElement& element = QDomElement() );
  
  //! dom element
  QDomElement domElement( QDomDocument& parent ) const;
  
  //! keyword
  virtual void setBegin( const std::string& keyword )
  { begin_.setPattern( keyword.c_str() ); }
  
  //! keyword
  virtual const QRegExp& begin( void ) const
  { return begin_; }
  
  //! keyword
  virtual void setEnd( const std::string& keyword )
  { end_.setPattern( keyword.c_str() ); }
  
  //! keyword
  virtual const QRegExp& end( void ) const
  { return end_; }
  
  //! validity
  virtual bool isValid( void ) const
  { return begin().isValid() && end().isValid(); }
  
  //! process text and returns the matching locations
  virtual void processText( LocationSet& locations, const QString& text, bool& active ) const;
  
  private:
  
  //! regular expression used to define begin of the range 
  QRegExp begin_;
  
  //! regular expression used to define end of the range 
  QRegExp end_;
    
};
#endif
