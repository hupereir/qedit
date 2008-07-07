#ifndef _TextParenthesisDialog_h_
#define _TextParenthesisDialog_h_

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
  \file TextParenthesisDialog.h
  \brief Syntax highlighting parenthesis editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "CustomDialog.h"
#include "TextParenthesis.h"

class LineEditor;

//! Syntax highlighting parenthesis editing dialog
class TextParenthesisDialog: public CustomDialog
{
  
  public:
  
  //! constructor
  TextParenthesisDialog( QWidget* parent );

  //! parenthesis
  void setParenthesis( const TextParenthesis& parenthesis );
  
  //! parenthesis
  TextParenthesis parenthesis( void );
   
  private:
     
  //! fist parenthesis editor
  LineEditor* first_editor_;
  
  //! end editor
  LineEditor* second_editor_;
  
  //! rexexp editor
  LineEditor* regexp_editor_;
  
};

#endif
