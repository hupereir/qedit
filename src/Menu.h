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

#ifndef Menu_h
#define Menu_h

/*!
   \file Menu.h
   \brief menu
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QMenuBar>
#include <map>

#include "Debug.h"
#include "Counter.h"
#include "Exception.h"
#include "File.h"

class OpenPreviousMenu;
class TextMacro;

//! menu
class Menu:public QMenuBar, public Counter 
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public:   
  
  //! creator
  Menu( QWidget* parent );
  
  //! destructor
  ~Menu( void );

  //! retrieve OpenPreviousMenu
  OpenPreviousMenu& openPreviousMenu( void ) const
  {
    Exception::checkPointer( open_previous_menu_, DESCRIPTION( "open_previous_menu_ not initialized.\n" ) );
    return *open_previous_menu_;
  }
  
  //! macro menu
  QMenu& macroMenu( void ) const
  { 
    Exception::checkPointer( macro_menu_, DESCRIPTION( "macro_menu_ not initialized.\n" ) );
    return *macro_menu_;
  }
  
  signals:
  
  //! emmited every time a document class is selected
  void documentClassSelected( std::string );
    
  private slots:
    
  //! update document class menu
  void _updateDocumentClassMenu( void );
  
  //! update edit menu
  void _updateEditMenu( void );
  
  //! update search menu
  void _updateSearchMenu( void );
  
  //! update preference menu
  void _updatePreferenceMenu( void );

  //! update tools menu
  void _updateToolsMenu( void );
    
  //! update macro menu
  void _updateMacroMenu( void );

  //! update windows menu
  void _updateWindowsMenu( void );
  
  //! update open mode
  void _toggleOpenMode( void );

  //! update orientation
  void _toggleOrientation( void );
  
  //! select document class from menu
  void _selectClassName( QAction* );
  
  //! select macro from menu
  void _selectMacro( QAction* );
  
  //! select file from windows menu
  void _selectFile( QAction* );
    
  private:
  
  //!@name children
  //@{
  
  //! open previous menu, in which the open files are stored
  OpenPreviousMenu* open_previous_menu_;       
  
  //! document class menu
  QMenu* document_class_menu_;
  
  //! edit menu
  QMenu* edit_menu_;
  
  //! search menu
  QMenu* search_menu_;
  
  //! preference menu
  QMenu* preference_menu_;
  
  //! toold menu
  QMenu* tools_menu_;
  
  //! macro menu
  QMenu* macro_menu_;
  
  //! diff menu
  //QMenu* diff_menu_; 
  
  //! windows menu
  /*! it is updated every time it is shown with the list of opened files */ 
  QMenu* windows_menu_;
  
  //@}
  
  //!@name actions
  //@{
  
  //! open new window mode
  QAction* new_window_action_;

  //! open new view mode
  QAction* new_view_action_;
  
  //! left/right orientated new view
  QAction* leftright_action_;

  //! top/bottom orientated new view
  QAction* topbottom_action_;
    
  //! map document class to macro names
  std::map< QAction*, std::string > document_classes_;
  
  //! map actions to macro names
  std::map< QAction*, std::string > macros_;

  //! map windows menu ID to file name
  std::map< QAction*, File > files_;

  //@}
  
};

#endif
