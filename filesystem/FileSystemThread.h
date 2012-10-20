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

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>

//! independent thread used to automatically save file
class FileSystemThread: public QThread, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    FileSystemThread( QObject* );

    //! set file
    void setPath( const File& path, const bool& show_hidden_files )
    {
        QMutexLocker lock( &mutex_ );
        path_ = path;
        showHiddenFiles_ = show_hidden_files;
    }

    //! Check files validity. Post a FileSystemEvent when finished
    void run( void );

    signals:

    //! files available
    void filesAvailable( const File&, const FileRecord::List& );

    private:

    //! mutex
    QMutex mutex_;

    //! size property id
    FileRecord::PropertyId::Id sizePropertyId_;

    //! filename where data is to be saved
    File path_;

    //! file list
    FileRecord::List records_;

    //! true if hidden files are to be listed
    bool showHiddenFiles_;

};
#endif
