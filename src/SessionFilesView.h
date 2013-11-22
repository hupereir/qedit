#ifndef SessionFilesView_h
#define SessionFilesView_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "AnimatedTreeView.h"
#include "File.h"

class SessionFilesView: public AnimatedTreeView
{

    Q_OBJECT

    public:

    //! constructor
    SessionFilesView( QWidget* parent = 0 ):
        AnimatedTreeView( parent )
        {}

    //! destructor
    virtual ~SessionFilesView( void )
    {}

    Q_SIGNALS:

    //! detach
    void detach( const File& );

    //! file reorganization
    void reparentFilesToMain( const File&, const File& );

    protected:

    //! start drag
    virtual void startDrag( Qt::DropActions );

};

#endif
