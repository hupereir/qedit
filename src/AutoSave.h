#ifndef AutoSave_h
#define AutoSave_h

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

#include "AutoSaveThread.h"

#include <QObject>
#include <QBasicTimer>
#include <QList>

class TextDisplay;

//! handles threads for file auto-save
class AutoSave: public QObject, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! constructor
    AutoSave( QObject* parent = 0 );

    //! destructor
    ~AutoSave( void );

    //! register new thread
    void newThread( TextDisplay* );

    //! Save files
    /* \param display if set to non 0, only threads that match the display are saved */
    void saveFiles( const TextDisplay* = 0 );

    protected:

    virtual void timerEvent( QTimerEvent* );

    private slots:

    //! update configuration
    /* update interval between threads */
    void _updateConfiguration( void );

    private:

    bool _enabled( void ) const
    { return enabled_ && interval_ > 0; }

    //! true when enabled
    bool enabled_;

    //! interval between two save (milliseconds)
    int interval_;

    //! AutoSave timer
    QBasicTimer timer_;

    //! list of threads
    typedef QList< AutoSaveThread* > ThreadList;

    //! list of threads
    ThreadList threads_;

};

#endif
