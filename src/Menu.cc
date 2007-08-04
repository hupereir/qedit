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

#include <qaccel.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <sstream>

#include "ConfigDialog.h"
#include "DebugMenu.h"
#include "DocumentClass.h"
#include "DocumentClassDialog.h"
#include "DocumentClassManager.h"
#include "EditFrame.h"
#include "Help.h"
#include "HelpText.h"
#include "MainFrame.h"
#include "Menu.h"
#include "OpenPreviousMenu.h"
#include "Options.h"
#include "TextDisplay.h"
#include "TextMacro.h"
#include "Util.h"

#include "Config.h"
#if WITH_ASPELL
#include "AutoSpellConfig.h"
#include "SpellConfig.h"
#include "SpellInterface.h"
#endif

using namespace std;
using namespace BASE;

//_______________________________________________
Menu::Menu( QWidget* parent, const string& name ):
  QMenuBar( parent, name.c_str() ),
  Counter( "Menu" )
{
  Debug::Throw( "Menu::Menu.\n" );

  // generic menu
  QAccel* accelerator = new QAccel( this );

  // file menu
  file_menu_ = new QPopupMenu( this, "file" );
  connect( file_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdateFileMenu() ) );

  insertItem( "&File", file_menu_ );
  file_menu_->insertItem( "&New", parent, SLOT( New() ), CTRL+Key_N );
  file_menu_->insertItem( "Clone", parent, SLOT( Clone() ), SHIFT+CTRL+Key_N );
  detach_id_ = file_menu_->insertItem( "Detach", parent, SLOT( Detach() ), SHIFT+CTRL+Key_O );
  file_menu_->insertItem( "&Open", parent, SLOT( Open() ), CTRL+Key_O );

  // open previous menu
  open_previous_menu_ = new OpenPreviousMenu( file_menu_, "open_previous_menu" );
  open_previous_menu_->SetDBFile( Util::Getenv( "HOME", "." ) + "/.qedit_db" );
  if( Options::Find( "DB_SIZE" ) ) open_previous_menu_->SetDBSize( Options::Get<int>( "DB_SIZE" ) );
  file_menu_->insertItem( "Open Pre&vious", open_previous_menu_ );
  connect( open_previous_menu_, SIGNAL( FileSelected( const std::string& ) ), parent, SLOT( Open( const std::string& ) ) );
  
  file_menu_->insertSeparator();
  close_view_id_ = file_menu_->insertItem( "&Close active view", parent, SLOT( CloseView() ), CTRL+Key_W );
  file_menu_->insertItem( "&Close window", parent, SLOT( CloseWindow() ), SHIFT+CTRL+Key_W );
  save_id_ = file_menu_->insertItem( "&Save", parent, SLOT( Save() ), CTRL+Key_S );
  file_menu_->insertItem( "Save &As", parent, SLOT( SaveAs() ) );
  revert_id_ = file_menu_->insertItem( "&Revert to Saved", parent, SLOT( RevertToSave() ) );
  
  file_menu_->insertSeparator();
  document_class_menu_ = new QPopupMenu( file_menu_, "document class" );
  document_class_menu_->setCheckable( true );

  file_menu_->insertItem( "&Set Document Class", document_class_menu_ );
  connect( document_class_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdateDocumentClassMenu() ) );
  connect( document_class_menu_, SIGNAL( activated( int ) ), this, SLOT( _SelectClassName( int ) ) );
  file_menu_->insertItem( "Convert to &HTML", parent, SLOT( ConvertToHtml() ) );
  file_menu_->insertItem( "&Print", parent, SLOT( Print() ), CTRL+Key_P );

  file_menu_->insertSeparator();
  file_menu_->insertItem( "E&xit", qApp, SLOT( Quit() ), CTRL+Key_Q );

  // Edit menu
  edit_menu_ = new QPopupMenu( this, "edit" );
  insertItem( "&Edit", edit_menu_ );
  connect( edit_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdateEditMenu() ) );
  undo_id_ = edit_menu_->insertItem( "&Undo", parent, SLOT( Undo() ), CTRL+Key_Z );
  redo_id_ = edit_menu_->insertItem( "&Redo", parent, SLOT( Redo() ), SHIFT+CTRL+Key_Z );
  edit_menu_->insertSeparator();
  cut_id_ = edit_menu_->insertItem( "&Cut", parent, SLOT( Cut() ), CTRL+Key_X );
  copy_id_ = edit_menu_->insertItem( "&Copy", parent, SLOT( Copy() ), CTRL+Key_C );
  paste_id_ = edit_menu_->insertItem( "&Paste", parent, SLOT( Paste() ), CTRL+Key_V );
  edit_menu_->insertItem( "&SelectAll", parent, SLOT( SelectAll() ), CTRL+Key_A );
  edit_menu_->insertSeparator();
  upper_case_id_ = edit_menu_->insertItem( "Upp&er-case", parent, SLOT( UpperCase() ), CTRL+Key_U );
  lower_case_id_ = edit_menu_->insertItem( "Lo&wer-case", parent, SLOT( LowerCase() ), SHIFT+CTRL+Key_U );
  edit_menu_->insertSeparator();

  fill_id_ = edit_menu_->insertItem( "Fill selection", parent, SLOT( Fill() ), CTRL+Key_J );
  glue_id_ = edit_menu_->insertItem( "Glue selection", parent, SLOT( Glue() ), SHIFT+CTRL+Key_J );

  // Search menu
  search_menu_ = new QPopupMenu( this, "search" );
  insertItem( "&Search", search_menu_ );
  connect( search_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdateSearchMenu() ) );
  search_menu_->insertItem( "&Find", parent, SLOT( Find() ), CTRL+Key_F );
  search_menu_->insertItem( "F&ind Again", parent, SLOT( FindAgain() ), CTRL+Key_G );
  find_selection_id_ = search_menu_->insertItem( "Find &Selection", parent, SLOT( FindSelection() ), CTRL+Key_H );
  replace_id_ = search_menu_->insertItem( "&Replace", parent, SLOT( Replace() ), CTRL+Key_R );
  replace_again_id_ = search_menu_->insertItem( "Re&place Again", parent, SLOT( ReplaceAgain() ), CTRL+Key_T );
  search_menu_->insertSeparator();
  search_menu_->insertItem( "Goto &Line Number", parent, SLOT( GotoLineNumber() ), CTRL+Key_L );

  // accelerators
  accelerator->connectItem( accelerator->insertItem(CTRL+Key_K), parent, SLOT( RemoveLine( void ) ) );
  accelerator->connectItem( accelerator->insertItem(CTRL+Key_G), parent, SLOT( FindAgain( void ) ) );
  accelerator->connectItem( accelerator->insertItem(CTRL+Key_H), parent, SLOT( FindSelection( void ) ) );
  accelerator->connectItem( accelerator->insertItem(CTRL+Key_T), parent, SLOT( ReplaceAgain( void ) ) );
  accelerator->connectItem( accelerator->insertItem(CTRL+Key_U), parent, SLOT( UpperCase( void ) ) );
  accelerator->connectItem( accelerator->insertItem(CTRL+Key_L), parent, SLOT( GotoLineNumber( void ) ) );

  accelerator->connectItem( accelerator->insertItem(SHIFT+CTRL+Key_G), parent, SLOT( FindAgainBackward( void ) ) );
  accelerator->connectItem( accelerator->insertItem(SHIFT+CTRL+Key_H), parent, SLOT( FindSelectionBackward( void ) ) );
  accelerator->connectItem( accelerator->insertItem(SHIFT+CTRL+Key_T), parent, SLOT( ReplaceAgainBackward( void ) ) );
  accelerator->connectItem( accelerator->insertItem(SHIFT+CTRL+Key_U), parent, SLOT( LowerCase( void ) ) );

  // open mode menu
  open_mode_menu_ = new QPopupMenu( this, "open_mode" );
  open_mode_menu_->setCheckable( true );
  new_window_id_ = open_mode_menu_->insertItem( "Open in new &window" );
  new_view_id_ = open_mode_menu_->insertItem( "Open in new &view" );

  connect( open_mode_menu_, SIGNAL( activated( int ) ), this, SLOT( _ToggleOpenMode( int ) ) );
  connect( open_mode_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdateOpenModeMenu() ) );
  
  // orientation menu
  orientation_menu_ = new QPopupMenu( this, "orientation_menu" );
  orientation_menu_->setCheckable( true );
  left_right_id_ = orientation_menu_->insertItem( "&Left/Right" );
  top_bottom_id_ = orientation_menu_->insertItem( "&Top/Bottom" );

  connect( orientation_menu_, SIGNAL( activated( int ) ), this, SLOT( _ToggleOrientation( int ) ) );
  connect( orientation_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdateOrientationMenu() ) );

  // preferences
  pref_menu_ = new QPopupMenu( this, "preferences" );
  insertItem( "&Preferences", pref_menu_ );
  pref_menu_->setCheckable( true );
  pref_menu_->insertItem( "Default &Configuration", this, SLOT( _DefaultConfiguration() ) );
  pref_menu_->insertItem( "D&ocument Classes", this, SLOT( _DocumentClassesConfiguration() ) );
  #if WITH_ASPELL
  pref_menu_->insertItem( "S&pell Check Configuration", this, SLOT( _SpellCheckConfiguration() ) );
  #endif

  pref_menu_->insertSeparator();
  pref_menu_->insertItem( "Open mode", open_mode_menu_ );
  pref_menu_->insertItem( "Layout orientation", orientation_menu_ );

  pref_menu_->insertSeparator();

  wrap_id_ = pref_menu_->insertItem( "&Wrap" );
  tab_emulation_id_ = pref_menu_->insertItem( "&Emulate tabs" ); 
  indent_id_ = pref_menu_->insertItem( "Indent Text" );
  highlight_id_ = pref_menu_->insertItem( "&Highlight Text" );
  highlight_paragraph_id_ = pref_menu_->insertItem( "Highlight &Paragraph" );
  braces_id_ = pref_menu_->insertItem( "Highlight &Braces" );
  #if WITH_ASPELL
  autospell_id_ = pref_menu_->insertItem( "Automatic &Spell Check" );
  #endif

  connect( pref_menu_, SIGNAL( activated( int ) ), this, SLOT( _TogglePreferences( int ) ) );
  connect( pref_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdatePrefMenu() ) );
  
  // macros
  macro_menu_ = new QPopupMenu( this, "macro" );
  connect( macro_menu_, SIGNAL( activated( int ) ), this, SLOT( _SelectMacro( int ) ) );
  connect( macro_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdateMacroMenu() ) );
  insertItem( "Mac&ro", macro_menu_ );
  
  // diff menu
  diff_menu_ = new QPopupMenu( this, "diff" );
  connect( diff_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdateDiffMenu() ) );
  diff_id_ = diff_menu_->insertItem( "&Highlight", parent, SLOT( RunDiff() ) );
  diff_menu_->insertItem( "&Clear", parent, SLOT( ClearParagraphsBackground() ) );
  
  // windows
  windows_menu_ = new QPopupMenu( this, "windows" );
  windows_menu_->setCheckable( true );
  connect( windows_menu_, SIGNAL( activated( int ) ), this, SLOT( _SelectFile( int ) ) );
  connect( windows_menu_, SIGNAL( aboutToShow() ), this, SLOT( _UpdateWindowsMenu() ) );
  insertItem( "&Windows", windows_menu_ );

  // create help menu
  QPopupMenu* menu = new QPopupMenu( this, "help" );
  insertItem( "&Help", menu, -1 );
  menu->insertItem( "&Reference manual", &HelpManager::Get(), SLOT( Display() ) );

  // try load from help file
  if( Options::Find( "HELP_FILE" ) )
  {

    File help_file( Options::Get<File>( "HELP_FILE" ) );
    if( help_file.Exist() ) HelpManager::Get().Install( help_file );
    else
    {
      HelpManager::Get().SetFile( help_file );
      HelpManager::Get().Install( HelpText );
    }
  } else HelpManager::Get().Install( HelpText );

  HelpManager::Get().SetCaption( "QEdit reference manual" );

  menu->insertSeparator();
  menu->insertItem( "About &Qt", qApp, SLOT( aboutQt() ) );
  menu->insertItem( "About Q&Edit", qApp, SLOT( About() ), 0 );

  // debug menu
  menu->insertSeparator();
  DebugMenu *debug_menu( new DebugMenu( this ) );
  menu->insertItem( "&Debug", debug_menu );
  debug_menu->insertItem( "D&ump Help", &HelpManager::Get(), SLOT( DumpHelpString() ) );

}

//_______________________________________________
Menu::~Menu( void )
{ Debug::Throw( "Menu::~Menu.\n" ); }
  
//_______________________________________________
void Menu::_DefaultConfiguration( void )
{
  Debug::Throw( "Menu::_DefaultConfiguration.\n" );
  ConfigDialog dialog( this );
  connect( &dialog, SIGNAL( ConfigChanged() ), qApp, SLOT( UpdateConfiguration() ) );
  dialog.exec();
}

//_______________________________________________
void Menu::_DocumentClassesConfiguration( void )
{
  Debug::Throw( "Menu::_DocumentClassesConfiguration.\n" );
  DocumentClassDialog dialog( this );
  connect( &dialog, SIGNAL( ClassSelected( std::string ) ), parent(), SLOT( SelectClassName( std::string ) ) );
  dialog.exec();
}

//_______________________________________________
void Menu::_SpellCheckConfiguration( void )
{
  
  Debug::Throw( "Menu::_SpellCheckConfiguration.\n" );
  
  #if WITH_ASPELL
  
  // retrieve frame
  EditFrame& frame( *static_cast<EditFrame*>(parent()) );
  
  // retrieve current display
  TextDisplay& display( frame.ActiveDisplay() );
  
  // retrieve Record
  if( !display.GetFile().empty() )
  {
    FileRecord record( GetOpenPreviousMenu().Get( display.GetFile() ) );

    string filter( record.GetInformation( "filter" ) );
    if( !filter.empty() ) Options::Get().SetRaw( "DICTIONARY_FILTER", filter );
    
    string dictionary( record.GetInformation( "dictionary" ) );
    if( !dictionary.empty() ) Options::Get().SetRaw( "DICTIONARY_FILTER", filter );
  }
  
  // create dialog
  CustomDialog dialog( this );
  QVBox* box( &dialog.GetVBox() );
  
  SpellConfig* spell = new SpellConfig( box );
  AutoSpellConfig* autospell = new AutoSpellConfig( box );
  
  spell->Read();
  autospell->Read();
  
  if( dialog.exec() == QDialog::Rejected ) return;
  spell->Write();
  autospell->Write();
  Options::Get().Write();

  // retrieve Record
  SPELLCHECK::SpellInterface::ResetDictionary();
  SPELLCHECK::SpellInterface::ResetFilter();
  
  if( !display.GetFile().empty() )
  {
    SPELLCHECK::SpellInterface interface;
    
    FileRecord& record( GetOpenPreviousMenu().Get( display.GetFile() ) );
    record.AddInformation( "dictionary", interface.Dictionary() );
    record.AddInformation( "filter", interface.Filter() );
    
  }
    
  // update frame
  frame.UpdateFlags();
      
  #endif
  
}

//_______________________________________________
void Menu::_UpdateFileMenu( void )
{
  Debug::Throw( "Menu::_UpdateFileMenu.\n" );
 
  // retrieve parent frame, associated text displays, and current display
  EditFrame &frame( *static_cast<EditFrame*>(parent () ) );
  KeySet<TextDisplay> displays( &frame );
  TextDisplay& display( frame.ActiveDisplay() );
  
  const File& file( display.GetFile() );
  
  file_menu_->setItemEnabled( detach_id_, frame.GetIndependentDisplays() > 1 );
  file_menu_->setItemEnabled( close_view_id_, displays.size() > 1 );
  file_menu_->setItemEnabled( save_id_, !file.empty() );
  file_menu_->setItemEnabled( revert_id_, (!file.empty() && file.Exist() ) );
  
  return;
}

//_______________________________________________
void Menu::_UpdateDocumentClassMenu( void )
{
  Debug::Throw( "Menu::_UpdateDocumentClassMenu.\n" );

  // clear menu
  document_class_menu_->clear();

  // retrieve current class from EditFrame
  EditFrame& frame( *static_cast<EditFrame*>(parent()) ); 
  const std::string& class_name( frame.ActiveDisplay().GetClassName() );
  
  // retrieve classes from DocumentClass manager
  const DocumentClassManager::ClassList& classes( static_cast<MainFrame*>(qApp)->GetClassManager().GetList() );
  for( DocumentClassManager::ClassList::const_iterator iter = classes.begin(); iter != classes.end(); iter++ )
  { 
    int id = document_class_menu_->insertItem( (*iter)->Name() ); 
    if( (*iter)->Name() == class_name )  document_class_menu_->setItemChecked( id, true );
  }
  
  return;

}
//_______________________________________________
void Menu::_UpdateEditMenu( void )
{
  Debug::Throw( "Menu::_UpdateEditMenu.\n" );
  TextDisplay& display( static_cast<EditFrame*>(parent())->ActiveDisplay() );

  bool editable( !display.isReadOnly() );
  bool has_selection( display.hasSelectedText() );

  edit_menu_->setItemEnabled( undo_id_, editable && display.isUndoAvailable() );
  edit_menu_->setItemEnabled( redo_id_, editable && display.isRedoAvailable() );
  edit_menu_->setItemEnabled( cut_id_, editable && has_selection );
  edit_menu_->setItemEnabled( copy_id_, has_selection );
  edit_menu_->setItemEnabled( paste_id_, editable );
  edit_menu_->setItemEnabled( upper_case_id_, editable && has_selection );
  edit_menu_->setItemEnabled( lower_case_id_, editable && has_selection );

  edit_menu_->setItemEnabled( fill_id_, editable && has_selection );
  edit_menu_->setItemEnabled( glue_id_, editable && has_selection );

}

//_______________________________________________
void Menu::_UpdateSearchMenu( void )
{
  Debug::Throw( "Menu::_UpdateSearchMenu.\n" );

  TextDisplay& display( static_cast<EditFrame*>(parent())->ActiveDisplay() );

  bool editable( !display.isReadOnly() );
  bool has_selection( display.hasSelectedText() );
  search_menu_->setItemEnabled( find_selection_id_, has_selection );
  search_menu_->setItemEnabled( replace_id_, editable );
  search_menu_->setItemEnabled( replace_again_id_, editable );
}

//_______________________________________________
void Menu::_UpdatePrefMenu( void )
{
  Debug::Throw( "Menu::_UpdatePrefMenu.\n" );
  const TextDisplay& display( static_cast<EditFrame*>(parent())->ActiveDisplay() );

  // set status
  pref_menu_->setItemChecked( wrap_id_, display.GetFlag( TextDisplay::WRAP ) );
  pref_menu_->setItemChecked( tab_emulation_id_, display.GetFlag( TextDisplay::TAB_EMULATION ) ); 
  pref_menu_->setItemChecked( indent_id_, display.GetFlag( TextDisplay::INDENT) && display.GetFlag( TextDisplay::HAS_INDENT) );
  pref_menu_->setItemChecked( highlight_id_, display.GetFlag( TextDisplay::HIGHLIGHT)&& display.GetFlag( TextDisplay::HAS_HIGHLIGHT) );
  pref_menu_->setItemChecked( highlight_paragraph_id_, display.GetFlag( TextDisplay::HIGHLIGHT_PARAGRAPH)&& display.GetFlag( TextDisplay::HAS_HIGHLIGHT_PARAGRAPH) );
  pref_menu_->setItemChecked( braces_id_, display.GetFlag( TextDisplay::BRACES) && display.GetFlag( TextDisplay::HAS_BRACES) );

  #if WITH_ASPELL
  pref_menu_->setItemChecked( autospell_id_, display.GetFlag( TextDisplay::AUTOSPELL ) );
  #endif
  
  // enable buttons
  pref_menu_->setItemEnabled( indent_id_, display.GetFlag( TextDisplay::HAS_INDENT ) );
  pref_menu_->setItemEnabled( highlight_id_, display.GetFlag( TextDisplay::HAS_HIGHLIGHT ) );
  pref_menu_->setItemEnabled( highlight_paragraph_id_, display.GetFlag( TextDisplay::HAS_HIGHLIGHT_PARAGRAPH ) );
  pref_menu_->setItemEnabled( braces_id_, display.GetFlag( TextDisplay::HAS_BRACES ) );
  
  return;
}

//_______________________________________________
void Menu::_UpdateOpenModeMenu( void )
{
  Debug::Throw( "Menu::_UpdateOpenModeMenu.\n" );
  EditFrame &frame( *static_cast<EditFrame*>( parent() ) );
  
  open_mode_menu_->setItemChecked( new_window_id_,  frame.GetOpenMode() == EditFrame::NEW_WINDOW );
  open_mode_menu_->setItemChecked( new_view_id_,  frame.GetOpenMode() == EditFrame::NEW_VIEW );
  
  return;
}

//_______________________________________________
void Menu::_UpdateOrientationMenu( void )
{
  Debug::Throw( "Menu::_UpdateOrientationMenu.\n" );
  EditFrame &frame( *static_cast<EditFrame*>( parent() ) );
  
  orientation_menu_->setItemChecked( left_right_id_,  frame.GetOrientation() == Qt::Horizontal );
  orientation_menu_->setItemChecked( top_bottom_id_,  frame.GetOrientation() == Qt::Vertical );
  
  return;
}

//_______________________________________________
void Menu::_UpdateMacroMenu( void )
{

  Debug::Throw( "Menu::_UpdateMacroMenu.\n" );

  // retrieve current display
  TextDisplay& display( static_cast<EditFrame*>(parent())->ActiveDisplay() );
  
  Debug::Throw( "Menu::UpdateMacroMenu.\n" );

  // clear menu
  macro_menu_->clear();
  macros_.clear();

  // retrieve flags needed to set button state
  bool editable( !display.isReadOnly() );
  bool has_selection( display.hasSelectedText() );
  bool has_indent( display.GetFlag( TextDisplay::INDENT ) && display.GetFlag( TextDisplay::HAS_INDENT ) );

  // insert document class specific macros
  const TextDisplay::MacroList& macros( display.GetMacros() );
  for( TextDisplay::MacroList::const_iterator iter = macros.begin(); iter != macros.end(); iter++ )
  {
    
    if( (*iter)->IsSeparator() ) macro_menu_->insertSeparator();
    else {
      
      // create menu entry
      int id = macro_menu_->insertItem( (*iter)->Name().c_str() );
      
      // set accelerator
      if( (*iter)->Accelerator().size() ) macro_menu_->setAccel( QKeySequence( (*iter)->Accelerator().c_str() ), id );
      
      // set item enable state
      macro_menu_->setItemEnabled( id, has_selection );
      
      // insert in map
      macros_.insert( make_pair( id, (*iter)->Name() ) );
      
    }
  }

  // add default macros
  if( !macros.empty() ) macro_menu_->insertSeparator();

  // diff files
  macro_menu_->insertItem( "&Diff", diff_menu_ );
  
  // selection indentation
  sel_indent_id_ = macro_menu_->insertItem( "&Indent Selection", parent(), SLOT( IndentSelection() ), CTRL+Key_I );
  macro_menu_->setItemEnabled( sel_indent_id_, editable && has_selection && has_indent );

  // tab replacement
  tabs_id_ = macro_menu_->insertItem( "Replace Leading &Tabs", parent(), SLOT( ReplaceLeadingTabs() ) );

  #if WITH_ASPELL
  // spell checking
  macro_menu_->insertItem( "S&pell check", parent(), SLOT( SpellCheck() ) );
  #endif
  
  // syntax highlighting
  macro_menu_->insertItem( "&Rehighlight", parent(), SLOT( Rehighlight() ) ); 
  
  return;
}

//_______________________________________________
void Menu::_UpdateDiffMenu( void )
{

  Debug::Throw( "Menu::_UpdateDiffMenu.\n" );
  diff_menu_->setItemEnabled( diff_id_, static_cast<EditFrame*>(parent())->GetIndependentDisplays() == 2 );
  return;
  
}

//_______________________________________________
void Menu::_UpdateWindowsMenu( void )
{

  Debug::Throw( "Menu::_UpdateWindowsMenu.\n" );
  windows_menu_->clear();
  windows_menu_->insertItem( "&File Information", parent(), SLOT( FileInfo() ) );
  
  // retrieve current selected file if any
  TextDisplay& display( static_cast<EditFrame*>( parent() )->ActiveDisplay() );
  const string& current_file( display.GetFile() );
  
  // retrieve list of EditFrames
  KeySet<EditFrame> frames( dynamic_cast<BASE::Key*>(qApp) );

  // clear files map
  files_.clear();
  
  // insert files into menu
  bool first = true;
  set<File> file_set;
  for( KeySet<EditFrame>::const_iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
  { 
    
    // retrieve associated TextDisplays
    KeySet<TextDisplay> displays( *frame_iter );
    for( KeySet<TextDisplay>::const_iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      
      // retrieve file and check
      const File& file( (*iter)->GetFile() );
      if( file.empty() ) continue;
      
      // check if file was already processed
      if( file_set.find( file ) != file_set.end() ) continue;
      file_set.insert( file );
      
      // if first valid file, add separator
      if( first ) 
      {
        windows_menu_->insertSeparator(); 
        first = false;
      }
      
      // add menu item
      // select if current
      int id = windows_menu_->insertItem( file.c_str() );
      if( current_file == file ) windows_menu_->setItemChecked( id, true );
      
      // insert in map for later callback.
      files_.insert( make_pair( id, file ) );
    }
    
  }
  
}

//_______________________________________________
void Menu::_SelectClassName( int id )
{
  Debug::Throw( "Menu::_SelectClassName.\n" );
  emit DocumentClassSelected( document_class_menu_->text(id) );
  return;
}

//_______________________________________________
void Menu::_SelectMacro( int id )
{
  Debug::Throw( "Menu::_SelectMacro.\n" );
  
  // try retrieve id in map
  MacroMap::iterator iter = macros_.find( id );
  if( iter == macros_.end() ) return;
  
  // retrieve current Text Display
  TextDisplay& display( static_cast<EditFrame*>(parent())->ActiveDisplay() );
  display.ProcessMacro( iter->second );
  
  return;
}

//_______________________________________________
void Menu::_SelectFile( int id )
{
  Debug::Throw( "Menu::_SelectFile.\n" );
  
  // try retrieve id in map
  FileMap::iterator iter = files_.find( id );
  if( iter == files_.end() ) return;
  
  // retrieve all editFrames
  KeySet<EditFrame> frames( dynamic_cast< BASE::Key* >( qApp ) );
  
  // retrieve frame matching file name
  KeySet<EditFrame>::iterator frame_iter( find_if(
    frames.begin(),
    frames.end(),
    EditFrame::SameFileFTor( iter->second ) ) );
  
  // check if frame was found
  if( frame_iter == frames.end() )
  { 
    ostringstream what;
    what << "Unable to find a window containing file " << iter->second;
    QtUtil::InfoDialogExclusive( this, what.str() );
    return;
  }
  
  // select display in found frame
  (*frame_iter)->SelectDisplay( iter->second );
  (*frame_iter)->Uniconify();
  
  return;
}

//_______________________________________________
void Menu::_TogglePreferences( int id )
{
  Debug::Throw("Menu::_TogglePreferences.\n" );
  EditFrame& frame( *static_cast<EditFrame*>(parent()) );
  
  // update state of all associated editors
  KeySet<TextDisplay> displays( &frame );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    TextDisplay& display( **iter );
    if( id == wrap_id_ ) {
      
      display.SetFlag( TextDisplay::WRAP, !pref_menu_->isItemChecked( id ) );
      display.SetFlag( TextDisplay::HAS_WRAP, true );
      
    } else if( id == tab_emulation_id_ ) display.SetFlag( TextDisplay::TAB_EMULATION, !pref_menu_->isItemChecked( id ) );
    else if( id == indent_id_ ) display.SetFlag( TextDisplay::INDENT, !pref_menu_->isItemChecked( id ) );
    else if( id == highlight_id_ ) display.SetFlag( TextDisplay::HIGHLIGHT, !pref_menu_->isItemChecked( id ) );
    else if( id == highlight_paragraph_id_ ) display.SetFlag( TextDisplay::HIGHLIGHT_PARAGRAPH, !pref_menu_->isItemChecked( id ) );
    else if( id == braces_id_ ) 
    {
      // change flag
      display.SetFlag( TextDisplay::BRACES, !pref_menu_->isItemChecked( id ) );
      
      // update Braces at current position if any
      display.HighlightBraces();
    }
    
    #if WITH_ASPELL
    else if( id == autospell_id_ ) display.SetFlag( TextDisplay::AUTOSPELL, !pref_menu_->isItemChecked( id ) );
    #endif
    
    else return;
    
  }
  
  // update editor flags and possibly Rehighlight
  if( frame.UpdateFlags() ) frame.Rehighlight();

  return;

}

//_______________________________________________
void Menu::_ToggleOpenMode( int id )
{
  Debug::Throw("Menu::_ToggleOpenMode.\n" );
  
  // do nothing if item is checked (radiobutton behavior)
  if( open_mode_menu_->isItemChecked( id ) ) return;
  
  EditFrame& frame( *static_cast<EditFrame*>(parent()) );
  frame.SetOpenMode( (id == new_window_id_) ? EditFrame::NEW_WINDOW : EditFrame::NEW_VIEW );
  return;
}

//_______________________________________________
void Menu::_ToggleOrientation( int id )
{
  Debug::Throw("Menu::_ToggleOrientation.\n" );

  // do nothing if item is checked (radiobutton behavior)
  if( orientation_menu_->isItemChecked( id ) ) return;

  EditFrame& frame( *static_cast<EditFrame*>(parent()) );
  frame.SetOrientation( ( id == left_right_id_ ) ? Qt::Horizontal : Qt::Vertical );
  return;
}
