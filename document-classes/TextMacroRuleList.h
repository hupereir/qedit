#ifndef TextMacroRuleList_h
#define TextMacroRuleList_h

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
  \file TextMacroRuleList.h
  \brief List box for TextMacroRules
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QPushButton>
#include <QGroupBox>

#include "Counter.h"
#include "TextMacroRuleModel.h"
#include "DocumentClass.h"

class TreeView;

//! List box for TextMacroRules
class TextMacroRuleList: public QGroupBox, public Counter
{
  
  Q_OBJECT
  
  public:
  
  //! constructor
  TextMacroRuleList( QWidget* parent = 0 );
 
  //! rule
  void setRules( const TextMacro::Rule::List& );
  
  //! Rule
  TextMacro::Rule::List rules( void );
  
  //! true when Rule are modified
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

  //! move up selected task
  void _up( void );
  
  //! move down selected task
  void _down( void );
  
  private:
  
  //! list
  TreeView* list_;
  
  //! model
  TextMacroRuleModel model_;
  
  //! buttons
  QPushButton* edit_button_;
  
  //! buttons
  QPushButton* remove_button_;

  //! buttons
  QPushButton* move_up_button_;
  
  //! buttons
  QPushButton* move_down_button_;
  
  //! modification state
  bool modified_;
  
};

#endif
