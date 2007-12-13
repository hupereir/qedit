#ifndef HighlightStyleList_h
#define HighlightStyleList_h

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
  \file HighlightStyleList.h
  \brief List box for HighlightStyles
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QPushButton>

#include "Counter.h"
#include "HighlightStyle.h"

class ListWidget;

//! List box for HighlightStyles
class HighlightStyleList: public QWidget, public Counter
{
  
  Q_OBJECT
  
  public:
  
  //! constructor
  HighlightStyleList( QWidget* parent = 0 );
  
  //! styles
  void setStyles( const HighlightStyle::Set& );
  
  //! styles
  HighlightStyle::Set styles( void );
  
  //! true when styles are modified
  bool modified( void ) const
  { return modified_; }
  
  private slots:
  
  //! update buttons enability
  void _updateButtons( void );
  
  //! edit selected style
  void _edit( void );
  
  //! remove selected style
  void _remove( void );
  
  //! add new style
  void _add( void );
  
  private:
  
  //! list
  ListWidget* list_;
  
  //! buttons
  QPushButton* edit_button_;
  
  //! buttons
  QPushButton* remove_button_;
  
  //! modification state
  bool modified_;
  
};

#endif
