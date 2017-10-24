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

#include "File.h"
#include "FileCheckData.h"
#include "Key.h"
#include "TimeStamp.h"

#include <QBasicTimer>
#include <QFileSystemWatcher>
#include <QObject>
#include <QTimerEvent>
#include <QSet>

class TextDisplay;

//* handles threads for file auto-save
class FileCheck: public QObject, public Base::Key, private Base::Counter<FileCheck>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit FileCheck( QObject* = nullptr );

    //* register new dispay
    void registerDisplay( TextDisplay* );

    //* add file
    void addFile( const QString& );

    //* remove file
    void removeFile( const QString& );

    //* file system watcher
    const QFileSystemWatcher& fileSystemWatcher() const
    { return fileSystemWatcher_; }

    private Q_SLOTS:

    void _fileChanged( const QString& );

    private:

    //* check data
    void _checkData( const FileCheckData& );

    //* file system watcher
    QFileSystemWatcher fileSystemWatcher_;

    //* file set
    using FileSet = QSet<QString>;

    //* file set
    FileSet files_;

};

#endif
