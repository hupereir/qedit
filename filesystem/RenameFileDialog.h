#ifndef RenameFileDialog_h
#define RenameFileDialog_h

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
 * ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or         
 * FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License         
 * for more details.                    
 *                         
 * You should have received a copy of the GNU General Public License along with 
 * software; if not, write to the Free Software Foundation, Inc., 59 Temple     
 * Place, Suite 330, Boston, MA   02111-1307 USA                          
 *                        
 *                        
 *******************************************************************************/

/*!
   \file RenameFileDialog.h
   \brief Edit/create keyword popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <string>

#include "CustomDialog.h"
#include "FileRecord.h"

class LineEditor;

//! Edit/create keyword popup dialog
class RenameFileDialog: public CustomDialog
{
  
  Q_OBJECT
  
  public:
      
  //! constructor
  RenameFileDialog( QWidget* parent, FileRecord );
  
  //! destructor
  virtual ~RenameFileDialog( void )
  {}

  //! new file name
  File file( void ) const;
  
  private slots:
  
  //! button update
  void _updateButtons( void );
  
  private:
        
  //! line editor
  LineEditor& _editor( void ) const
  { return *editor_; }
  
  //! editor
  LineEditor *editor_;  
        
};

#endif