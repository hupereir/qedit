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
  \file EditFrame.cc
  \brief editor main window
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QApplication>
#include <QDomElement>
#include <QDomDocument>
#include <QObjectList>

#include "AutoSave.h"
#include "ClockLabel.h"
#include "Config.h"
#include "CustomFileDialog.h"
#include "CustomLineEdit.h"
#include "CustomPixmap.h"
#include "CustomToolBar.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "DocumentClassManager.h"
#include "DocumentClassDialog.h"
#include "EditFrame.h"
#include "IconEngine.h"
#include "Icons.h"
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
#include "Util.h"
#include "ViewHtmlDialog.h"
#include "WindowTitle.h"

using namespace std;
using namespace Qt;

//_____________________________________________________
EditFrame::EditFrame(  QWidget* parent ):
  CustomMainWindow( parent ),
  Counter( "EditFrame" ),
  menu_( 0 ),
  active_display_( 0 ),
  statusbar_( 0 ),
  file_editor_( 0 ),
  default_orientation_( Horizontal ),
  default_open_mode_( NEW_WINDOW )
{

  Debug::Throw( "EditFrame::EditFrame.\n" );

  // tell frame to delete on exit
  setAttribute( WA_DeleteOnClose );
  
  // install actions
  _installActions();
  
  // menu
  setMenuBar( menu_ = new Menu( this ) );
  connect( menu_, SIGNAL( documentClassSelected( std::string ) ), this, SLOT( selectClassName( std::string ) ) );

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
  TextDisplay& display = _newTextDisplay( main_ );
  main_->layout()->addWidget( &display );
  
  display.setActive( true );
  dynamic_cast<MainFrame*>(qApp)->autoSave().newThread( &display );
  Debug::Throw( "EditFrame::EditFrame - thread created.\n" );

  // state frame
  layout->addWidget( statusbar_ = new StatusBar( main ) );

  // create "hidden" line editor to display filename
  statusbar_->getLayout().addWidget( file_editor_ = new CustomLineEdit( statusbar_ ), 1, AlignVCenter );
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
  CustomToolBar* toolbar;
  toolbar = new CustomToolBar( "Main", this );
  toolbars_.push_back( make_pair( toolbar, "FILE_TOOLBAR" ) );
  addToolBar( toolbar );

  toolbar->addAction( &newFileAction() );
  toolbar->addAction( &openAction() ); 
  toolbar->addAction( &saveAction() ); 
  //toolbar->addAction( &printAction() );
  
  // edition toolbar
  toolbar = new CustomToolBar( "Edition", this );
  toolbars_.push_back( make_pair( toolbar, "EDITION_TOOLBAR" ) );
  addToolBar( toolbar );

  toolbar->addAction( &undoAction() ); 
  toolbar->addAction( &redoAction() ); 
  toolbar->addAction( &cutAction() );
  toolbar->addAction( &copyAction() );
  toolbar->addAction( &pasteAction() );

  // extra toolbar
  toolbar = new CustomToolBar( "Tools", this );
  toolbars_.push_back( make_pair( toolbar, "EXTRA_TOOLBAR" ) );
  addToolBar( toolbar );

  toolbar->addAction( &fileInfoAction() ); 
  toolbar->addAction( &spellcheckAction() ); 
  
  // splitting toolbar
  toolbar = new CustomToolBar( "Display", this );
  toolbars_.push_back( make_pair( toolbar, "SPLIT_TOOLBAR" ) );
  addToolBar( toolbar );
  
  // retrieve pixmap path
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  QAction* action;
  action = toolbar->addAction( IconEngine::get( ICONS::VIEW_TOPBOTTOM, path_list ), "Clone view top/bottom", this, SLOT( _splitViewVertical() ) );
  action->setToolTip( "Clone current view vertically" );

  action = toolbar->addAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT, path_list ), "Clone view left/right", this, SLOT( _splitViewHorizontal() ) );
  action->setToolTip( "Clone current view horizontally" );

  action = toolbar->addAction( IconEngine::get( ICONS::VIEW_BOTTOM, path_list ), "Clone view top/bottom", this, SLOT( openVertical() ) );
  action->setToolTip( "Open a new view vertically" );

  action = toolbar->addAction( IconEngine::get( ICONS::VIEW_RIGHT, path_list ), "Open view left/right", this, SLOT( openHorizontal() ) );
  action->setToolTip( "Open a new view horizontally" );
  
  toolbar->addAction( &detachAction() );
  toolbar->addAction( &closeViewAction() );
 
  //! configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( saveConfiguration() ) );
  updateConfiguration();
  
  // update buttons
  _update( TextDisplay::ALL );
  
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
  BASE::KeySet<TextDisplay> displays( this );
  BASE::KeySet<TextDisplay>::iterator iter = find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() );
  Exception::check( iter != displays.end(), DESCRIPTION( "no empty display" ) );
  TextDisplay &display( **iter );
   
  // open file in active display
  if( !file.empty() ) display.openFile( file );
  
  // set focus
  setActiveDisplay( display );
  display.setFocus();
  
  return;
}

//________________________________________________________
void EditFrame::updateConfiguration( void )
{
  
  Debug::Throw( "EditFrame::updateConfiguration.\n" );
    
  CustomMainWindow::updateConfiguration(); 
  
  // resize
  resize( QSize( XmlOptions::get().get<int>( "WINDOW_WIDTH" ), XmlOptions::get().get<int>( "WINDOW_HEIGHT" ) ) );
   
  // toolbars visibility and location
  for( ToolbarList::iterator iter = toolbars_.begin(); iter != toolbars_.end(); iter++ )
  {
     
    QToolBar* toolbar( iter->first );
    string option_name( iter->second );
    string location_name( option_name + "_LOCATION" );
     
    bool visibility( XmlOptions::get().find( option_name ) ? XmlOptions::get().get<bool>( option_name ):true );
    bool current_visibility( toolbar->isVisible() );
    
    ToolBarArea location = (XmlOptions::get().find( location_name )) ? (ToolBarArea) CustomToolBar::nameToArea( XmlOptions::get().get<string>( location_name ) ):TopToolBarArea ;
    ToolBarArea current_location = toolBarArea( toolbar );
    
    Debug::Throw() << "EditFrame::updateConfiguration - " << option_name << " visibility: " << visibility << " location: " << (int)location << endl;
    
    if( visibility )
    {
      if( !( current_visibility && (location == current_location) ) ) 
      {
        addToolBar( location, toolbar );
        toolbar->show();
      }
    } else toolbar->hide();
     
    XmlOptions::get().set<bool>( option_name, !toolbar->isHidden() );
    XmlOptions::get().set<string>( location_name, CustomToolBar::areaToName( toolBarArea( toolbar ) ) );
  }  
  
  Debug::Throw( "EditFrame::updateConfiguration - done.\n" );

}

//________________________________________________________
void EditFrame::saveConfiguration( void )
{
  Debug::Throw( "EditFrame::saveConfiguration.\n" );

  // save size
  XmlOptions::get().set<int>( "WINDOW_HEIGHT", height() );
  XmlOptions::get().set<int>( "WINDOW_WIDTH", width() );
   
  // save toolbars location and visibility
  for( ToolbarList::iterator iter = toolbars_.begin(); iter != toolbars_.end(); iter++ )
  {
    
    QToolBar* toolbar( iter->first );
    string option_name( iter->second );
    string location_name( option_name + "_LOCATION" );
    XmlOptions::get().set<bool>( option_name, !toolbar->isHidden() );
    XmlOptions::get().set<string>( location_name, CustomToolBar::areaToName( toolBarArea( toolbar ) ) );
  }

  // write open previous menu.
  menu_->openPreviousMenu().write();
  
}

//________________________________________________________________
void EditFrame::setActiveDisplay( TextDisplay& display )
{ 
  Debug::Throw() << "EditFrame::setActiveDisplay - key: " << display.key() << std::endl;
  Exception::check( display.isAssociated( this ), DESCRIPTION( "invalid display" ) );
  
  active_display_ = &display;
  if( !activeDisplay().isActive() )
  {

    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { (*iter)->setActive( false ); }
    
    activeDisplay().setActive( true );
    _update( TextDisplay::ALL );

  }
  
  Debug::Throw( "EditFrame::setActiveDisplay - done.\n" );
  
}

//____________________________________________
void EditFrame::saveAll( void )
{
  Debug::Throw( "EditFrame::saveAll.\n" );

  // retrieve all displays
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { if( (*iter)->document()->isModified() ) (*iter)->save(); }

  return;

}

//________________________________________________________________
void EditFrame::selectClassName( string name  )
{
  Debug::Throw( "EditFrame::SelectClassName.\n" );

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
void EditFrame::rehighlight( void )
{
  Debug::Throw( "EditFrame::Rehighlight.\n" );

  // retrieve associated TextDisplay
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    // this trick allow to run the rehighlight only once per set of associated displays
    if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) == iter )
    { (*iter)->rehighlight(); }  
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
  BASE::KeySet<TextDisplay> associated_displays( &activeDisplay() );
  if( !
    ( associated_displays.empty() ||
      QtUtil::questionDialog( this,
      "Active view has clones in the current window.\n"
      "They will be closed when the view is detached.\n"
      "Continue ?" ) ) ) return;

  // keep active display local.
  TextDisplay& active_display_local( activeDisplay() );

  // close all clone views
  for( BASE::KeySet<TextDisplay>::iterator iter = associated_displays.begin(); iter != associated_displays.end(); iter++ )
  { _closeView( **iter ); }

  // create EditFrame
  EditFrame& frame( dynamic_cast<MainFrame*>(qApp)->newEditFrame() );

  // clone its display from the current
  frame.activeDisplay().synchronize( &active_display_local );

  // delete active display local
  _closeView( active_display_local );

  // show the new frame
  frame.show();
  frame.updateConfiguration();

  return;
  
}

//___________________________________________________________
void EditFrame::_revertToSave( void )
{
  
  Debug::Throw( "EditFrame::_revertToSave.\n" );

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
void EditFrame::_convertToHtml( void )
{
  Debug::Throw( "EditFrame::_convertToHtml.\n" );

  // create default file
  File default_file = activeDisplay().file().empty() ?
    File( "document.html" ).addPath( activeDisplay().workingDirectory() ):
    File( activeDisplay().file().truncatedName() + ".html" );

  ViewHtmlDialog dialog( this );
  dialog.setFile( default_file );
  dialog.setCommand( XmlOptions::get().raw("HTML_EDITOR") );
  dialog.setUseCommand( XmlOptions::get().get<bool>("USE_HTML_EDITOR" ) );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve output file
  File fullname( dialog.file().expand() );

  // check if file is directory
  if( fullname.isDirectory() )
  {
    ostringstream what;
    what << "file \"" << fullname << "\" is a directory. <Convert to Html> canceled.";
    QtUtil::infoDialog( this, what.str() );
    return;
  }

  // check if file exist
  if( fullname.exist() )
  {
    if( !fullname.isWritable() )
    {
      ostringstream what;
      what << "file \"" << fullname << "\" is read-only. <Convert to Html> canceled.";
      QtUtil::infoDialog( this, what.str() );
      return;
    } else if( !QtUtil::questionDialog( this, "selected file already exist. Overwrite ?" ) )
    return;
  }

  // open stream
  QFile out( fullname.c_str() );
  if( !out.open( QIODevice::WriteOnly ) )
  {
    ostringstream what;
    what << "cannot write to file \"" << fullname << "\" <Convert to Html> canceled.";
    QtUtil::infoDialog( this, what.str() );
    return;
  }

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
    appendChild( activeDisplay().htmlNode( document ) );

  /*
    the following replacements are needed
    to have correct implementation of leading space characters, tabs
    and end of line
  */
  QString html_string( document.toString(0) );
  html_string = html_string.replace( "</span>\n", "</span>" );
  html_string = html_string.replace( "<br/>", "" );
  out.write( html_string.toAscii() );
  out.close();

  // see if file is to be opened
  bool use_command( dialog.useCommand() );
  XmlOptions::get().set<bool>("USE_HTML_EDITOR", use_command );

  // edit file if requested
  if( !use_command ) return;
  string command( dialog.command() );
  string path( fullname.path() );
  
  command += string( " " ) + fullname + "&";
  Util::runAt( path, command );
  
  return;

}

//___________________________________________________________
void EditFrame::_print( void )
{
  Debug::Throw( "EditFrame::_print.\n" );

  // retrieve activeDisplay file
  const File& file( activeDisplay().file() );

  // check if file is modified
  if( activeDisplay().document()->isModified() && activeDisplay().askForSave() == AskForSaveDialog::CANCEL ) return;

  // check if file is valid and exists
  if( file.empty() || !file.exist() )
  {
    QtUtil::infoDialog( this, "File is not valid for printing. <print> canceled." );
    return;
  }

  // create dialog
  PrintDialog dialog( this );
  dialog.setFile( file );

  // exec
  if( dialog.exec() == QDialog::Rejected ) return;

  ostringstream path;
  path << "\"" << file.path() << "\"";
  Util::runAt( path.str(), Str(dialog.command()).append( " &") );

  // update options
  XmlOptions::get().set<bool>( "USE_A2PS", dialog.useA2Ps() );
  XmlOptions::get().setRaw( "A2PS_COMMAND", dialog.a2psCommand() );
  XmlOptions::get().setRaw( "PRINT_COMMAND", dialog.printCommand() );

  return;

}

//____________________________________________
void EditFrame::closeEvent( QCloseEvent* event )
{
  Debug::Throw( "EditFrame::closeEvent.\n" );

  // check for modifications
  event->accept();

  if( Debug::level() > 0 )
  {
    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      
      // this trick allow to run  only once per set of associated displays
      if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) != iter ) continue;
      if( (*iter)->file().empty() ) continue;
      
      Debug::Throw() << "EditFrame::closeEvent - closing: " << (*iter)->file() << endl;
    }
  }
      
  // look over TextDisplays
  BASE::KeySet<TextDisplay> displays( this );
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
      if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) != iter ) continue;
      
      // ask for save
      int state( display.askForSave( modifiedDisplayCount() > 1 ) );      
      if( state == AskForSaveDialog::ALL ) {
        
        // for this frame, only save displays located after the current
        for( BASE::KeySet<TextDisplay>::iterator display_iter = iter; display_iter != displays.end(); display_iter++ )
        { if( (*display_iter)->document()->isModified() ) (*display_iter)->save(); }

      } else if( state == AskForSaveDialog::CANCEL ) {

        event->ignore();
        return;

      }

    }

  }

  // save configuration before closing
  saveConfiguration();
  
  return;
}

//____________________________________________
void EditFrame::enterEvent( QEvent* e )
{

  Debug::Throw( "EditFrame::enterEvent.\n" );
  QMainWindow::enterEvent( e );

  // keep track of displays to be deleted, if any
  BASE::KeySet<TextDisplay> dead_displays;

  // retrieve displays
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {

    // this trick allow to run  only once per set of associated displays
    if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) != iter ) continue;

    // keep local reference of current display
    TextDisplay &display( **iter );
    
    // check file
    if( display.checkFileRemoved() == FileRemovedDialog::CLOSE ) 
    { 
        
      // register displays as dead
      BASE::KeySet<TextDisplay> associated_displays( &display );
      dead_displays.insert( associated_displays.begin(), associated_displays.end() );
      dead_displays.insert( &display );
            
    } else (*iter)->checkFileModified();
    
  }
  
  // update window title
  _updateWindowTitle();

  // delete dead_displays
  if( !dead_displays.empty() )
  {

    Debug::Throw() << "EditFrame::enterEvent - dead displays: " << dead_displays.size() << endl;
    for( BASE::KeySet<TextDisplay>::iterator iter = dead_displays.begin(); iter != dead_displays.end(); iter++ )
    { _closeView( **iter ); }

    // need to close window manually if there is no remaining displays
    if( dead_displays.size() == displays.size() ) close();

  }

  Debug::Throw( "EditFrame::enterEvent - done.\n" );

}

//_______________________________________________________
void EditFrame::_documentClassDialog( void )
{
  
  Debug::Throw( "EditFrame::_documentClassDialog.\n" );
  DocumentClassDialog dialog( this );
  connect( &dialog, SIGNAL( classSelected( std::string ) ), SLOT( selectClassName( std::string ) ) );
  connect( &dialog, SIGNAL( classRemoved( std::string ) ), qApp, SIGNAL( documentClassesChanged() ) );
  dialog.exec();


}

//_______________________________________________________
void EditFrame::_update( unsigned int flags )
{

  Debug::Throw( "EditFrame::_update().\n" );

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

}

//_____________________________________________
void EditFrame::_updateCursorPosition( void )
{
  
  return;
  //return;
  TextPosition position( activeDisplay().textPosition() );
  statusbar_->label(0).setText( Str( "line : " ).append<int>( position.paragraph()+1 ).c_str() , false );
  statusbar_->label(1).setText( Str( "column : " ).append<int>( position.index()+1 ).c_str() , false );

  return;
}

//____________________________________________
void EditFrame::_displayFocusChanged( TextDisplay* display )
{
  Debug::Throw() << "EditFrame::_DisplayFocusChanged - " << display->key() << endl;
  setActiveDisplay( *display );
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
  connect( detach_action_, SIGNAL( triggered() ), SLOT( _detach() ) );

  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN, path_list ), "&Open", this ) );
  open_action_->setShortcut( SHIFT+CTRL+Key_O );
  open_action_->setToolTip( "Open an existing file" );
  connect( open_action_, SIGNAL( triggered() ), SLOT( open() ) );
 
  addAction( close_view_action_ = new QAction( IconEngine::get( ICONS::VIEW_REMOVE, path_list ), "&Close view", this ) );
  close_view_action_->setShortcut( CTRL+Key_W );
  close_view_action_->setToolTip( "Close current view" );
  connect( close_view_action_, SIGNAL( triggered() ), SLOT( _closeView() ) );
 
  addAction( close_window_action_ = new QAction( "&Close view", this ) );
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
 
  addAction( html_action_ = new QAction( IconEngine::get( ICONS::HTML, path_list ), "&Html", this ) );
  html_action_->setToolTip( "convert file to Html" );
  connect( html_action_, SIGNAL( triggered() ), SLOT( _convertToHtml() ) );

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

  addAction( document_class_action_ = new QAction( "&Document classes", this ) );
  connect( document_class_action_, SIGNAL( triggered() ), SLOT( _documentClassDialog() ) ); 

  addAction( file_info_action_ = new QAction( IconEngine::get( ICONS::INFO, path_list ), "&File information", this ) );
  file_info_action_->setToolTip( "Display file informations" );
  connect( file_info_action_, SIGNAL( triggered() ), SLOT( _fileInfo() ) );

  addAction( spellcheck_action_ = new QAction( IconEngine::get( ICONS::SPELLCHECK, path_list ), "&Spell check", this ) );
  #if WITH_ASPELL
  connect( spellcheck_action_, SIGNAL( triggered() ), SLOT( _spellcheck( void ) ) );
  #else 
  spellcheck_action_->setVisible( false );
  #endif

}

//___________________________________________________________
void EditFrame::_updateWindowTitle()
{ 
  Debug::Throw( "EditFrame::_updateWindowTitle.\n" );
  setWindowTitle( WindowTitle( activeDisplay().file() )
    .setReadOnly( activeDisplay().isReadOnly() )
    .setModified( activeDisplay().document()->isModified() ) );
  Debug::Throw( "EditFrame::_updateWindowTitle - done.\n" );
}

//___________________________________________________________
void EditFrame::_newFile( const OpenMode& mode, const Orientation& orientation )
{

  Debug::Throw( "EditFrame::_New.\n" );

  // check open_mode
  if( mode == NEW_WINDOW ) dynamic_cast<MainFrame*>(qApp)->open();
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
    dialog.setDirectory( QDir( activeDisplay().workingDirectory().c_str() ) );
    if( dialog.exec() == QDialog::Rejected ) return;
    
    QStringList files( dialog.selectedFiles() );
    if( files.empty() ) return;
  
    record.setFile( File( qPrintable( files.front() ) ).expand() );
  
  }
  
  // check open_mode
  if( mode == NEW_WINDOW )
  {
    // open via the MainFrame to create a new editor
    dynamic_cast<MainFrame*>(qApp)->open( record );
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
  if( !record.file().exist() )
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
  BASE::KeySet<EditFrame> frames( dynamic_cast<BASE::Key*>(qApp) );
  BASE::KeySet<EditFrame>::iterator iter = find_if( frames.begin(), frames.end(), EditFrame::SameFileFTor( record.file() ) );
  if( iter != frames.end() )
  {

    // select found display in EditFrame
    (*iter)->selectDisplay( record.file() );

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
    BASE::KeySet<TextDisplay> displays( this );
    BASE::KeySet<TextDisplay>::iterator display_iter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
    TextDisplay& display( display_iter == displays.end() ? _splitView( orientation, false ):**display_iter );

    // retrieve active display from previous frame
    TextDisplay& previous_display( (*iter)->activeDisplay() );

    // clone
    display.synchronize( &previous_display );

    // store modification state
    bool modified( previous_display.document()->isModified() );
    
    // set previous display as unmdified
    previous_display.document()->setModified( false );

    // close display, or frame, depending on its number of independent files
    if( (*iter)->independentDisplayCount() == 1 ) (*iter)->close();
    else
    {
      displays = BASE::KeySet<TextDisplay>( &previous_display );
      displays.insert( &previous_display );
      for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
      { (*iter)->_closeView( **display_iter ); }
    }

    // restore modification state and make new display active
    display.document()->setModified( modified );
    setActiveDisplay( display );
    display.setFocus();

  } else {

    // look for an empty display
    BASE::KeySet<TextDisplay> displays( this );
    BASE::KeySet<TextDisplay>::iterator display_iter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
    if( display_iter == displays.end() ) _splitView( orientation, false );

    // open file in this window
    setFile( record.file() );

    // update configuration
    updateConfiguration();
    
  }
 
  return;
}

//___________________________________________________________
void EditFrame::_closeView( TextDisplay& display )
{
  Debug::Throw( "EditFrame::_closeView.\n" );

  // retrieve number of displays
  // if only one display, close the entire window
  BASE::KeySet<TextDisplay> displays( this );
  if( displays.size() < 2 )
  {
    Debug::Throw() << "EditFrame::_closeView - full close." << endl;
    close();
    return;
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
  displays = BASE::KeySet<TextDisplay>( &display );
    
  // delete display
  delete &display;

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
    displays = BASE::KeySet<TextDisplay>( this );

  }

  // update active display
  setActiveDisplay( **displays.rbegin() );

  // update close_view button
  detach_action_->setEnabled( independentDisplayCount() > 1 );
  close_view_action_->setEnabled( BASE::KeySet<TextDisplay>(this).size() > 1 );
  
  // change focus
  activeDisplay().setFocus();
  Debug::Throw( "EditFrame::_closeView - done.\n" );

}

//___________________________________________________________
TextDisplay& EditFrame::_splitView( const Orientation& orientation, const bool& clone )
{
  
  Debug::Throw( "EditFrame::_splitView.\n" );

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
  if( clone ) splitter.insertWidget( splitter.indexOf( &active_display_local)+1, &display  );
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
    Debug::Throw( "EditFrame::_splitView - synchronized (associates).\n" );

    // associate this display to AutoSave threads
    BASE::KeySet<AutoSaveThread> threads( &active_display_local );
    for( BASE::KeySet<AutoSaveThread>::iterator iter = threads.begin(); iter != threads.end(); iter++ )
    { BASE::Key::associate( &display, *iter ); }
 
    // associate new display to active
    BASE::Key::associate( &display, &active_display_local );

  } else {

    // register new AutoSave thread
    dynamic_cast<MainFrame*>(qApp)->autoSave().newThread( &display );

    // enable detach
    detach_action_->setEnabled( true );

  }

  // update close view
  close_view_action_->setEnabled( true );
  Debug::Throw( "EditFrame::_splitView - done.\n" );

  return display;

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
    QWidget *parent = activeDisplay().parentWidget();

    // try catch to splitter
    // do not create a new splitter if the parent has same orientation
    QSplitter *parent_splitter( dynamic_cast<QSplitter*>( parent ) );
    if( parent_splitter && parent_splitter->orientation() == orientation ) splitter = parent_splitter;
    else {
      
      // create a splitter with correct orientation
      splitter = new LocalSplitter( parent );
      splitter->setOrientation( orientation );

      // move splitter to the first place if needed
      if( parent_splitter ) 
      {
        
        Debug::Throw( "EditFrame::_newSplitter - found parent splitter.\n" );
        parent_splitter->insertWidget( parent_splitter->indexOf( &activeDisplay() ), splitter );
        
      } else parent->layout()->addWidget( splitter );
      
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
    Exception::checkPointer( child, DESCRIPTION( "invalid first child" ) );

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
TextDisplay& EditFrame::_newTextDisplay( QWidget* parent )
{
  Debug::Throw( "EditFrame::newTextDisplay.\n" );

  // retrieve existing displays
  BASE::KeySet<TextDisplay> displays( this );

  // create textDisplay
  // disable accelerator because they are handled in the menu
  TextDisplay* display = new TextDisplay( parent );
  display->setMenu( &menu_->openPreviousMenu() );

  // connections
  connect( display, SIGNAL( needUpdate( unsigned int ) ), this, SLOT( _update( unsigned int ) ) );
  connect( display, SIGNAL( cursorPositionChanged() ), this, SLOT( _updateCursorPosition() ) );
  connect( display, SIGNAL( hasFocus( TextDisplay* ) ), this, SLOT( _displayFocusChanged( TextDisplay* ) ) );
  
  connect( display, SIGNAL( undoAvailable( bool ) ), &undoAction(), SLOT( setEnabled( bool ) ) );
  connect( display, SIGNAL( redoAvailable( bool ) ), &redoAction(), SLOT( setEnabled( bool ) ) );
  
  // associate display to this editFrame
  BASE::Key::associate( this, display );
  
  // update current display and focus
  setActiveDisplay( *display );
  display->setFocus();
  Debug::Throw() << "EditFrame::_newTextDisplay - key: " << display->key() << endl;
  Debug::Throw( "EditFrame::newTextDisplay - done.\n" );
  
  return *display;
  
}
