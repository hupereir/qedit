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

#include "NewDocumentNameServer.h"
#include <QTextStream>
#include <algorithm>

//_____________________________________
const QString NewDocumentNameServer::defaultName_ = "new document";

//______________________________________
QString NewDocumentNameServer::get( void )
{

    int version( versions_.empty() ? 0: versions_.back()+1 );
    QString out( _get( version ) );
    versions_ << version;
    return out;

}

//______________________________________
void NewDocumentNameServer::remove( QString name )
{ versions_.erase( std::remove_if( versions_.begin(), versions_.end(), SameVersionFTor( name ) ), versions_.end() ); }

//______________________________________
QString NewDocumentNameServer::_get( int version )
{

    QString buffer;
    QTextStream what ( &buffer );
    what << defaultName_;
    if( version > 0 ) { what << " (" << version+1 << ")"; }
    return buffer;

}
