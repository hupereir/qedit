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
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file ParenthesisHighlight.cc
  \brief handles multiple clicks and timeout
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QTextDocument>

#include "ParenthesisHighlight.h"
#include "CustomTextEdit.h"
#include "HighlightBlockData.h"

using namespace std;

//_______________________________________________________________________
ParenthesisHighlight::ParenthesisHighlight( CustomTextEdit* parent ):
  QObject( parent ),
  Counter( "ParenthesisHighlight" ),
  parent_( parent ),
  timer_( parent ),
  enabled_( false ),
  location_( -1 ),
  cleared_( true )
{ 
  Debug::Throw( "ParenthesisHighlight::ParenthesisHighlight.\n" );
  timer_.setSingleShot( true );
  timer_.setInterval( 50 );
  connect( &timer_, SIGNAL( timeout() ), SLOT( _highlight() ) );
}

//______________________________________________________________________
void ParenthesisHighlight::clear( void )
{

  if( cleared_ ) return;
  
  // loop over all blocks
  for( QTextBlock block = parent_->document()->begin(); block.isValid(); block = block.next() )
  {
        
    // retrieve block data
    HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( !( data && data->hasParenthesis() ) ) continue;
    
    // do not clear current block if parenthesis is unchanged
    if( 
      block.contains( location_ ) && 
      data->hasParenthesis() &&
      data->parenthesis() + block.position() == location_ &&
      isEnabled() ) continue;

    // clear parenthesis
    data->clearParenthesis();
    parent_->document()->markContentsDirty(block.position(), block.length()-1);
    
  }
  
  cleared_ = true;

}

//______________________________________________________________________
void ParenthesisHighlight::highlight( const int& location )
{
  if( !isEnabled() ) return;
  clear();
  location_ = location;
  _highlight();
}

//______________________________________________________________________
void ParenthesisHighlight::_highlight( void )
{
  
  // retrieve block matching location
  QTextBlock block( parent_->document()->findBlock( location_ ) );  
  if( !block.isValid() ) return;
  
  HighlightBlockData* data = dynamic_cast<HighlightBlockData*>( block.userData() );
  if( !data )
  {
    // try retrieve data from parent type
    TextBlockData* text_data = dynamic_cast<TextBlockData*>( block.userData() );
    data = text_data ? new HighlightBlockData( text_data ) : new HighlightBlockData();
    block.setUserData( data );
    
  } else if( data->hasParenthesis() && data->parenthesis() + block.position() == location_ ) return;
  
  // update parenthesis
  data->setParenthesis( location_ - block.position() );
  parent_->document()->markContentsDirty( location_, 1 );
  cleared_ = false;
  
  // reset location
  location_ = -1;
}
