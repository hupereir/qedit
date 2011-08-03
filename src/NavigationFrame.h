// $Id$
#ifndef NavigationFrame_h
#define NavigationFrame_h

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
\file NavigationFrame.h
\brief editor windows navigator
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <cassert>
#include <map>
#include <QAction>
#include <QShowEvent>
#include <QToolButton>

#include "AnimatedStackedWidget.h"
#include "FileList.h"
#include "FileRecord.h"

class SessionFilesFrame;
class RecentFilesFrame;
class FileSystemFrame;

//! editor windows navigator
/*!
displays an up-to-date list of recent files
as well as files opened in current session
*/
class NavigationFrame: public AnimatedStackedWidget
{

    Q_OBJECT

        public:

        //! creator
        NavigationFrame( QWidget* parent, FileList&  );

    //! destructor
    virtual ~NavigationFrame( void )
    {}

    //! default size
    void setDefaultWidth( const int& );

    //! default width
    const int& defaultWidth( void ) const
    { return defaultWidth_; }

    //! size
    QSize sizeHint( void ) const;

    //! session files
    SessionFilesFrame& sessionFilesFrame( void ) const
    {
        assert( sessionFilesFrame_ );
        return *sessionFilesFrame_;
    }

    //! recent files
    RecentFilesFrame& recentFilesFrame( void ) const
    {
        assert( recentFilesFrame_ );
        return *recentFilesFrame_;
    }

    //! file system
    FileSystemFrame& fileSystemFrame( void ) const
    {
        assert( fileSystemFrame_ );
        return *fileSystemFrame_;
    }

    //!@name actions
    //@{

    //! visibility
    QAction& visibilityAction( void ) const
    { return *visibilityAction_; }

    //@}

    private slots:

    //! update current widget
    void _updateCurrentWidget( void );

    private:

    //! install actions
    void _installActions( void );

    //! default width;
    int defaultWidth_;

    //! session files
    SessionFilesFrame *sessionFilesFrame_;

    //! recent files
    RecentFilesFrame *recentFilesFrame_;

    //! file system
    FileSystemFrame* fileSystemFrame_;

    //@}

    //!@name actions
    //@{

    //! visibility
    QAction* visibilityAction_
        ;

    //@}

};

#endif
