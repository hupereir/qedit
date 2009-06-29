
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
   \file DocumentClassToolBar.h
   \brief editor main window
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLabel>

#include "Application.h"
#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "DocumentClassToolBar.h"
#include "Singleton.h"

using namespace std;

//________________________________________________________________
DocumentClassToolBar::DocumentClassToolBar( QWidget* parent ):
  CustomToolBar( "Document Class", parent, "DOCUMENT_CLASS_TOOLBAR" )
{
  
  Debug::Throw( "DocumentClassToolBar::DocumentClassToolBar.\n" );
  addWidget( new QLabel( " Document class: ", this ) );
  addWidget( combobox_ = new QComboBox( this ) );

  connect( Singleton::get().application(), SIGNAL( documentClassesChanged() ), SLOT( update() ) );
    
}

//________________________________________________________________
void DocumentClassToolBar::update( QString class_name )
{
  Debug::Throw( "DocumentClassToolBar::update.\n" );
  _comboBox().setCurrentIndex( _comboBox().findText( class_name ) );
}

//________________________________________________________________
void DocumentClassToolBar::update( void )
{
  Debug::Throw( "DocumentClassToolBar::update.\n" );
  
  // store current item
  QString current_class( _comboBox().currentText() );
  
  // clear box
  _comboBox().clear();
  
  // add all document classes
  const DocumentClassManager &manager( Singleton::get().application<Application>()->classManager() );
  const DocumentClassManager::List& classes( manager.list() );
  for( DocumentClassManager::List::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { _comboBox().addItem( iter->name() ); }
  
  // try select old class if possible
  _comboBox().setCurrentIndex( _comboBox().findText( current_class ) );
  
}
