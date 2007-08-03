#ifndef _HighlightStyle_h_
#define _HighlightStyle_h_
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
  \file HighlightStyle.h
  \brief Base class for syntax highlighting style
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QColor>
#include <QFont>
#include <QDomDocument>
#include <QDomElement>

#include <list>
#include <string>

#include "Counter.h"
#include "TextFormat.h"

//! Base class for syntax highlighting
class HighlightStyle: public Counter
{
  
  public: 
            
  //! constructor
  HighlightStyle( 
    const std::string& name = "default",
    const unsigned int& format = FORMAT::DEFAULT,
    const QColor& color = Qt::black 
  ):
    Counter( "HighlightStyle" ),
    name_( name ),
    format_( format ),
    color_( color )
  {}
  
  //! constructor from DomElement
  HighlightStyle( const QDomElement& element );
  
  //! write to DomElement
  QDomElement domElement( QDomDocument& parent ) const;
  
  //! name
  virtual const std::string& name( void ) const
  { return name_; }
  
  //! lower than operator
  bool operator < (const HighlightStyle& style ) const
  { return name() < style.name(); }
  
  //! equal to operator
  bool operator == (const HighlightStyle& style ) const
  { return name() == style.name(); }
  
  //! different from operator
  bool operator != (const HighlightStyle& style ) const
  { return name() != style.name(); }
  
  //! name
  virtual void setName( const std::string& name ) 
  { name_ = name; }
  
  //! format
  virtual const unsigned int& format( void ) const
  { return format_; }
  
  //! format 
  virtual void setFormat( const unsigned int format )
  { format_ = format; }
  
  //! fond
  virtual QFont font( const QFont& font ) const
  {
    QFont out( font );
    out.setBold( format()&FORMAT::BOLD );
    out.setItalic( format()&FORMAT::ITALIC );
    out.setUnderline( format()&FORMAT::UNDERLINE );
    out.setStrikeOut( format()&FORMAT::STRIKE );
    return out;
  }
  
  //! color
  virtual const QColor& color( void ) const
  { return color_; }
  
  //! color
  virtual void setColor( const QColor& color ) 
  { color_ = color; }
  
  private:
      
  //! pattern name
  std::string name_;
  
  //! format (bitwise or of TextFormatInfo)
  unsigned int format_;  
      
  //! color
  QColor color_;
  
};
#endif
