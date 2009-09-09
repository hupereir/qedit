// $Id$
#ifndef _FileCheckDialog_h_
#define _FileCheckDialog_h_

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public license as published by the Free Software
* Foundation; either version 2 of the license, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public license
* for more details.
*
* You should have received a copy of the GNU General Public license along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file FileCheckDialog.h
  \brief QDialog used to select opened files
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QStringList>

#include "CustomDialog.h"
#include "FileRecordModel.h"

class TreeView;

//! QDialog used to select opened files
class FileCheckDialog: public CustomDialog
{

  public:

  //! constructor
  FileCheckDialog( QWidget* parent );

  //! set files
  void setFiles( const QStringList& );

  private:

  //! list
  TreeView& _list( void ) const
  { return *list_; }

  //! model
  FileRecordModel model_;

  //! list of files
  TreeView* list_;

};

#endif
