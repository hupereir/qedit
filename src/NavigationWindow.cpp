
/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later-
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
   \file NavigationWindow.cpp
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>
#include <QPainter>
#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <QLayout>

#include "CustomToolButton.h"
#include "Debug.h"
#include "NavigationWindow.h"
#include "TreeView.h"

using namespace std;

//_______________________________________________________________
NavigationWindow::NavigationWindow( QWidget* parent ):
  CustomMainWindow( parent ),
  Counter( "MainWindow" )
{
  
  Debug::Throw( "NavigationWindow:NavigationWindow.\n" );
  
  QWidget* main( new QWidget( this ) );
  setCentralWidget( main );
  
  // add horizontal layout for toolbar and stacked widget
  QHBoxLayout *h_layout = new QHBoxLayout();
  h_layout->setSpacing(2);
  h_layout->setMargin(2);
  main->setLayout( h_layout );
  
  // add vertical layout for toolbar buttons
  QVBoxLayout *v_layout = new QVBoxLayout();
  v_layout->setSpacing(2);
  v_layout->setMargin(2);
  h_layout->addLayout( v_layout, 0 );
    
  // create stack widget
  h_layout->addWidget( stack_ = new QStackedWidget( this ) );
  
  // create session files tree view
  session_files_list_ = new TreeView(0);  
  sessionFilesList().setModel( &session_files_model_ );  
  sessionFilesList().setMask( (1<<FileRecordModel::FILE) );
  stack_->addWidget( &sessionFilesList() );
  
  // create recent files tree view
  recent_files_list_ = new TreeView(0);  
  recentFilesList().setModel( &recent_files_model_ );  
  recentFilesList().setMask( (1<<FileRecordModel::FILE) );
  stack_->addWidget( &recentFilesList() );
  
  // create button group
  QButtonGroup* button_group = new QButtonGroup( this );
  connect( button_group, SIGNAL( buttonClicked( QAbstractButton* ) ), SLOT( _display( QAbstractButton* ) ) );
  button_group->setExclusive( true );

  // create matching buttons
  CustomToolButton* button;
  
  // session files
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->setChecked( true );
  button->setRotation( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&Session files" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &sessionFilesList() ) );
  
  // recent files
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->setRotation( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&Recent files" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &recentFilesList() ) );
  
  v_layout->addStretch( 1 );
}

//______________________________________________________________________
NavigationWindow::~NavigationWindow( void )
{ Debug::Throw( "NavigationWindow::~NavigationWindow.\n" ); }

//______________________________________________________________________
void NavigationWindow::update( void )
{ Debug::Throw( "NavigationWindow:update.\n" ); }

//______________________________________________________________________
void NavigationWindow::_display( QAbstractButton* button )
{ 
  Debug::Throw( "NavigationWindow:_display.\n" ); 
  if( !button->isChecked() ) return;
  
  // retrieve item in map
  ButtonMap::const_iterator iter( buttons_.find( button ) );
  assert( iter != buttons_.end() );

  // display corresponding widget
  _stack().setCurrentWidget( iter->second );  

}
