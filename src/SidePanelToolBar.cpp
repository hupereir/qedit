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
#include "BaseFileSystemWidget.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "IconSizeMenu.h"
#include "MainWindow.h"
#include "SidePanelToolBar.h"
#include "SidePanelWidget.h"
#include "QtUtil.h"
#include "RecentFilesWidget.h"
#include "SessionFilesWidget.h"
#include "ToolBarMenu.h"
#include "ToolButtonStyleMenu.h"

#include <QButtonGroup>
#include <memory>

//_______________________________________________________________
SidePanelToolBar::SidePanelToolBar( QWidget* parent ):
    CustomToolBar( tr( "Side bar" ), parent, QStringLiteral("SIDE_PANEL_TOOLBAR") ),
    sidePanelWidget_( nullptr ),
    enabled_( true )
{
    Debug::Throw( QStringLiteral("SidePanelToolBar:SidePanelToolBar.\n") );
    setTransparent( true );
}

//_______________________________________________________________
void SidePanelToolBar::connect( SidePanelWidget& widget )
{

    Debug::Throw( QStringLiteral("SidePanelToolBar::connect.\n") );
    sidePanelWidget_ = &widget;

    connect( &sidePanelWidget_->visibilityAction(), &QAction::toggled, this, &SidePanelToolBar::_sidePanelWidgetVisibilityChanged );

    // button group
    auto buttonGroup = new QButtonGroup( this );
    connect( buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &SidePanelToolBar::_display );
    buttonGroup->setExclusive( false );

    // matching buttons
    QToolButton* button;

    // session files
    addWidget( button = _newToolButton( this,  IconEngine::get( IconNames::Documents ) ) );
    button->setText( tr( "Session Files" ) );
    button->setToolTip( tr( "Files currently opened" ) );
    button->setChecked( true );

    buttonGroup->addButton( button );
    buttons_.insert( button, &sidePanelWidget_->sessionFilesWidget() );

    // recent files
    addWidget( button = _newToolButton( this, IconEngine::get( IconNames::History ) ) );
    button->setText( tr( "Recent Files" ) );
    button->setToolTip( tr( "Files recently opened" ) );

    buttonGroup->addButton( button );
    buttons_.insert( button, &sidePanelWidget_->recentFilesWidget() );

    // file system
    addWidget( button = _newToolButton( this, IconEngine::get( IconNames::FileSystem ) ) );
    button->setText( tr( "File System" ) );
    button->setToolTip( tr( "File system browser" ) );

    buttonGroup->addButton( button );
    buttons_.insert( button, &sidePanelWidget_->fileSystemWidget() );

    _updateConfiguration();

}

//____________________________________________________________
void SidePanelToolBar::_updateConfiguration()
{

    Debug::Throw( QStringLiteral("SidePanelToolBar::_updateConfiguration.\n") );

    // icon size
    const IconSize iconSize( static_cast<IconSize::Size>( XmlOptions::get().get<int>( QStringLiteral("SIDE_PANEL_TOOLBAR_ICON_SIZE") ) ) );

    // text label for toolbars
    const int toolButtonTextPosition( XmlOptions::get().get<int>( QStringLiteral("SIDE_PANEL_TOOLBAR_TEXT_POSITION") ) );
    const auto style = static_cast<Qt::ToolButtonStyle>( toolButtonTextPosition >= 0 ?
        toolButtonTextPosition:
        this->style()->styleHint( QStyle::SH_ToolButtonStyle ) );

    // assign to toolbar
    setIconSize( iconSize.get() );
    setToolButtonStyle( style );

    // also update buttons independently
    for( auto&& iter = buttons_.begin(); iter != buttons_.end(); ++iter )
    {
        iter.key()->setToolButtonStyle( style );
        iter.key()->setIconSize( iconSize.get() );
    }

    adjustSize();

}

//____________________________________________________________
void SidePanelToolBar::_updateToolButtonStyle( int style )
{

    Debug::Throw( QStringLiteral("SidePanelToolBar::_updateToolButtonStyle.\n") );
    XmlOptions::get().set<int>( QStringLiteral("SIDE_PANEL_TOOLBAR_TEXT_POSITION"), (int)style );
    _updateConfiguration();

}

//____________________________________________________________
void SidePanelToolBar::_updateToolButtonIconSize( IconSize::Size size )
{

    Debug::Throw( QStringLiteral("SidePanelToolBar::_updateToolButtonIconSize.\n") );
    XmlOptions::get().set<int>( QStringLiteral("SIDE_PANEL_TOOLBAR_ICON_SIZE"), size );
    _updateConfiguration();

}

//______________________________________________________________________
void SidePanelToolBar::_sidePanelWidgetVisibilityChanged( bool state )
{

    Debug::Throw() << "SidePanelToolBar::_sidePanelWidgetVisibilityChanged - state: " << state << endl;
    if( !state )
    {

        // make sure no button is checked
        for( auto&& iter = buttons_.begin(); iter != buttons_.end(); ++iter )
        { iter.key()->setChecked( false ); }

    }
    else {

        // make sure that one button is checked
        bool found( false );
        for( auto&& iter = buttons_.begin(); iter != buttons_.end() && !found; ++iter )
        {
            if( iter.value() == sidePanelWidget_->currentWidget() )
            {
                iter.key()->setChecked( true );
                found = true;
                break;
            }
        }

    }


}

//______________________________________________________________________
void SidePanelToolBar::_display( QAbstractButton* button )
{

    Debug::Throw( QStringLiteral("SidePanelToolBar:_display.\n") );

    if( !enabled_ ) return;
    enabled_ = false;

    // retrieve widget in map
    bool state( button->isChecked() );
    QWidget* widget (0);
    for( auto&& iter = buttons_.begin(); iter != buttons_.end(); ++iter )
    {
        if( iter.key() == button ) widget = iter.value();
        else iter.key()->setChecked( false );
    }

    Q_CHECK_PTR( widget );

    if( state )
    {

        // warning: the order is important. The current widget must be changed first, otherwise two buttons
        // will be checked in the end due to the _sidePanelWidgetVisibilityChanged slot.
        sidePanelWidget_->setCurrentWidget( widget );
        sidePanelWidget_->visibilityAction().setChecked( true );

    } else if( widget == sidePanelWidget_->currentWidget() ) {

        sidePanelWidget_->visibilityAction().setChecked( false );

    }

    enabled_ = true;
    return;

}

//______________________________________________________________________
void SidePanelToolBar::contextMenuEvent( QContextMenuEvent* event )
{
    Debug::Throw( QStringLiteral("SidePanelToolBar::contextMenuEvent.\n") );

    MainWindow* mainwindow( qobject_cast<MainWindow*>( window() ) );
    if( !mainwindow ) return;
    std::unique_ptr<ToolBarMenu> menu( mainwindow->toolBarMenu( this ) );

    menu->toolButtonStyleMenu().select( (Qt::ToolButtonStyle) XmlOptions::get().get<int>( QStringLiteral("SIDE_PANEL_TOOLBAR_TEXT_POSITION") ) );
    menu->iconSizeMenu().select( (IconSize::Size) XmlOptions::get().get<int>( QStringLiteral("SIDE_PANEL_TOOLBAR_ICON_SIZE") ) );

    connect( &menu->toolButtonStyleMenu(), &ToolButtonStyleMenu::styleSelected, this, &SidePanelToolBar::_updateToolButtonStyle );
    connect( &menu->iconSizeMenu(), &IconSizeMenu::iconSizeSelected, this, &SidePanelToolBar::_updateToolButtonIconSize );

    // move and show menu
    menu->adjustSize();
    menu->exec( event->globalPos() );

}

//______________________________________________________________________
QToolButton* SidePanelToolBar::_newToolButton( QWidget* parent, const QIcon &icon ) const
{

    CustomToolButton* button = new CustomToolButton( parent );
    button->setIcon( icon );
    button->setCheckable( true );
    button->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    button->setUpdateFromOptions( false );
    button->setProperty( "_kde_toolButton_alignment", Qt::AlignLeft );

    return button;

}
