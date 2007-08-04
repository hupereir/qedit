#ifndef AutoSaveEvent_h
#define AutoSaveEvent_h

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
  \file AutoSaveEvent.h
  \brief used to notify if backup was successfully done or not
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QEvent>
#include "Counter.h"

//! used to notify if backup was successfully done or not
class AutoSaveEvent: public QEvent, public Counter
{
  
  public:
      
  //! constructor
  AutoSaveEvent( const std::string& file, const bool& success ):
    QEvent( QEvent::User  ),
    Counter( "AutoSaveEvent" ),
    file_( file ),
    success_( success )
  {}
  
  //! autosave filename
  const std::string& file( void ) const
  { return file_; }
  
  //! return success of the backup
  const bool& success( void ) const
  { return success_; }
  
  private:
  
  //! saved filename
  std::string file_;
  
  //! true if save is successfull
  bool success_;
  
};

#endif
