#ifndef DocumentClassPreview_h
#define DocumentClassPreview_h

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
  \file DocumentClassPreview.h
  \brief document class preview
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QPushButton>

#include "Counter.h"
#include "IndentPatternModel.h"
#include "DocumentClass.h"

class DocumentClassTextEditor;
class TextMacroMenu;

//! document class preview
class DocumentClassPreview: public QWidget, public Counter
{
    
  //! Qt meta object
  Q_OBJECT
  
  public:
  
  //! constructor
  DocumentClassPreview( QWidget* parent = 0 );
 
  //! editor
  DocumentClassTextEditor& editor( void ) const
  { return *editor_; }
  
  //! reload button
  QPushButton& reloadButton( void ) const
  { return *reload_button_; }

  //! text macro menu
  TextMacroMenu& macroMenu( void ) const
  { return *menu_; }
  
  protected slots:
  
  //! selection
  void _updateSelection( void );
  
  private:
  
  //! editor
  DocumentClassTextEditor* editor_;
  
  //! reload button
  QPushButton* reload_button_;
  
  //! text macro menu
  TextMacroMenu* menu_;
  
};

#endif
