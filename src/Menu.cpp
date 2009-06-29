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
 \file Menu.cpp
 \brief main menu
 \author Hugo Pereira
 \version $Revision$
 \date $Date$
*/

#include "Application.h"
#include "BlockDelimiterDisplay.h"
#include "Config.h"
#include "DebugMenu.h"
#include "DefaultHelpText.h"
#include "DocumentClassMenu.h"
#include "HelpManager.h"
#include "HelpText.h"
#include "MainWindow.h"
#include "Menu.h"
#include "NavigationFrame.h"
#include "RecentFilesMenu.h"
#include "SessionFilesFrame.h"
#include "InformationDialog.h"
#include "Singleton.h"
#include "TextDisplay.h"
#include "TextMacro.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlOptions.h"

using namespace std;
using namespace Qt;

//_______________________________________________
Menu::Menu( QWidget* parent ):
  QMenuBar( parent ),
  Counter( "Menu" )
{
  Debug::Throw( "Menu::Menu.\n" );

  // file menu
  QMenu* menu = addMenu( "&File" );

  // retrieve mainwindow
  Application& application( *Singleton::get().application<Application>() );
  MainWindow& mainwindow( *static_cast<MainWindow*>( window() ) );
  
  menu->addAction( &mainwindow.newFileAction() );
  menu->addAction( &mainwindow.cloneAction() );
  menu->addAction( &mainwindow.detachAction() );
  menu->addAction( &mainwindow.openAction() );

  // open previous menu
  recent_files_menu_ = new RecentFilesMenu( this, application.recentFiles() );
  menu->addMenu( recent_files_menu_ );
  
  // additional actions
  menu->addSeparator();
  menu->addAction( &mainwindow.closeDisplayAction() );
  menu->addAction( &mainwindow.closeWindowAction() );
  menu->addAction( &mainwindow.saveAction() );
  menu->addAction( &mainwindow.saveAsAction() );
  menu->addAction( &application.windowServer().saveAllAction() );
  menu->addAction( &mainwindow.revertToSaveAction() );
  menu->addSeparator();

  // document class
  menu->addMenu( document_class_menu_ = new DocumentClassMenu( this ) );
  document_class_menu_->setTitle( "Set &document class" );

  // print and close
  menu->addAction( &mainwindow.printAction() );

  menu->addSeparator();
  menu->addAction( &application.closeAction() );

  // recent files menu current-file needs to be updated prior to the menu to be shown
  // this is performed every time the "file menu" is shown.
  connect( menu, SIGNAL( aboutToShow() ), SLOT( _updateRecentFilesMenu() ) );

  // Edit menu
  edit_menu_ = addMenu( "&Edit" );
  connect( edit_menu_, SIGNAL( aboutToShow() ), SLOT( _updateEditMenu() ) );

  // Search menu
  search_menu_ = addMenu( "&Search" );
  connect( search_menu_, SIGNAL( aboutToShow() ), SLOT( _updateSearchMenu() ) );

  // tools
  tools_menu_ = addMenu( "&Tools" );
  connect( tools_menu_, SIGNAL( aboutToShow() ), this, SLOT( _updateToolsMenu() ) );
  
  // macros
  macro_menu_ = addMenu( "&Macro" );
  connect( macro_menu_, SIGNAL( aboutToShow() ), this, SLOT( _updateMacroMenu() ) );
  connect( macro_menu_, SIGNAL( triggered( QAction* ) ), SLOT( _selectMacro( QAction* ) ) );

  // Settings
  preference_menu_ = addMenu( "&Settings" );
  connect( preference_menu_, SIGNAL( aboutToShow() ), this, SLOT( _updatePreferenceMenu() ) );
    
  // windows
  windows_action_group_ = new ActionGroup( this );
  windows_menu_ = addMenu( "&Window" );
  connect( windows_menu_, SIGNAL( aboutToShow() ), this, SLOT( _updateWindowsMenu() ) );
  connect( windows_menu_, SIGNAL( triggered( QAction* ) ), SLOT( _selectFile( QAction* ) ) );

  // help manager
  BASE::HelpManager* help( new BASE::HelpManager( this ) );
  help->setWindowTitle( "Qedit Handbook" );
  File help_file( XmlOptions::get().raw( "HELP_FILE" ) );
  if( help_file.exists() ) help->install( help_file );
  else
  {
    help->setFile( help_file );
    help->install( HelpText );
    help->install( BASE::HelpText, false );
  }  

  // create help menu
  menu = addMenu( "&Help" );
  menu->addAction( &help->displayAction() );
  menu->addSeparator();
  menu->addAction( &application.aboutQtAction() );
  menu->addAction( &application.aboutAction() );
  
  // debug menu
  #ifdef DEBUG
  menu->addSeparator();
  DebugMenu *debug_menu( new DebugMenu( this ) );
  debug_menu->setTitle( "&Debug" );
  debug_menu->addAction( &help->dumpAction() );
  debug_menu->addAction( &application.monitoredFilesAction() );
  menu->addMenu( debug_menu );
  #endif
  
}

//_______________________________________________
Menu::~Menu( void )
{ Debug::Throw( "Menu::~Menu.\n" ); }
  
//_______________________________________________
void Menu::_updateRecentFilesMenu( void )
{
  Debug::Throw( "Menu::_updateRecentFilesMenu.\n" );
  recent_files_menu_->setCurrentFile( static_cast<MainWindow*>( Menu::window() )->activeDisplay().file() );
}

//_______________________________________________
void Menu::_updateEditMenu( void )
{
  Debug::Throw( "Menu::_updateEditMenu.\n" );
  
  edit_menu_->clear();
  
  TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
  edit_menu_->addAction( &display.undoAction() );
  edit_menu_->addAction( &display.redoAction() );
  edit_menu_->addSeparator();
  
  edit_menu_->addAction( &display.cutAction() );
  edit_menu_->addAction( &display.copyAction() );
  edit_menu_->addAction( &display.pasteAction() );
  edit_menu_->addSeparator();
 
  edit_menu_->addAction( &display.upperCaseAction() );
  edit_menu_->addAction( &display.lowerCaseAction() );
  
}

//_______________________________________________
void Menu::_updateSearchMenu( void )
{
  Debug::Throw( "Menu::_updateSearchMenu.\n" );

  search_menu_->clear();
  
  TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
  search_menu_->addAction( &display.findAction() );
  search_menu_->addAction( &display.findAgainAction() );
  search_menu_->addAction( &display.findSelectionAction() );
  search_menu_->addAction( &display.replaceAction() );
  search_menu_->addAction( &display.replaceAgainAction() );
  search_menu_->addSeparator();
  
  search_menu_->addAction( &display.gotoLineAction() );

}

//_______________________________________________
void Menu::_updatePreferenceMenu( void )
{

  Debug::Throw( "Menu::_updatePreferenceMenu.\n" );

  // reference to needed objects
  Application& application( *Singleton::get().application<Application>() );
  MainWindow& mainwindow( *static_cast<MainWindow*>(window()) );
  TextDisplay& display( mainwindow.activeDisplay() );

  // clear menu
  preference_menu_->clear();
  
  // textdisplay actions
  preference_menu_->addAction( &mainwindow.navigationFrame().visibilityAction() );
  preference_menu_->addAction( &display.showLineNumberAction() );
  preference_menu_->addAction( &display.showBlockDelimiterAction() );
  preference_menu_->addAction( &display.wrapModeAction() );
  preference_menu_->addAction( &display.tabEmulationAction() );
  preference_menu_->addAction( &display.textIndentAction() );
  preference_menu_->addAction( &display.textHighlightAction() );
  preference_menu_->addAction( &display.blockHighlightAction() );
  preference_menu_->addAction( &display.parenthesisHighlightAction() );
  
  #if WITH_ASPELL
  preference_menu_->addSeparator();
  preference_menu_->addAction( &display.autoSpellAction() );
  preference_menu_->addAction( &display.dictionaryMenuAction() );
  preference_menu_->addAction( &display.filterMenuAction() );
  #endif
  
  // configurations (from application)
  preference_menu_->addSeparator();
  #if WITH_ASPELL
  preference_menu_->addAction( &application.spellCheckConfigurationAction() );
  #endif
  preference_menu_->addAction( &application.documentClassConfigurationAction() );
  preference_menu_->addAction( &application.configurationAction() );

  return;
}

//_______________________________________________
void Menu::_updateToolsMenu( void )
{
  
  Debug::Throw( "Menu::_updateToolsMenu.\n" );

  // retrieve mainwindow and current display
  MainWindow& mainwindow( *static_cast<MainWindow*>(window()) );
  TextDisplay& display( mainwindow.activeDisplay() );
  
  // retrieve flags needed to set button state
  bool editable( !display.isReadOnly() );
  bool has_selection( display.textCursor().hasSelection() );
  bool has_indent( display.textIndentAction().isEnabled() );

  // clear menu
  tools_menu_->clear();
  
  // selection indentation
  tools_menu_->addAction( &display.indentSelectionAction() );
  display.indentSelectionAction().setEnabled( editable && has_selection && has_indent );

  //if( display.baseIndentAction().isEnabled() ) 
  { tools_menu_->addAction( &display.baseIndentAction() ); }
  
  // tab replacement
  tools_menu_->addAction( &display.leadingTabsAction() );
  display.leadingTabsAction().setEnabled( display.hasLeadingTabs() );
    
  // spell checker
  tools_menu_->addAction( &display.spellcheckAction() );
    
  // diff files
  tools_menu_->addSeparator();
  tools_menu_->addAction( &mainwindow.diffAction() );
 
  bool has_tags( display.hasTaggedBlocks() );
  bool current_block_tagged( has_tags && display.isCurrentBlockTagged() );
  
  tools_menu_->addAction( &display.tagBlockAction() );
  display.tagBlockAction().setEnabled( has_selection );
  
  tools_menu_->addAction( &display.nextTagAction() );
  display.nextTagAction().setEnabled( has_tags );
  
  tools_menu_->addAction( &display.previousTagAction() );
  display.previousTagAction().setEnabled( has_tags );

  tools_menu_->addAction( &display.clearTagAction() );
  display.clearTagAction().setEnabled( current_block_tagged );

  tools_menu_->addAction( &display.clearAllTagsAction() );
  display.clearAllTagsAction().setEnabled( has_tags );

  // blocks delimiters
  if( display.showBlockDelimiterAction().isEnabled() && display.showBlockDelimiterAction().isChecked() )
  {
    tools_menu_->addSeparator();
    display.blockDelimiterDisplay().updateCurrentBlockActionState();
    display.blockDelimiterDisplay().addActions( *tools_menu_ );
  }
  
  // rehighlight
  tools_menu_->addSeparator();
  QAction* action = tools_menu_->addAction( "&Rehighlight", window(), SLOT( rehighlight() ) ); 
  bool enabled( display.textHighlightAction().isEnabled() && display.textHighlightAction().isChecked() );
  
  #if WITH_ASPELL
  enabled |= ( display.autoSpellAction().isEnabled() || display.autoSpellAction().isChecked() );
  #endif

  action->setEnabled( enabled );

    
}

//_______________________________________________
void Menu::_updateMacroMenu( void )
{

  Debug::Throw( "Menu::_updateMacroMenu.\n" );

  // retrieve current display
  TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
  
  // clear menu
  macro_menu_->clear();
  macro_actions_.clear();

  // retrieve flags needed to set button state
  bool has_selection( display.textCursor().hasSelection() );

  // insert document class specific macros
  const TextMacro::List& macros( display.macros() );
  QAction* action;
  for( TextMacro::List::const_iterator iter = macros.begin(); iter != macros.end(); iter++ )
  {
    
    if( iter->isSeparator() ) macro_menu_->addSeparator();
    else {
      
      // create menu entry
      action = iter->action();
      action->setEnabled( has_selection );
      macro_menu_->addAction( action );
            
      // insert in map
      macro_actions_.insert( make_pair( action, iter->name() ) );
      
    }
  }
  
  return;
}

//_______________________________________________
void Menu::_updateWindowsMenu( void )
{

  Debug::Throw( "Menu::_updateWindowsMenu.\n" );
  windows_menu_->clear();

  // retrieve current display
  TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
  windows_menu_->addAction( &display.filePropertiesAction() );
  
  const QString& current_file( display.file() );
  
  // clear files map
  file_actions_.clear();
      
  // retrieve all files
  bool first = true;
  FileRecord::List records( Singleton::get().application<Application>()->windowServer().records() );
  for( FileRecord::List::const_iterator iter = records.begin(); iter != records.end(); iter++ )
  { 
    
    // retrieve file and check
    const File& file( iter->file() );
    
    // if first valid file, add separator
    if( first ) 
    {
      windows_menu_->addSeparator(); 
      first = false;
    }
      
    // add menu item
    QAction* action = windows_menu_->addAction( file );
    action->setCheckable( true );
    action->setChecked( current_file == file );
    windows_action_group_->addAction( action );
      
    // insert in map for later callback.
    file_actions_.insert( make_pair( action, file ) );
    
  }
  
  
  windows_menu_->addSeparator(); 
  SessionFilesFrame& session_files_frame( static_cast<MainWindow*>(window())->navigationFrame().sessionFilesFrame() );
  windows_menu_->addAction( &session_files_frame.previousFileAction() );
  windows_menu_->addAction( &session_files_frame.nextFileAction() );
  
}

//_______________________________________________
void Menu::_selectMacro( QAction* action )
{
  Debug::Throw( "Menu::_SelectMacro.\n" );
  
  // try retrieve id in map
  std::map< QAction*, QString >::iterator iter = macro_actions_.find( action );
  if( iter == macro_actions_.end() ) return;
  
  // retrieve current Text Display
  TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
  display.processMacro( iter->second );
  
  return;
}

//_______________________________________________
void Menu::_selectFile( QAction* action )
{
  Debug::Throw( "Menu::_selectFile.\n" );
  
  // try retrieve id in map
  std::map<QAction*, File>::iterator iter = file_actions_.find( action );
  if( iter == file_actions_.end() ) return;
  
  // retrieve all mainwindows
  BASE::KeySet<MainWindow> windows( &Singleton::get().application<Application>()->windowServer() );
  
  // retrieve window matching file name
  BASE::KeySet<MainWindow>::iterator window_iter( find_if(
    windows.begin(),
    windows.end(),
    MainWindow::SameFileFTor( iter->second ) ) );
  
  // check if window was found
  if( window_iter == windows.end() )
  { 
    QString buffer;
    QTextStream( &buffer ) << "Unable to find a window containing file " << iter->second;
    InformationDialog( this, buffer ).exec();
    return;
  }
  
  // select display in found window
  (*window_iter)->selectDisplay( iter->second );
  (*window_iter)->uniconify();
  
  return;
}
