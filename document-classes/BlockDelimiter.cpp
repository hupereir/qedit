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
  \file BlockDelimiter.cpp
  \brief Text parenthesis (for highlighting)
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/
#include <QDataStream>

#include "Debug.h"
#include "Str.h"
#include "BlockDelimiter.h"
#include "XmlDef.h"
#include "XmlUtil.h"

using namespace std;

//_________________________________________________________
BlockDelimiter::BlockDelimiter( const QDomElement& element ):
  Counter( "BlockDelimiter" )
{

  Debug::Throw( "BlockDelimiter::BlockDelimiter.\n" );
  
  // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    string name( qPrintable( attribute.name() ) );
    string value( qPrintable( attribute.value() ) );
    if( name == XML::BEGIN ) first = Str( XmlUtil::xmlToText(value) ).get<char>();
    else if( name == XML::END ) second = Str( XmlUtil::xmlToText(value) ).get<char>();
    else if( name == XML::REGEXP ) regexp_.setPattern( XmlUtil::xmlToText( value ).c_str() );
    else cout << "BlockDelimiter::BlockDelimiter - unrecognized attribute: " << name << endl;
  }
  
  // create regexp
  if( regexp_.pattern().isEmpty() )
  {
    ostringstream what;
    what << first.toAscii() << "|" << second.toAscii();
    regexp_.setPattern( what.str().c_str() );
  }
    
}


//_____________________________________________________
QDomElement BlockDelimiter::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "BlockDelimiter::DomElement.\n" );
  QDomElement out( parent.createElement( XML::PARENTHESIS.c_str() ) );
  
  // dump attributes
  out.setAttribute( XML::BEGIN.c_str(), XmlUtil::textToXml( Str().assign<char>(first.toAscii()) ).c_str() );
  out.setAttribute( XML::END.c_str(), XmlUtil::textToXml( Str().assign<char>(second.toAscii()) ).c_str() );
  out.setAttribute( XML::REGEXP.c_str(), XmlUtil::textToXml( qPrintable( regexp().pattern() ) ).c_str() );
  return out;
}