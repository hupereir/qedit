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

/*!
  \file AutoSaveThread.h
  \brief independent thread used to make regular automatic saves of files
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/


#include <QThread>
#include <QEvent>

#include "AutoSaveEvent.h"
#include "Counter.h"
#include "Debug.h"
#include "File.h"
#include "Key.h"

//! independent thread used to automatically save file
class AutoSaveThread: public QThread, public BASE::Key, public Counter
{
    
  public:
      
  //! constructor
  AutoSaveThread( QObject* reciever ):
    Counter( "AutoSaveThread" ),
    reciever_( reciever ),
    file_changed_( true ),
    contents_changed_( true )
  {}
  
  //! file
  void SetFile( const File& file )
  { 
    File tmp( autoSaveName( file ) );
    if( tmp != file_ )
    {
      file_changed_ = true;
      file_ = tmp;
    } else file_changed_ = false;
  }
  
  //! file
  const File& file( void ) const
  { return file_; }
  
  //! set content
  void setContents( const QString& contents )
  { 
    if( contents_ != contents )
    { 
      contents_changed_ = true;
      contents_ = contents; 
    } else contents_changed_ = false;
  }
  
  //! generate a new grid. Post a AutoSaveEvent when finished
  void run( void );  
  
  //! create backup file name from file
  static File autoSaveName( const File& file );
  
  private:
 
  //! reciever object for posted events
  QObject* reciever_;
  
  //! filename where data is to be saved
  File file_;
  
  //! content to be saved
  QString contents_;
  
  //! modification flag
  bool file_changed_;
  
  //! modification flag
  bool contents_changed_;

};

#endif
