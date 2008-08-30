// $Id$
#ifndef _FileModifiedDialog_h_
#define _FileModifiedDialog_h_

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
  \file FileModifiedDialog.h
  \brief QDialog used when a file has been modified on disk
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "BaseDialog.h"

#include "Counter.h"
#include "File.h"

//! QDialog used to ask if a new file should be created
class FileModifiedDialog: public BaseDialog, public Counter
{

  //! Qt macro
  Q_OBJECT

  public:

  //! return codes
  enum ReturnCode
  {

    //! re-saved removed file
    RESAVE = 0,

    //! save file with new name
    SAVE_AS = 1,

    //! re-load modified file
    RELOAD = 2,
    
    //! ignore warning
    IGNORE = 3

  };

  //! constructor
  FileModifiedDialog( QWidget* parent, const File& file );

  private slots:

  //! re-saved removed file
  void _reLoad( void )
  { done( RELOAD ); }

  //! re-saved removed file
  void _reSave( void )
  { done( RESAVE ); }

  //! save file with new name
  void _saveAs( void )
  { done( SAVE_AS ); }

  //! save file with new name
  void _ignore( void )
  { done( IGNORE ); }

};

#endif
