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
 \file Menu.cc
 \brief main menu
 \author Hugo Pereira
 \version $Revision$
 \date $Date$
*/

#include <QApplication>
#include <sstream>

#include "DebugMenu.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "EditFrame.h"
#include "HelpManager.h"
#include "HelpText.h"
#include "MainFrame.h"
#include "Menu.h"
#include "OpenPreviousMenu.h"
#include "QtUtil.h"
#include "XmlOptions.h"
#include "TextDisplay.h"
#include "TextMacro.h"
#include "Util.h"

#include "Config.h"

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

  // retrieve editframe
  EditFrame* editframe( static_cast<EditFrame*>( window() ) );
  
  menu->addAction( editframe->newAction() );
  menu->addAction( editFrame->cloneAction() );
  menu->addAction( editFrame->detachAction() );
  menu->addAction( editFrame->openAction() );

  // open previous menu
  open_previous_menu_ = new OpenPreviousMenu( this );
  open_previous_menu_->setTitle( "Open pre&vious" );
  open_previous_menu_->setCheck( true );
  menu->addMenu( open_previous_menu_ );

  // connections
  connect( open_previous_menu_, SIGNAL( fileSelected( FileRecord ) ), editFrame, SLOT( open( FileRecord ) ) );
  
  menu->addSeparator();
  menu->addAction( editframe->closeViewAction() );
  menu->addAction( editframe->closeWindowAction() );
  menu->addAction( editframe->saveAction() );
  menu->addAction( editframe->saveAsAction() );
  menu->addAction( editframe->revertToSaveAction() );  
  menu->addSeparator();
  
  document_class_menu_ = menu->addMenu( "Set &document class" );
  connect( document_class_menu_, SIGNAL( aboutToShow() ), SLOT( _updateDocumentClassMenu() ) );
  connect( document_class_menu_, SIGNAL( triggered( QAction* ) ), SLOT( _selectClassName( QAction* ) ) );
  
  menu->addAction( editframe->htmlAction() );
  menu->addAction( editframe->printAction() );

  menu->addSeparator();
  menu->addAction( "E&xit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

  // Edit menu
  edit_menu_ = addMenu( "&Edit" );
  connect( edit_menu_, SIGNAL( aboutToShow() ), SLOT( _updateEditMenu() ) );

  // Search menu
  search_menu_ = addMenu( "&Search" );
  connect( search_menu_, SIGNAL( aboutToShow() ), SLOT( _updateSearchMenu() ) );

  // preferences
  preference_menu_ = addMenu( "&Preferences" );
  connect( search_menu_, SIGNAL( aboutToShow() ), SLOT( _updatePreferenceMenu() ) );
  connect( preference_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdatePrefMenu() ) );
  
  // macros
  macro_menu_ = addMenu( "&Macro" );
  connect( macro_menu_, SIGNAL( aboutToShow() ), this, SLOT( _updateMacroMenu() ) );
  connect( macro_menu_, SIGNAL( triggered( QAction* ) ), SLOT( _selectMacro( QAction* ) ) );
    
  // windows
  windows_menu_ = addMenu( "&Windows" );
  connect( windows_menu_, SIGNAL( aboutToShow() ), this, SLOT( _updateWindowsMenu() ) );
  connect( windows_menu_, SIGNAL( triggered( QAction* ) ), SLOT( _selectFile( QAction* ) ) );

  // create help menu
  menu = addMenu( "&Help" );
  menu->addAction( HelpManager::get().displayAction() );
  menu->addSeparator();
  menu->addAction( "About &Qt", qApp, SLOT( aboutQt() ), 0 );
  menu->addAction( "About Q&Edit", qApp, SLOT( about() ), 0 );

  File help_file( XmlOptions::get().get<File>( "HELP_FILE" ) );
  if( help_file.exist() ) HelpManager::get().install( help_file );
  else
  {
    HelpManager::get().setFile( help_file );
    HelpManager::get().install( HelpText );
  }

  // debug menu
  menu->addSeparator();
  DebugMenu *debug_menu( new DebugMenu( this ) );
  debug_menu->setTitle( "&Debug" );
  debug_menu->addAction( HelpManager::get().dumpAction() );
  menu->addMenu( debug_menu );

}

//_______________________________________________
Menu::~Menu( void )
{ Debug::Throw( "Menu::~Menu.\n" ); }
  
//_______________________________________________
void Menu::_updateDocumentClassMenu( void )
{
  Debug::Throw( "Menu::_UpdateDocumentClassMenu.\n" );

  // clear menu
  document_class_menu_->clear();
  document_classes_.clear();
  QFont font( QMenuBar::font() );
  font.setWeight( QFont::Bold );
  
  // retrieve current class from EditFrame
  EditFrame& frame( *static_cast<EditFrame*>(window()) ); 
  const std::string& class_name( frame.activeDisplay().className() );
  
  // retrieve classes from DocumentClass manager
  const DocumentClassManager::ClassList& classes( static_cast<MainFrame*>(qApp)->classManager().list() );
  for( DocumentClassManager::ClassList::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { 
    // insert actions
    QAction* action = document_class_menu_->addAction( (*iter)->name().c_str() ); 
    if( (*iter)->name() == class_name ) action->setFont( font );
    document_classes_.insert( make_pair( action, (*iter)->name() ) );    
  }
  
  return;

}
//_______________________________________________
void Menu::_updateEditMenu( void )
{
  Debug::Throw( "Menu::_updateEditMenu.\n" );
  
  edit_menu_->clear();
  
  TextDisplay& display( static_cast<EditFrame*>(window())->activeDisplay() );
  edit_menu_->addAction( display.undoAction() );
  edit_menu_->addAction( display.redoAction() );
  edit_menu_->addSeparator();
  
  edit_menu_->addAction( display.cutAction() );
  edit_menu_->addAction( display.copyAction() );
  edit_menu_->addAction( display.pasteAction() );
  edit_menu_->addSeparator();
 
  edit_menu_->addAction( display.upperCaseAction() );
  edit_menu_->addAction( display.lowerCaseAction() );
  
}

//_______________________________________________
void Menu::_updateSearchMenu( void )
{
  Debug::Throw( "Menu::_updateSearchMenu.\n" );

  search_menu_->clear();
  
  TextDisplay& display( static_cast<EditFrame*>(window())->activeDisplay() );
  search_menu_->addAction( display.findAction() );
  search_menu_->addAction( display.findAgainAction() );
  search_menu_->addAction( display.findSelectionAction() );
  search_menu_->addAction( display.replaceAction() );
  search_menu_->addAction( display.replaceAgainAction() );
  search_menu_->addSeparator();
  
  search_menu_->addAction( display.gotoLineAction() );

}

//_______________________________________________
void Menu::_updatePreferenceMenu( void )
{

  Debug::Throw( "Menu::_updatePreferenceMenu.\n" );

  // clear menu
  preference_menu_->clear();
  
  // configurations (from mainFrame)
  MainFrame* mainframe( dynamic_cast<MainFrame*>( qApp ) );
  preference_menu_->addAction( mainframe->configurationAction() );
  preference_menu_->addAction( mainframe->documentClassesAction() );

  EditFrame& frame( *static_cast<EditFrame*>(window()) );
  
  // open mode menu
  QMenu* open_mode_menu = new QMenu( "&Default open mode", this );
  QActionGroup* group( new QActionGroup( open_mode_menu )  );
  group->setExclusive( true );
  
  new_window_action_ = group->addAction( "Open in new &window" );
  new_window_action_->setCheckable( true );
  new_window_action_->setChecked( frame.openMode() == EditFrame::NEW_WINDOW );
  
  new_view_action_ = group->addAction( "Open in new &view" );
  new_view_action_->setCheckable( true );
  new_view_action_->setChecked( frame.openMode() == EditFrame::NEW_VIEW );
  
  open_mode_menu->addActions( group->actions() );
  connect( open_mode_menu, SIGNAL( triggered( QAction* ) ), SLOT( _toggleOpenMode() ) );
  
  // orientation menu
  QMenu* orientation_menu = new QMenu( "&Default layout orientation", this );
  group = new QActionGroup( orientation_menu );
  group->setExclusive( true );  

  leftright_action_  = group->addAction( "&Left/Right" );
  leftright_action_->setCheckable( true );
  leftright_action_->setChecked( frame.GetOrientation() == Qt::Horizontal );
  
  topbottom_action_  = group->addAction( "&Top/Bottom" );
  topbottom_action_->setCheckable( true );
  topbottom_action_->setChecked( frame.GetOrientation() == Qt::Vertical );

  orientation_menu->addActions( group->actions() );
  connect( orientation_menu, SIGNAL( triggered( QAction* ) ), SLOT( _toggleOrientation() ) );
 
  preference_menu_->addSeparator();

  // textdisplay actions
  TextDisplay& display( frame.activeDisplay() );
  preference_menu_->addAction( display.wrapModeAction() );
  preference_menu_->addAction( display.tabEmulationAction() );
  preference_menu_->addAction( display.textIndentAction() );
  preference_menu_->addAction( display.textHighlightAction() );
  preference_menu_->addAction( display.bracesHighlightAction() );
  
  return;
}

//_______________________________________________
void Menu::_updateMacroMenu( void )
{

  Debug::Throw( "Menu::_updateMacroMenu.\n" );

  // retrieve current display
  TextDisplay& display( static_cast<EditFrame*>(window())->activeDisplay() );
  
  // clear menu
  macro_menu_->clear();
  macros_.clear();

  // retrieve flags needed to set button state
  bool editable( !display.isReadOnly() );
  bool has_selection( display.textCursor().hasSelection() );
  bool has_indent( display.textIndentAction().isEnabled() );

  // insert document class specific macros
  const TextDisplay::MacroList& macros( display.macros() );
  QAction* action;
  for( TextDisplay::MacroList::const_iterator iter = macros.begin(); iter != macros.end(); iter++ )
  {
    
    if( (*iter)->isSeparator() ) macro_menu_->addSeparator();
    else {
      
      // create menu entry
      action = (*iter)->action();
      action->setEnabled( has_selection );
      macro_menu_->addAction( action );
            
      // insert in map
      macros_.insert( make_pair( action, (*iter)->name() ) );
      
    }
  }

  // add default macros
  if( !macros.empty() ) macro_menu_->addSeparator();
  
  // selection indentation
  action = macro_menu_->addAction( "&Indent selection", &display, SLOT( indentSelection() ), CTRL+Key_I );
  action->setEnabled( editable && has_selection && has_indent );

  // tab replacement
  action = macro_menu_->addAction( "Replace Leading &Tabs", &display, SLOT( replaceLeadingTabs() ) );
  action->setEnabled( display.hasLeadingTabs() );
  
  // syntax highlighting
  macro_menu_->addAction( "&Rehighlight", window(), SLOT( rehighlight() ) ); 
  
  return;
}

//_______________________________________________
void Menu::_updateWindowsMenu( void )
{

  Debug::Throw( "Menu::_updateWindowsMenu.\n" );
  windows_menu_->clear();
  
  // retrieve current display
  TextDisplay& display( static_cast<EditFrame*>(window())->activeDisplay() );
  windows_menu_->addAction( display.fileInfoAction() );
  
  const string& current_file( display.file() );
  
  // retrieve list of EditFrames
  BASE::KeySet<EditFrame> frames( dynamic_cast<BASE::Key*>(qApp) );

  // clear files map
  files_.clear();
  
  // insert files into menu
  QFont font( QMenuBar::font() );
  font.setWeight( QFont::Bold );
    
  bool first = true;
  set<File> files;
  for( BASE::KeySet<EditFrame>::const_iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
  { 
    
    // retrieve associated TextDisplays
    BASE::KeySet<TextDisplay> displays( *frame_iter );
    for( BASE::KeySet<TextDisplay>::const_iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      
      // retrieve file and check
      const File& file( (*iter)->file() );
      if( file.empty() ) continue;
      
      // check if file was already processed
      if( !files.insert( file ).second ) continue;
      
      // if first valid file, add separator
      if( first ) 
      {
        windows_menu_->addSeparator(); 
        first = false;
      }
      
      // add menu item
      QAction* action = windows_menu_->addAction( file.c_str() );
      if( current_file == file ) action->setFont( font );
      
      // insert in map for later callback.
      files_.insert( make_pair( action, file ) );
    
    }
    
  }
  
}

//_______________________________________________
void Menu::_selectClassName( QAction* action )
{
  Debug::Throw( "Menu::_selectClassName.\n" );
  std::map< QAction*, string >::iterator iter = document_classes_.find( action );
  if( iter != document_classes_.end() ) 
  { emit documentClassSelected( iter->second ); }
  
  return;
  
}

//_______________________________________________
void Menu::_selectMacro( QAction* action )
{
  Debug::Throw( "Menu::_SelectMacro.\n" );
  
  // try retrieve id in map
  std::map< QAction*, string >::iterator iter = macros_.find( action );
  if( iter == macros_.end() ) return;
  
  // retrieve current Text Display
  TextDisplay& display( static_cast<EditFrame*>(window())->activeDisplay() );
  display.processMacro( iter->second );
  
  return;
}

//_______________________________________________
void Menu::_selectFile( QAction* action )
{
  Debug::Throw( "Menu::_selectFile.\n" );
  
  // try retrieve id in map
  std::map<QAction*, File>::iterator iter = files_.find( action );
  if( iter == files_.end() ) return;
  
  // retrieve all editFrames
  BASE::KeySet<EditFrame> frames( dynamic_cast< BASE::Key* >( qApp ) );
  
  // retrieve frame matching file name
  BASE::KeySet<EditFrame>::iterator frame_iter( find_if(
    frames.begin(),
    frames.end(),
    EditFrame::SameFileFTor( iter->second ) ) );
  
  // check if frame was found
  if( frame_iter == frames.end() )
  { 
    ostringstream what;
    what << "Unable to find a window containing file " << iter->second;
    QtUtil::infoDialog( this, what.str() );
    return;
  }
  
  // select display in found frame
  (*frame_iter)->selectDisplay( iter->second );
  (*frame_iter)->uniconify();
  
  return;
}

//_______________________________________________
void Menu::_toggleOpenMode( void )
{
  Debug::Throw("Menu::_toggleOpenMode.\n" );
    
  EditFrame& frame( *static_cast<EditFrame*>(window()) );
  frame.setOpenMode( new_window_action_->isChecked() ? EditFrame::NEW_WINDOW : EditFrame::NEW_VIEW );
  
  return;
}

//_______________________________________________
void Menu::_toggleOrientation( void )
{
  Debug::Throw("Menu::_toggleOrientation.\n" );

  EditFrame& frame( *static_cast<EditFrame*>(window()) );
  frame.setOrientation(  leftright_action_->isChecked() ? Qt::Horizontal : Qt::Vertical );
  return;
}
