// $Id$
#ifndef _NewFileDialog_h_
#define _NewFileDialog_h_

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
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file NewFileDialog.h
  \brief QDialog used to ask if a new file should be created
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <qdialog.h>
#include "Counter.h"
#include "File.h"

//! QDialog used to ask if a new file should be created
class NewFileDialog: public QDialog, public Counter
{

  //! Qt macro
  Q_OBJECT

  public:

  //! return codes
  /*! also used to decide which buttons are to be drawn */
  enum {

    //! file is to be saved
    CREATE = 1<<0,

    //! file is not to be saved
    CANCEL = 1<<1,

    //! exit application
    EXIT = 1<<2

  };

  //! constructor
  NewFileDialog( QWidget* parent, const File& file, const unsigned int& buttons = CREATE|CANCEL|EXIT );

  private slots:

  //! create new file
  void _create( void )
  { done( CREATE ); }

  //! cancel creation/exit editor
  void _cancel( void )
  { done( CANCEL ); }

  //! exit application
  void _exit( void )
  { done( EXIT ); }

};

#endif
