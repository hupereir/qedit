#ifndef _TextMacroDialog_h_
#define _TextMacroDialog_h_

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
  \file TextMacroDialog.h
  \brief Syntax highlighting macro editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "CustomDialog.h"
#include "TextMacro.h"

class AnimatedLineEditor;
class TextMacroRuleList;

//! Syntax highlighting macro editing dialog
class TextMacroDialog: public CustomDialog
{
  
  public:
  
  //! constructor
  TextMacroDialog( QWidget* parent );

  //! macro
  void setMacro( const TextMacro& macro );
  
  //! macro
  TextMacro macro( void );
   
  private:
  
  TextMacro macro_;
     
  //! name
  AnimatedLineEditor* name_editor_;
  
  //! accelerator
  AnimatedLineEditor* accelerator_editor_;
  
  //! rules
  TextMacroRuleList* rule_list_;
  
};

#endif
