// $Id$
#ifndef _DocumentClassModifiedDialog_h_
#define _DocumentClassModifiedDialog_h_
 
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
  \file DocumentClassModifiedDialog.h
  \brief QDialog used when a file has been removed from disk
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "BaseDialog.h"
#include "Counter.h"
#include "File.h"

//! QDialog used when a file has been removed from disk
class DocumentClassModifiedDialog: public BaseDialog, public Counter
{
  
  //! Qt macro
  Q_OBJECT
      
  public:
  
  //! return codes
  enum ReturnCode
  {
    
    //! re-saved removed file
    YES = 0,
        
    //! save file with new name
    NO = 1,
        
    //! close window
    CANCEL = 2
        
  };
          
  //! constructor
  DocumentClassModifiedDialog( QWidget* parent );
  
  private slots:
      
  //! save modifications
  void _yes( void )
  { done( YES ); }
  
  //! ignore modifications
  void _no( void )
  { done( NO ); }
  
  //! cancel event
  void _cancel( void )
  { done( CANCEL ); }
    
};

#endif
