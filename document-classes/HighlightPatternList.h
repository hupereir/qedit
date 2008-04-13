#ifndef HighlightPatternList_h
#define HighlightPatternList_h

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
  \file HighlightPatternList.h
  \brief List box for HighlightPatterns
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QPushButton>

#include "Counter.h"
#include "HighlightPatternModel.h"

class TreeView;

//! List box for HighlightPatterns
class HighlightPatternList: public QWidget, public Counter
{
  
  Q_OBJECT
  
  public:
  
  //! constructor
  HighlightPatternList( QWidget* parent = 0 );
  
  //! Patterns
  void setPatterns( const HighlightPattern::List& );
  
  //! Patterns
  HighlightPattern::List patterns( void );
  
  //! true when Patterns are modified
  bool modified( void ) const
  { return modified_; }
  
  private slots:
  
  //! update buttons enability
  void _updateButtons( void );
  
  //! edit selected Pattern
  void _edit( void );
  
  //! remove selected Pattern
  void _remove( void );
  
  //! add new Pattern
  void _add( void );
  
  //! store selection
  void _storeSelection( void );
  
  //! restore selection
  void _restoreSelection( void );
  
  private:
  
  //! list
  TreeView* list_;
  
  //! model
  HighlightPatternModel model_;
  
  //! buttons
  QPushButton* edit_button_;
  
  //! buttons
  QPushButton* remove_button_;
  
  //! modification state
  bool modified_;
  
};

#endif
