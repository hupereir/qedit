// $Id$
#ifndef NavigationToolBar_h
#define NavigationToolBar_h

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
\file NavigationToolBar.h
\brief editor windows navigator
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "IconSize.h"
#include "CustomToolBar.h"
#include "CustomToolButton.h"

#include <QtGui/QAbstractButton>
#include <QtGui/QContextMenuEvent>
#include <QtCore/QHash>
#include <cassert>

class NavigationFrame;

//! editor windows navigator
/*!
displays an up-to-date list of recent files
as well as files opened in current session
*/
class NavigationToolBar: public CustomToolBar
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! creator
    NavigationToolBar( QWidget* parent );

    //! destructor
    ~NavigationToolBar( void );

    //! set target
    void connect( NavigationFrame& );

    protected:

    //! context menu
    virtual void contextMenuEvent( QContextMenuEvent* );

    private slots:

    //! update configuration
    void _updateConfiguration( void );

    //! toolbar text position
    void _updateToolButtonStyle( Qt::ToolButtonStyle );

    //! toolbar text position
    void _updateToolButtonIconSize( IconSize::Size );

    //! make sure proper buttons are changed when navigation frame visibility is changed
    virtual void _navigationFrameVisibilityChanged( bool );

    //! change orientation
    virtual void _orientationChanged( Qt::Orientation );

    //! display item page
    virtual void _display( QAbstractButton* );

    private:

    //! navigation frame
    NavigationFrame& _navigationFrame( void )
    {
        assert( navigationFrame_ );
        return *navigationFrame_;
    }

    //! tool button
    CustomToolButton* _newToolButton( QWidget* parent, QIcon ) const;

    //! map widget to action
    typedef QHash<CustomToolButton*, QWidget* > ButtonMap;

    //! map widget to action in the toolbar
    ButtonMap buttons_;

    //! stack widget
    NavigationFrame* navigationFrame_;

    //! enablility
    /*! this is used to avoid circular action triggers */
    bool enabled_;

};

#endif
