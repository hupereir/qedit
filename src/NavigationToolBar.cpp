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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "CustomPixmap.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "FileSystemFrame.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "IconSizeMenu.h"
#include "MainWindow.h"
#include "NavigationToolBar.h"
#include "NavigationFrame.h"
#include "QtUtil.h"
#include "RecentFilesFrame.h"
#include "SessionFilesFrame.h"
#include "ToolBarMenu.h"
#include "ToolButtonStyleMenu.h"

#include <QButtonGroup>

//_______________________________________________________________
NavigationToolBar::NavigationToolBar( QWidget* parent ):
    CustomToolBar( tr( "Side bar" ), parent, "NAVIGATION_SIDEBAR" ),
    navigationFrame_( 0 ),
    enabled_( true )
{
    Debug::Throw( "NavigationToolBar:NavigationToolBar.\n" );
    setTransparent( true );
    CustomToolBar::connect( this, SIGNAL(orientationChanged(Qt::Orientation)), SLOT(_orientationChanged(Qt::Orientation)) );
}

//_______________________________________________________________
void NavigationToolBar::connect( NavigationFrame& frame )
{

    Debug::Throw( "NavigationToolBar::connect.\n" );
    Q_ASSERT( !navigationFrame_ );
    navigationFrame_ = &frame;

    CustomToolBar::connect( &_navigationFrame().visibilityAction(), SIGNAL(toggled(bool)), SLOT(_navigationFrameVisibilityChanged(bool)) );

    // button group
    QButtonGroup* button_group = new QButtonGroup( this );
    CustomToolBar::connect( button_group, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(_display(QAbstractButton*)) );
    button_group->setExclusive( false );

    // matching buttons
    CustomToolButton* button;

    // session files
    addWidget( button = _newToolButton( this,  IconEngine::get( IconNames::Documents ) ) );
    button->setText( tr( "Session Files" ) );
    button->setToolTip( tr( "Files currently opened" ) );
    button->setChecked( true );

    button_group->addButton( button );
    buttons_.insert( button, &_navigationFrame().sessionFilesFrame() );

    // recent files
    addWidget( button = _newToolButton( this, IconEngine::get( IconNames::History ) ) );
    button->setText( tr( "Recent Files" ) );
    button->setToolTip( tr( "Files recently opened" ) );

    button_group->addButton( button );
    buttons_.insert( button, &_navigationFrame().recentFilesFrame() );

    // file system
    addWidget( button = _newToolButton( this, IconEngine::get( IconNames::FileSystem ) ) );
    button->setText( tr( "File System" ) );
    button->setToolTip( tr( "File system browser" ) );

    button_group->addButton( button );
    buttons_.insert( button, &_navigationFrame().fileSystemFrame() );

    _updateConfiguration();

}

//____________________________________________________________
void NavigationToolBar::_updateConfiguration( void )
{

    Debug::Throw( "NavigationToolBar::_updateConfiguration.\n" );

    // icon size
    const IconSize iconSize( (IconSize::Size)XmlOptions::get().get<int>( "NAVIGATION_SIDEBAR_ICON_SIZE" ) );
    setIconSize( iconSize );

    // text label for toolbars
    const Qt::ToolButtonStyle style( (Qt::ToolButtonStyle) XmlOptions::get().get<int>( "NAVIGATION_SIDEBAR_TEXT_POSITION" ) );
    setToolButtonStyle( style );

    // also update buttons independently
    for( ButtonMap::const_iterator iter = buttons_.begin(); iter != buttons_.end(); ++iter )
    {
        iter.key()->setToolButtonStyle( style );
        iter.key()->setIconSize( iconSize );
    }

    adjustSize();

}

//____________________________________________________________
void NavigationToolBar::_updateToolButtonStyle( int style )
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
        for( ButtonMap::const_iterator iter = buttons_.begin(); iter != buttons_.end(); ++iter )
        { iter.key()->setChecked( false ); }

    }
    else {

        // make sure that one button is checked
        bool found( false );
        for( ButtonMap::iterator iter = buttons_.begin(); iter != buttons_.end() && !found; ++iter )
        {
            if( iter.value() == _navigationFrame().currentWidget() )
            {
                iter.key()->setChecked( true );
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

    for( ButtonMap::iterator iter = buttons_.begin(); iter != buttons_.end(); ++iter )
    { iter.key()->rotate( orientation == Qt::Horizontal ? CustomPixmap::None : CustomPixmap::CounterClockwise ); }

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
    for( ButtonMap::iterator iter = buttons_.begin(); iter != buttons_.end(); ++iter )
    {
        if( iter.key() == button ) widget = iter.value();
        else iter.key()->setChecked( false );
    }

    Q_CHECK_PTR( widget );

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

    MainWindow* mainwindow( qobject_cast<MainWindow*>( window() ) );
    if( !mainwindow ) return;
    ToolBarMenu& menu( mainwindow->toolBarMenu( this ) );

    menu.toolButtonStyleMenu().select( (Qt::ToolButtonStyle) XmlOptions::get().get<int>( "NAVIGATION_SIDEBAR_TEXT_POSITION" ) );
    menu.iconSizeMenu().select( (IconSize::Size) XmlOptions::get().get<int>( "NAVIGATION_SIDEBAR_ICON_SIZE" ) );

    CustomToolBar::connect( &menu.toolButtonStyleMenu(), SIGNAL(styleSelected(int)), SLOT(_updateToolButtonStyle(int)) );
    CustomToolBar::connect( &menu.iconSizeMenu(), SIGNAL(iconSizeSelected(IconSize::Size)), SLOT(_updateToolButtonIconSize(IconSize::Size)) );

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
    { button->rotate( CustomPixmap::CounterClockwise ); }

    return button;

}
