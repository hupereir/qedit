#ifndef FileCheck_h
#define FileCheck_h

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

#include "Key.h"
#include "TimeStamp.h"

#include <QBasicTimer>
#include <QFileSystemWatcher>
#include <QObject>
#include <QTimerEvent>
#include <QSet>

class TextDisplay;

//! handles threads for file auto-save
class FileCheck: public QObject, public Base::Key, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! constructor
    FileCheck( QObject* parent = 0 );

    //! destructor
    ~FileCheck( void );

    //! register new dispay
    void registerDisplay( TextDisplay* );

    //! add file
    void addFile( const QString& );

    //! remove file
    void removeFile( const QString& );

    //! used to monitor file changes
    class Data
    {
        public:

        //! flag
        enum Flag
        {
            None,
            Removed,
            Modified
        };

        //! constructor
        Data( QString file = QString(), Flag flag = None, TimeStamp stamp = TimeStamp() ):
            file_( file ),
            flag_( flag ),
            timeStamp_( stamp )
        {}

        //! equal to operator
        bool operator == ( const Data& data ) const
        { return file() == data.file(); }

        //! less than operator
        bool operator < ( const Data& data ) const
        { return file() < data.file(); }

        //! file
        void setFile( const QString& file )
        { file_ = file; }

        //! file
        const QString& file( void ) const
        { return file_; }

        //! flag
        void setFlag( const Flag& flag )
        { flag_ = flag; }

        //! flag
        const Flag& flag( void ) const
        { return flag_; }

        //! timestamp
        void setTimeStamp( const TimeStamp& stamp )
        { timeStamp_ = stamp; }

        //! timestamp
        const TimeStamp& timeStamp( void ) const
        { return timeStamp_; }

        private:

        //! file
        QString file_;

        //! flag
        Flag flag_;

        //! timestamp
        TimeStamp timeStamp_;

    };

    //! map data to file
    using DataSet = QSet<Data>;


    //! file system watcher
    const QFileSystemWatcher& fileSystemWatcher( void ) const
    { return fileSystemWatcher_; }

    protected:

    //! timer event, to handle multiple file modification at once
    virtual void timerEvent( QTimerEvent* event );

    private Q_SLOTS:

    void _fileChanged( const QString& );

    private:

    //! file system watcher
    QFileSystemWatcher fileSystemWatcher_;

    //! file set
    using FileSet = QSet<QString>;

    //! file set
    FileSet files_;

    //! map files and modification data
    DataSet data_;

    //! resize timer
    QBasicTimer timer_;

};

inline uint qHash( const FileCheck::Data& data )
{ return qHash( data.file() ); }

#endif
