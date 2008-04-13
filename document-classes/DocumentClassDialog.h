#ifndef _DocumentClassDialog_h_
#define _DocumentClassDialog_h_

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
  \file DocumentClassDialog.h
  \brief Syntax highlighting style editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "CustomDialog.h"
#include "DocumentClass.h"

class CustomLineEdit;
class TreeWidget;
class HighlightStyleList; 
class HighlightPatternList;

//! Syntax highlighting style editing dialog
class DocumentClassDialog: public CustomDialog
{
  
  Q_OBJECT
  
  public:
  
  //! constructor
  DocumentClassDialog( QWidget* parent );
  
  //! style
  void setDocumentClass( const DocumentClass& );
  
  //! true if any document class attribute has been modified
  bool modified( void );
  
  private:
  
  //! name editor
  CustomLineEdit* name_editor_;
  
  //! file pattern editor
  CustomLineEdit* file_pattern_editor_;
  
  //! first line pattern editor
  CustomLineEdit* first_line_pattern_editor_;
  
  //! highlight style list
  HighlightStyleList* highlight_style_list_;
  
  //! highlight pattern list
  HighlightPatternList* highlight_pattern_list_;
  
//   //! indent pattern list
//   TreeWidget* indent_pattern_list_;

  //! modification state
  bool modified_;
  
};

#endif
