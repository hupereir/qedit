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
  \file TextParenthesis.cpp
  \brief Text parenthesis (for highlighting)
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/
#include <QDataStream>

#include "Debug.h"
#include "Str.h"
#include "TextParenthesis.h"
#include "XmlDef.h"
#include "XmlUtil.h"

using namespace std;

//_________________________________________________________
TextParenthesis::TextParenthesis( const QDomElement& element ):
  Counter( "TextParenthesis" )
{

  Debug::Throw( "TextParenthesis::TextParenthesis.\n" );
  
  // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    if( attribute.name() == XML::BEGIN ) first_ = XmlUtil::xmlToText(attribute.value());
    else if( attribute.name() == XML::END ) second_ = XmlUtil::xmlToText(attribute.value());
    else if( attribute.name() == XML::REGEXP ) regexp_.setPattern( XmlUtil::xmlToText( attribute.value() ) );
    else cout << "TextParenthesis::TextParenthesis - unrecognized attribute: " << qPrintable( attribute.name() ) << endl;
  }
  
  // create regexp
  if( regexp_.pattern().isEmpty() )
  {
    QString pattern = QString("(") + first() + ")|(" + second() + ")";
    regexp_.setPattern( pattern );
  }
    
}

//_____________________________________________________
QDomElement TextParenthesis::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "TextParenthesis::DomElement.\n" );
  QDomElement out( parent.createElement( XML::PARENTHESIS ) );
  
  // dump attributes
  out.setAttribute( XML::BEGIN, XmlUtil::textToXml( first() ) );
  out.setAttribute( XML::END, XmlUtil::textToXml( second() ) );
  out.setAttribute( XML::REGEXP, XmlUtil::textToXml( qPrintable( regexp().pattern() ) ) );
  return out;
}
