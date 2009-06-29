
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
****************************************************************************/

/*!
   \file DocumentClassMenu.cpp
   \brief display available document classes
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include "Application.h"
#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "DocumentClassMenu.h"
#include "IconEngine.h"
#include "MainWindow.h"
#include "Singleton.h"
#include "TextDisplay.h"

using namespace std;

//_____________________________________________
DocumentClassMenu::DocumentClassMenu( QWidget* parent ):
  QMenu( parent ),
  Counter( "DocumentClassMenu" )
{
  Debug::Throw( "DocumentClassMenu::DocumentClassMenu.\n" );
  action_group_ = new QActionGroup( this );
  connect( this, SIGNAL( aboutToShow() ), SLOT( _update() ) );
  connect( this, SIGNAL( triggered( QAction* ) ), SLOT( _selectClassName( QAction* ) ) );
}

//_____________________________________________
void DocumentClassMenu::_update( void )
{
  Debug::Throw( "DocumentClassMenu::_update.\n" );
  
  // clear menu
  clear();
  actions_.clear();
  
  // retrieve current class from MainWindow
  MainWindow& window( *static_cast<MainWindow*>( DocumentClassMenu::window()) ); 
  const QString& class_name( window.activeDisplay().className() );
    
  // retrieve classes from DocumentClass manager
  const DocumentClassManager::List& classes( Singleton::get().application<Application>()->classManager().list() );
  for( DocumentClassManager::List::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { 
    // insert actions
    QAction* action = addAction( iter->name() );
    if( !iter->icon().isEmpty() ) action->setIcon( IconEngine::get( iter->icon() ) );
    
    action->setCheckable( true );
    action->setChecked( iter->name() == class_name );
    action_group_->addAction( action );
    
    actions_.insert( make_pair( action, iter->name() ) );    
  
  }
 
}

//_____________________________________________
void DocumentClassMenu::_selectClassName( QAction* action )
{
  Debug::Throw( "DocumentClassMenu::_selectClassName.\n" );
  std::map< QAction*, QString >::iterator iter = actions_.find( action );
  if( iter != actions_.end() ) 
  { emit documentClassSelected( iter->second ); }
  
  return;
  
}
