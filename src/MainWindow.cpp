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
#include "FileRecordProperties.h"
#include "HighlightBlockFlags.h"
#include "IconEngine.h"
#include "Icons.h"
#include "MainWindow.h"
#include "Menu.h"
#include "NewFileDialog.h"
#include "RecentFilesMenu.h"
#include "PixmapEngine.h"
#include "XmlOptions.h"
#include "PrintDialog.h"
#include "QtUtil.h"
#include "StatusBar.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "Util.h"
#include "WindowServer.h"
#include "WindowTitle.h"

using namespace std;
using namespace Qt;

//_____________________________________________________
MainWindow::MainWindow(  QWidget* parent ):
  CustomMainWindow( parent ),
  Counter( "MainWindow" ),
  menu_( 0 ),
  active_display_( 0 ),
  statusbar_( 0 ),
  file_editor_( 0 ),
  default_orientation_( Horizontal ),
  default_open_mode_( NEW_WINDOW ),
  position_timer_(this)
{

  Debug::Throw( "MainWindow::MainWindow.\n" );

  // tell window to delete on exit
  setAttribute( WA_DeleteOnClose );
  
  // install actions
  _installActions();
  
  // menu
  setMenuBar( menu_ = new Menu( this ) );
  connect( menu_, SIGNAL( documentClassSelected( QString ) ), this, SLOT( selectClassName( QString ) ) );

  // main vbox
  QWidget* main( new QWidget( this ) );
  setCentralWidget( main );

  QVBoxLayout* layout( new QVBoxLayout() );
  main->setLayout( layout );
  layout->setMargin(2);
  layout->setSpacing(2);

  // TextDisplay container
  main_ = new QWidget( main );
  main_->setLayout( new QVBoxLayout() );
  main_->layout()->setMargin(0);
  main_->layout()->setSpacing(0);
  layout->addWidget( main_, 1 );
  
  // create new Text display and register autosave thread
  TextDisplay& display = _newTextDisplay( main_ );
  main_->layout()->addWidget( &display );
  
  display.setActive( true );
  static_cast<Application*>(qApp)->autoSave().newThread( &display );
  Debug::Throw( "MainWindow::MainWindow - thread created.\n" );

  // state frame
  setStatusBar( statusbar_ = new StatusBar( this ) );

  // create "hidden" line editor to display filename
  statusbar_->addPermanentWidget( file_editor_ = new LineEditor( statusbar_ ), 1 );
  statusbar_->addLabels( 3, 0 );
  statusbar_->label(0).setAlignment( AlignCenter ); 
  statusbar_->label(1).setAlignment( AlignCenter ); 
  statusbar_->label(2).setAlignment( AlignCenter ); 
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
  
  // position update timer
  position_timer_.setSingleShot( true );
  position_timer_.setInterval( 100 );
  connect( &position_timer_, SIGNAL( timeout() ), SLOT( _updateCursorPosition() ) );
  
  Debug::Throw( "MainWindow::MainWindow - done.\n" );
 
}

//___________________________________________________________
MainWindow::~MainWindow( void )
{ Debug::Throw( "MainWindow::~MainWindow.\n" ); }

//____________________________________________
void MainWindow::setFile( File file )
{
  Debug::Throw() << "MainWindow::setFile - " << file << endl;

  // look for first empty display
  BASE::KeySet<TextDisplay> displays( this );
  BASE::KeySet<TextDisplay>::iterator iter = find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() );
  assert( iter != displays.end() );
  TextDisplay &display( **iter );
   
  // open file in active display
  if( !file.empty() ) display.openFile( file );
  else display.updateDocumentClass();
  
  // set focus
  setActiveDisplay( display );
  display.setFocus();

  Debug::Throw( "MainWindow::setFile - done.\n" );
  
  return;
}

//________________________________________________________________
void MainWindow::setActiveDisplay( TextDisplay& display )
{ 
  Debug::Throw() << "MainWindow::setActiveDisplay - key: " << display.key() << std::endl;
  assert( display.isAssociated( this ) );
  
  active_display_ = &display;
  if( !activeDisplay().isActive() )
  {

    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { (*iter)->setActive( false ); }
    
    activeDisplay().setActive( true );
    _update( TextDisplay::ALL );

  }
  
  Debug::Throw( "MainWindow::setActiveDisplay - done.\n" );
  
}

//____________________________________________
void MainWindow::saveAll( void )
{
  Debug::Throw( "MainWindow::saveAll.\n" );

  // retrieve all displays
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { if( (*iter)->document()->isModified() ) (*iter)->save(); }

  return;

}

//________________________________________________________________
void MainWindow::selectClassName( QString name )
{
  Debug::Throw( "MainWindow::SelectClassName.\n" );

  // retrieve all displays matching active
  // and update class name
  BASE::KeySet<TextDisplay> displays( &activeDisplay() );
  displays.insert( &activeDisplay() );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    (*iter)->setClassName( name );
    (*iter)->updateDocumentClass();
  }
  
  // rehighlight
  activeDisplay().rehighlight();

}

//________________________________________________________________
void MainWindow::rehighlight( void )
{
  Debug::Throw( "MainWindow::Rehighlight.\n" );

  // retrieve associated TextDisplay
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    // this trick allow to run the rehighlight only once per set of associated displays
    if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) == iter ) (*iter)->rehighlight();
  }

  return;
}

//____________________________________________
void MainWindow::_detach( void )
{

  Debug::Throw( "MainWindow::_detach.\n" );

  // check number of independent displays
  if( independentDisplayCount() < 2 )
  {
    QtUtil::infoDialog( this,
      "There must be at least two different files opened\n"
      "in the same window for the displays to be detachable" );
    return;
  }

  // check number of displays associated to active
  BASE::KeySet<TextDisplay> associated_displays( &activeDisplay() );
  if( !
    ( associated_displays.empty() ||
      QtUtil::questionDialog( this,
      "Active display has clones in the current window.\n"
      "They will be closed when the display is detached.\n"
      "Continue ?" ) ) ) return;

  // keep active display local.
  TextDisplay& active_display_local( activeDisplay() );
  bool modified( active_display_local.document()->isModified() );

  // close all clone displays
  for( BASE::KeySet<TextDisplay>::iterator iter = associated_displays.begin(); iter != associated_displays.end(); iter++ )
  { _closeDisplay( *dynamic_cast<TextDisplay*>((*iter)->parentWidget()) ); }

  // create MainWindow
  MainWindow& window( static_cast<Application*>(qApp)->windowServer().newMainWindow() );

  // clone its display from the current
  window.activeDisplay().synchronize( &active_display_local );

  // delete active display local
  active_display_local.document()->setModified( false );
  _closeDisplay( active_display_local );

  // show the new window
  window.activeDisplay().document()->setModified( modified );
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

  // check for modifications
  event->accept();
      
  // look over TextDisplays
  if( isModified() )
  {

    // look over TextDisplays
    BASE::KeySet<TextDisplay> displays( this );
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

  // save configuration before closing
  _saveConfiguration();
  
  return;
}

//____________________________________________
void MainWindow::enterEvent( QEvent* e )
{

  Debug::Throw( "MainWindow::enterEvent.\n" );
  QMainWindow::enterEvent( e );

  // keep track of displays to be deleted, if any
  BASE::KeySet<TextDisplay> dead_displays;

  // retrieve displays
  BASE::KeySet<TextDisplay> displays( this );
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
      { dead_displays.insert( dynamic_cast<TextDisplay*>( (*display_iter)->parentWidget() ) ); }
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
    { _closeDisplay( **iter ); }

    // need to close window manually if there is no remaining displays
    if( dead_displays.size() == displays.size() ) close();

  }

  Debug::Throw( "MainWindow::enterEvent - done.\n" );

}

//_______________________________________________________
void MainWindow::resizeEvent( QResizeEvent* event )
{
  resize_timer_.start( 200, this );
  return CustomMainWindow::resizeEvent( event );
}

//_______________________________________________________
void MainWindow::timerEvent( QTimerEvent* event )
{

  if( event->timerId() == resize_timer_.timerId() )
  {
    
    // stop timer
    resize_timer_.stop();
    
    // save size
    XmlOptions::get().set<int>( "WINDOW_HEIGHT", height() );
    XmlOptions::get().set<int>( "WINDOW_WIDTH", width() );
  
  } else return CustomMainWindow::timerEvent( event );
  
}

//_______________________________________________________
void MainWindow::_diff( void )
{
  Debug::Throw( "MainWindow::_diff.\n" );
 
  // retrieve displays
  int n_displays( independentDisplayCount() );

  // check number of files
  if( n_displays > 2 )
  {
    QtUtil::infoDialog( this, "Too many files opened. Diff canceled." );
    return;
  }

  if( n_displays < 2 )
  {
    QtUtil::infoDialog( this, "Too few files opened. Diff canceled." );
    return;
  }

  // create diff object
  Diff* diff = new Diff( this );

  // store active display as first to compare
  TextDisplay& first = activeDisplay();
  
  // retrieve displays associated to window
  // look for the first one that is not associated to the active display
  BASE::KeySet<TextDisplay> displays( this );
  BASE::KeySet<TextDisplay>::iterator iter = displays.begin();
  for(; iter != displays.end(); iter++ )
  {
    if( !( *iter == &first || (*iter)->isAssociated( &first ) ) )
    {
      diff->setTextDisplays( first, *(*iter) );
      break;
    }
  }
  
  // check that one display was found
  assert( iter != displays.end() );
  
  // try run
  if( !diff->run() )
  {
    QtUtil::infoDialog( this, diff->error() );
    delete diff;
    return;
  }
  
  return;
  
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

//____________________________________________
void MainWindow::_displayFocusChanged( TextEditor* editor )
{
  Debug::Throw() << "MainWindow::_DisplayFocusChanged - " << editor->key() << endl;
  setActiveDisplay( *static_cast<TextDisplay*>(editor) );  
}

//___________________________________________________________
void MainWindow::_installActions( void )
{

  Debug::Throw( "MainWindow::_installActions.\n" );
  
  addAction( new_file_action_ = new QAction( IconEngine::get( ICONS::NEW ), "&New", this ) );
  new_file_action_->setShortcut( CTRL+Key_N );
  new_file_action_->setToolTip( "Create a new empty file" );
  connect( new_file_action_, SIGNAL( triggered() ), SLOT( _newFile() ) );

  addAction( clone_action_ = new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), "&Clone", this ) );
  clone_action_->setShortcut( SHIFT+CTRL+Key_N );
  clone_action_->setToolTip( "Clone current display" );
  connect( clone_action_, SIGNAL( triggered() ), SLOT( _splitDisplay() ) );

  addAction( detach_action_ = new QAction( IconEngine::get( ICONS::VIEW_DETACH ), "&Detach", this ) );
  detach_action_->setShortcut( SHIFT+CTRL+Key_O );
  detach_action_->setToolTip( "Detach current display" );
  detach_action_->setEnabled( false );
  connect( detach_action_, SIGNAL( triggered() ), SLOT( _detach() ) );

  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open", this ) );
  open_action_->setShortcut( CTRL+Key_O );
  open_action_->setToolTip( "Open an existsing file" );
  connect( open_action_, SIGNAL( triggered() ), SLOT( open() ) );
 
  addAction( close_display_action_ = new QAction( IconEngine::get( ICONS::VIEW_REMOVE ), "&Close display", this ) );
  close_display_action_->setShortcut( CTRL+Key_W );
  close_display_action_->setToolTip( "Close current display" );
  connect( close_display_action_, SIGNAL( triggered() ), SLOT( _closeDisplay() ) );
 
  addAction( close_window_action_ = new QAction( IconEngine::get( ICONS::CLOSE ), "&Close window", this ) );
  close_window_action_->setShortcut( SHIFT+CTRL+Key_W );
  close_window_action_->setToolTip( "Close current display" );
  connect( close_window_action_, SIGNAL( triggered() ), SLOT( _closeWindow() ) );
 
  addAction( save_action_ = new QAction( IconEngine::get( ICONS::SAVE ), "&Save", this ) );
  save_action_->setShortcut( CTRL+Key_S );
  save_action_->setToolTip( "Save current file" );
  connect( save_action_, SIGNAL( triggered() ), SLOT( _save() ) );
 
  addAction( save_as_action_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save &As", this ) );
  save_as_action_->setShortcut( SHIFT+CTRL+Key_S );
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
  
  Debug::Throw( "MainWindow::_updateWindowTitle - done.\n" );
}

//________________________________________________________
void MainWindow::_updateConfiguration( void )
{
  
  Debug::Throw( "MainWindow::_updateConfiguration.\n" );
      
  // resize
  resize( QSize( XmlOptions::get().get<int>( "WINDOW_WIDTH" ), XmlOptions::get().get<int>( "WINDOW_HEIGHT" ) ) );

  // assign icons to file in open previous menu based on class manager
  list<File> files( menu_->recentFilesMenu().files() );
  for( list<File>::const_iterator iter = files.begin(); iter != files.end(); iter++ )
  {
    
    FileRecord& record( menu_->recentFilesMenu().get( *iter ) ); 
    if( !record.hasProperty( FileRecordProperties::CLASS_NAME ) ) continue; 
    DocumentClass document_class( static_cast<Application*>(qApp)->classManager().get( record.property( FileRecordProperties::CLASS_NAME ).c_str() ) );
    if( !document_class.icon().isEmpty() ) record.addProperty( FileRecordProperties::ICON, qPrintable( document_class.icon() ) );
  
  }
    
}

//________________________________________________________
void MainWindow::_saveConfiguration( void )
{ Debug::Throw( "MainWindow::_saveConfiguration.\n" ); }

//___________________________________________________________
void MainWindow::_newFile( const OpenMode& mode, const Orientation& orientation )
{

  Debug::Throw( "MainWindow::_New.\n" );

  // check open_mode
  if( mode == NEW_WINDOW ) static_cast<Application*>(qApp)->windowServer().open();
  else _splitDisplay( orientation, false );

}

//___________________________________________________________
void MainWindow::_open( FileRecord record, const OpenMode& mode, const Orientation& orientation )
{

  Debug::Throw( "MainWindow::_Open.\n" );

  // copy to local
  if( record.file().empty() )
  {

    // create file dialog
    CustomFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setDirectory( QDir( activeDisplay().workingDirectory().c_str() ) );
    QtUtil::centerOnParent( &dialog );
    if( dialog.exec() == QDialog::Rejected ) return;
    
    QStringList files( dialog.selectedFiles() );
    if( files.empty() ) return;
  
    record.setFile( File( qPrintable( files.front() ) ).expand() );
  
  }
  
  // check open_mode
  if( mode == NEW_WINDOW )
  {
    // open via the Application to create a new editor
    static_cast<Application*>(qApp)->windowServer().open( record );
    return;
  }

  // see if file is directory
  if( record.file().isDirectory() )
  {
    
    ostringstream what;
    what << "File \"" << record.file() << "\" is a directory. <Open> canceled.";
    QtUtil::infoDialog( this, what.str() );
    return;
    
  }

  // see if file exists
  if( !record.file().exists() )
  {
    
    // create NewFileDialog
    int state( NewFileDialog( this, record.file() ).exec() );
    switch( state )
    {
      
      case NewFileDialog::CREATE:
      {
        File fullname( record.file().expand() );
        if( !fullname.create() )
        {
          ostringstream what;
          what << "Unable to create file " << record.file() << ".";
          QtUtil::infoDialog( this, what.str() );
          return;
        }
        break;
      }
 
      case NewFileDialog::CANCEL: return;
      case NewFileDialog::EXIT: 
      close();
      return;
      
    }
    
  }
  
  // retrieve all edit windows
  // find one matching
  BASE::KeySet<MainWindow> windows( &static_cast<Application*>(qApp)->windowServer() );
  BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file() ) );
  if( iter != windows.end() )
  {

    // select found display in MainWindow
    (*iter)->selectDisplay( record.file() );

    // check if the found window is the current
    if( *iter == this )
    {
      uniconify();
      return;
    }

    ostringstream what;
    what
      << "The file " << record.file() << " is already opened in another window.\n"
      << "Do you want to close the other display and open the file here ?";
    if( !QtUtil::questionDialog( this, what.str() ) )
    {
      (*iter)->uniconify();
      return;
    }

    // look for an empty display
    // create a new display if none is found
    BASE::KeySet<TextDisplay> displays( this );
    BASE::KeySet<TextDisplay>::iterator display_iter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
    TextDisplay& display( display_iter == displays.end() ? _splitDisplay( orientation, false ):**display_iter );

    // retrieve active display from previous window
    TextDisplay& previous_display( (*iter)->activeDisplay() );

    // store modification state
    bool modified( previous_display.document()->isModified() );

    // clone
    display.synchronize( &previous_display );
    
    // set previous display as unmdified
    previous_display.document()->setModified( false );

    // close display, or window, depending on its number of independent files
    if( (*iter)->independentDisplayCount() == 1 ) (*iter)->close();
    else
    {
      BASE::KeySet<TextDisplay> displays( &previous_display );
      displays.insert( &previous_display );
      for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
      { (*iter)->_closeDisplay( *dynamic_cast<TextDisplay*>( (*display_iter)->parentWidget() ) ); }
    }

    // restore modification state and make new display active
    display.setModified( modified );
    setActiveDisplay( display );
    display.setFocus();

  } else {

    // look for an empty display
    BASE::KeySet<TextDisplay> displays( this );
    BASE::KeySet<TextDisplay>::iterator display_iter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
    if( display_iter == displays.end() ) _splitDisplay( orientation, false );

    // open file in this window
    setFile( record.file() );

    // update configuration
    _updateConfiguration();
    
  }
 
  return;
}

//___________________________________________________________
void MainWindow::_closeDisplay( TextDisplay& display )
{
  Debug::Throw( "MainWindow::_closeDisplay.\n" );

  // retrieve number of displays
  // if only one display, close the entire window
  {
    BASE::KeySet<TextDisplay> displays( this );
    if( displays.size() < 2 )
    {
      Debug::Throw() << "MainWindow::_closeDisplay - full close." << endl;
      close();
      return;
    }
  }
  
  // check if display is modified and has no associates in window
  if( 
    display.document()->isModified() && 
    BASE::KeySet<TextDisplay>( &display ).empty() &&
    display.askForSave() ==  AskForSaveDialog::CANCEL ) return;

  // retrieve parent and grandparent of current display
  QWidget* parent( display.parentWidget() );    
  QSplitter* parent_splitter( dynamic_cast<QSplitter*>( parent ) );
  
  // retrieve displays associated to current
  BASE::KeySet<TextDisplay> displays( &display );
    
  // check how many children remain in parent_splitter if any
  // take action if it is less than 2 (the current one to be deleted, and another one)
  if( parent_splitter && parent_splitter->count() == 2 ) 
  {
    
    // retrieve child that is not the current editor
    // need to loop over existing widgets because the editor above has not been deleted yet
    QWidget* child(0);
    for( int index = 0; index < parent_splitter->count(); index++ )
    { 
      if( parent_splitter->widget( index ) != &display ) 
      {
        child = parent_splitter->widget( index );
        break;
      }
    }    
    assert( child );
    Debug::Throw( "MainWindow::_closeDisplay - found child.\n" );
    
    // retrieve splitter parent
    QWidget* grand_parent( parent_splitter->parentWidget() );
    
    // try cast to a splitter
    QSplitter* grand_parent_splitter( dynamic_cast<QSplitter*>( grand_parent ) );
    
    // move child to grand_parent_splitter if any
    if( grand_parent_splitter )
    {  grand_parent_splitter->insertWidget( grand_parent_splitter->indexOf( parent_splitter ), child ); }
    else
    {
      child->setParent( grand_parent );
      grand_parent->layout()->addWidget( child );
    }
    
    // delete parent_splitter, now that it is empty
    // delete parent_splitter;
    parent_splitter->deleteLater();
    
  } else {
    
    // the editor is deleted only if its parent splitter is not
    // otherwise this will trigger double deletion of the editor 
    // which will then crash
    display.deleteLater();
    
  }
  
  // if no associated displays, retrieve all, set the first as active
  if( displays.empty() )  displays = BASE::KeySet<TextDisplay>( this );
  
  bool active_found( false );
  for( BASE::KeySet<TextDisplay>::reverse_iterator iter = displays.rbegin(); iter != displays.rend(); iter++ )
  { 
    if( (*iter) != &display ) {
      setActiveDisplay( **iter ); 
      active_found = true;
      break;
    }
  }  
  assert( active_found );
  
//   // update close_display button
//   int independent_display_count( independentDisplayCount() );
//   detachAction().setEnabled( independent_display_count > 1 );
//   diffAction().setEnabled( independent_display_count == 2 );
  
  // change focus
  activeDisplay().setFocus();
  Debug::Throw( "MainWindow::_closeDisplay - done.\n" );

}

//___________________________________________________________
TextDisplay& MainWindow::_splitDisplay( const Orientation& orientation, const bool& clone )
{
  
  Debug::Throw( "MainWindow::_splitDisplay.\n" );

  // keep local pointer to current active display
  TextDisplay& active_display_local( activeDisplay() );  
  
  // compute desired dimension of the new splitter
  // along its splitting direction
  int dimension(0);
  if( clone ) dimension = (orientation == Horizontal) ? active_display_local.width():active_display_local.height();
  else dimension = (orientation == Horizontal) ? main_->width():main_->height();

  // create new splitter
  QSplitter& splitter( _newSplitter( orientation, clone ) );
  
  // create new display
  TextDisplay& display( _newTextDisplay(0) );
  
  // insert in splitter, at correct position
  if( clone ) splitter.insertWidget( splitter.indexOf( &active_display_local )+1, &display );
  else splitter.addWidget( &display );

  // recompute dimension
  // take the max of active display and splitter,
  // in case no new splitter was created.
  dimension = max( dimension, (orientation == Horizontal) ? splitter.width():splitter.height() );
  
  // assign equal size to all splitter children
  QList<int> sizes;
  for( int i=0; i<splitter.count(); i++ )
  { sizes.push_back( dimension/splitter.count() ); }
  splitter.setSizes( sizes );

  // synchronize both displays, if cloned
  if( clone )
  {

    /*
     if there exists no clone of active display,
     backup text and register a new Sync object
    */
    BASE::KeySet<TextDisplay> displays( &active_display_local );

    // clone new display
    display.synchronize( &active_display_local );
    
    // perform associations
    // check if active displays has associates and propagate to new
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { BASE::Key::associate( &display, *iter ); }

    // associate this display to AutoSave threads
    BASE::KeySet<AutoSaveThread> threads( &active_display_local );
    for( BASE::KeySet<AutoSaveThread>::iterator iter = threads.begin(); iter != threads.end(); iter++ )
    { BASE::Key::associate( &display, *iter ); }
 
    // associate new display to active
    BASE::Key::associate( &display, &active_display_local );

  } else {

    // register new AutoSave thread
    static_cast<Application*>(qApp)->autoSave().newThread( &display );

    // enable detach
    detachAction().setEnabled( true );
    diffAction().setEnabled( independentDisplayCount() == 2 );
    
  }

  return display;

}

//____________________________________________________________
QSplitter& MainWindow::_newSplitter( const Orientation& orientation, const bool& clone )
{

  Debug::Throw( "MainWindow::_newSplitter.\n" );
  QSplitter *splitter = 0;
  
  if( clone )
  {
    
    /* in clone mode, a new splitter is added as a parent of the activeDisplay.
    the new (cloned) TextDisplay will appear side by side with it */

    // retrieve parent of current display
    QWidget* parent( activeDisplay().parentWidget() );  

    // try catch to splitter
    // do not create a new splitter if the parent has same orientation
    QSplitter *parent_splitter( dynamic_cast<QSplitter*>( parent ) );
    if( parent_splitter && parent_splitter->orientation() == orientation ) splitter = parent_splitter;
    else {
      
 
      // move splitter to the first place if needed
      if( parent_splitter ) 
      {
        
        Debug::Throw( "MainWindow::_newSplitter - found parent splitter.\n" );
        // create a splitter with correct orientation
        // give him no parent, because the parent is set in QSplitter::insertWidget()
        splitter = new LocalSplitter(0);
        splitter->setOrientation( orientation );
        parent_splitter->insertWidget( parent_splitter->indexOf( &activeDisplay() ), splitter );
        
      } else {
        
        // create a splitter with correct orientation
        splitter = new LocalSplitter(parent);
        splitter->setOrientation( orientation );
        parent->layout()->addWidget( splitter );
        
      }
      
      // reparent current display
      splitter->addWidget( &activeDisplay() );
      
      // resize parent splitter if any
      if( parent_splitter )
      {
        int dimension = ( parent_splitter->orientation() == Horizontal) ? 
          parent_splitter->width():
          parent_splitter->height();
    
        QList<int> sizes;
        for( int i=0; i<parent_splitter->count(); i++ )
        { sizes.push_back( dimension/parent_splitter->count() ); }
        parent_splitter->setSizes( sizes );
      
      }
      
    }
    
  } else {

    /*
      in no clone mode, a new splitter is created at the top level
      the new (cloned) TextDisplay will appear side by side with all other displays
    */

    // keep track of first (either TextDisplay or QSplitter) from main_
    QWidget *child(0);

    // retrieve children and loop
    const QObjectList& children( main_->children() );
    for( QObjectList::const_iterator iter = children.begin(); iter != children.end() && !child; iter++ )
    { child = dynamic_cast<QWidget*>( *iter ); }

    // check child could be retrieved
    assert( child );

    // try cast child to splitter
    // if exists and have same orientation, do not create a new one
    QSplitter* child_splitter( dynamic_cast<QSplitter*>( child ) );
    if( child_splitter && child_splitter->orientation() == orientation ) splitter = child_splitter;
    else {
      // create new splitter
      splitter = new LocalSplitter( main_ );
      splitter->setOrientation( orientation );
      main_->layout()->addWidget( splitter );

      // reparent first child
      splitter->addWidget( child );
    }
  }

  // return created splitter
  return *splitter;

}

//_____________________________________________________________
TextDisplay& MainWindow::_newTextDisplay( QWidget* parent )
{
  Debug::Throw( "\nMainWindow::_newTextDisplay.\n" );

  // create textDisplay
  TextDisplay* display = new TextDisplay( parent );  
  display->setRecentFilesMenu( &menu_->recentFilesMenu() );

  // connections
  connect( display, SIGNAL( needUpdate( unsigned int ) ), SLOT( _update( unsigned int ) ) );
  connect( display, SIGNAL( hasFocus( TextEditor* ) ), SLOT( _displayFocusChanged( TextEditor* ) ) );
  connect( display, SIGNAL( cursorPositionChanged() ), &position_timer_, SLOT( start() ) );
  connect( display, SIGNAL( overwriteModeChanged() ), SLOT( _updateOverwriteMode() ) );
  
  connect( display, SIGNAL( undoAvailable( bool ) ), &undoAction(), SLOT( setEnabled( bool ) ) );
  connect( display, SIGNAL( redoAvailable( bool ) ), &redoAction(), SLOT( setEnabled( bool ) ) );
  
  // associate display to this editFrame
  BASE::Key::associate( this, display );
  
  // update current display and focus
  setActiveDisplay( *display );
  display->setFocus();
  Debug::Throw() << "MainWindow::_newTextDisplay - key: " << display->key() << endl;
  Debug::Throw( "MainWindow::_newTextDisplay - done.\n" );
    
  return *display;
  
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