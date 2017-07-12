#ifndef AutoSaveThread_h
#define AutoSaveThread_h

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

#include "Counter.h"
#include "Debug.h"
#include "File.h"
#include "Key.h"

#include <QMutex>
#include <QMutexLocker>
#include <QThread>

//* independent thread used to automatically save file
class AutoSaveThread: public QThread, public Base::Key, private Base::Counter<AutoSaveThread>
{

    public:

    //* constructor
    explicit AutoSaveThread( QObject* parent ):
        QThread( parent ),
        Counter( "AutoSaveThread" )
    {}

    //*@name accessors
    //@{

    //* file
    File file() const
    { return file_; }

    //@}

    //*@name modifiers
    //@{

    //* file
    void setFile( File );

    //* set content
    void setContent( const QString& );

    //* set encoding
    void setTextEncoding( QByteArray );

    //* set use compression
    void setUseCompression( bool );

    //@}

    //* create backup file name from file
    static File autoSaveName( const File& );

    //* state flags
    enum Flag
    {
        None = 0,
        FileChanged = 1<<0,
        ContentChanged = 1<<1,
        EncodingChanged = 1<<2,
        CompressionChanged = 1<<3
    };

    Q_DECLARE_FLAGS( Flags, Flag )

    protected:

    //* generate a new grid. Post a AutoSaveEvent when finished
    void run() override;

    private:

    //* mutex
    QMutex mutex_;

    //* filename where data is to be saved
    File file_;

    //* content to be saved
    QString content_;

    //* text encoding
    QByteArray textEncoding_;

    //* compression
    bool useCompression_ = false;

    //* modification flags
    Flags flags_ = None;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( AutoSaveThread::Flags );


#endif
