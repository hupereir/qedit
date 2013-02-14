#ifndef ProgressDialog_h
#define ProgressDialog_h

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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "Counter.h"

#include <QApplication>
#include <QProgressDialog>

//! command progress
class ProgressDialog:public QProgressDialog, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    ProgressDialog( QWidget* parent = 0 ):
        QProgressDialog( parent ),
        Counter( "ProgressDialog" ),
        offset_( 0 )
    {}

    //! destructor
    virtual ~ProgressDialog( void )
    {}

    //! offset
    void setOffset( int value )
    { offset_ = value; }

    public slots:

    //! value
    void setValue( int value )
    { QProgressDialog::setValue( _offset()+value ); }

    private:

    //! offset
    int _offset( void ) const
    { return offset_; }

    //! offset
    int offset_;

};

#endif
