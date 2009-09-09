#ifndef _WindowTitle_h_
#define _WindowTitle_h_
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
   \file WindowTitle.h
   \brief Generate window title from filename
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QString>

#include "Counter.h"
#include "File.h"

class WindowTitle: public Counter
{

  public:

  //! Flags
  enum Flags
  {
    NONE,
    MODIFIED,
    READ_ONLY
  };

  //! constructor
  WindowTitle( const File& file = File(), const Flags& flag = NONE ):
    Counter( "WindowTitle" ),
    file_( file ),
    flag_( flag )
  {}

  //! change flag
  WindowTitle& setFlag( const Flags& flag, const bool& value = true )
  {
    if( value ) flag_ = flag;
    return *this;
  }

  //! change flag
  WindowTitle& setModified( const bool& value = true )
  { return setFlag( MODIFIED, value ); }

  //! change flag
  WindowTitle& setReadOnly( const bool& value = true )
  { return setFlag( READ_ONLY, value ); }

  //! cast to string
  operator const QString& (void)
  {
    QString out;
    QTextStream what( &out );
    if( file_.size() ) what << file_.localName();
    else what << "QEdit";
    if( flag_ == MODIFIED ) what << " (modified)";
    if( flag_ == READ_ONLY ) what << " (read-only)";
    if( file_.size() ) what << " - " << file_.path();
    return title_ = out;
  }

  private:

  //! filename
  File file_;

  //! flags
  Flags flag_;

  //! stored string
  QString title_;

};

#endif
