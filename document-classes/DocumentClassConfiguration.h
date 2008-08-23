#ifndef DocumentClassConfiguration_h
#define DocumentClassConfiguration_h

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
  \file DocumentClassConfiguration.h
  \brief document class configuration
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/
#include <QSpinBox>
#include <QGroupBox>
#include <QCheckBox>

#include "Counter.h"
#include "DocumentClass.h"

class LineEditor;
class BrowsedLineEditor;

//! List box for HighlightPatterns
class DocumentClassConfiguration: public QGroupBox, public Counter
{
  
  public:
  
  //! constructor
  DocumentClassConfiguration( QWidget* parent = 0 );

  //! set document class
  void setDocumentClass( const DocumentClass& document_class );
  
  //! update document class
  DocumentClass documentClass( void );
    
  private:
    
  //! document class
  DocumentClass document_class_;
  
  //! name editor
  LineEditor* name_editor_;
  
  //! icon
  LineEditor* icon_editor_;
  
  //! name editor
  BrowsedLineEditor* file_editor_;

  //! file pattern editor
  LineEditor* file_pattern_editor_;
  
  //! first line pattern editor
  LineEditor* first_line_pattern_editor_;
  
  //! base indentation
  QSpinBox* base_indentation_spinbox_;
  
  //! default
  QCheckBox* default_checkbox_;
  
  //! wrap
  QCheckBox* wrap_checkbox_;
  
  //! wrap
  QCheckBox* tab_emulation_checkbox_;

};

#endif
