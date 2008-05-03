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
  \file EditFrame.cpp
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

#include "AutoSave.h"
#include "BlockDelimiterWidget.h"
#include "ClockLabel.h"
#include "Config.h"
#include "CustomFileDialog.h"
#include "LineEditor.h"
#include "CustomPixmap.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "Diff.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "EditFrame.h"
#include "HighlightBlockFlags.h"
#include "IconEngine.h"
#include "Icons.h"
#include "LineNumberWidget.h"
#include "MainFrame.h"
#include "Menu.h"
#include "NewFileDialog.h"
#include "OpenPreviousMenu.h"
#include "XmlOptions.h"
#include "PrintDialog.h"
#include "QtUtil.h"
#include "StatusBar.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "TextView.h"
#include "Util.h"
#include "WindowTitle.h"

using namespace std;
using namespace Qt;

//_____________________________________________________
EditFrame::EditFrame(  QWidget* parent ):
  CustomMainWindow( parent ),
  Counter( "EditFrame" ),
  menu_( 0 ),
  active_view_( 0 ),
  statusbar_( 0 ),
  file_editor_( 0 ),
  default_orientation_( Horizontal ),
  default_open_mode_( NEW_WINDOW ),
  position_timer_(this)
{

  Debug::Throw( "EditFrame::EditFrame.\n" );

  // tell frame to delete on exit
  setAttribute( WA_DeleteOnClose );
  
  // install actions
  _installActions();
  
  // menu
  setMenuBar( menu_ = new Menu( this ) );
  connect( menu_, SIGNAL( documentClassSelected( QString ) ), this, SLOT( selectClassName( QString ) ) );

  // main vbox
  QWidget* main( new QWidget( this ) );
  QVBoxLayout* layout( new QVBoxLayout() );
  main->setLayout( layout );
  layout->setMargin(2);
  layout->setSpacing(2);
  setCentralWidget( main );

  // TextDisplay container
  main_ = new QWidget( main );
  main_->setLayout( new QVBoxLayout() );
  main_->layout()->setMargin(0);
  main_->layout()->setSpacing(0);
  layout->addWidget( main_, 1 );
  
  // create new Text display and register autosave thread
  TextView& view = _newTextView( main_ );
  main_->layout()->addWidget( &view );
  
  view.editor().setActive( true );
  static_cast<MainFrame*>(qApp)->autoSave().newThread( &view.editor() );
  Debug::Throw( "EditFrame::EditFrame - thread created.\n" );

  // state frame
  setStatusBar( statusbar_ = new StatusBar( this ) );

  // create "hidden" line editor to display filename
  statusbar_->addPermanentWidget( file_editor_ = new LineEditor( statusbar_ ), 1 );
  statusbar_->addLabels( 2, 0 );
  statusbar_->label(0).setAlignment( AlignCenter ); 
  statusbar_->label(1).setAlignment( AlignCenter ); 
  statusbar_->addClock();

  // modify frame and set readOnly
  file_editor_->setReadOnly( true );
  file_editor_->setFrame( false );
  
  // modify color
  QPalette palette( file_editor_->palette() );
  palette.setColor( QPalette::Base, palette.color( QPalette::Window ) );
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
  toolbar = new CustomToolBar( "Multiple views", this, "SPLIT_TOOLBAR" );
  toolbar->addAction( &splitViewHorizontalAction() ); 
  toolbar->addAction( &splitViewVerticalAction() ); 
  toolbar->addAction( &openHorizontalAction() ); 
  toolbar->addAction( &openVerticalAction() ); 
  toolbar->addAction( &closeViewAction() );
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
  
  Debug::Throw( "EditFrame::EditFrame - done.\n" );
 
}

//___________________________________________________________
EditFrame::~EditFrame( void )
{ Debug::Throw( "EditFrame::~EditFrame.\n" ); }

//____________________________________________
void EditFrame::setFile( File file )
{
  Debug::Throw() << "EditFrame::setFile - " << file << endl;

  // look for first empty view
  BASE::KeySet<TextView> views( this );
  BASE::KeySet<TextView>::iterator iter = find_if( views.begin(), views.end(), TextView::EmptyFileFTor() );
  assert( iter != views.end() );
  TextView &view( **iter );
   
  // open file in active display
  if( !file.empty() ) view.editor().openFile( file );
  else view.editor().updateDocumentClass();
  
  // set focus
  setActiveView( view );
  view.editor().setFocus();

  Debug::Throw( "EditFrame::setFile - done.\n" );
  
  return;
}

//________________________________________________________________
void EditFrame::setActiveView( TextView& view )
{ 
  Debug::Throw() << "EditFrame::setActiveView - key: " << view.key() << std::endl;
  assert( view.isAssociated( this ) );
  
  active_view_ = &view;
  if( !activeView().editor().isActive() )
  {

    BASE::KeySet<TextView> views( this );
    for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
    { (*iter)->editor().setActive( false ); }
    
    activeView().editor().setActive( true );
    _update( TextDisplay::ALL );

  }
  
  Debug::Throw( "EditFrame::setActiveDisplay - done.\n" );
  
}

//____________________________________________
void EditFrame::saveAll( void )
{
  Debug::Throw( "EditFrame::saveAll.\n" );

  // retrieve all displays
  BASE::KeySet<TextView> views( this );
  for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
  { if( (*iter)->editor().document()->isModified() ) (*iter)->editor().save(); }

  return;

}

//________________________________________________________________
void EditFrame::selectClassName( QString name )
{
  Debug::Throw( "EditFrame::SelectClassName.\n" );

  // retrieve all displays matching active
  // and update class name
  BASE::KeySet<TextDisplay> displays( &activeView().editor() );
  displays.insert( &activeView().editor() );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    (*iter)->setClassName( name );
    (*iter)->updateDocumentClass();
  }
  
  // rehighlight
  activeView().editor().rehighlight();

}

//________________________________________________________________
void EditFrame::rehighlight( void )
{
  Debug::Throw( "EditFrame::Rehighlight.\n" );

  // retrieve associated TextDisplay
  BASE::KeySet<TextView> views( this );
  BASE::KeySet<TextDisplay> displays;
  for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
  {

    // this trick allow to run the rehighlight only once per set of associated displays
    if( std::find_if( displays.begin(), displays.end(), BASE::Key::IsAssociatedFTor( &(*iter)->editor() ) ) == displays.end() )
    { (*iter)->editor().rehighlight(); }  
    
    displays.insert( &(*iter)->editor() );
    
  }

  return;
}

//____________________________________________
void EditFrame::_detach( void )
{

  Debug::Throw( "EditFrame::_detach.\n" );

  // check number of independent displays
  if( independentDisplayCount() < 2 )
  {
    QtUtil::infoDialog( this,
      "There must be at least two different files opened\n"
      "in the same window for the views to be detachable" );
    return;
  }

  // check number of displays associated to active
  BASE::KeySet<TextDisplay> associated_displays( &activeView().editor() );
  if( !
    ( associated_displays.empty() ||
      QtUtil::questionDialog( this,
      "Active view has clones in the current window.\n"
      "They will be closed when the view is detached.\n"
      "Continue ?" ) ) ) return;

  // keep active display local.
  TextView& active_view_local( activeView() );
  bool modified( active_view_local.editor().document()->isModified() );

  // close all clone views
  for( BASE::KeySet<TextDisplay>::iterator iter = associated_displays.begin(); iter != associated_displays.end(); iter++ )
  { _closeView( *dynamic_cast<TextView*>((*iter)->parentWidget()) ); }

  // create EditFrame
  EditFrame& frame( static_cast<MainFrame*>(qApp)->newEditFrame() );

  // clone its display from the current
  frame.activeView().synchronize( &active_view_local );

  // delete active display local
  active_view_local.editor().document()->setModified( false );
  _closeView( active_view_local );

  // show the new frame
  frame.activeView().editor().document()->setModified( modified );
  frame.show();
  frame._updateConfiguration();

  return;
  
}

//___________________________________________________________
void EditFrame::_revertToSave( void )
{
  
  Debug::Throw( "EditFrame::_revertToSave.\n" );

  // check filename
  if( activeView().editor().file().empty() )
  {
    QtUtil::infoDialog( this, "No filename given. <Revert to save> canceled." );
    return;
  }

  // ask for confirmation
  ostringstream what;
  if( activeView().editor().document()->isModified() ) what << "Discard changes to " << activeView().editor().file().localName() << "?";
  else what << "Reload file " << activeView().editor().file().localName() << "?";
  if( !QtUtil::questionDialog( this, what.str() ) ) return;

  activeView().editor().revertToSave();
  
}

//___________________________________________________________
void EditFrame::_print( void )
{
  Debug::Throw( "EditFrame::_print.\n" );

  // retrieve activeDisplay file
  const File& file( activeView().editor().file() );

  // check if file is modified
  if( activeView().editor().document()->isModified() && activeView().editor().askForSave() == AskForSaveDialog::CANCEL ) return;

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
  
  Debug::Throw( "EditFrame::_print - options saved.\n" );
  
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
  Debug::Throw( "EditFrame::_print - retrieved html string.\n" );
  
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
    Debug::Throw( "EditFrame::_print - html file saved.\n" );
    
  } else if( mode == PrintDialog::PDF ) {
    
    QTextEdit local(0);
    local.setLineWrapMode( QTextEdit::WidgetWidth );
    local.setHtml( html_string );
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fullname.c_str());
    
    local.document()->print(&printer);
    Debug::Throw( "EditFrame::_print - pdf file saved.\n" );
    
  } else return;
  
  // try open
  if( dialog.useCommand() )
  { Util::run( QStringList() << dialog.command() << fullname.c_str() ); }   
  
  Debug::Throw( "EditFrame::_print - done.\n" );
  return;

}

//____________________________________________
void EditFrame::closeEvent( QCloseEvent* event )
{
  Debug::Throw( "EditFrame::closeEvent.\n" );

  // check for modifications
  event->accept();
      
  // look over TextDisplays
  if( isModified() )
  {

    // look over TextDisplays
    BASE::KeySet<TextView> views( this );
    BASE::KeySet<TextDisplay> displays;
    for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
    {

      // get local reference to display
      TextView& view( **iter );
      
      // check if this display is modified
      if( !view.editor().document()->isModified() ) continue;

      // this trick allow to run  only once per set of associated displays
      if( std::find_if( displays.begin(), displays.end(), BASE::Key::IsAssociatedFTor( &view.editor() ) ) != displays.end() ) continue;
      displays.insert( &view.editor() );
      
      // ask for save
      int state( view.editor().askForSave( modifiedDisplayCount() > 1 ) );
      if( state == AskForSaveDialog::ALL ) {
        
        // for this frame, only save displays located after the current
        for( BASE::KeySet<TextView>::iterator view_iter = iter; view_iter != views.end(); view_iter++ )
        { if( (*view_iter)->editor().document()->isModified() ) (*view_iter)->editor().save(); }

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
void EditFrame::enterEvent( QEvent* e )
{

  Debug::Throw( "EditFrame::enterEvent.\n" );
  QMainWindow::enterEvent( e );

  // keep track of displays to be deleted, if any
  BASE::KeySet<TextView> dead_views;

  // retrieve displays
  BASE::KeySet<TextView> views( this );
  BASE::KeySet<TextDisplay> displays;
  for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
  {

    // this trick allow to run only once per set of displays associated to the same file
    if( std::find_if( displays.begin(), displays.end(), BASE::Key::IsAssociatedFTor( &(*iter)->editor() ) ) != displays.end() ) continue;
    displays.insert( &(*iter)->editor() );
    
    // keep local reference of current display
    TextView &view( **iter );
    
    // check file
    if( view.editor().checkFileRemoved() == FileRemovedDialog::CLOSE ) 
    { 
        
      // register displays as dead
      BASE::KeySet<TextDisplay> associated_displays( &view.editor() );
      for( BASE::KeySet<TextDisplay>::iterator display_iter = associated_displays.begin(); display_iter != associated_displays.end(); display_iter++ )
      { dead_views.insert( dynamic_cast<TextView*>( (*display_iter)->parentWidget() ) ); }
      dead_views.insert( &view );
            
    } else {
 
      (*iter)->editor().checkFileReadOnly();
      (*iter)->editor().checkFileModified();

    }
    
  }
  
  // update window title
  _updateWindowTitle();

  // delete dead_displays
  if( !dead_views.empty() )
  {

    Debug::Throw() << "EditFrame::enterEvent - dead views: " << dead_views.size() << endl;
    for( BASE::KeySet<TextView>::iterator iter = dead_views.begin(); iter != dead_views.end(); iter++ )
    { _closeView( **iter ); }

    // need to close window manually if there is no remaining displays
    if( dead_views.size() == views.size() ) close();

  }

  Debug::Throw( "EditFrame::enterEvent - done.\n" );

}

//_______________________________________________________
void EditFrame::_diff( void )
{
  Debug::Throw( "EditFrame::_diff.\n" );
 
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
  TextDisplay& first = activeView().editor();
  
  // retrieve displays associated to frame
  // look for the first one that is not associated to the active display
  BASE::KeySet<TextView> views( this );
  BASE::KeySet<TextView>::iterator iter = views.begin();
  for(; iter != views.end(); iter++ )
  {
    if( !( &(*iter)->editor() == &first || (*iter)->editor().isAssociated( &first ) ) )
    {
      diff->setTextDisplays( first, (*iter)->editor() );
      break;
    }
  }
  
  // check that one display was found
  assert( iter != views.end() );
  
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
void EditFrame::_update( unsigned int flags )
{

  Debug::Throw( "EditFrame::_update().\n" );

  if( flags & TextDisplay::WINDOW_TITLE )
  { _updateWindowTitle(); }

  if( flags & TextDisplay::FILE_NAME && file_editor_ )
  { 
    file_editor_->setText( activeView().editor().file().c_str() ); 
    fileInfoAction().setEnabled( !activeView().editor().file().empty() );
  }

  if( flags & TextDisplay::CUT )
  { cutAction().setEnabled( activeView().editor().cutAction().isEnabled() ); }

  if( flags & TextDisplay::COPY )
  { copyAction().setEnabled( activeView().editor().copyAction().isEnabled() ); }

  if( flags & TextDisplay::PASTE )
  { pasteAction().setEnabled( activeView().editor().pasteAction().isEnabled() ); }

  if( flags & TextDisplay::UNDO_REDO )
  {
    undoAction().setEnabled( activeView().editor().undoAction().isEnabled() );
    redoAction().setEnabled( activeView().editor().redoAction().isEnabled() );
  }
  
  if( flags & TextDisplay::SAVE )
  { saveAction().setEnabled( !activeView().editor().isReadOnly() ); }

}

//_____________________________________________
void EditFrame::_updateCursorPosition( void )
{
  
  TextPosition position( activeView().editor().textPosition() );
  statusbar_->label(0).setText( Str( "line : " ).append<int>( position.paragraph()+1 ).c_str() , false );
  statusbar_->label(1).setText( Str( "column : " ).append<int>( position.index()+1 ).c_str() , false );

  return;
}

//____________________________________________
void EditFrame::_displayFocusChanged( TextDisplay* display )
{
  Debug::Throw() << "EditFrame::_DisplayFocusChanged - " << display->key() << endl;
  setActiveView( *dynamic_cast<TextView*>(display->parentWidget()) );  
}

//___________________________________________________________
void EditFrame::_installActions( void )
{

  Debug::Throw( "EditFrame::_installActions.\n" );
  
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  addAction( new_file_action_ = new QAction( IconEngine::get( ICONS::NEW, path_list ), "&New", this ) );
  new_file_action_->setShortcut( CTRL+Key_N );
  new_file_action_->setToolTip( "Create a new empty file" );
  connect( new_file_action_, SIGNAL( triggered() ), SLOT( _newFile() ) );

  addAction( clone_action_ = new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT, path_list ), "&Clone", this ) );
  clone_action_->setShortcut( SHIFT+CTRL+Key_N );
  clone_action_->setToolTip( "Clone current view" );
  connect( clone_action_, SIGNAL( triggered() ), SLOT( _splitView() ) );

  addAction( detach_action_ = new QAction( IconEngine::get( ICONS::VIEW_DETACH, path_list ), "&Detach", this ) );
  detach_action_->setShortcut( SHIFT+CTRL+Key_O );
  detach_action_->setToolTip( "Detach current view" );
  detach_action_->setEnabled( false );
  connect( detach_action_, SIGNAL( triggered() ), SLOT( _detach() ) );

  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN, path_list ), "&Open", this ) );
  open_action_->setShortcut( CTRL+Key_O );
  open_action_->setToolTip( "Open an existsing file" );
  connect( open_action_, SIGNAL( triggered() ), SLOT( open() ) );
 
  addAction( close_view_action_ = new QAction( IconEngine::get( ICONS::VIEW_REMOVE, path_list ), "&Close view", this ) );
  close_view_action_->setShortcut( CTRL+Key_W );
  close_view_action_->setToolTip( "Close current view" );
  connect( close_view_action_, SIGNAL( triggered() ), SLOT( _closeView() ) );
 
  addAction( close_window_action_ = new QAction( IconEngine::get( ICONS::CLOSE, path_list ), "&Close window", this ) );
  close_window_action_->setShortcut( SHIFT+CTRL+Key_W );
  close_window_action_->setToolTip( "Close current view" );
  connect( close_window_action_, SIGNAL( triggered() ), SLOT( _closeWindow() ) );
 
  addAction( save_action_ = new QAction( IconEngine::get( ICONS::SAVE, path_list ), "&Save", this ) );
  save_action_->setShortcut( CTRL+Key_S );
  save_action_->setToolTip( "Save current file" );
  connect( save_action_, SIGNAL( triggered() ), SLOT( _save() ) );
 
  addAction( save_as_action_ = new QAction( IconEngine::get( ICONS::SAVE_AS, path_list ), "Save &As", this ) );
  save_as_action_->setShortcut( SHIFT+CTRL+Key_S );
  save_as_action_->setToolTip( "Save current file with a different name" );
  connect( save_as_action_, SIGNAL( triggered() ), SLOT( _saveAs() ) );

  addAction( revert_to_save_action_ = new QAction( IconEngine::get( ICONS::RELOAD, path_list ), "&Revert to saved", this ) );
  revert_to_save_action_->setToolTip( "Reload saved version of current file" );
  connect( revert_to_save_action_, SIGNAL( triggered() ), SLOT( _revertToSave() ) );
 
  addAction( print_action_ = new QAction( IconEngine::get( ICONS::PRINT, path_list ), "&Print", this ) );
  print_action_->setToolTip( "Print current file" );
  connect( print_action_, SIGNAL( triggered() ), SLOT( _print() ) );

  addAction( undo_action_ = new QAction( IconEngine::get( ICONS::UNDO, path_list ), "&Undo", this ) );
  undo_action_->setToolTip( "Undo last action" );
  connect( undo_action_, SIGNAL( triggered() ), SLOT( _undo() ) );

  addAction( redo_action_ = new QAction( IconEngine::get( ICONS::REDO, path_list ), "&Redo", this ) );
  redo_action_->setToolTip( "Redo last un-done action" );
  connect( redo_action_, SIGNAL( triggered() ), SLOT( _redo() ) );

  addAction( cut_action_ = new QAction( IconEngine::get( ICONS::CUT, path_list ), "&Cut", this ) );
  cut_action_->setToolTip( "Cut current selection and copy to clipboard" );
  connect( cut_action_, SIGNAL( triggered() ), SLOT( _cut() ) );

  addAction( copy_action_ = new QAction( IconEngine::get( ICONS::COPY, path_list ), "&Copy", this ) );
  copy_action_->setToolTip( "Copy current selection to clipboard" );
  connect( copy_action_, SIGNAL( triggered() ), SLOT( _copy() ) );

  addAction( paste_action_ = new QAction( IconEngine::get( ICONS::PASTE, path_list ), "&Paste", this ) );
  paste_action_->setToolTip( "Paste clipboard to text" );
  connect( paste_action_, SIGNAL( triggered() ), SLOT( _paste() ) );

  addAction( file_info_action_ = new QAction( IconEngine::get( ICONS::INFO, path_list ), "&File information", this ) );
  file_info_action_->setToolTip( "Display file informations" );
  connect( file_info_action_, SIGNAL( triggered() ), SLOT( _fileInfo() ) );

  addAction( spellcheck_action_ = new QAction( IconEngine::get( ICONS::SPELLCHECK, path_list ), "&Spell check", this ) );
  #if WITH_ASPELL
  connect( spellcheck_action_, SIGNAL( triggered() ), SLOT( _spellcheck( void ) ) );
  #else 
  spellcheck_action_->setVisible( false );
  #endif

  addAction( diff_action_ = new QAction( "&Diff files", this ) );
  connect( diff_action_, SIGNAL( triggered() ), SLOT( _diff() ) );
  diff_action_->setEnabled( false );
  
  addAction( split_view_horizontal_action_ =new QAction( IconEngine::get( ICONS::VIEW_TOPBOTTOM, path_list ), "Clone view top/bottom", this ) );
  split_view_horizontal_action_->setToolTip( "Clone current view vertically" );
  connect( split_view_horizontal_action_, SIGNAL( triggered() ), SLOT( _splitViewVertical() ) );

  addAction( split_view_vertical_action_ =new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT, path_list ), "Clone view left/right", this ) );
  split_view_vertical_action_->setToolTip( "Clone current view horizontally" );
  connect( split_view_vertical_action_, SIGNAL( triggered() ), SLOT( _splitViewHorizontal() ) );

  addAction( open_horizontal_action_ =new QAction( IconEngine::get( ICONS::VIEW_BOTTOM, path_list ), "Clone view top/bottom", this ) );
  open_horizontal_action_->setToolTip( "Open a new view vertically" );
  connect( open_horizontal_action_, SIGNAL( triggered() ), SLOT( openVertical() ) );

  addAction( open_vertical_action_ =new QAction( IconEngine::get( ICONS::VIEW_RIGHT, path_list ), "Open view left/right", this ) );
  open_vertical_action_->setToolTip( "Open a new view horizontally" );
  connect( open_vertical_action_, SIGNAL( triggered() ), SLOT( openHorizontal() ) );
  
}

//___________________________________________________________
void EditFrame::_updateWindowTitle()
{ 
  Debug::Throw( "EditFrame::_updateWindowTitle.\n" );
  
  {
    bool readonly( activeView().editor().isReadOnly() );
    bool modified( activeView().editor().document()->isModified() );
    Debug::Throw() << "EditFrame::_updateWindowTitle -"
      << " readonly: " << readonly 
      << " modified: " << modified
      << endl;
  }
  
  setWindowTitle( WindowTitle( activeView().editor().file() )
    .setReadOnly( activeView().editor().isReadOnly() )
    .setModified( activeView().editor().document()->isModified() )
    );
  
  Debug::Throw( "EditFrame::_updateWindowTitle - done.\n" );
}

//________________________________________________________
void EditFrame::_updateConfiguration( void )
{
  
  Debug::Throw( "EditFrame::_updateConfiguration.\n" );
      
  // resize
  resize( QSize( XmlOptions::get().get<int>( "WINDOW_WIDTH" ), XmlOptions::get().get<int>( "WINDOW_HEIGHT" ) ) );

  // assign icons to file in open previous menu based on class manager
  list<File> files( menu_->openPreviousMenu().files() );
  for( list<File>::const_iterator iter = files.begin(); iter != files.end(); iter++ )
  {
    
    FileRecord& record( menu_->openPreviousMenu().get( *iter ) ); 
    if( !record.hasInformation( "class_name" ) ) continue; 
    DocumentClass document_class( static_cast<MainFrame*>(qApp)->classManager().get( record.information( "class_name" ).c_str() ) );
    if( !document_class.icon().isEmpty() ) record.addInformation( "icon", qPrintable( document_class.icon() ) );
  
  }
    
}

//________________________________________________________
void EditFrame::_saveConfiguration( void )
{
  Debug::Throw( "EditFrame::_saveConfiguration.\n" );

  // save size
  XmlOptions::get().set<int>( "WINDOW_HEIGHT", height() );
  XmlOptions::get().set<int>( "WINDOW_WIDTH", width() );
  
}

//___________________________________________________________
void EditFrame::_newFile( const OpenMode& mode, const Orientation& orientation )
{

  Debug::Throw( "EditFrame::_New.\n" );

  // check open_mode
  if( mode == NEW_WINDOW ) static_cast<MainFrame*>(qApp)->open();
  else _splitView( orientation, false );

}

//___________________________________________________________
void EditFrame::_open( FileRecord record, const OpenMode& mode, const Orientation& orientation )
{

  Debug::Throw( "EditFrame::_Open.\n" );

  // copy to local
  if( record.file().empty() )
  {

    // create file dialog
    CustomFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setDirectory( QDir( activeView().editor().workingDirectory().c_str() ) );
    QtUtil::centerOnParent( &dialog );
    if( dialog.exec() == QDialog::Rejected ) return;
    
    QStringList files( dialog.selectedFiles() );
    if( files.empty() ) return;
  
    record.setFile( File( qPrintable( files.front() ) ).expand() );
  
  }
  
  // check open_mode
  if( mode == NEW_WINDOW )
  {
    // open via the MainFrame to create a new editor
    static_cast<MainFrame*>(qApp)->open( record );
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
  
  // retrieve all edit frames
  // find one matching
  BASE::KeySet<EditFrame> frames( static_cast<MainFrame*>(qApp) );
  BASE::KeySet<EditFrame>::iterator iter = find_if( frames.begin(), frames.end(), EditFrame::SameFileFTor( record.file() ) );
  if( iter != frames.end() )
  {

    // select found display in EditFrame
    (*iter)->selectView( record.file() );

    // check if the found frame is the current
    if( *iter == this )
    {
      uniconify();
      return;
    }

    ostringstream what;
    what
      << "The file " << record.file() << " is already opened in another window.\n"
      << "Do you want to close the other view and open the file here ?";
    if( !QtUtil::questionDialog( this, what.str() ) )
    {
      (*iter)->uniconify();
      return;
    }

    // look for an empty display
    // create a new view if none is found
    BASE::KeySet<TextView> views( this );
    BASE::KeySet<TextView>::iterator view_iter( find_if( views.begin(), views.end(), TextView::EmptyFileFTor() ) );
    TextView& view( view_iter == views.end() ? _splitView( orientation, false ):**view_iter );

    // retrieve active display from previous frame
    TextView& previous_view( (*iter)->activeView() );

    // store modification state
    bool modified( previous_view.editor().document()->isModified() );

    // clone
    view.synchronize( &previous_view );
    
    // set previous display as unmdified
    previous_view.editor().document()->setModified( false );

    // close display, or frame, depending on its number of independent files
    if( (*iter)->independentDisplayCount() == 1 ) (*iter)->close();
    else
    {
      BASE::KeySet<TextDisplay> displays( &previous_view.editor() );
      displays.insert( &previous_view.editor() );
      for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
      { (*iter)->_closeView( *dynamic_cast<TextView*>( (*display_iter)->parentWidget() ) ); }
    }

    // restore modification state and make new display active
    view.editor().setModified( modified );
    setActiveView( view );
    view.editor().setFocus();

  } else {

    // look for an empty display
    BASE::KeySet<TextView> views( this );
    BASE::KeySet<TextView>::iterator view_iter( find_if( views.begin(), views.end(), TextView::EmptyFileFTor() ) );
    if( view_iter == views.end() ) _splitView( orientation, false );

    // open file in this window
    setFile( record.file() );

    // update configuration
    _updateConfiguration();
    
  }
 
  return;
}

//___________________________________________________________
void EditFrame::_closeView( TextView& view )
{
  Debug::Throw( "EditFrame::_closeView.\n" );

  // retrieve number of displays
  // if only one display, close the entire window
  BASE::KeySet<TextView> views( this );
  if( views.size() < 2 )
  {
    Debug::Throw() << "EditFrame::_closeView - full close." << endl;
    close();
    return;
  }

  // check if display is modified and has no associates in window
  if( 
    view.editor().document()->isModified() && 
    BASE::KeySet<TextDisplay>( &view.editor() ).empty() &&
    view.editor().askForSave() ==  AskForSaveDialog::CANCEL ) return;

  // retrieve parent and grandparent of current display
  QWidget* parent( view.parentWidget() );    
  QSplitter* parent_splitter( dynamic_cast<QSplitter*>( parent ) );
  
  // retrieve displays associated to current
  BASE::KeySet<TextView> displays( &view.editor() );
    
  // delete display
  delete &view;
  
  // check how many children remain in parent_splitter if any
  if( parent_splitter && parent_splitter->count() == 1 ) 
  {
    
    // retrieve child
    QWidget* child( dynamic_cast<QWidget*>( parent_splitter->children().first() ) );
    
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
    delete parent_splitter;

  }
    
  // if no associated displays, retrieve all, set the first as active
  if( displays.empty() )
  {

    Debug::Throw( "EditFrame::_closeView - no associated display.\n" );
    setActiveView( **BASE::KeySet<TextView>( this ).rbegin() );

  } else {

    // update active display
    setActiveView( *dynamic_cast<TextView*>((*displays.rbegin())->parentWidget()) );

  }
  
  // update close_view button
  int independent_display_count( independentDisplayCount() );
  detachAction().setEnabled( independent_display_count > 1 );
  diffAction().setEnabled( independent_display_count == 2 );
  
  // change focus
  activeView().editor().setFocus();
  Debug::Throw( "EditFrame::_closeView - done.\n" );

}

//___________________________________________________________
TextView& EditFrame::_splitView( const Orientation& orientation, const bool& clone )
{
  
  Debug::Throw( "EditFrame::_splitView.\n" );

  // keep local pointer to current active display
  TextView& active_view_local( activeView() );  
  
  // compute desired dimension of the new splitter
  // along its splitting direction
  int dimension(0);
  if( clone ) dimension = (orientation == Horizontal) ? active_view_local.width():active_view_local.height();
  else dimension = (orientation == Horizontal) ? main_->width():main_->height();

  // create new splitter
  QSplitter& splitter( _newSplitter( orientation, clone ) );
  
  // create new display
  TextView& view( _newTextView(0) );
  
  // insert in splitter, at correct position
  if( clone ) splitter.insertWidget( splitter.indexOf( &active_view_local )+1, &view );
  else splitter.addWidget( &view );

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
    BASE::KeySet<TextDisplay> displays( &active_view_local.editor() );

    // clone new display
    view.synchronize( &active_view_local );
    
    // perform associations
    // check if active displays has associates and propagate to new
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { BASE::Key::associate( &view.editor(), *iter ); }

    // associate this display to AutoSave threads
    BASE::KeySet<AutoSaveThread> threads( &active_view_local.editor() );
    for( BASE::KeySet<AutoSaveThread>::iterator iter = threads.begin(); iter != threads.end(); iter++ )
    { BASE::Key::associate( &view.editor(), *iter ); }
 
    // associate new display to active
    BASE::Key::associate( &view.editor(), &active_view_local.editor() );

  } else {

    // register new AutoSave thread
    static_cast<MainFrame*>(qApp)->autoSave().newThread( &view.editor() );

    // enable detach
    detachAction().setEnabled( true );
    diffAction().setEnabled( independentDisplayCount() == 2 );
    
  }

  return view;

}

//____________________________________________________________
QSplitter& EditFrame::_newSplitter( const Orientation& orientation, const bool& clone )
{

  Debug::Throw( "EditFrame::_newSplitter.\n" );
  QSplitter *splitter = 0;
  
  if( clone )
  {
    
    /* in clone mode, a new splitter is added as a parent of the activeDisplay.
    the new (cloned) TextDisplay will appear side by side with it */

    // retrieve parent of current display
    QWidget* parent( activeView().parentWidget() );  

    // try catch to splitter
    // do not create a new splitter if the parent has same orientation
    QSplitter *parent_splitter( dynamic_cast<QSplitter*>( parent ) );
    if( parent_splitter && parent_splitter->orientation() == orientation ) splitter = parent_splitter;
    else {
      
 
      // move splitter to the first place if needed
      if( parent_splitter ) 
      {
        
        Debug::Throw( "EditFrame::_newSplitter - found parent splitter.\n" );
        // create a splitter with correct orientation
        // give him no parent, because the parent is set in QSplitter::insertWidget()
        splitter = new LocalSplitter(0);
        splitter->setOrientation( orientation );
        parent_splitter->insertWidget( parent_splitter->indexOf( &activeView() ), splitter );
        
      } else {
        
        // create a splitter with correct orientation
        splitter = new LocalSplitter(parent);
        splitter->setOrientation( orientation );
        parent->layout()->addWidget( splitter );
        
      }
      
      // reparent current display
      splitter->addWidget( &activeView() );
      
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
TextView& EditFrame::_newTextView( QWidget* parent )
{
  Debug::Throw( "EditFrame::_newTextDisplay.\n" );

  // create textDisplay
  TextView* view = new TextView( parent );  
  view->editor().setOpenPreviousMenu( &menu_->openPreviousMenu() );

  // connections
  connect( &view->editor(), SIGNAL( needUpdate( unsigned int ) ), SLOT( _update( unsigned int ) ) );
  connect( &view->editor(), SIGNAL( hasFocus( TextDisplay* ) ), SLOT( _displayFocusChanged( TextDisplay* ) ) );
  connect( &view->editor(), SIGNAL( cursorPositionChanged() ), &position_timer_, SLOT( start() ) );
  
  connect( &view->editor(), SIGNAL( undoAvailable( bool ) ), &undoAction(), SLOT( setEnabled( bool ) ) );
  connect( &view->editor(), SIGNAL( redoAvailable( bool ) ), &redoAction(), SLOT( setEnabled( bool ) ) );
  
  // associate display to this editFrame
  BASE::Key::associate( this, view );
  
  // update current display and focus
  setActiveView( *view );
  view->editor().setFocus();
  Debug::Throw() << "EditFrame::_newTextDisplay - key: " << view->key() << endl;
  Debug::Throw( "EditFrame::newTextDisplay - done.\n" );
    
  return *view;
  
}

//_____________________________________________________________________
QString EditFrame::_htmlString( const int& max_line_size )
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
  title.appendChild( document.createTextNode( activeView().editor().file().c_str() ) );

  // body
  html.
    appendChild( document.createElement( "body" ) ).
    appendChild( activeView().editor().htmlNode( document, max_line_size ) );

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
