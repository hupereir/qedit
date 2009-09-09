
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
   \file NavigationToolBar.cpp
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>

#include "CustomPixmap.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "FileSystemFrame.h"
#include "IconEngine.h"
#include "Icons.h"
#include "IconSizeMenu.h"
#include "MainWindow.h"
#include "NavigationToolBar.h"
#include "NavigationFrame.h"
#include "QtUtil.h"
#include "RecentFilesFrame.h"
#include "SessionFilesFrame.h"
#include "ToolBarMenu.h"
#include "ToolButtonStyleMenu.h"


using namespace std;

//_______________________________________________________________
NavigationToolBar::NavigationToolBar( QWidget* parent ):
  CustomToolBar( "Side bar", parent, "NAVIGATION_SIDEBAR" ),
  navigation_frame_( 0 ),
  enabled_( true )
{
  Debug::Throw( "NavigationToolBar:NavigationToolBar.\n" );
  CustomToolBar::connect( this, SIGNAL( orientationChanged( Qt::Orientation ) ), SLOT( _orientationChanged( Qt::Orientation ) ) );
}

//_______________________________________________________________
void NavigationToolBar::connect( NavigationFrame& frame )
{

  Debug::Throw( "NavigationToolBar::connect.\n" );
  assert( !navigation_frame_ );
  navigation_frame_ = &frame;

  CustomToolBar::connect( &_navigationFrame().visibilityAction(), SIGNAL( toggled( bool ) ), SLOT( _navigationFrameVisibilityChanged( bool ) ) );

  // button group
  QButtonGroup* button_group = new QButtonGroup( this );
  CustomToolBar::connect( button_group, SIGNAL( buttonClicked( QAbstractButton* ) ), SLOT( _display( QAbstractButton* ) ) );
  button_group->setExclusive( false );

  // matching buttons
  CustomToolButton* button;

  // session files
  addWidget( button = _newToolButton( this,  IconEngine::get( ICONS::DOCUMENTS ) ) );
  button->setText( " &Session Files" );
  button->setToolTip( "Files currently opened" );
  button->setChecked( true );

  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_navigationFrame().sessionFilesFrame() ) );

  // recent files
  addWidget( button = _newToolButton( this, IconEngine::get( ICONS::HISTORY ) ) );
  button->setText( " &Recent Files" );
  button->setToolTip( "Files recently opened" );

  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_navigationFrame().recentFilesFrame() ) );

  // file system
  addWidget( button = _newToolButton( this, IconEngine::get( ICONS::FILESYSTEM ) ) );
  button->setText( " &File System" );
  button->setToolTip( "File system browser" );

  button_group->addButton( button );
  buttons_.insert( make_pair( button, &_navigationFrame().fileSystemFrame() ) );

  _updateConfiguration();

}

//______________________________________________________________________
NavigationToolBar::~NavigationToolBar( void )
{ Debug::Throw( "NavigationToolBar::~NavigationToolBar.\n" ); }

//____________________________________________________________
void NavigationToolBar::_updateConfiguration( void )
{

  Debug::Throw( "NavigationToolBar::_updateConfiguration.\n" );

  // icon size
  IconSize icon_size( IconSize( this, (IconSize::Size)XmlOptions::get().get<int>( "NAVIGATION_SIDEBAR_ICON_SIZE" ) ) );
  setIconSize( icon_size );

  // text label for toolbars
  Qt::ToolButtonStyle style( (Qt::ToolButtonStyle) XmlOptions::get().get<int>( "NAVIGATION_SIDEBAR_TEXT_POSITION" ) );
  setToolButtonStyle( style );

  // also update buttons independently
  for( ButtonMap::const_iterator iter = buttons_.begin(); iter != buttons_.end(); iter++ )
  {
    iter->first->setToolButtonStyle( style );
    iter->first->setIconSize( icon_size );
  }

  adjustSize();

}

//____________________________________________________________
void NavigationToolBar::_updateToolButtonStyle( Qt::ToolButtonStyle style )
{

  Debug::Throw( "NavigationToolBar::_updateToolButtonStyle.\n" );
  XmlOptions::get().set<int>( "NAVIGATION_SIDEBAR_TEXT_POSITION", (int)style );
  _updateConfiguration();

}

//____________________________________________________________
void NavigationToolBar::_updateToolButtonIconSize( IconSize::Size size )
{

  Debug::Throw( "NavigationToolBar::_updateToolButtonIconSize.\n" );
  XmlOptions::get().set<int>( "NAVIGATION_SIDEBAR_ICON_SIZE", size );
  _updateConfiguration();

}

//______________________________________________________________________
void NavigationToolBar::_navigationFrameVisibilityChanged( bool state )
{

  Debug::Throw() << "NavigationToolBar::_navigationFrameVisibilityChanged - state: " << state << endl;
  if( !state )
  {

    // make sure no button is checked
    for( ButtonMap::const_iterator iter = buttons_.begin(); iter != buttons_.end(); iter++ )
    { iter->first->setChecked( false ); }

  }
  else {

    // make sure that one button is checked
    bool found( false );
    for( ButtonMap::iterator iter = buttons_.begin(); iter != buttons_.end() && !found; iter++ )
    {
      if( iter->second == _navigationFrame().currentWidget() )
      {
        iter->first->setChecked( true );
        found = true;
        break;
      }
    }

  }


}

//______________________________________________________________________
void NavigationToolBar::_orientationChanged( Qt::Orientation orientation )
{

  Debug::Throw() << "NavigationToolBar::_orientationChanged - orientation: " << orientation << endl;

  for( ButtonMap::iterator iter = buttons_.begin(); iter != buttons_.end(); iter++ )
  { iter->first->rotate( orientation == Qt::Horizontal ? CustomToolButton::NONE : CustomToolButton::COUNTERCLOCKWISE ); }

  adjustSize();

}

//______________________________________________________________________
void NavigationToolBar::_display( QAbstractButton* button )
{

  Debug::Throw( "NavigationToolBar:_display.\n" );

  if( !enabled_ ) return;
  enabled_ = false;

  // retrieve widget in map
  bool state( button->isChecked() );
  QWidget* widget (0);
  for( ButtonMap::iterator iter = buttons_.begin(); iter != buttons_.end(); iter++ )
  {
    if( iter->first == button ) widget = iter->second;
    else iter->first->setChecked( false );
  }

  assert( widget );

  if( state )
  {

    // warning: the order is important. The current widget must be changed first, otherwise two buttons
    // will be checked in the end due to the _navigationFrameVisibilityChanged slot.
    _navigationFrame().setCurrentWidget( widget );
    _navigationFrame().visibilityAction().setChecked( true );

  } else if( widget == _navigationFrame().currentWidget() ) {

    _navigationFrame().visibilityAction().setChecked( false );

  }

  enabled_ = true;
  return;

}

//______________________________________________________________________
void NavigationToolBar::contextMenuEvent( QContextMenuEvent* event )
{
  Debug::Throw( "NavigationToolBar::contextMenuEvent.\n" );

  MainWindow* mainwindow( dynamic_cast<MainWindow*>( window() ) );
  if( !mainwindow ) return;
  ToolBarMenu& menu( mainwindow->toolBarMenu( this ) );

  menu.toolButtonStyleMenu().select( (Qt::ToolButtonStyle) XmlOptions::get().get<int>( "NAVIGATION_SIDEBAR_TEXT_POSITION" ) );
  menu.iconSizeMenu().select( (IconSize::Size) XmlOptions::get().get<int>( "NAVIGATION_SIDEBAR_ICON_SIZE" ) );

  CustomToolBar::connect( &menu.toolButtonStyleMenu(), SIGNAL( styleSelected( Qt::ToolButtonStyle ) ), SLOT( _updateToolButtonStyle( Qt::ToolButtonStyle ) ) );
  CustomToolBar::connect( &menu.iconSizeMenu(), SIGNAL( iconSizeSelected( IconSize::Size ) ), SLOT( _updateToolButtonIconSize( IconSize::Size ) ) );

  // move and show menu
  menu.adjustSize();
  menu.exec( event->globalPos() );
  menu.deleteLater();

}

//______________________________________________________________________
CustomToolButton* NavigationToolBar::_newToolButton( QWidget* parent, QIcon icon ) const
{

  CustomToolButton* button = new CustomToolButton( parent );
  button->setIcon( icon );
  button->setCheckable( true );

  // customize button appearence
  button->setUpdateFromOptions( false );

  if( orientation() == Qt::Vertical )
  { button->rotate( CustomToolButton::COUNTERCLOCKWISE ); }

  return button;

}
