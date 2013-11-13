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

//! independent thread used to automatically save file
class AutoSaveThread: public QThread, public BASE::Key, public Counter
{

    public:

    //! constructor
    AutoSaveThread( QObject* parent ):
        QThread( parent ),
        Counter( "AutoSaveThread" ),
        flags_( None )
    { Debug::Throw( "AutoSaveThread::AutoSaveThread.\n" ); }

    //! destructor
    virtual ~AutoSaveThread( void )
    { Debug::Throw( "AutoSaveThread::~AutoSaveThread.\n" ); }

    //!@name accessors
    //@{

    //! file
    const File& file( void ) const
    { return file_; }

    //@}

    //!@name modifiers
    //@{

    //! file
    void setFile( const File& );

    //! set content
    void setContents( const QString& );

    //! set encoding
    void setTextEncoding( const QString& );

    //@}

    //! create backup file name from file
    static File autoSaveName( const File& );

    //! state flags
    enum Flag
    {
        None = 0,
        FileChanged = 1<<0,
        ContentChanged = 1<<1,
        TextEncodingChanged = 1<<2
    };

    Q_DECLARE_FLAGS( Flags, Flag )

    protected:

    //! generate a new grid. Post a AutoSaveEvent when finished
    virtual void run( void );

    private:

    //! mutex
    QMutex mutex_;

    //! filename where data is to be saved
    File file_;

    //! content to be saved
    QString contents_;

    //! text encoding
    QString textEncoding_;

    Flags flags_;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( AutoSaveThread::Flags );


#endif
