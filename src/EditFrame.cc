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

#include "AskForSaveDialog.h"
#include "AutoSave.h"
#include "ClockLabel.h"
#include "Config.h"
#include "CustomFileDialog.h"
#include "CustomLineEdit.h"
#include "CustomPixmap.h"
#include "CustomToolBar.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "Diff.h"
#include "DocumentClassManager.h"
#include "DocumentClassDialog.h"
#include "EditFrame.h"
#include "Icons.h"
#include "MainFrame.h"
#include "Menu.h"
#include "NewFileDialog.h"
#include "OpenPreviousMenu.h"
#include "XmlOptions.h"
#include "PrintDialog.h"
#include "QtUtil.h"
#include "FileModifiedDialog.h"
#include "FileRemovedDialog.h"
#include "StateFrame.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "Util.h"
#include "ViewHtmlDialog.h"
#include "WindowTitle.h"

#if WITH_ASPELL
#include "SpellDialog.h"
#include "SpellInterface.h"
#endif

using namespace std;

//_____________________________________________________
EditFrame::EditFrame(  QWidget* parent, const string& name ):
  QMainWindow( parent, name.c_str(), WType_TopLevel|WDestructiveClose  ),
  Counter( "EditFrame" ),
  menu_( 0 ),
  active_display_( 0 ),
  statusbar_( 0 ),
  file_editor_( 0 ),
  enable_save_all_( false ),
  enable_check_( true ),
  default_orientation_( Qt::Horizontal ),
  default_open_mode_( NEW_WINDOW )
{
  Debug::Throw( "EditFrame::EditFrame.\n" );

  // menu
  menu_ = new Menu( this );
  connect( menu_, SIGNAL( documentClassSelected( std::string ) ), this, SLOT( selectClassName( std::string ) ) );

  // retrieve pixmap path
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  // main vbox
  QWidget* main( new QWidget( this ) );
  QVBoxLayout* layout( new QVBoxLayout() );
  main->setLayout( layout );
  layout->setMargin(2);
  layout->setMargin(2);
  setCentralWidget( main );

  // TextDisplay container
  main_ = new QWidget( main );
  main_->setLayout( new QVBoxLayout() );
  layout->addWidget( main_, 1 );
  
  // create new Text display and register autosave thread
  TextDisplay& display = _newTextDisplay( main_ );
  main_->layout()->addWidget( &display );
  display.setActive( true );
  static_cast<MainFrame*>(qApp)->autoSave().newThread( &display );

  // state frame
  layout->addWidget( statusbar_ = new StatusBar( main ) );

  // create "hidden" line editor to display filename
  status_bar_->getLayout().addWidget( file_editor_ = new CustomLineEdit( statusbar_ ), 1, QtAlign::VCenter );
  statusbar_->addLabels( 2, 0 );
  statusbar_->label(0).setAlignment( Qt::AlignHCenter | Qt::AlignVCenter | Qt::expandTabs );
  statusbar_->label(1).setAlignment( Qt::AlignHCenter | Qt::AlignVCenter | Qt::expandTabs );
  statusbar_->addClock();

  // modify frame and set readOnly
  file_editor_->setReadOnly( true );
  file_editor_->setFrame( false );
  
  // modify color
  QPalette palette( file_editor_->palette() );
  palette->setColor( QPalette::Base, palette.color( QPalette::Window ) )
  file_editor_->setPalette( palette );

  // assign non fixed font
  QFont font;
  font.fromString( XmlOptions::get().raw( "FONT_NAME" ).c_str() );
  file_editor_->setFont( font );

  // install actions
  _installActions();
  
  // file toolbar
  CustomToolBar* toolbar;
  toolbar = new CustomToolBar( "Main", this );
  toolbars_.push_back( make_pair( toolbar, "FILE_TOOLBAR" ) );
  addToolBar( toolbar );

  QToolButton* button;
  toolbar->addAction( newAction() );
  toolbar->addAction( openAction() ); 
  toolbar->addAction( saveAction() ); 
  //toolbar->addAction( printAction() );
  
  // edition toolbar
  toolbar = new CustomToolBar( "Edition", this );
  toolbars_.push_back( make_pair( toolbar, "EDITION_TOOLBAR" ) );
  addToolBar( toolbar );

  toolbar->addAction( undoAction() ); 
  toolbar->addAction( redoAction() ); 
  toolbar->addAction( cutAction() );
  toolbar->addAction( copyAction() );
  toolbar->addAction( pasteAction() );

  // extra toolbar
  toolbar = new CustomToolBar( "Tools", this );
  toolbars_.push_back( make_pair( toolbar, "EXTRA_TOOLBAR" ) );
  addToolBar( toolbar );

  toolbar->addAction( fileInfoAction() ); 
  
  // splitting toolbar
  toolbar = new CustomToolBar( "Display", this );
  toolbars_.push_back( make_pair( toolbar, "SPLIT_TOOLBAR" ) );
  addToolBar( toolbar );
  
  QAction* action;
  action = toolbar->addAction( IconEngine::get( ICONS::VIEW_TOPBOTTOM, path_list ), "Clone view top/bottom", this, SLOT( _splitViewVertical() );
  action->setToolTip( "Clone current view vertically" );

  action = toolbar->addAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT, path_list ), "Clone view left/right", this, SLOT( _splitViewHorizontal() );
  action->setToolTip( "Clone current view horizontally" );

  action = toolbar->addAction( IconEngine::get( ICONS::VIEW_BOTTOM, path_list ), "Clone view top/bottom", this, SLOT( openVertical() );
  action->setToolTip( "Open a new view vertically" );

  action = toolbar->addAction( IconEngine::get( ICONS::VIEW_RIGHT, path_list ), "Open view left/right", this, SLOT( openHorizontal() );
  action->setToolTip( "Open a new view horizontally" );
  
  toolbar->addAction( detachAction() );
  toolbar->addAction( closeViewAction() );
 
  //! configuration
  connect( qApp, SIGNAL( configurationChanged() ), this, SLOT( updateConfiguration() );
  connect( qApp, SIGNAL( aboutToQuit() ), this, SLOT( saveConfiguration() );
 
}

//___________________________________________________________
EditFrame::~EditFrame( void )
{ Debug::Throw( "EditFrame::~EditFrame.\n" ); }

//____________________________________________
void EditFrame::setFile( File file , const bool& reset_document_class )
{
  Debug::Throw() << "EditFrame::setFile - " << file << endl;

  // add file to Menu
  menu_->openPreviousMenu().add( file );

  // look for first empty view
  BASE::KeySet<TextDisplay> displays( this );
  BASE::KeySet<TextDisplay>::iterator iter = find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() );
  Exception::check( iter != displays.end(), DESCRIPTION( "no empty display" ) );
  TextDisplay &display( **iter );

  // retrieve document class name from menu, if any
  if( reset_document_class )
  {
    string class_name( menu_->openPreviousMenu().get( file ).information("class_name") );
    display.setClassName( class_name );
  }

  // open file in active display
  display.openFile( file );

  // store class name from activeDisplay in Menu
  menu_->openPreviousMenu().get( file ).addInformation( "class_name", display.className() );

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
  for( list< pair<QToolBar*, string> >::iterator iter = toolbars_.begin(); iter != toolbars_.end(); iter++ )
  {
    
    QToolBar* toolbar( iter->first );
    string option_name( iter->second );
    string location_name( option_name + "_LOCATION" );
    
    bool visibility( XmlOptions::get().find( option_name ) ? XmlOptions::get().get<bool>( option_name ):true );
    bool current_visibility( toolbar->isVisible() );
    
    ToolBarArea location( (XmlOptions::get().find( location_name )) ? (ToolBarArea) CustomToolBar::nameToArea( XmlOptions::get().get<string>( location_name ) ):LeftToolBarArea );
    ToolBarArea current_location( toolBarArea( toolbar ) );
    
    Debug::Throw() << "EditFrame::updateConfiguration - " << option_name << " visibility: " << visibility << " location: " << (int)location << endl;
    
    if( visibility )
    {
      if( !( current_visibility && location == current_location ) ) 
      {
        addToolBar( location, toolbar );
        toolbar->show();
      }
    } else toolbar->hide();
   
    XmlOptions::get().set<bool>( option_name, !toolbar->isHidden() );
    XmlOptions::get().set<string>( location_name, CustomToolBar::areaToName( toolBarArea( toolbar ) ) );
  }  
  
  // associated displays
  BASE::KeySet<TextDisplay> displays;
  if( active_display_only )
  {
    displays = BASE::KeySet<TextDisplay>( &activeDisplay() );
    displays.insert( &activeDisplay() );
  } else displays = BASE::KeySet<TextDisplay>( this );

  // update flags for all displays
  updateFlags( active_display_only );

  // update document classes
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {

    // this trick allow to run  only once per set of associated displays
    if( std::find_if( displays.begin(), iter, BASE::Key::isAssociatedFTor( *iter ) ) == iter )
    { 
      (*iter)->rehighlight(); 
      if( !(*iter)->file().empty() )
      { menu_->openPreviousMenu().get( (*iter)->file() ).addInformation( "class_name", (*iter)->className() ); }
    }  

  }

}

//________________________________________________________
void EditFrame::saveConfiguration( void )
{
  Debug::Throw( "EditFrame::saveConfiguration.\n" );

  // save size
  XmlOptions::get().set<int>( "WINDOW_HEIGHT", height() );
  XmlOptions::get().set<int>( "WINDOW_WIDTH", width() );
   
  // save toolbars location and visibility
  for( list< pair<QToolBar*, string> >::iterator iter = toolbars_.begin(); iter != toolbars_.end(); iter++ )
  {
    
    QToolBar* toolbar( iter->first );
    string option_name( iter->second );
    string location_name( option_name + "_LOCATION" );
    XmlOptions::get().set<bool>( option_name, !toolbar->isHidden() );
    XmlOptions::get().set<string>( location_name, CustomToolBar::areaToName( toolBarArea( toolbar ) ) );
  }

}

//________________________________________________________
bool EditFrame::updateFlags( const bool& active_display_only )
{
  Debug::Throw( "EditFrame::updateFlags.\n" );

  // auto spell dictionary and filtering
  BASE::KeySet<TextDisplay> displays;
  if( active_display_only ) {
    displays = BASE::KeySet<TextDisplay>( &activeDisplay() );
    displays.insert( &activeDisplay() );
  } else displays = BASE::KeySet<TextDisplay>( this );

  bool changed( false );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {

    // check file
    if( (*iter)->file().empty() ) continue;

    // update display flags
    changed |= (*iter)->updateFlags();

  }

  return changed;

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

  // add information to Menu
  if( !activeDisplay().file().empty() )
  { menu_->openPreviousMenu().get( activeDisplay().file() ).addInformation( "class_name", name ); }

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
    if( std::find_if( displays.begin(), iter, BASE::Key::isAssociatedFTor( *iter ) ) == iter )
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
  EditFrame& frame( static_cast<MainFrame*>(qApp)->newEditFrame() );

  // clone its display from the current
  frame.activeDisplay().synchronize( &active_display_local );

  // delete active display local
  _closeView( active_display_local );

  // show the new frame
  frame.show();
  frame.updateConfiguration();

  return;
  
}

//____________________________________________
void EditFrame::_save( TextDisplay* display )
{
  Debug::Throw( "EditFrame::_save.\n" );

  // if no display passed in argument, use active display
  if( !display ) display = &activeDisplay();

  // save display
  display->save();

  // add file to menu
  menu_->openPreviousMenu().add( display->file() );

  return;

}


//____________________________________________
void EditFrame::_saveAs( TextDisplay* display )
{
  Debug::Throw( "EditFrame::_saveAs.\n" );

  // if no display passed in argument, use active display
  if( !display ) display = &activeDisplay();

  display->saveAs();
  menu_->openPreviousMenu().add( display->file() );
  menu_->openPreviousMenu().get( display->file() ).addInformation( "class_name", display->className() );

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

  activeDisplay()._revertToSave();
  
}

//____________________________________________
void EditFrame::_saveAll( void )
{
  Debug::Throw( "EditFrame::_saveAll.\n" );

  // retrieve all displays
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    if( !(*iter)->document()->isModified() ) continue;

    // save display
    (*iter)->save();

    // add file to menu
    menu_->openPreviousMenu().add( (*iter)->file() );
  }

  return;

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
  string command( dialog.Command() );
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
  if( activeDisplay().document()->isModified() )
  {
    AskForSaveDialog dialog( this, file );
    int state = dialog.exec();
    if( state == AskForSaveDialog::YES ) save();
    if( state == AskForSaveDialog::CANCEL ) return;
  }

  // check if file is valid and exists
  if( file.empty() || !file.exist() )
  {
    QtUtil::infoDialog( this, "File is not valid for printing. <print> canceled." );
    return;
  }

  // create dialog
  PrintDialog dialog( this );
  dialog.setCaption( "print" );
  dialog.setFile( file );

  // exec
  if( dialog.exec() == QDialog::Rejected ) return;

  ostringstream path;
  path << "\"" << file.path() << "\"";
  Util::RunAt( path.str(), dialog.Command() );

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

  // look over TextDisplays
  BASE::KeySet<TextDisplay> displays( this );
  if( isModified() )
  {

    // update enable_save_all_ state, depending on how many files are modified in this window
    enable_save_all_ |= modifiedDisplayCount() > 1;

    // retrieve all edit frames
    BASE::KeySet<EditFrame> frames( static_cast<MainFrame*>(qApp) );

    // look over TextDisplays
    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {

      // this trick allow to run  only once per set of associated displays
      if( std::find_if( displays.begin(), iter, BASE::Key::isAssociatedFTor( *iter ) ) != iter ) continue;

      // get local reference to display
      TextDisplay& display( **iter );
      
      // check if this display is modified
      if( !display.document()->isModified() ) continue;

      // create dialog
      int state( AskForSaveDialog( this, display.file(), enable_save_all_ ).exec() );
      if( state == AskForSaveDialog::YES ) save( &display );
      else if( state == AskForSaveDialog::NO ) 
      {
        
        display.document()->setModified( false );
      
      } else if( state == AskForSaveDialog::ALL ) {

        // loop over edit frames, only consider frames *after* this one
        bool found_this( false );
        for( BASE::KeySet<EditFrame>::iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
        {
          
          if( *frame_iter == this ) 
          {
            
            // for this frame, only save displays located after the current
            for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
            {
              if( *display_iter == &display ) found_this = true;
              else if( found_this && (*iter)->document()->isModified() ) save( *display_iter );
            }
            
          } else if( found_this && (*frame_iter)->isModified() ) (*frame_iter)->saveAll();
          
        }

      } else if( state == AskForSaveDialog::CANCEL ) {

        enable_save_all_ = false;
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
    if( std::find_if( displays.begin(), iter, BASE::Key::isAssociatedFTor( *iter ) ) != iter ) continue;

    // keep local reference of current display
    TextDisplay &display( **iter );
    
    // check file
    if( !display.ignoreWarnings() && display.fileRemoved() )
    {

      // disable check
      int state( FileRemovedDialog( this, display.file() ) );

      if( state == FileRemovedDialog::RESAVE ) { _save( &display ); }
      else if( state == FileRemovedDialog::SAVE_AS ) { _saveAs( &display ); }
      else if( state == FileRemovedDialog::CLOSE ) { 
                
        // register displays as dead
        dead_displays.insert( associated_displays.begin(), associated_displays.end() );
        dead_displays.insert( &display );
        
      } else if( state == FileRemovedDialog::IGNORE ) {
        
        BASE::KeySet<TextDisplay> associated_displays( &display );
        associated_displays.insert( &display );
        for( BASE::KeySet<TextDisplay>::iterator associated_iter = displays.begin(); associated_iter != displays.end(); associated_iter++ )
        { (*associated_iter)->setIgnoreWarnings( true ); }
        
    } else if( !display.ignoreWarnings() && (*iter)->FileModified() )
    {

      int state = FileModifiedDialog( this, display.file() ).exec();
      if( state == FileModifiedDialog::RESAVE ) { _save( &display ); }
      else if( state == FileModifiedDialog::SAVE_AS ) { _saveAs( &display ); }
      else if( state == FileModifiedDialog::RELOAD ) { 
        
        display.document()->setModified( false ); 
        display.revertToSave(); 
        
      } else if( state == FileModifiedDialog::IGNORE ) { 

        BASE::KeySet<TextDisplay> associated_displays( &display );
        associated_displays.insert( &display );
        for( BASE::KeySet<TextDisplay>::iterator associated_iter = displays.begin(); associated_iter != displays.end(); associated_iter++ )
        { (*associated_iter)->setIgnoreWarnings( true ); }
    
      }

    }

  }

  // update window title
  _updateWindowTitle();

  // delete dead_displays
  if( !dead_displays.empty() )
  {

    Debug::Throw() << "EditFrame::enterEvent - dead displays: " << dead_displays.size() << endl;
    for( BASE::KeySet<TextDisplay>::iterator iter = dead_displays.begin(); iter != dead_displays.end(); iter++ )
    { 
      (*iter)->setIgnoreWarnings( true );
      (*iter)->document()->setModified( false );
      _closeView( **iter ); 
    }

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
  dialog.exec();


}

//_______________________________________________________
void EditFrame::_update( unsigned int flags )
{

  Debug::Throw( "EditFrame::_update().\n" );

  if( flags & TextDisplay::WINDOW_TITLE )
  { _updateWindowTitle(); }

  if( flags & TextDisplay::FILE_NAME && file_editor_ )
  { file_editor_->setText( activeDisplay().file().c_str() ); }

  if( flags & TextDisplay::CUT && cut_ )
  { cutAction()->setEnabled( activeDisplay().cutAction()->isEnabled() ); }

  if( flags & TextDisplay::COPY && copy_ )
  { copyAction()->setEnabled( activeDisplay().copyAction()->isEnabled() ); }

  if( flags & TextDisplay::PASTE && paste_ )
  { pasteAction()->setEnabled( activeDisplay().pasteAction()->isEnabled() ); }

  if( flags & TextDisplay::UNDO_REDO )
  {
    undoAction()->setEnabled( activeDisplay().undoAction()->isEnabled() );
    redoAction()->setEnabled( activeDisplay().redoAction()->isEnabled() );
  }

}

//_____________________________________________
void EditFrame::_updateCursorPosition( void )
{

  TextPosition position( activeDisplay().textPosition() );
  statusBar().label(0).setText( Str( "line : " ).append<int>( position.paragraph()+1 ).c_str() , false );
  statusBar().label(1).setText( Str( "column : " ).append<int>( position.index()+1 ).c_str() , false );

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

  addAction( new_file_action_ = new QAction( IconEngine::get( ICON::FILE_NEW, path_list ), "&New", this ) );
  new_file_action_->setShortCut( CTRL+Key_N );
  new_file_action_->setTooltip( "Create a new empty file" );
  connect( new_file_action_, SIGNAL( triggered() ) SLOT( _newFile() ) );

  addAction( clone_action_ = new QAction( IconEngine::get( ICON::VIEW_LEFTRIGHT, path_list ), "&Clone", this ) );
  clone_action_->setShortCut( SHIFT+CTRL+Key_N );
  clone_action_->setTooltip( "Clone current view" );
  connect( clone_action_, SIGNAL( triggered() ) SLOT( _splitView() ) );

  addAction( detach_action_ = new QAction( IconEngine::get( ICON::DETACH, path_list ), "&Detach", this ) );
  detach_action_->setShortCut( SHIFT+CTRL+Key_O );
  detach_action_->setTooltip( "Detach current view" );
  connect( detach_action_, SIGNAL( triggered() ) SLOT( _detach() ) );

  addAction( open_action_ = new QAction( IconEngine::get( ICON::OPEN, path_list ), "&Open", this ) );
  open_action_->setShortCut( SHIFT+CTRL+Key_O );
  open_action_->setTooltip( "Open an existing file" );
  connect( open_action_, SIGNAL( triggered() ) SLOT( _open() ) );
 
  addAction( close_view_action_ = new QAction( IconEngine::get( ICON::VIEW_REMOVE, path_list ), "&Close view", this ) );
  close_view_action_->setShortCut( CTRL+Key_W );
  close_view_action_->setTooltip( "Close current view" );
  connect( close_view_action_, SIGNAL( triggered() ) SLOT( _closeView() ) );
 
  addAction( close_window_action_ = new QAction( "&Close view", this ) );
  close_window_action_->setShortCut( SHIFT+CTRL+Key_W );
  close_window_action_->setTooltip( "Close current view" );
  connect( close_window_action_, SIGNAL( triggered() ) SLOT( _closeWindow() ) );
 
  addAction( save_action_ = new QAction( IconEngine::get( ICON::SAVE, path_list ), "&Save", this ) );
  save_action_->setShortCut( CTRL+Key_S );
  save_action_->setTooltip( "Save current file" );
  connect( save_action_, SIGNAL( triggered() ) SLOT( _save() ) );
 
  addAction( save_as_action_ = new QAction( IconEngine::get( ICON::SAVE_AS, path_list ), "Save &As", this ) );
  save_as_action_->setShortCut( SHIFT+CTRL+Key_S );
  save_as_action_->setTooltip( "Save current file with a different name" );
  connect( save_as_action_, SIGNAL( triggered() ) SLOT( _saveAs() ) );

  addAction( revert_to_save_action_ = new QAction( IconEngine::get( ICON::RELOAD, path_list ), "&Revert to saved", this ) );
  revert_to_save_action_->setTooltip( "Reload saved version of current file" );
  connect( revert_to_save_action_, SIGNAL( triggered() ) SLOT( _revertToSave() ) );
 
  addAction( html_action_ = new QAction( IconEngine::get( ICON::HTML, path_list ), "&Html", this ) );
  html_action_->setTooltip( "convert file to Html" );
  connect( html_action_, SIGNAL( triggered() ) SLOT( _convertToHtml() ) );

  addAction( print_action_ = new QAction( IconEngine::get( ICON::PRINT, path_list ), "&Print", this ) );
  paste_action_->setTooltip( "Paste clipboard to text" );
  connect( print_action_, SIGNAL( triggered() ) SLOT( _print() ) );

  addAction( undo_action_ = new QAction( IconEngine::get( ICON::UNDO, path_list ), "&Undo", this ) );
  undo_action_->setTooltip( "Undo last action" );
  connect( undo_action_, SIGNAL( triggered() ) SLOT( _undo() ) );

  addAction( redo_action_ = new QAction( IconEngine::get( ICON::REDO, path_list ), "&Redo", this ) );
  redo_action_->setTooltip( "Redo last un-done action" );
  connect( redo_action_, SIGNAL( triggered() ) SLOT( _redo() ) );

  addAction( cut_action_ = new QAction( IconEngine::get( ICON::CUT, path_list ), "&Cut", this ) );
  cut_action_->setTooltip( "Cut current selection and copy to clipboard" );
  connect( cut_action_, SIGNAL( triggered() ) SLOT( _cut() ) );

  addAction( copy_action_ = new QAction( IconEngine::get( ICON::COPY, path_list ), "&Copy", this ) );
  copy_action_->setTooltip( "Copy current selection to clipboard" );
  connect( copy_action_, SIGNAL( triggered() ) SLOT( _copy() ) );

  addAction( paste_action_ = new QAction( IconEngine::get( ICON::PASTE, path_list ), "&Paste", this ) );
  paste_action_->setTooltip( "Paste clipboard to text" );
  connect( paste_action_, SIGNAL( triggered() ) SLOT( _paste() ) );

  addAction( document_class_action_ = new QAction( "&Document classes", this ) );
  connect( document_class_action_, SIGNAL( triggered() ), SLOT( _documentClassConfiguration() ) ); 

  addAction( file_info_action_ = new QAction( IconEngine::get( ICON::INFO, path_list ), "&File information", this ) );
  file_info_action_->setTooltip( "Display file informations" );
  connect( file_info_action_, SIGNAL( triggered() ) SLOT( _fileInfo() ) );

}

//___________________________________________________________
void EditFrame::_updateWindowTitle()
{ 
  Debug::Throw( "EditFrame::_updateWindowTitle.\n" );
  setWindowTitle( WindowTitle( activeDisplay().file() )
    .setReadOnly( activeDisplay().isReadOnly() )
    .setModified( activeDisplay().document()->isModified() )
    .c_str() );
}

//___________________________________________________________
void EditFrame::_newFile( const OpenMode& mode, const Qt::Orientation& orientation )
{

  Debug::Throw( "EditFrame::_New.\n" );

  // check open_mode
  if( mode == NEW_WINDOW ) static_cast<MainFrame*>(qApp)->open( FileRecord("") );
  else _splitView( orientation, false );

}

//___________________________________________________________
void EditFrame::_open( FileRecord record, const OpenMode& mode, const Qt::Orientation& orientation )
{

  Debug::Throw( "EditFrame::_Open.\n" );

  // copy to local
  if( record.file().empty() )
  {

    // create file dialog
    CustomFileDialog dialog( this, "file dialog", TRUE );
    dialog.setMode( QFileDialog::existingFile );
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
  if( !record.file().exist() )
  {
    
    // create NewFileDialog
    
    int state( NewFileDialog( this, record.file() ).exec();
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
      << "The file " << file << " is already opened in another window.\n"
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
    display.synchronize( previous_display );

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
    close();
    return;
  }

  // check if display is modified and has no associates in window
  if( display.document()->siModified() && BASE::KeySet<TextDisplay>( &display ).empty() )
  {

    // create dialog
    AskForSaveDialog dialog( this, display.file(), false );
    int state = dialog.exec();
    if( state == AskForSaveDialog::YES ) { save( &display ); }
    if( state == AskForSaveDialog::NO ) { display.document()->setModified( false ) };
    else if( state == AskForSaveDialog::CANCEL ) return;

  }

  // retrieve parent and grandparent of current display
  QWidget* parent( display.parentWidget() );
  Exception::checkPointer( parent, DESCRIPTION( "invalid parent" ) );
  
  QWidget* grand_parent( parent->parentWidget() );
  Exception::checkPointer( grand_parent, DESCRIPTION( "invalid parent" ) );  
  Debug::Throw("EditFrame::_closeView - got parent and grand parent.\n" );

  // retrieve children of parent and loop
  QObjectList children( *parent->children() );
  for( QObject* child = children.first(); child; child = children.next() )
  {

    // check if child is the display
    if( child == &display )
    {
      Debug::Throw("EditFrame::_closeView - got current display.\n" );
      continue;
    }

    /*
      check if child is a QSplitter
      The parsing must stop once an object is reparented
      otherwise the loop is broken and the code crashes.
      this makes no harm because the parent splitter cannot
      contain more than one display or splitter on top of the
      current display
    */
    QSplitter *child_splitter( dynamic_cast<QSplitter*>(child) );
    if( child_splitter )
    {
      child_splitter->setParent( grand_parent );
      break;
    }

    // check if child is a text display
    TextDisplay *child_display( dynamic_cast<TextDisplay*>(child) );
    if( child_display )
    {
      child_display->setParent( grand_parent );
      break;
    }

  }

  // retrieve displays associated to current
  displays = BASE::KeySet<TextDisplay>( &display );
  display.close();
  parent->close();

  // try resize grand parent (if splitter)
  QSplitter* splitter( static_cast<QSplitter*>( grand_parent ) );
  if( splitter )
  {
    
    int dimension = (splitter->orientation() == Qt::Horizontal) ? splitter->width():splitter->height();
    
    QList<int> sizes;
    sizes.push_back( dimension/2 );
    sizes.push_back( dimension/2 );
    splitter->setSizes( sizes );
  }

  // if no associated displays, retrieve all, set the first as active
  if( displays.empty() )
  {

    Debug::Throw( "EditFrame::_closeView - no associated display.\n" );
    displays = BASE::KeySet<TextDisplay>( this );

    /*
      At this point the TextDisplay widget has not been deleted yet.
      One must make sure it is not picked as the next active display
      Since there is at least 2 displays in the window, one can safely
      increment the reverse iterator without check.
    */
    BASE::KeySet<TextDisplay>::reverse_iterator iter = displays.rbegin();
    if( *iter == &display ) iter++;

    Exception::Assert( iter != displays.rend(), DESCRIPTION( "wrong TextDisplay iterator" ) );
    setActiveDisplay( **iter );

    // update detach view
    detach_action_->setEnabled( independentDisplayCount() > 2 );

  } else {

    Debug::Throw( "EditFrame::_closeView - using associated display.\n" );
    setActiveDisplay( **displays.rbegin() );

    // update detach view
    detach_view_->setEnabled( independentDisplayCount() > 1 );

  }

  // update close_view button
  /*
    note that the closed text display still exists at this point.
    The button is disabled if less than 2 views are found
  */
  close_view_action_->setEnabled( BASE::KeySet<TextDisplay>(this).size() > 2 );

  // change focus
  activeDisplay().setFocus();

}

//___________________________________________________________
TextDisplay& EditFrame::_splitView( const Qt::Orientation& orientation, const bool& clone )
{
  
  Debug::Throw( "EditFrame::_splitView.\n" );

  // keep local pointer to current active display
  TextDisplay& active_display_local( activeDisplay() );

  // create new splitter
  QSplitter& splitter( _newSplitter( orientation, clone ) );

  // create new display
  TextDisplay& display( _newTextDisplay( &splitter ) );

  // assign equal size to displays
  int dimension = (orientation == Qt::Horizontal) ? splitter.width():splitter.height();
    
  QList<int> sizes;
  sizes.push_back( dimension/2 );
  sizes.push_back( dimension/2 );
  splitter.setSizes( sizes );
    
  // also resize parent
  QSplitter *parent_splitter( static_cast<QSplitter*>( splitter.parent() ) );
  if( parent_splitter ) 
  {
    
    // assign equal size to displays
    int dimension = ( parent_splitter->orientation() == Qt::Horizontal) ? splitter.width():splitter.height();
    
    QList<int> sizes;
    sizes.push_back( dimension/2 );
    sizes.push_back( dimension/2 );
    splitter.setSizes( sizes );
    
    parent_splitter->setSizes( sizes );
  
  }
  
  // show splitter
  splitter.show();

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
    static_cast<MainFrame*>(qApp)->autoSave().newThread( &display );

    // enable detach
    detach_action_->setEnabled( true );

  }

  // update close view
  close_view_action_->setEnabled( true );

  return display;

}

//____________________________________________________________
QSplitter& EditFrame::_newSplitter( const Qt::Orientation& orientation, const bool& clone )
{

  Debug::Throw( "EditFrame::_newSplitter.\n" );
  QSplitter *splitter = 0;
  
  if( clone )
  {
    
    /* in clone mode, a new splitter is added as a parent of the activeDisplay.
    the new (cloned) TextDisplay will appear side by side with it */

    // retrieve parent of current display
    QWidget *parent = activeDisplay().parentWidget();

    // create a splitter with correct orientation
    splitter = new DestructiveCloseSplitter( parent );
    splitter->setOrientation( orientation );

    // move splitter to the first place if needed
    QSplitter *parent_splitter = dynamic_cast<QSplitter*>( parent );
    if( parent_splitter ) parent_splitter->insertWidget( splitter, parent_splitter->indexOf( &activeWidget() );
    else parent->layout()->addWidget( splitter );

    // reparent current display
    activeDisplay().setParent( splitter );

  } else {

    /*
      in no clone mode, a new splitter is created at the top level
      the new (cloned) TextDisplay will appear side by side with all other displays
    */

    // keep track of first (either TextDisplay or QSplitter) from main_
    QWidget *first_child(0);

    // retrieve children and loop
    QObjectList children( *main_->children() );
    for( QObject* child = children.first(); child; child = children.next() )
    {
      if(
        dynamic_cast<TextDisplay*>( child ) ||
        dynamic_cast<QSplitter*>( child ) )
      {
        first_child = dynamic_cast<QWidget*>( child );
        break;
      }
    }

    // check child could be retrieved
    Exception::CheckPointer( first_child, DESCRIPTION( "invalid first child" ) );

    // create new splitter
    splitter = new DestructiveCloseSplitter( main_ );
    splitter->setOrientation( orientation );
    main_->layout()->addWidget( splitter );

    // reparent first child
    first_child->setParent( splitter );

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
  display->EnableAccelerator( false );

  // connections
  connect( display, SIGNAL( needUpdate( unsigned int ) ), this, SLOT( _update( unsigned int ) ) );
  connect( display, SIGNAL( cursorPositionChanged() ), this, SLOT( _updateCursorPosition() ) );
  connect( display, SIGNAL( hasFocus( TextDisplay* ) ), this, SLOT( _displayFocusChanged( TextDisplay* ) ) );
  
  // associate display to this editFrame
  BASE::Key::associate( this, display );

  // update current display and focus
  setActiveDisplay( *display );
  display->setFocus();
  Debug::Throw() << "EditFrame::_newTextDisplay - key: " << display->key() << endl;

  return *display;

}
