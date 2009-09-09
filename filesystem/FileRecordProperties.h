#ifndef FileRecordProperties_h
#define FileRecordProperties_h

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
/*!
  \file    FileRecordProperties.h
  \brief   File record property names
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QString>
#include "FileRecordBaseProperties.h"

namespace FileRecordProperties
{

  static const QString CLASS_NAME = "class_name";
  static const QString WRAPPED = "wrap";
  static const QString DICTIONARY = "dictionary";
  static const QString FILTER = "filter";
  static const QString SIZE = "size";

  //! needed to tag session file records
  enum State
  {
    NONE = 0,

    //! tag modified files
    MODIFIED = 1<<0,

    //! tag files that belong to current window
    ACTIVE = 1<<1,

    //! tag current file
    SELECTED = 1<<2,

    //! file has been altered (modified/removed) by external application
    ALTERED = 1<<3

  };
};

#endif
