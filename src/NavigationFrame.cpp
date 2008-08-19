
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
   \file NavigationFrame.cpp
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>
#include <QHeaderView>
#include <QLayout>

#include "Application.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "FileSystemFrame.h"
#include "NavigationFrame.h"
#include "RecentFilesFrame.h"
#include "SessionFilesFrame.h"

using namespace std;

//_______________________________________________________________
NavigationFrame::NavigationFrame( QWidget* parent, FileList& files ):
  QWidget( parent ),
  Counter( "NavigationFrame" ),
  default_width_( -1 )
{
  
  Debug::Throw( "NavigationFrame:NavigationFrame.\n" );

  // actions
  _installActions();
  
  // add horizontal layout for toolbar and stacked widget
  QHBoxLayout *h_layout = new QHBoxLayout();
  h_layout->setSpacing(2);
  h_layout->setMargin(0);
  setLayout( h_layout );
  
  // add vertical layout for toolbar buttons
  QVBoxLayout *v_layout = new QVBoxLayout();
  v_layout->setSpacing(2);
  v_layout->setMargin(0);
  h_layout->addLayout( v_layout, 0 );
    
  // stack widget
  h_layout->addWidget( stack_ = new QStackedWidget( this ) );
  
  _stack().addWidget( session_files_frame_ = new SessionFilesFrame(0) );  
  _stack().addWidget( recent_files_frame_ = new RecentFilesFrame(0, files) );  
  _stack().addWidget( file_system_frame_ = new FileSystemFrame(0) );
  
  // connections
  connect( &_sessionFilesFrame(), SIGNAL( fileSelected( FileRecord ) ), SIGNAL( fileSelected( FileRecord ) ) );
  connect( &_sessionFilesFrame(), SIGNAL( fileActivated( FileRecord ) ), SIGNAL( fileActivated( FileRecord ) ) );
  connect( &_recentFilesFrame(), SIGNAL( fileActivated( FileRecord ) ), SIGNAL( fileActivated( FileRecord ) ) );
  connect( &_fileSystemFrame(), SIGNAL( fileActivated( FileRecord ) ), SIGNAL( fileActivated( FileRecord ) ) );
  
  // button group
  QButtonGroup* button_group = new QButtonGroup( this );
  connect( button_group, SIGNAL( buttonClicked( QAbstractButton* ) ), SLOT( _display( QAbstractButton* ) ) );
  button_group->setExclusive( true );

  // matching buttons
  CustomToolButton* button;
  
  // session files
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->setChecked( true );
  button->rotate( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&Session files" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_sessionFilesFrame() ) );
  
  // recent files
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->rotate( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&Recent files" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_recentFilesFrame() ) );

  // file system
  v_layout->addWidget( button = new CustomToolButton( this ) );
  button->setCheckable( true );
  button->rotate( CustomToolButton::COUNTERCLOCKWISE );
  button->setText( "&File system" );
  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_fileSystemFrame() ) );
  v_layout->addStretch( 1 );
    
}

//______________________________________________________________________
NavigationFrame::~NavigationFrame( void )
{ Debug::Throw( "NavigationFrame::~NavigationFrame.\n" ); }

//______________________________________________________________________
void NavigationFrame::setDefaultWidth( const int& value )
{ default_width_ = value; }

//______________________________________________________________________
QAction& NavigationFrame::updateSessionFilesAction( void ) const
{ return _sessionFilesFrame().updateAction(); }

//______________________________________________________________________
QAction& NavigationFrame::updateRecentFilesAction( void ) const
{ return _recentFilesFrame().updateAction(); }

//____________________________________________
QSize NavigationFrame::sizeHint( void ) const
{ return (default_width_ ) >= 0 ? QSize( default_width_, 0 ):QWidget::sizeHint(); }

//______________________________________________________________________
void NavigationFrame::_display( QAbstractButton* button )
{  
  
  Debug::Throw( "NavigationFrame:_display.\n" ); 
  if( !button->isChecked() ) return;
  
  // retrieve item in map
  ButtonMap::const_iterator iter( buttons_.find( button ) );
  assert( iter != buttons_.end() );

  // display corresponding widget
  _stack().setCurrentWidget( iter->second );
  
}

//______________________________________________________________________
void NavigationFrame::_installActions( void )
{
  
  Debug::Throw( "NavigationFrame::_installActions.\n" );
  addAction( visibility_action_ = new QAction( "Show &navigation panel", this ) );
  visibility_action_->setCheckable( true );
  visibility_action_->setChecked( true );
  visibility_action_->setShortcut( Qt::Key_F5 );
  connect( visibility_action_, SIGNAL( toggled( bool ) ), SLOT( setVisible( bool ) ) );
    
}
