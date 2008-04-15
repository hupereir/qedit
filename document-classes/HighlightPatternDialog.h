#ifndef _HighlightPatternDialog_h_
#define _HighlightPatternDialog_h_

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
  \file HighlightPatternDialog.h
  \brief Syntax highlighting pattern editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QComboBox>
#include <QLabel>

#include "CustomDialog.h"
#include "HighlightStyle.h"
#include "HighlightPattern.h"
#include "HighlightStyle.h"

class CustomLineEdit;
class CustomTextEdit;
class HighlightPatternOptions;
class HighlightPatternType;

//! Syntax highlighting pattern editing dialog
class HighlightPatternDialog: public CustomDialog
{
  
  //! qt object
  Q_OBJECT
  
  public:
  
  //! constructor
  HighlightPatternDialog( QWidget* parent );
 
  //! patterns
  void setPatterns( const HighlightPattern::List& );

  //! styles
  void setStyles( const HighlightStyle::Set& );

  //! pattern
  void setPattern( const HighlightPattern& pattern );
  
  //! pattern
  HighlightPattern pattern( void );
 
  private slots:
  
  //! update editors depending on type
  void _updateEditors( HighlightPattern::Type );
  
  private:
    
  //! highlight styles
  HighlightStyle::Set styles_;
  
  //! initial pattern (to avoid duplication of ids)
  HighlightPattern pattern_;
  
  //! name editor
  CustomLineEdit* name_editor_;
  
  //! keyword regexp editor
  CustomLineEdit* keyword_regexp_editor_;
  
  //! end regexp label
  QLabel* end_regexp_label_;
  
  //! end regexp edit
  CustomLineEdit* end_regexp_editor_;

  //! options
  HighlightPatternOptions* pattern_options_;
  
  //! type
  HighlightPatternType* pattern_type_;
  
  //! parent highlight pattern
  QComboBox* parent_combobox_;
  
  //! highlight style combobox
  QComboBox* style_combobox_;
  
  //! comments editor
  CustomTextEdit* comments_editor_;
  
};

#endif
