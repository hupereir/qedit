#ifndef AutoSaveThread_h
#define AutoSaveThread_h

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
* Place, Suite 330, Boston, MA 02111-1307 USA
*
*
*******************************************************************************/

/*!
\file AutoSaveThread.h
\brief independent thread used to make regular automatic saves of files
\author  Hugo Pereira
\version $Revision$
\date $Date$
*/


#include <QThread>

#include "Counter.h"
#include "Debug.h"
#include "File.h"
#include "Key.h"

//! independent thread used to automatically save file
class AutoSaveThread: public QThread, public BASE::Key, public Counter
{

    public:

    //! constructor
    AutoSaveThread( QObject* parent ):
        QThread( parent ),
        Counter( "AutoSaveThread" ),
        fileChanged_( true ),
        contentsChanged_( true )
    { Debug::Throw( "AutoSaveThread::AutoSaveThread.\n" ); }

    //! destructor
    virtual ~AutoSaveThread( void )
    { Debug::Throw( "AutoSaveThread::~AutoSaveThread.\n" ); }

    //! file
    void setFile( const File& file );

    //! file
    const File& file( void ) const
    { return file_; }

    //! set content
    void setContents( const QString& contents );

    //! generate a new grid. Post a AutoSaveEvent when finished
    void run( void );

    //! create backup file name from file
    static File autoSaveName( const File& file );

    private:

    //! filename where data is to be saved
    File file_;

    //! content to be saved
    QString contents_;

    //! modification flag
    bool fileChanged_;

    //! modification flag
    bool contentsChanged_;

};

#endif
