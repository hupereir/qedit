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

#include <cassert>
#include <QMenuBar>
#include <map>

#include "Debug.h"
#include "Counter.h"
#include "File.h"

class RecentFilesMenu;
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
  
  //! recent files menu
  RecentFilesMenu& recentFilesMenu( void ) const
  { 
    assert( recent_files_menu_ );
    return *recent_files_menu_;
  }
  
  //! macro menu
  QMenu& macroMenu( void ) const
  { 
    assert( macro_menu_ );
    return *macro_menu_;
  }
  
  signals:
  
  //! emmited every time a document class is selected
  void documentClassSelected( QString );
    
  private slots:
    
  //! update recent files menu
  /*! this is needed so that the current file appears checked in the menu */
  void _updateRecentFilesMenu( void );
  
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
    
  //! select document class from menu
  void _selectClassName( QAction* );
  
  //! select macro from menu
  void _selectMacro( QAction* );
  
  //! select file from windows menu
  void _selectFile( QAction* );
    
  private:
    
  //! local action group
  /*! used to track memory leaks */
  class ActionGroup: public QActionGroup, public Counter
  {
    
    public:
    
    //! constructor
    ActionGroup( QObject* parent ):
      QActionGroup( parent ),
      Counter( "Menu::ActionGroup" )
    {}
    
  };
  
  //!@name children
  //@{
    
  //! recent files menu
  RecentFilesMenu* recent_files_menu_;
  
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
    
  //! windows menu
  QMenu* windows_menu_;
  
  //@}
  
  //!@name action groups
  //@{
  
  QActionGroup* document_class_action_group_;
    
  QActionGroup* windows_action_group_;

  //@}
  
  
  //!@name actions
  //@{
        
  //! map document class to macro names
  std::map< QAction*, QString > document_class_actions_;
  
  //! map actions to macro names
  std::map< QAction*, QString > macro_actions_;

  //! map windows menu ID to file name
  std::map< QAction*, File > file_actions_;
  
  //@}
  
};

#endif
