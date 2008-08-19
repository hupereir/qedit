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
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 *******************************************************************************/


/*!
  \file WindowServer.cpp
  \brief handles opened edition windows
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QAction>

#include "Application.h"
#include "CustomFileDialog.h"
#include "Debug.h"
#include "FileList.h"
#include "Icons.h"
#include "IconEngine.h"
#include "MainWindow.h"
#include "Menu.h"
#include "NavigationFrame.h"
#include "NewFileDialog.h"
#include "QtUtil.h"
#include "RecentFilesMenu.h"
#include "SaveAllDialog.h"
#include "Util.h"
#include "WindowServer.h"

using namespace std;



//________________________________________________________________
WindowServer::WindowServer( QObject* parent ):
  QObject( parent ),
  Counter( "WindowServer" ),
  active_window_( 0 )
{ 
  Debug::Throw( "WindowServer::WindowServer.\n" ); 
  
  // create actions
  save_all_action_ = new QAction( IconEngine::get( ICONS::SAVE_ALL ), "Save A&ll", this );
  connect( save_all_action_, SIGNAL( triggered() ), SLOT( _saveAll() ) );
    
}

//________________________________________________________________
WindowServer::~WindowServer( void )
{ Debug::Throw( "WindowServer::~WindowServer.\n" ); }

//_____________________________________
MainWindow& WindowServer::newMainWindow( void )
{  
  Debug::Throw( "WindowServer::newMainWindow.\n" );
  MainWindow* window = new MainWindow();

  BASE::Key::associate( this, window );
  _setActiveWindow( *window );
  
  connect( window, SIGNAL( destroyed() ), SIGNAL( sessionFilesChanged() ) );
  connect( window, SIGNAL( modificationChanged() ), SIGNAL( sessionFilesChanged() ) );
  connect( window, SIGNAL( modificationChanged() ), SLOT( _updateActions() ) );
  connect( window, SIGNAL( activated( MainWindow* ) ), SLOT( _activeWindowChanged( MainWindow* ) ) );
  
  connect( this, SIGNAL( sessionFilesChanged() ), &window->navigationFrame().updateSessionFilesAction(), SLOT( trigger() ) );
  connect( &static_cast<Application*>(qApp)->recentFiles(), SIGNAL( contentsChanged() ), &window->navigationFrame().updateRecentFilesAction(), SLOT( trigger() ) );
  connect( &static_cast<Application*>(qApp)->recentFiles(), SIGNAL( validFilesChecked() ), &window->navigationFrame().updateRecentFilesAction(), SLOT( trigger() ) );

  connect( &window->newFileAction(), SIGNAL( triggered() ), SLOT( _newFile() ) );
  connect( &window->openAction(), SIGNAL( triggered() ), SLOT( _open() ) );
  connect( &window->openHorizontalAction(), SIGNAL( triggered() ), SLOT( _openHorizontal() ) );
  connect( &window->openVerticalAction(), SIGNAL( triggered() ), SLOT( _openVertical() ) );
  connect( &window->detachAction(), SIGNAL( triggered() ), SLOT( _detach() ) );

  connect( &window->menu().recentFilesMenu(), SIGNAL( fileSelected( FileRecord ) ), SLOT( _open( FileRecord ) ) );
  connect( &window->navigationFrame(), SIGNAL( fileActivated( FileRecord ) ), SLOT( _open( FileRecord ) ) );

  return *window;
}

//______________________________________________________
WindowServer::FileRecordMap WindowServer::files( bool modified_only ) const
{ 
  
  Debug::Throw( "WindowServer::files.\n" );
  
  // output
  FileRecordMap files;
  
  // get associated main windows
  BASE::KeySet<MainWindow> windows( this );
  for( BASE::KeySet<MainWindow>::iterator window_iter = windows.begin(); window_iter != windows.end(); window_iter++ )
  {
      
    // retrieve associated TextDisplays
    BASE::KeySet<TextDisplay> displays( (*window_iter)->associatedDisplays() );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {

      // check modification status
      if( modified_only && !(*iter)->document()->isModified() ) continue;
        
      // retrieve file
      // store in map if not empty
      const File& file( (*iter)->file() );
      if( file.empty() ) continue;
        
      // insert in map (together with modification status
      files.insert( make_pair( static_cast<Application*>(qApp)->recentFiles().get(file), (*iter)->document()->isModified() ) );
        
    }
    
  }
  
  return files;
  
}

//______________________________________________________
bool WindowServer::closeAllWindows( void )
{ 
  
  Debug::Throw( "WindowServer::closeAllWindows.\n" );
  
  int state( AskForSaveDialog::UNKNOWN );
  
  // try close all windows one by one
  BASE::KeySet<MainWindow> windows( this );
  for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); iter++ )
  {
    
    // if window is not modified, close
    //Debug::Throw(0) << "WindowServer::closeAllWindows - window: " << (*iter)->key() << endl;
    if( !(*iter)->isModified() ) 
    {
      assert( (*iter)->close() );
      continue;
    }

    // save everything if YES_TO_ALL is selected
    if( state == AskForSaveDialog::YES_TO_ALL ) 
    {
      (*iter)->saveAll();
      assert( (*iter)->close() );
      continue;
    }

    // ignore everuthing if NO_TO_ALL was selected
    if( state == AskForSaveDialog::NO_TO_ALL ) 
    {
      (*iter)->ignoreAll();
      assert( (*iter)->close() );
      continue;
    }
    
    // retrieve all text views associated to this display
    unsigned int modified_displays(0);
    BASE::KeySet<TextDisplay> displays;
    BASE::KeySet<TextView> views( *iter );
    for( BASE::KeySet<TextView>::iterator view_iter = views.begin(); view_iter != views.end(); view_iter++ )
    {
      
      // update the number of modified displays
      modified_displays += (*view_iter)->modifiedDisplayCount();
      
      // store associated textDisplays in main set
      BASE::KeySet<TextDisplay> view_displays( *view_iter );
      displays.insert( view_displays.begin(), view_displays.end() );
      
    }
    
    // loop over displays
    bool save_all_enabled = count_if( iter, windows.end(), MainWindow::IsModifiedFTor() ) > 1;
    
    save_all_enabled |= ( modified_displays > 1 );
    for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
    {
      
      //Debug::Throw(0) << "WindowServer::closeAllWindows - display: " << (*display_iter)->key() << " file: " << (*display_iter)->file() << endl;
      if( !(*display_iter)->document()->isModified() ) continue;

      state = (*display_iter)->askForSave( save_all_enabled );
      //Debug::Throw(0) << "WindowServer::closeAllWindows - state: " << state << endl;
      if( state == AskForSaveDialog::YES_TO_ALL ) (*iter)->saveAll();
      else if( state == AskForSaveDialog::NO_TO_ALL ) (*iter)->ignoreAll();      
      else if( state == AskForSaveDialog::CANCEL ) return false;

    }
    
    // try close. Should succeed, otherwise it means there is a flow in the algorithm above
    assert( (*iter)->close() );
    
  }

  return true;
  
}

//______________________________________________________
void WindowServer::readFilesFromArguments( ArgList args )
{
      
  // retrieve files from arguments
  ArgList::Arg last_arg( args.get().back() );
  
  // load files
  list<string> files( last_arg.options() );
  
  // close mode
  if( args.find( "--close" ) ) 
  {
    _closeFiles( last_arg.options() );
    return;
  }
  
  // check number of files
  if( files.size() > 10 )
  {
    ostringstream what;
    what << "Do you really want to open " << files.size() << " files at the same time ?" << endl;
    what << "This might be very resource intensive and can overload your computer." << endl;
    what << "If you choose No, only the first file will be opened.";
    if( !QtUtil::questionDialog( &_activeWindow(), what.str() ) )
    {
      list<string> tmp;
      tmp.push_back( files.front() );
      files = tmp;
    }
  }
  
  // tabbed | diff mode
  bool tabbed( args.find( "--tabbed" ) );
  bool diff( args.find( "--diff" ) );
  if( ( tabbed || diff ) && files.size() > 1 )
  {

    bool first( true );
    for( list< string >::const_iterator iter = files.begin(); iter != files.end(); iter++ )
    {
    
      if( first )
      {
      
        if( _open( File( *iter ).expand() ) ) first = false;
        
      } else { _open( File( *iter ).expand(), _activeWindow().orientation() ); }
    
    }
    
    if( diff )
    { 
      if( !first && _activeWindow().activeView().independentDisplayCount() == 2 ) _activeWindow().diffAction().trigger();
      else QtUtil::infoDialog( &_activeWindow(), "invalid number of files selected. <Diff> canceled." );
    }
    
  } else {
  
    // default mode
    for( list< string >::const_iterator iter = files.begin(); iter != files.end(); iter++ )
    { _open( File( *iter ).expand() ); }
     
  }
  
  return;
  
}

//___________________________________________________________
void WindowServer::multipleFileReplace( std::list<File> files, TextSelection selection )
{
  Debug::Throw( "WindowServer::multipleFileRepplace.\n" );
    
  // keep track of number of replacements
  unsigned int counts(0);
  
  // retrieve frame associated with file
  BASE::KeySet<MainWindow> windows( this );
  for( list<File>::iterator iter = files.begin(); iter != files.end(); iter++ )
  {
    
    File& file( *iter );
    
    // find matching window
    BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( file ) );
    assert( iter != windows.end() );
    
    // select the corresponding display
    /* at least one display must be found, otherwise the algorithm is wrong */
    assert( (*iter)->selectDisplay( file ) );
    
    // perform replacement
    counts += (*iter)->activeDisplay().replaceInWindow( selection, false );
  
  }
  
  // popup dialog
  ostringstream what;
  if( !counts ) what << "string not found.";
  else if( counts == 1 ) what << "1 replacement performed";
  else what << counts << " replacements performed";
  QtUtil::infoDialog( &_activeWindow(), what.str() );
  
  return;
}

//____________________________________________
void WindowServer::_activeWindowChanged( MainWindow* window )
{
  Debug::Throw() << "WindowServer::_activeWindowChanged - " << window->key() << endl;
  _setActiveWindow( *window );
}

//_______________________________________________
void WindowServer::_updateActions( void )
{
  
  Debug::Throw( "WindowServer::_updateActions.\n" );
  saveAllAction().setEnabled( !files( true ).empty() );

}

//_______________________________________________
void WindowServer::_newFile( void )
{
  
  Debug::Throw( "WindowServer::_newFile.\n" );
  
  // retrieve all MainWindows
  MainWindow* window( 0 );
  BASE::KeySet<MainWindow> windows( this );
 
  // try find empty editor
  BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
  if( iter != windows.end() ) window = (*iter );
  
  // if no window found, create a new one
  if( !window ) {  window = &newMainWindow(); }

  // need to show window immediately to avoid application
  // to quit if at some point no window remains open
  window->show();
  return;
   
}


//_______________________________________________
void WindowServer::_newFile( Qt::Orientation orientation )
{
  
  Debug::Throw( "WindowServer::_newFile.\n" );
   
  // retrieve active view
  TextView& active_view( _activeWindow().activeView() );
  TextDisplay* display;
  
  // look for an empty display
  // create a new display if none is found
  BASE::KeySet<TextDisplay> displays( active_view );
  BASE::KeySet<TextDisplay>::iterator iter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
  display = ( iter == displays.end() ) ? &active_view.splitDisplay( orientation, false ):*iter;
  
  active_view.setActiveDisplay( *display );
  return;

}
  
//_______________________________________________
bool WindowServer::_open( FileRecord record )
{
  
  Debug::Throw( "WindowServer::_open.\n" );
  
  // do nothing if record is empty
  if( record.file().empty() ) return false;
  
  // create file if it does not exist
  if( !( record.file().exists() || _createNewFile( record ) ) ) return false;
  
  // retrieve all MainWindows
  MainWindow* window( 0 );
  BASE::KeySet<MainWindow> windows( this );
  
  // try find editor with matching name
  BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file() ) );
  if( iter != windows.end() )
  {
    
    (*iter)->uniconify();
    (*iter)->selectDisplay( record.file() );
    _setActiveWindow( **iter );
    return true;
    
  } 
    
  // try find empty editor
  iter = find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
  if( iter != windows.end() ) window = (*iter );
  
  // if no window found, create a new one
  // if( !window ) {  window = &newMainWindow(); }
 
  if( !window ) {  
    window = &_activeWindow();
    window->newTextView();
  }

  // need to show window immediately to avoid application
  // to quit if at some point no window remains open
  window->show();
  window->activeView().setFile( record.file() );
  return true;
  
}

//_______________________________________________
bool WindowServer::_open( FileRecord record, Qt::Orientation orientation )
{  
  Debug::Throw( "WindowServer::_open.\n" );
  
  // do nothing if record is empty
  if( record.file().empty() ) return false;

  // create file if it does not exist
  if( !( record.file().exists() || _createNewFile( record ) ) ) return false;
  
  // retrieve active view
  TextView& active_view( _activeWindow().activeView() );
  
  // retrieve all windows and find one matching
  BASE::KeySet<MainWindow> windows( this );
  BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file() ) );
  if( iter != windows.end() )
  {

    // select found display in TextView
    (*iter)->activeView().selectDisplay( record.file() );

    // check if the found window is the current
    if( &(*iter)->activeView() == &active_view )
    {
      (*iter)->uniconify();
      _setActiveWindow( **iter );
      return true;
    }

    ostringstream what;
    what
      << "The file " << record.file() << " is already opened in another window.\n"
      << "Do you want to close the other display and open the file here ?";
    if( !QtUtil::questionDialog( &_activeWindow(), what.str() ) )
    {
      (*iter)->uniconify();
      return false;
    }
    
    // look for an empty display
    // create a new display if none is found
    BASE::KeySet<TextDisplay> displays( &active_view );
    BASE::KeySet<TextDisplay>::iterator display_iter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
    TextDisplay& display( display_iter == displays.end() ? active_view.splitDisplay( orientation, false ):**display_iter );

    // retrieve active display from previous window
    TextDisplay& previous_display( (*iter)->activeDisplay() );

    // store modification state
    bool modified( previous_display.document()->isModified() );

    // clone
    display.synchronize( &previous_display );
    
    // set previous display as unmdified
    previous_display.document()->setModified( false );

    // close display      
    displays = BASE::KeySet<TextDisplay>( &previous_display );
    displays.insert( &previous_display );
    for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
    { (*iter)->activeView().closeDisplay( **display_iter ); }
      
    // restore modification state and make new display active
    display.setModified( modified );
    active_view.setActiveDisplay( display );
    display.setFocus();

  } else {

    // look for an empty display
    // create a new display if none is found
    BASE::KeySet<TextDisplay> displays( active_view );
    BASE::KeySet<TextDisplay>::iterator display_iter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
    if( display_iter == displays.end() ) active_view.splitDisplay( orientation, false );

    // open file in this window
    active_view.setFile( record.file() );
    
  }
  
  return true;
  
}

//_______________________________________________
void WindowServer::_detach( void )
{
  
  Debug::Throw( "WindowServer::_detach.\n" );

  MainWindow& active_window_local( _activeWindow() );
  
  // check number of independent displays
  assert( active_window_local.activeView().independentDisplayCount() > 1 || BASE::KeySet<TextView>( &_activeWindow() ).size() > 1 );
  
  // check number of displays associated to active
  TextDisplay& active_display_local( active_window_local.activeView().activeDisplay() );
  BASE::KeySet<TextDisplay> associated_displays( active_display_local );
  if( !( associated_displays.empty() ||
    QtUtil::questionDialog( &active_window_local,
    "Active display has clones in the current window.\n"
    "They will be closed when the display is detached.\n"
    "Continue ?" ) ) ) return;

  // save modification state
  bool modified( active_display_local.document()->isModified() );

  // close all clone displays
  for( BASE::KeySet<TextDisplay>::iterator iter = associated_displays.begin(); iter != associated_displays.end(); iter++ )
  { active_window_local.activeView().closeDisplay( **iter ); }

  // create MainWindow
  MainWindow& window( newMainWindow() );

  // clone its display from the current
  window.activeView().activeDisplay().synchronize( &active_display_local );

  // delete active display local
  active_display_local.document()->setModified( false );
  active_window_local.activeView().closeDisplay( active_display_local );

  // show the new window
  window.activeView().activeDisplay().document()->setModified( modified );
  window.show();
  // window._updateConfiguration();

  return;
  
}
  
//_______________________________________________
void WindowServer::_saveAll( void )
{
  Debug::Throw( "WindowServer::_saveAll.\n" );

  // load files
  FileRecordMap files( WindowServer::files( true ) );
  
  // check how many files are modified
  if( files.empty() )
  {
    QtUtil::infoDialog( &_activeWindow(), "No files to save" );
    return;
  }
  
  // ask for confirmation
  if( !SaveAllDialog( &_activeWindow(), files ).exec() ) return;
  
  // retrieve windows
  BASE::KeySet<MainWindow> windows( this );
  for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); iter++ ) { (*iter)->saveAll(); }

  return;
  
}

//________________________________________________________________
void WindowServer::_closeFiles( const list<string>& files )
{
  
  // loop over files 
  for( list< string >::const_iterator iter = files.begin(); iter != files.end(); iter++ )
  {
    File file( *iter );

    // retrieve all MainWindows
    BASE::KeySet<MainWindow> windows( this );
    BASE::KeySet<MainWindow>::iterator window_iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( file ) );
    if( window_iter == windows.end() ) continue;
    
    // select display and close
    (*window_iter)->selectDisplay( file );
    (*window_iter)->closeDisplayAction().trigger();
  }
  
  return;
    
}

//_______________________________________________
FileRecord WindowServer::_selectFileFromDialog( void )
{
  
  Debug::Throw( "WindowServer::_selectFileFromDialog.\n" );
  
  FileRecord record;
  
  // create file dialog
  CustomFileDialog dialog( &_activeWindow() );
  dialog.setFileMode( QFileDialog::ExistingFile );
  
  File working_directory( _activeWindow().activeDisplay().workingDirectory() );
  dialog.setDirectory( QDir( working_directory.c_str() ) );
  
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return record;
  
  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return record;
  
  File file = File( qPrintable( files.front() ) ).expand();

  // see if file is directory
  if( file.isDirectory() )
  {
    
    ostringstream what;
    what << "File \"" << record.file() << "\" is a directory. <Open> canceled.";
    QtUtil::infoDialog( &_activeWindow(), what.str() );
    return record;
    
  }
  
  // assign file to record
  record.setFile( file );
  return record;
  
}

//_______________________________________________
bool WindowServer::_createNewFile( const FileRecord& record )
{
  Debug::Throw( "WindowServer::_createNewFile.\n" );
  
  // check file is valid
  if( record.file().empty() ) return false;
  
  // create NewFileDialog
  int buttons( NewFileDialog::CREATE | NewFileDialog::CANCEL );
  if( BASE::KeySet<MainWindow>(this).size() == 1 ) buttons |= NewFileDialog::EXIT;
    
  NewFileDialog dialog( &_activeWindow(), record.file(), buttons );
  QtUtil::centerOnParent( &dialog );
  int state = dialog.exec();
    
  Debug::Throw() << "WindowServer::Open - New file dialog state: " << state << endl; 
  switch( state )
  {
    
    case NewFileDialog::CREATE:
    {
      File fullname( record.file().expand() );
      if( !fullname.create() )
      {
        
        ostringstream what;
        what << "Unable to create file " << record.file() << ".";
        QtUtil::infoDialog( &_activeWindow(), what.str() );
        return false;
        
      } else return true;
        
    }
    
    break;

    case NewFileDialog::EXIT: qApp->quit();
  
    case NewFileDialog::CANCEL: return false;
      
    default: 
    assert(0);
    break; 
  
  }
  
  return false;
  
}

//________________________________________________________________
void WindowServer::_setActiveWindow( MainWindow& window )
{ 
  Debug::Throw() << "WindowServer::setActiveWindow - key: " << window.key() << std::endl;
  assert( window.isAssociated( this ) );
  active_window_ = &window;
}


//________________________________________________________________
bool WindowServer::_hasActiveWindow( void ) const
{ 
  
  // check if active window exists
  if( !active_window_ ) return false;
  
  // check if found in list of associated windows
  BASE::KeySet<MainWindow> windows( this );
  
  // not sure it works when the window pointed to by active_window_ has been removed
  return windows.find( active_window_ ) != windows.end();
  
}
