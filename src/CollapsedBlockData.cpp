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
  \file CollapsedBlockData.cpp
  \brief stores collapsed block informations
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "CollapsedBlockData.h"
#include "HighlightBlockData.h"
using namespace std;

//_____________________________________________________________
CollapsedBlockData::CollapsedBlockData( const QTextBlock& block ):
  Counter( "CollapsedBlockData" ),
  text_( block.text() )
{
  
  HighlightBlockData* data( (dynamic_cast<HighlightBlockData*>( block.userData() ) ) );
  if( data )
  {
    
    delimiters_ = data->delimiters();
    collapsed_ = data->collapsed();
    
    // only store children if block is collapsed
    if( data->collapsed() ) { children_ = data->collapsedData(); }
    
  }
  
  // would need to retrieve the "children" data from the block format rather that from the HighlightBlockData
  
}

//_____________________________________________________________
unsigned int CollapsedBlockData::blockCount( void ) const
{
  unsigned int out(1);
  for( List::const_iterator iter = children().begin(); iter != children().end(); iter++ )
  { out += iter->blockCount(); }
  
  return out;
}

//_____________________________________________________________
QString CollapsedBlockData::toPlainText( void ) const
{
  
  QString out( text() + "\n" );
  for( List::const_iterator iter = children().begin(); iter != children().end(); iter++ )
  { out += iter->toPlainText(); }
  
  return out;
  
}
