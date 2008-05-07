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
  \file TextView.cpp
  \brief compound widget to associate TextDisplay and LineNumberWidget
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>

#include "BlockDelimiterWidget.h"
#include "LineNumberWidget.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TextView.h"
#include "XmlOptions.h"

using namespace std;

//__________________________________________________________________
TextView::TextView( QWidget* parent ):
  QFrame( parent ),
  Counter( "TextView" )
{
  
  Debug::Throw( "TextView::TextView.\n" );
  
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout( layout );
  
  setFrameStyle( QFrame::StyledPanel | QFrame::NoFrame );
  
  editor_ = new TextDisplay( this );
  editor_->setFrameStyle( QFrame::NoFrame );
  
  line_number_widget_ = new LineNumberWidget( &editor(), this );
  block_delimiter_widget_ = new BlockDelimiterWidget( &editor(), this );
  
  layout->addWidget( &blockDelimiterWidget(), 0 );
  layout->addWidget( &lineNumberWidget(), 0 );
  layout->addWidget( &editor(), 1 );
  
  // connections
  connect( &editor(), SIGNAL( blockDelimitersAvailable( BlockDelimiter::List ) ), SLOT( _loadBlockDelimiters( BlockDelimiter::List ) ) );
  connect( &editor().showBlockDelimiterAction(), SIGNAL( toggled( bool ) ), SLOT( _toggleShowBlockDelimiters( bool ) ) );
  connect( &editor().showLineNumberAction(), SIGNAL( toggled( bool ) ), SLOT( _toggleShowLineNumbers( bool ) ) );

  _loadBlockDelimiters( BlockDelimiter::List() );
  _toggleShowBlockDelimiters( editor().showBlockDelimiterAction().isChecked() );
  _toggleShowLineNumbers( editor().showLineNumberAction().isChecked() );
    
}

//___________________________________________
void TextView::synchronize( const TextView* view )
{

  Debug::Throw( "TextView::synchronize.\n" );
  editor().synchronize( &view->editor() );
  blockDelimiterWidget().synchronize( &view->blockDelimiterWidget() );
  lineNumberWidget().synchronize( &view->lineNumberWidget() );
  
}

//________________________________________________
void TextView::contextMenuEvent( QContextMenuEvent* event )
{
  Debug::Throw( "TextView::contextMenuEvent.\n" );
 
  blockDelimiterWidget().updateCurrentBlockActionState();

  QMenu menu( this );
  editor().installContextMenuActions( menu );
  
  if( !blockDelimiterWidget().isHidden() )
  {
    menu.addSeparator();
    menu.addAction( &blockDelimiterWidget().collapseCurrentAction() );
    menu.addAction( &blockDelimiterWidget().expandCurrentAction() );
  }
  
  menu.exec( event->globalPos() );
  
  return;
}

//___________________________________________
void TextView::_toggleShowLineNumbers( bool state )
{
  Debug::Throw( "TextView::_toggleShowLineNumbers.\n" );
  lineNumberWidget().setVisible( state );

  // update option
  XmlOptions::get().set<bool>( "SHOW_LINE_NUMBERS", state );

}

//___________________________________________
void TextView::_loadBlockDelimiters( BlockDelimiter::List delimiters )
{
  
  Debug::Throw( "TextView::_enableBlockDelimiters.\n" );

  // expand all collapsed blocks prior to changing the delimiters
  if( !blockDelimiterWidget().isHidden() && blockDelimiterWidget().expandAllAction().isEnabled() )   
  { blockDelimiterWidget().expandAllAction().trigger(); } 
  
  // update block delimiters
  blockDelimiterWidget().setBlockDelimiters( delimiters );

  // update widget visibility
  bool visible( editor().showBlockDelimiterAction().isChecked() && !delimiters.empty() );
  blockDelimiterWidget().setVisible( visible );
  blockDelimiterWidget().setActionVisibility( visible );
  editor().textHighlight().setBlockDelimitersEnabled( visible );
  
}

//___________________________________________
void TextView::_toggleShowBlockDelimiters( bool state )
{
  Debug::Throw( "TextView::_toggleShowBlockDelimiters.\n" );
  
  // check if blockDelimiter is allowed to be shown/hidden
  if( editor().showBlockDelimiterAction().isVisible() ) 
  {
    
    // expand all collapsed blocks prior to hiding the widget    
    if( !state && blockDelimiterWidget().expandAllAction().isEnabled() ) 
    { blockDelimiterWidget().expandAllAction().trigger(); }
    
    // update visibility
    blockDelimiterWidget().setVisible( state );
    blockDelimiterWidget().setActionVisibility( state );
    
    // update text highlight object
    editor().textHighlight().setBlockDelimitersEnabled( 
      (!blockDelimiterWidget().blockDelimiters().empty() ) &&
      state );
    
    // update option
    XmlOptions::get().set<bool>( "SHOW_BLOCK_DELIMITERS", state );
    
  }
  
}
