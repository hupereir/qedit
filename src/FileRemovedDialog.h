// $Id$
#ifndef _FileRemovedDialog_h_
#define _FileRemovedDialog_h_
 
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
  \file FileRemovedDialog.h
  \brief QDialog used when a file has been removed from disk
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <qdialog.h>
#include "Counter.h"
#include "File.h"

//! QDialog used when a file has been removed from disk
class FileRemovedDialog: public QDialog, public Counter
{
  
  //! Qt macro
  Q_OBJECT
      
  public:
  
  //! return codes
  enum {
    
    //! re-saved removed file
    RESAVE = 0,
        
    //! save file with new name
    SAVE_AS = 1,
        
    //! close window
    CLOSE = 2,
    
    //! ignore warning
    IGNORE = 3
        
  };
          
  //! constructor
  FileRemovedDialog( QWidget* parent, const File& file );
  
  private slots:
      
  //! re-saved removed file
  void _reSave( void )
  { done( RESAVE ); }
  
  //! save file with new name
  void _saveAs( void )
  { done( SAVE_AS ); }
  
  //! close current editor
  void _close( void )
  { done( CLOSE ); }
  
  //! ignore warning
  void _ignore( void )
  { done( IGNORE ); }
  
};

#endif
