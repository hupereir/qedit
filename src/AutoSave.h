#ifndef AutoSave_h
#define AutoSave_h

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

#include "AutoSaveThread.h"

#include <QObject>
#include <QBasicTimer>
#include <QList>

class TextDisplay;

//* handles threads for file auto-save
class AutoSave: public QObject, private Base::Counter<AutoSave>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit AutoSave( QObject* = nullptr );

    //* destructor
    ~AutoSave( void ) override;

    //* register new thread
    void newThread( TextDisplay* );

    //* Save files
    /* if set to non 0, only threads that match the display are saved */
    void saveFiles( const TextDisplay* = nullptr );

    protected:

    virtual void timerEvent( QTimerEvent* );

    private Q_SLOTS:

    //* update configuration
    /* update interval between threads */
    void _updateConfiguration( void );

    private:

    bool _enabled( void ) const
    { return enabled_ && interval_ > 0; }

    //* true when enabled
    bool enabled_ = false;

    //* interval between two save (milliseconds)
    int interval_ = 0;

    //* AutoSave timer
    QBasicTimer timer_;

    //* list of threads
    using ThreadList = QList<AutoSaveThread*>;

    //* list of threads
    ThreadList threads_;

};

#endif
