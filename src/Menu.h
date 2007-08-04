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

#include <qmenubar.h>
#include <vector>

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
  Menu( QWidget* parent, const std::string& name = "menu" );
  
  //! destructor
  ~Menu( void );

  //! retrieve OpenPreviousMenu
  OpenPreviousMenu& GetOpenPreviousMenu( void ) const
  {
    Exception::CheckPointer( open_previous_menu_, DESCRIPTION( "open_previous_menu_ not initialized.\n" ) );
    return *open_previous_menu_;
  }
  
  signals:
  
  //! emmited every time a document class is selected
  void DocumentClassSelected( std::string );
    
  private slots:
  
  //! configuration
  void _DefaultConfiguration( void );
  
  //! document classes
  void _DocumentClassesConfiguration( void );
  
  //! spell check
  void _SpellCheckConfiguration( void );
  
  //! update file menu
  void _UpdateFileMenu( void );
  
  //! update document class menu
  void _UpdateDocumentClassMenu( void );
  
  //! update edit menu
  void _UpdateEditMenu( void );
  
  //! update search menu
  void _UpdateSearchMenu( void );
  
  //! update preference menu
  void _UpdatePrefMenu( void );
  
  //! update open mode menu
  void _UpdateOpenModeMenu( void );
  
  //! update orientation menu
  void _UpdateOrientationMenu( void );
  
  //! update macro menu
  void _UpdateMacroMenu( void );

  //! update diff menu
  void _UpdateDiffMenu( void );
  
  //! update windows menu
  void _UpdateWindowsMenu( void );
  
  //! update preferences
  void _TogglePreferences( int );

  //! update open mode
  void _ToggleOpenMode( int );

  //! update orientation
  void _ToggleOrientation( int );
  
  //! select document class from menu
  void _SelectClassName( int );
  
  //! select macro from menu
  void _SelectMacro( int );
  
  //! select macro from menu
  void _SelectFile( int );
    
  private:
  
  //!@name children
  //@{
  
  //! open previous menu, in which the open files are stored
  OpenPreviousMenu* open_previous_menu_;       
  
  //! document class menu
  QPopupMenu* document_class_menu_;
  
  //! file menu
  QPopupMenu* file_menu_;
  
  //! edit menu
  QPopupMenu* edit_menu_;
  
  //! search menu
  QPopupMenu* search_menu_;
  
  //! preference menu
  QPopupMenu* pref_menu_;
  
  //! open mode menu
  QPopupMenu* open_mode_menu_;
  
  //! orientation menu
  QPopupMenu* orientation_menu_;
  
  //! macro menu
  QPopupMenu* macro_menu_;
  
  //! diff menu
  QPopupMenu* diff_menu_; 
  
  //! macro menu id
  int macro_menu_id_;
  
  //! map macro menu ID to macro names
  typedef std::map< int, std::string > MacroMap;
  
  //! map macro menu ID to macro names
  MacroMap macros_;
  
  //! windows menu
  /*! it is updated every time it is shown with the list of opened files */ 
  QPopupMenu* windows_menu_;

  //! map windows menu ID to file name
  typedef std::map< int, File > FileMap;
  
  //! map windows menu ID to file name
  FileMap files_;
  
  //@}
  
  //!@name menu items
  //@{

  int detach_id_;
  int close_view_id_;
  int save_id_;
  int revert_id_;
  int undo_id_;
  int redo_id_;
  int cut_id_;
  int copy_id_;
  int paste_id_;
  int upper_case_id_;
  int lower_case_id_;
  int fill_id_;
  int glue_id_;
  int sel_indent_id_;
  int tabs_id_;
    
  int find_selection_id_; 
  int replace_id_;
  int replace_again_id_;
  
  int wrap_id_;
  int tab_emulation_id_;
  int indent_id_;
  int highlight_id_;
  int highlight_paragraph_id_;
  int braces_id_;
  
  int new_window_id_;
  int new_view_id_;
  
  int left_right_id_;
  int top_bottom_id_;
  
  #if WITH_ASPELL
  int autospell_id_;
  #endif
  
  int diff_id_;
  
  //@}
};

#endif
