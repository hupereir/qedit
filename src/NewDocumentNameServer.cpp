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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software , Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file NewDocumentNameServer.cpp
  \brief keep track of new document names (prior to being first saved)
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <algorithm>
#include <sstream>

#include "NewDocumentNameServer.h"

using namespace std;

//_____________________________________
const string NewDocumentNameServer::default_name_ = "new document";

//______________________________________
string NewDocumentNameServer::get( void )
{
  
  unsigned int version( versions_.empty() ? 0: versions_.back()+1 );
  string out( _get( version ) );
  versions_.push_back( version );
  return out;

}

//______________________________________
void NewDocumentNameServer::remove( string name )
{ versions_.erase( remove_if( versions_.begin(), versions_.end(), SameVersionFTor( name ) ), versions_.end() ); }

//______________________________________
string NewDocumentNameServer::_get( const unsigned int& version )
{

  ostringstream what;
  what << default_name_;
  if( version > 0 ) { what << " (" << version+1 << ")"; }
  return what.str();
  
}
