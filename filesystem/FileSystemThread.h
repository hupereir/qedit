#ifndef FileSystemThread_h
#define FileSystemThread_h

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

#include "Counter.h"
#include "Debug.h"
#include "FileSystemModel.h"

#include <QtCore/QThread>
#include <QtCore/QEvent>

//! used to post a new grid when ready
class FileSystemEvent: public QEvent, public Counter
{

    public:

    //! constructor
    FileSystemEvent( File path, FileSystemModel::List files ):
        QEvent( eventType() ),
        Counter( "FileSystemEvent" ),
        path_( path ),
        files_( files )
    {}

    //! destructor
    ~FileSystemEvent( void )
    {}

    //! static event type
    static QEvent::Type eventType( void );

    //! path
    const File& path( void )
    { return path_; }

    //! files
    const FileSystemModel::List& files()
    { return files_; }

    private:

    //! path
    File path_;

    //! ValidFile success flag
    FileSystemModel::List files_;

};

//! independent thread used to automatically save file
class FileSystemThread: public QThread, public Counter
{

    public:

    //! constructor
    FileSystemThread( QObject* );


    //! set file
    void setPath( const File& path, const bool& show_hidden_files )
    {
        path_ = path;
        showHiddenFiles_ = show_hidden_files;
    }

    //! Check files validity. Post a FileSystemEvent when finished
    void run( void );

    private:

    //! reciever object for posted events
    QObject* reciever_;

    //! size property id
    FileRecord::PropertyId::Id sizePropertyId_;

    //! filename where data is to be saved
    File path_;

    //! true if hidden files are to be listed
    bool showHiddenFiles_;

};
#endif
