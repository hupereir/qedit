// $Id$

/***************************************************************************
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
****************************************************************************/

/*!
  \file MainWindow.cpp
  \brief editor main window
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QApplication>
#include <QDomElement>
#include <QDomDocument>
#include <QObjectList>
#include <QPrinter>

#include "Application.h"
#include "AutoSave.h"
#include "BlockDelimiterDisplay.h"
#include "ClockLabel.h"
#include "Config.h"
#include "CustomFileDialog.h"
#include "LineEditor.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "Diff.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "HighlightBlockFlags.h"
#include "IconEngine.h"
#include "Icons.h"
#include "MainWindow.h"
#include "Menu.h"
#include "NavigationFrame.h"
#include "NavigationWindow.h"
#include "NewFileDialog.h"
#include "PixmapEngine.h"
#include "PrintDialog.h"
#include "QtUtil.h"
#include "StatusBar.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "Util.h"
#include "WindowServer.h"
#include "WindowTitle.h"
#include "XmlOptions.h"

using namespace std;

//_____________________________________________________
MainWindow::MainWindow(  QWidget* parent ):
  CustomMainWindow( parent ),
  Counter( "MainWindow" ),
  menu_( 0 ),
  statusbar_( 0 ),
  file_editor_( 0 ),
  default_orientation_( Qt::Horizontal ),
  default_open_mode_( TextView::NEW_WINDOW )
{

  Debug::Throw( "MainWindow::MainWindow.\n" );
  _setSizeOptionName( "WINDOW" );
  
  // tell window to delete on exit
  setAttribute( Qt::WA_DeleteOnClose );
  
  // install actions
  _installActions();
  
  // menu
  setMenuBar( menu_ = new Menu( this ) );
  connect( menu_, SIGNAL( documentClassSelected( QString ) ), this, SLOT( selectClassName( QString ) ) );

  // main widget is a splitter to store navigation window and active view 
  setCentralWidget( splitter_ = new QSplitter( this ) );
  splitter_->setOrientation( Qt::Horizontal );
  setCentralWidget( splitter_ );

  // insert navigationFrame
  navigation_frame_ = new NavigationFrame(0, static_cast<Application*>(qApp)->recentFiles() );
  navigationFrame().setDefaultWidth( XmlOptions::get().get<int>( "NAVIGATION_FRAME_WIDTH" ) );
  connect( &navigationFrame().visibilityAction(), SIGNAL( toggled( bool ) ), SLOT( _toggleNavigationFrame( bool ) ) );
  splitter_->addWidget( &navigationFrame() );
  
  // insert main view
  setActiveView( _newTextView(0) );
  splitter_->addWidget( &activeView() );
  
  // assign stretch factors
  splitter_->setStretchFactor( 0, 0 );
  splitter_->setStretchFactor( 1, 1 );
  
  connect( splitter_, SIGNAL( splitterMoved( int, int ) ), SLOT( _splitterMoved( int, int ) ) );
  
  // state frame
  setStatusBar( statusbar_ = new StatusBar( this ) );

  // create "hidden" line editor to display filename
  statusbar_->addPermanentWidget( file_editor_ = new LineEditor( statusbar_ ), 1 );
  statusbar_->addLabels( 3, 0 );
  statusbar_->label(0).setAlignment( Qt::AlignCenter ); 
  statusbar_->label(1).setAlignment( Qt::AlignCenter ); 
  statusbar_->label(2).setAlignment( Qt::AlignCenter ); 
  statusbar_->addClock();

  // modify frame and set readOnly
  file_editor_->setReadOnly( true );
  file_editor_->setHasClearButton( false );
  file_editor_->setFrame( false );

  // modify color
  QPalette palette( file_editor_->palette() );
  palette.setColor( QPalette::Base, Qt::transparent );
  file_editor_->setPalette( palette );

  // assign non fixed font
  QFont font;
  font.fromString( XmlOptions::get().raw( "FONT_NAME" ).c_str() );
  file_editor_->setFont( font );
  
  // file toolbar
  CustomToolBar* toolbar = new CustomToolBar( "Main", this, "FILE_TOOLBAR" );
  toolbar->addAction( &newFileAction() );
  toolbar->addAction( &openAction() ); 
  toolbar->addAction( &saveAction() ); 
  
  // edition toolbar
  toolbar = new CustomToolBar( "Edition", this, "EDITION_TOOLBAR" );
  toolbar->addAction( &undoAction() ); 
  toolbar->addAction( &redoAction() ); 
  toolbar->addAction( &cutAction() );
  toolbar->addAction( &copyAction() );
  toolbar->addAction( &pasteAction() );

  // extra toolbar
  toolbar = new CustomToolBar( "Tools", this, "EXTRA_TOOLBAR" );
  toolbar->addAction( &fileInfoAction() ); 
  toolbar->addAction( &spellcheckAction() ); 
  
  // splitting toolbar
  toolbar = new CustomToolBar( "Multiple displays", this, "SPLIT_TOOLBAR" );
  toolbar->addAction( &splitDisplayHorizontalAction() ); 
  toolbar->addAction( &splitDisplayVerticalAction() ); 
  toolbar->addAction( &openHorizontalAction() ); 
  toolbar->addAction( &openVerticalAction() ); 
  toolbar->addAction( &closeDisplayAction() );
  toolbar->addAction( &detachAction() );
  
  //! configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( saveConfiguration() ), SLOT( _saveConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
 _updateConfiguration();
  
  // update buttons
  _update( TextDisplay::ALL );
  
  Debug::Throw( "MainWindow::MainWindow - done.\n" );
 
}

//___________________________________________________________
MainWindow::~MainWindow( void )
{ Debug::Throw( "MainWindow::~MainWindow.\n" ); }

//____________________________________________
void MainWindow::_detach( void )
{

  Debug::Throw( "MainWindow::_detach.\n" );

  // check number of independent displays
  if( activeView().independentDisplayCount() < 2 )
  {
    QtUtil::infoDialog( this,
      "There must be at least two different files opened\n"
      "in the same window for the displays to be detachable" );
    return;
  }

  // check number of displays associated to active
  BASE::KeySet<TextDisplay> associated_displays( &activeView().activeDisplay() );
  if( !
    ( associated_displays.empty() ||
      QtUtil::questionDialog( this,
      "Active display has clones in the current window.\n"
      "They will be closed when the display is detached.\n"
      "Continue ?" ) ) ) return;

  // keep active display local.
  TextDisplay& active_display_local( activeView().activeDisplay() );
  bool modified( active_display_local.document()->isModified() );

  // close all clone displays
  for( BASE::KeySet<TextDisplay>::iterator iter = associated_displays.begin(); iter != associated_displays.end(); iter++ )
  { activeView().closeDisplay( **iter ); }

  // create MainWindow
  MainWindow& window( static_cast<Application*>(qApp)->windowServer().newMainWindow() );

  // clone its display from the current
  window.activeView().activeDisplay().synchronize( &active_display_local );

  // delete active display local
  active_display_local.document()->setModified( false );
  activeView().closeDisplay( active_display_local );

  // show the new window
  window.activeView().activeDisplay().document()->setModified( modified );
  window.show();
  window._updateConfiguration();

  return;
  
}

//___________________________________________________________
void MainWindow::_revertToSave( void )
{
  
  Debug::Throw( "MainWindow::_revertToSave.\n" );

  // check filename
  if( activeDisplay().file().empty() )
  {
    QtUtil::infoDialog( this, "No filename given. <Revert to save> canceled." );
    return;
  }

  // ask for confirmation
  ostringstream what;
  if( activeDisplay().document()->isModified() ) what << "Discard changes to " << activeDisplay().file().localName() << "?";
  else what << "Reload file " << activeDisplay().file().localName() << "?";
  if( !QtUtil::questionDialog( this, what.str() ) ) return;

  activeDisplay().revertToSave();
  
}

//___________________________________________________________
void MainWindow::_print( void )
{
  Debug::Throw( "MainWindow::_print.\n" );

  // retrieve activeDisplay file
  const File& file( activeDisplay().file() );

  // check if file is modified
  if( activeDisplay().document()->isModified() && activeDisplay().askForSave() == AskForSaveDialog::CANCEL ) return;

  // check if file is valid and exists
  if( file.empty() || !file.exists() )
  {
    QtUtil::infoDialog( this, "File is not valid for printing. <print> canceled." );
    return;
  }

  // create dialog
  PrintDialog dialog( this );
  dialog.setFile( file );
  dialog.setMode( XmlOptions::get().get<string>("PRINT_MODE") == "PDF" ? PrintDialog::PDF : PrintDialog::HTML );
  dialog.setMaximumLineSize( XmlOptions::get().get<int>( "PRINT_LINE_SIZE" ) ); 
  dialog.setUseCommand( XmlOptions::get().get<bool>( "USE_PRINT_COMMAND" ) );
  
  // add commands
  /* command list contains the HTML editor, PDF editor and any additional user specified command */
  list<string> commands( XmlOptions::get().specialOptions<string>( "PRINT_COMMAND" ) );
  commands.push_back( XmlOptions::get().raw( "PDF_EDITOR" ) );
  commands.push_back( XmlOptions::get().raw( "HTML_EDITOR" ) );
  for( list<string>::iterator iter = commands.begin(); iter != commands.end(); iter++ )
  { dialog.addCommand( *iter ); }

  // set command manually that match the selection mode
  dialog.setCommand( XmlOptions::get().raw( ( dialog.mode() == PrintDialog::PDF ? "PDF_EDITOR":"HTML_EDITOR" ) ) );
  
  // exec
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // store options
  XmlOptions::get().set<string>( "PRINT_MODE", dialog.mode() == PrintDialog::PDF ? "PDF":"HTML" );
  XmlOptions::get().set<int>("PRINT_LINE_SIZE", dialog.maximumLineSize() );
  XmlOptions::get().set<bool>( "USE_PRINT_COMMAND", dialog.useCommand() );
  list<string> new_commands( dialog.commands() );
  for( list<string>::iterator iter = new_commands.begin(); iter != new_commands.end(); iter++ )
  { 
    if( std::find( commands.begin(), commands.end(), *iter ) == commands.end() ) 
    {
      Option option( "PRINT_COMMAND", *iter );
      XmlOptions::get().add( option );    
    }
  }
  
  Debug::Throw( "MainWindow::_print - options saved.\n" );
  
  // try open output file
  File fullname = File( qPrintable( dialog.destinationFile() ) ).expand();
 
  // check if file is directory
  if( fullname.isDirectory() )
  {
    ostringstream what;
    what << "file \"" << fullname << "\" is a directory. <Print> canceled.";
    QtUtil::infoDialog( this, what.str(), QtUtil::CENTER_ON_PARENT );
    return;
  }


  // check if file exists
  if( fullname.exists() )
  {
    if( !fullname.isWritable() )
    {
      ostringstream what;
      what << "file \"" << fullname << "\" is read-only. <Print> canceled.";
      QtUtil::infoDialog( this, what.str(), QtUtil::CENTER_ON_PARENT );
      return;
    } else if( !QtUtil::questionDialog( this, "selected file already exists. Overwrite ?", QtUtil::CENTER_ON_PARENT ) )
    return;
  }
  
  // retrieve HTML string from current display
  QString html_string( _htmlString( dialog.maximumLineSize() ) );
  Debug::Throw( "MainWindow::_print - retrieved html string.\n" );
  
  // check print mode
  PrintDialog::Mode mode( dialog.mode() );
  if( mode == PrintDialog::HTML )
  {
    // open stream
    QFile out( fullname.c_str() );
    if( !out.open( QIODevice::WriteOnly ) )
    {
      ostringstream what;
      what << "cannot write to file \"" << fullname << "\" <Print> canceled.";
      QtUtil::infoDialog( this, what.str() );
      return;
    }
    
    out.write( html_string.toAscii() );
    out.close();    
    Debug::Throw( "MainWindow::_print - html file saved.\n" );
    
  } else if( mode == PrintDialog::PDF ) {
    
    QTextEdit local(0);
    local.setLineWrapMode( QTextEdit::WidgetWidth );
    local.setHtml( html_string );
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fullname.c_str());
    
    local.document()->print(&printer);
    Debug::Throw( "MainWindow::_print - pdf file saved.\n" );
    
  } else return;
  
  // try open
  if( dialog.useCommand() )
  { Util::run( QStringList() << dialog.command() << fullname.c_str() ); }   
  
  Debug::Throw( "MainWindow::_print - done.\n" );
  return;

}

//____________________________________________
void MainWindow::closeEvent( QCloseEvent* event )
{
  Debug::Throw( "MainWindow::closeEvent.\n" );

  // accept event
  event->accept();
      
  // look over TextDisplays
  if( isModified() )
  {

    // look over TextDisplays
    BASE::KeySet<TextDisplay> displays( &activeView() );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {

      // get local reference to display
      TextDisplay& display( **iter );
      
      // check if this display is modified
      if( !display.document()->isModified() ) continue;

      // this trick allow to run  only once per set of associated displays
      if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( &display ) ) != iter ) continue;
      
      // ask for save
      int state( display.askForSave( modifiedDisplayCount() > 1 ) );
      if( state == AskForSaveDialog::YES_TO_ALL ) 
      {
        
        // for this window, only save displays located after the current
        for( BASE::KeySet<TextDisplay>::iterator display_iter = iter; display_iter != displays.end(); display_iter++ )
        { if( (*display_iter)->document()->isModified() ) (*display_iter)->save(); }

      } else if( state == AskForSaveDialog::YES_TO_ALL ) {
        
        // for this window, only save displays located after the current
        for( BASE::KeySet<TextDisplay>::iterator display_iter = iter; display_iter != displays.end(); display_iter++ )
        { if( (*display_iter)->document()->isModified() ) (*display_iter)->setModified( false ); }
        
      } else if( state == AskForSaveDialog::CANCEL ) {

        event->ignore();
        return;

      }

    }

  }
  
  return;
}

//____________________________________________
void MainWindow::enterEvent( QEvent* e )
{

  Debug::Throw( "MainWindow::enterEvent.\n" );
  CustomMainWindow::enterEvent( e );

  // keep track of displays to be deleted, if any
  BASE::KeySet<TextDisplay> dead_displays;

  // retrieve displays
  BASE::KeySet<TextDisplay> displays( &activeView() );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {

    // this trick allow to run only once per set of displays associated to the same file
    if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) != iter ) continue;
    
    // keep local reference of current display
    TextDisplay &display( **iter );
    
    // check file
    if( display.checkFileRemoved() == FileRemovedDialog::CLOSE ) 
    { 
        
      // register displays as dead
      BASE::KeySet<TextDisplay> associated_displays( &display );
      for( BASE::KeySet<TextDisplay>::iterator display_iter = associated_displays.begin(); display_iter != associated_displays.end(); display_iter++ )
      { dead_displays.insert( *display_iter ); }
      dead_displays.insert( &display );
            
    } else {
 
      (*iter)->checkFileReadOnly();
      (*iter)->checkFileModified();

    }
    
  }
  
  // update window title
  _updateWindowTitle();

  // delete dead_displays
  if( !dead_displays.empty() )
  {

    Debug::Throw() << "MainWindow::enterEvent - dead displays: " << dead_displays.size() << endl;
    for( BASE::KeySet<TextDisplay>::iterator iter = dead_displays.begin(); iter != dead_displays.end(); iter++ )
    { activeView().closeDisplay( **iter ); }

    // need to close window manually if there is no remaining displays
    if( dead_displays.size() == displays.size() ) close();

  }

  Debug::Throw( "MainWindow::enterEvent - done.\n" );

}

//________________________________________________________
void MainWindow::_updateConfiguration( void )
{
  
  Debug::Throw( "MainWindow::_updateConfiguration.\n" );
      
  resize( sizeHint() );
  
  // navigation frame visibility
  navigationFrame().visibilityAction().setChecked( XmlOptions::get().get<bool>("SHOW_NAVIGATION_FRAME") );
    
  // assign icons to file in open previous menu based on class manager
  FileRecord::List& records( static_cast<Application*>(qApp)->recentFiles().records() );
  for( FileRecord::List::iterator iter = records.begin(); iter != records.end(); iter++ )
  {
    
    FileRecord& record( *iter ); 
    if( !record.hasProperty( FileRecordProperties::CLASS_NAME ) ) continue; 
    DocumentClass document_class( static_cast<Application*>(qApp)->classManager().get( record.property( FileRecordProperties::CLASS_NAME ).c_str() ) );
    if( !document_class.icon().isEmpty() ) record.addProperty( FileRecordProperties::ICON, qPrintable( document_class.icon() ) );
  
  }
    
}

//________________________________________________________
void MainWindow::_saveConfiguration( void )
{ Debug::Throw( "MainWindow::_saveConfiguration.\n" ); }

//________________________________________________________
void MainWindow::_toggleNavigationFrame( bool state )
{
  Debug::Throw( "MainWindow::_toggleNavigationFrame.\n" );
  XmlOptions::get().set<bool>( "SHOW_NAVIGATION_FRAME", state );
}

//________________________________________________________
void MainWindow::_splitterMoved( int position, int index )
{
  Debug::Throw( "MainWindow::_splitterMoved.\n" );
  XmlOptions::get().set<int>( "NAVIGATION_FRAME_WIDTH", position ); 
}

//_______________________________________________________
void MainWindow::_update( unsigned int flags )
{

  Debug::Throw( "MainWindow::_update().\n" );

  if( flags & TextDisplay::WINDOW_TITLE )
  { _updateWindowTitle(); }

  if( flags & TextDisplay::FILE_NAME && file_editor_ )
  { 
    file_editor_->setText( activeDisplay().file().c_str() ); 
    fileInfoAction().setEnabled( !activeDisplay().file().empty() );
  }

  if( flags & TextDisplay::CUT )
  { cutAction().setEnabled( activeDisplay().cutAction().isEnabled() ); }

  if( flags & TextDisplay::COPY )
  { copyAction().setEnabled( activeDisplay().copyAction().isEnabled() ); }

  if( flags & TextDisplay::PASTE )
  { pasteAction().setEnabled( activeDisplay().pasteAction().isEnabled() ); }

  if( flags & TextDisplay::UNDO_REDO )
  {
    undoAction().setEnabled( activeDisplay().undoAction().isEnabled() );
    redoAction().setEnabled( activeDisplay().redoAction().isEnabled() );
  }
 
  if( statusbar_ && flags & TextDisplay::OVERWRITE_MODE )
  {
    statusbar_->label(0).setText( activeDisplay().overwriteMode() ? "INS":"" );
  }
  
  if( flags & TextDisplay::DISPLAY_COUNT )
  {
    int count = activeView().independentDisplayCount();
    detachAction().setEnabled( count > 1 );
    diffAction().setEnabled( count == 2 );    
  }
  
  if( flags & TextDisplay::SAVE )
  { saveAction().setEnabled( !activeDisplay().isReadOnly() ); }

}

//_____________________________________________
void MainWindow::_updateCursorPosition( void )
{
  
  // retrieve position in text
  TextPosition position( activeDisplay().textPosition() );
  
  /*
  if block delimiters are shown, 
  need to count how many blocks are collapsed prior to current
  and increment paragraph consequently
  */
  if( activeDisplay().hasBlockDelimiterDisplay() ) position.paragraph() += activeDisplay().blockDelimiterDisplay().collapsedBlockCount( position.paragraph() );
  
  statusbar_->label(1).setText( Str( "line : " ).append<int>( position.paragraph()+1 ).c_str() , false );
  statusbar_->label(2).setText( Str( "column : " ).append<int>( position.index()+1 ).c_str() , false );

  return;
}

//___________________________________________________________
void MainWindow::_installActions( void )
{

  Debug::Throw( "MainWindow::_installActions.\n" );
  
  addAction( new_file_action_ = new QAction( IconEngine::get( ICONS::NEW ), "&New", this ) );
  new_file_action_->setShortcut( Qt::CTRL+Qt::Key_N );
  new_file_action_->setToolTip( "Create a new empty file" );
  connect( new_file_action_, SIGNAL( triggered() ), SLOT( _newFile() ) );

  addAction( clone_action_ = new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), "&Clone", this ) );
  clone_action_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_N );
  clone_action_->setToolTip( "Clone current display" );
  connect( clone_action_, SIGNAL( triggered() ), SLOT( _splitDisplay() ) );

  addAction( detach_action_ = new QAction( IconEngine::get( ICONS::VIEW_DETACH ), "&Detach", this ) );
  detach_action_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_O );
  detach_action_->setToolTip( "Detach current display" );
  detach_action_->setEnabled( false );
  connect( detach_action_, SIGNAL( triggered() ), SLOT( _detach() ) );

  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open", this ) );
  open_action_->setShortcut( Qt::CTRL+Qt::Key_O );
  open_action_->setToolTip( "Open an existsing file" );
  connect( open_action_, SIGNAL( triggered() ), SLOT( open() ) );
 
  addAction( close_display_action_ = new QAction( IconEngine::get( ICONS::VIEW_REMOVE ), "&Close display", this ) );
  close_display_action_->setShortcut( Qt::CTRL+Qt::Key_W );
  close_display_action_->setToolTip( "Close current display" );
  connect( close_display_action_, SIGNAL( triggered() ), SLOT( _closeDisplay() ) );
 
  addAction( close_window_action_ = new QAction( IconEngine::get( ICONS::CLOSE ), "&Close window", this ) );
  close_window_action_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_W );
  close_window_action_->setToolTip( "Close current display" );
  connect( close_window_action_, SIGNAL( triggered() ), SLOT( _closeWindow() ) );
 
  addAction( save_action_ = new QAction( IconEngine::get( ICONS::SAVE ), "&Save", this ) );
  save_action_->setShortcut( Qt::CTRL+Qt::Key_S );
  save_action_->setToolTip( "Save current file" );
  connect( save_action_, SIGNAL( triggered() ), SLOT( _save() ) );
 
  addAction( save_as_action_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save &As", this ) );
  save_as_action_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_S );
  save_as_action_->setToolTip( "Save current file with a different name" );
  connect( save_as_action_, SIGNAL( triggered() ), SLOT( _saveAs() ) );

  addAction( revert_to_save_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "&Revert to saved", this ) );
  revert_to_save_action_->setToolTip( "Reload saved version of current file" );
  connect( revert_to_save_action_, SIGNAL( triggered() ), SLOT( _revertToSave() ) );
 
  addAction( print_action_ = new QAction( IconEngine::get( ICONS::PRINT ), "&Print", this ) );
  print_action_->setToolTip( "Print current file" );
  connect( print_action_, SIGNAL( triggered() ), SLOT( _print() ) );

  addAction( undo_action_ = new QAction( IconEngine::get( ICONS::UNDO ), "&Undo", this ) );
  undo_action_->setToolTip( "Undo last action" );
  connect( undo_action_, SIGNAL( triggered() ), SLOT( _undo() ) );

  addAction( redo_action_ = new QAction( IconEngine::get( ICONS::REDO ), "&Redo", this ) );
  redo_action_->setToolTip( "Redo last un-done action" );
  connect( redo_action_, SIGNAL( triggered() ), SLOT( _redo() ) );

  addAction( cut_action_ = new QAction( IconEngine::get( ICONS::CUT ), "&Cut", this ) );
  cut_action_->setToolTip( "Cut current selection and copy to clipboard" );
  connect( cut_action_, SIGNAL( triggered() ), SLOT( _cut() ) );

  addAction( copy_action_ = new QAction( IconEngine::get( ICONS::COPY ), "&Copy", this ) );
  copy_action_->setToolTip( "Copy current selection to clipboard" );
  connect( copy_action_, SIGNAL( triggered() ), SLOT( _copy() ) );

  addAction( paste_action_ = new QAction( IconEngine::get( ICONS::PASTE ), "&Paste", this ) );
  paste_action_->setToolTip( "Paste clipboard to text" );
  connect( paste_action_, SIGNAL( triggered() ), SLOT( _paste() ) );

  addAction( file_info_action_ = new QAction( IconEngine::get( ICONS::INFO ), "&File information", this ) );
  file_info_action_->setToolTip( "Display file informations" );
  connect( file_info_action_, SIGNAL( triggered() ), SLOT( _fileInfo() ) );

  addAction( spellcheck_action_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), "&Spell check", this ) );
  #if WITH_ASPELL
  connect( spellcheck_action_, SIGNAL( triggered() ), SLOT( _spellcheck( void ) ) );
  #else 
  spellcheck_action_->setVisible( false );
  #endif

  addAction( diff_action_ = new QAction( "&Diff files", this ) );
  connect( diff_action_, SIGNAL( triggered() ), SLOT( _diff() ) );
  diff_action_->setEnabled( false );
  
  addAction( split_display_horizontal_action_ =new QAction( IconEngine::get( ICONS::VIEW_TOPBOTTOM ), "Clone display top/bottom", this ) );
  split_display_horizontal_action_->setToolTip( "Clone current display vertically" );
  connect( split_display_horizontal_action_, SIGNAL( triggered() ), SLOT( _splitDisplayVertical() ) );

  addAction( split_display_vertical_action_ =new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), "Clone display left/right", this ) );
  split_display_vertical_action_->setToolTip( "Clone current display horizontally" );
  connect( split_display_vertical_action_, SIGNAL( triggered() ), SLOT( _splitDisplayHorizontal() ) );

  addAction( open_horizontal_action_ =new QAction( IconEngine::get( ICONS::VIEW_BOTTOM ), "Clone display top/bottom", this ) );
  open_horizontal_action_->setToolTip( "Open a new display vertically" );
  connect( open_horizontal_action_, SIGNAL( triggered() ), SLOT( openVertical() ) );

  addAction( open_vertical_action_ =new QAction( IconEngine::get( ICONS::VIEW_RIGHT ), "Open display left/right", this ) );
  open_vertical_action_->setToolTip( "Open a new display horizontally" );
  connect( open_vertical_action_, SIGNAL( triggered() ), SLOT( openHorizontal() ) );
  
}

//___________________________________________________________
void MainWindow::_updateWindowTitle()
{ 
  Debug::Throw( "MainWindow::_updateWindowTitle.\n" );
  
  {
    bool readonly( activeDisplay().isReadOnly() );
    bool modified( activeDisplay().document()->isModified() );
    Debug::Throw() << "MainWindow::_updateWindowTitle -"
      << " readonly: " << readonly 
      << " modified: " << modified
      << endl;
  }
  
  setWindowTitle( WindowTitle( activeDisplay().file() )
    .setReadOnly( activeDisplay().isReadOnly() )
    .setModified( activeDisplay().document()->isModified() )
    );
  
}

//___________________________________________________________
TextView& MainWindow::_newTextView( QWidget *parent )
{ 
  Debug::Throw( "MainWindow::_newTextView.\n" );
  
  TextView* view = new TextView( parent );
 
  // connections
  connect( view, SIGNAL( overwriteModeChanged() ), SLOT( _updateOverwriteMode() ) );
  connect( view, SIGNAL( needUpdate( unsigned int ) ), SLOT( _update( unsigned int ) ) );
  connect( view, SIGNAL( displayCountChanged( void ) ), SLOT( _updateDisplayCount( void ) ) );
  connect( view, SIGNAL( displayCountChanged( void ) ), &static_cast<Application*>(qApp)->windowServer(), SIGNAL( sessionFilesChanged( void ) ) );
  
  connect( view, SIGNAL( undoAvailable( bool ) ), &undoAction(), SLOT( setEnabled( bool ) ) );
  connect( view, SIGNAL( redoAvailable( bool ) ), &redoAction(), SLOT( setEnabled( bool ) ) ); 
  connect( &view->positionTimer(), SIGNAL( timeout() ), SLOT( _updateCursorPosition() ) );  
  
  return *view;
  
}

//_____________________________________________________________________
QString MainWindow::_htmlString( const int& max_line_size )
{

  QDomDocument document( "html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\"" );

  // html
  QDomElement html = document.appendChild( document.createElement( "html" ) ).toElement();
  html.setAttribute( "xmlns", "http://www.w3.org/1999/xhtml" );

  // head
  QDomElement head = html.appendChild( document.createElement( "head" ) ).toElement();
  QDomElement meta;

  // meta information
  meta = head.appendChild( document.createElement( "meta" ) ).toElement();
  meta.setAttribute( "content", "text/html; charset=UTF-8" );
  meta.setAttribute( "http-equiv", "Content-Type" );
  meta = head.appendChild( document.createElement( "meta" ) ).toElement();
  meta.setAttribute( "content", "QEdit" );
  meta.setAttribute( "name", "Generator" );

  // title
  QDomElement title = head.appendChild( document.createElement( "title" ) ).toElement();
  title.appendChild( document.createTextNode( activeDisplay().file().c_str() ) );

  // body
  html.
    appendChild( document.createElement( "body" ) ).
    appendChild( activeDisplay().htmlNode( document, max_line_size ) );

  /*
    the following replacements are needed
    to have correct implementation of leading space characters, tabs
    and end of line
  */
  QString html_string( document.toString(0) );
  html_string = html_string.replace( "</span>\n", "</span>" );
  html_string = html_string.replace( "<br/>", "" );
  return html_string;
}
