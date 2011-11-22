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
   \file    FileSystemHistory.cpp
   \brief   Handles directory navigation history
   \author  Hugo Pereira
   \version $Revision$
   \date    $Date$
*/

#include "Debug.h"
#include "FileSystemHistory.h"



//__________________________________________________________________
void FileSystemHistory::add( File path )
{

  Debug::Throw( "FileSystemHistory::add.\n" );

  // make sure path is different from current
  if( !path_list_.empty() && path_list_[index_] == path ) return;

  // remove everything that is after index_ from path_list_
  while( path_list_.size() > index_+1 ) path_list_.pop_back();

  // insert new path if different from current
  path_list_.push_back( path );
  index_ = path_list_.size()-1;
  return;

}
