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
**********************************************************************************/

/*!
    \file HtmlUtil.cpp
    \brief Some Html utilities
    \author Hugo Pereira
    \version $Revision$
    \date $Date$
*/

#include <QStringList>

#include "HtmlUtil.h"
#include "Debug.h"

using namespace std;

//_________________________________________
void HtmlUtil::textNode( const QString& text, QDomElement& parent, QDomDocument& document )
{
  
  Debug::Throw( "HtmlUtil::TextNode.\n" );
  QStringList lines( text.split( '\n' ) );
  if( lines.empty() ) return;

  // loop over lines
  
  // need to convert line accents
  
  // first append the first line
  QStringList::iterator iter = lines.begin(); 
  parent.appendChild( document.createTextNode( *iter ) );
  iter++;
  
  // append remaining lines, after end of line tag
  for( ;iter != lines.end(); iter++ )
  {
    parent.appendChild( document.createElement( "br" ) );
    parent.appendChild( document.createTextNode( *iter ) );
  }
  
  Debug::Throw( "HtmlUtil::TextNode - done.\n" );
  return;
  
}
