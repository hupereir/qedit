#ifndef FileInformationDialog_h
#define FileInformationDialog_h
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
* Place, Suite 330, Boston, MA  02111-1307 USA                           
*                         
*                         
*******************************************************************************/

/*!
   \file FileInformationDialog.h
   \brief file informations
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include "BaseDialog.h"
#include "FileRecord.h"

class AnimatedTabWidget;

//! file informations
class FileInformationDialog: public BaseDialog
{
  
  public:
      
  //! constructor
  FileInformationDialog( QWidget*, const FileRecord& );

  //! tab widget
  /*! needed for customization */
  AnimatedTabWidget& tabWidget( void ) const
  { return *tab_widget_; }
  
  private:
  
  //! tab widget
  AnimatedTabWidget* tab_widget_;
  
};

#endif
