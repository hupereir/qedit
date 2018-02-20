#ifndef SidePanelToolBar_h
#define SidePanelToolBar_h

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

#include "IconSize.h"
#include "CustomToolBar.h"
#include "CustomToolButton.h"

#include <QAbstractButton>
#include <QContextMenuEvent>
#include <QHash>

class SidePanelWidget;

//* editor windows navigator
/**
displays an up-to-date list of recent files
as well as files opened in current session
*/
class SidePanelToolBar: public CustomToolBar
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    explicit SidePanelToolBar( QWidget* );

    using CustomToolBar::connect;

    //* set target
    void connect( SidePanelWidget& );

    protected:

    //* context menu
    void contextMenuEvent( QContextMenuEvent* ) override;

    private Q_SLOTS:

    //* update configuration
    void _updateConfiguration();

    //* toolbar text position
    void _updateToolButtonStyle( int );

    //* toolbar text position
    void _updateToolButtonIconSize( IconSize::Size );

    //* make sure proper buttons are changed when navigation frame visibility is changed
    void _sidePanelWidgetVisibilityChanged( bool );

    //* display item page
    void _display( QAbstractButton* );

    private:

    //* navigation frame
    SidePanelWidget& _sidePanelWidget()
    { return *sidePanelWidget_; }

    //* tool button
    CustomToolButton* _newToolButton( QWidget* parent, QIcon ) const;

    //* map widget to action
    using ButtonMap = QHash<CustomToolButton*, QWidget* >;

    //* map widget to action in the toolbar
    ButtonMap buttons_;

    //* stack widget
    SidePanelWidget* sidePanelWidget_ = nullptr;

    //* enablility
    /** this is used to avoid circular action triggers */
    bool enabled_ = true;

};

#endif
