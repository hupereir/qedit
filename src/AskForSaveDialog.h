// $Id$
#ifndef _AskForSaveDialog_h_
#define _AskForSaveDialog_h_
 
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
  \file AskForSaveDialog.h
  \brief QDialog used to ask if modifications of a file should be saved
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDialog>
#include "Counter.h"
#include "File.h"

//! QDialog used to ask if modifications of a file should be saved
class AskForSaveDialog: public QDialog, public Counter
{
  
  //! Qt macro
  Q_OBJECT
      
  public:
  
  //! return codes
  /*! it is also used to decide which buttons are to be shown */
  enum ReturnCode 
  {
    
    //! file is to be saved
    YES = 1<<0,
        
    //! file is not to be saved
    NO = 1<<1,
    
    //! all modified files are to be saved
    YES_TO_ALL = 1<<2,

    //! all modified files are to be saved
    NO_TO_ALL = 1<<3,

    //! action is canceled
    CANCEL = 1<<4
        
  };
          
  //! constructor
  AskForSaveDialog( QWidget* parent, const File& file, const unsigned int &buttons = YES|NO|CANCEL );
  
  private slots:
      
  //! save changes
  void _yes( void )
  { done( YES ); }
  
  //! discard changes
  void _no( void )
  { done( NO ); }

  //! discard changes
  void _yesToAll( void )
  { done( YES_TO_ALL ); }
  
  //! discard changes
  void _noToAll( void )
  { done( NO_TO_ALL ); }

  //! cancel action
  void _cancel( void )
  { done( CANCEL ); }
  
};

#endif
