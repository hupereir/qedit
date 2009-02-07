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
#include <QApplication>
#include <QProgressDialog>
#include <QTextStream>

#include "Application.h"
#include "CustomFileDialog.h"
#include "Debug.h"
#include "CloseFilesDialog.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "FileSystemFrame.h"
#include "Icons.h"
#include "IconEngine.h"
#include "MainWindow.h"
#include "Menu.h"
#include "NavigationFrame.h"
#include "NewFileDialog.h"
#include "InformationDialog.h"
#include "ProgressDialog.h"
#include "QuestionDialog.h"
#include "QtUtil.h"
#include "RecentFilesFrame.h"
#include "RecentFilesMenu.h"
#include "SaveAllDialog.h"
#include "SessionFilesFrame.h"
#include "Singleton.h"
#include "Util.h"
#include "WindowServer.h"

using namespace std;

//________________________________________________________________
const QString WindowServer::SINGLE_WINDOW = "open in current window";
const QString WindowServer::MULTIPLE_WINDOWS = "open in new window";

//________________________________________________________________
WindowServer::WindowServer( QObject* parent ):
  QObject( parent ),
  Counter( "WindowServer" ),
  first_call_( true ),
  default_orientation_( Qt::Horizontal ),
  open_mode_( ACTIVE_WINDOW ),
  active_window_( 0 )
{ 
  
  Debug::Throw( "WindowServer::WindowServer.\n" ); 
  
  // create actions
  save_all_action_ = new QAction( IconEngine::get( ICONS::SAVE_ALL ), "Save A&ll", this );
  connect( save_all_action_, SIGNAL( triggered() ), SLOT( _saveAll() ) );
 
  connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  _updateConfiguration();
  
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
  
  connect( this, SIGNAL( sessionFilesChanged() ), &window->navigationFrame().sessionFilesFrame(), SLOT( update() ) );

  connect( &window->newFileAction(), SIGNAL( triggered() ), SLOT( _newFile() ) );

  // open actions
  connect( &window->openAction(), SIGNAL( triggered() ), SLOT( _open() ) );
  connect( &window->openHorizontalAction(), SIGNAL( triggered() ), SLOT( _openHorizontal() ) );
  connect( &window->openVerticalAction(), SIGNAL( triggered() ), SLOT( _openVertical() ) );
  connect( &window->detachAction(), SIGNAL( triggered() ), SLOT( _detach() ) );

  // open actions
  connect( &window->menu().recentFilesMenu(), SIGNAL( fileSelected( FileRecord ) ), SLOT( _open( FileRecord ) ) );
  connect( &window->navigationFrame().sessionFilesFrame(), SIGNAL( fileActivated( FileRecord ) ), SLOT( _open( FileRecord ) ) );
  connect( &window->navigationFrame().recentFilesFrame(), SIGNAL( fileActivated( FileRecord ) ), SLOT( _open( FileRecord ) ) );
  connect( &window->navigationFrame().fileSystemFrame(), SIGNAL( fileActivated( FileRecord ) ), SLOT( _open( FileRecord ) ) );

  // other actions
  connect( &window->navigationFrame().sessionFilesFrame(), SIGNAL( filesSaved( FileRecord::List ) ), SLOT( _save( FileRecord::List ) ) );
  connect( &window->navigationFrame().sessionFilesFrame(), SIGNAL( filesClosed( FileRecord::List ) ), SLOT( _close( FileRecord::List ) ) );

  return *window;
}

//______________________________________________________
FileRecord::List WindowServer::records( bool modified_only, QWidget* window ) const
{ 
  
  Debug::Throw( "WindowServer::records.\n" );
  
  // output
  FileRecord::List records;
  
  // get associated main windows
  Application& application( *Singleton::get().application<Application>() );
  BASE::KeySet<MainWindow> windows( this );
  for( BASE::KeySet<MainWindow>::iterator window_iter = windows.begin(); window_iter != windows.end(); window_iter++ )
  {

    // check if current window match the one passed in argument
    bool active_window( *window_iter == window );
    
    // retrieve associated TextDisplays
    BASE::KeySet<TextDisplay> displays( (*window_iter)->associatedDisplays() );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {

      // check modification status
      if( modified_only && !(*iter)->document()->isModified() ) continue;
        
      // retrieve file
      // store in map if not empty
      const File& file( (*iter)->file() );
      if( file.isEmpty() ) continue;
      
      // insert in map (together with modification status)
      FileRecord record = (*iter)->isNewDocument() ? FileRecord( file ):application.recentFiles().get(file);
            
      // set flags
      unsigned int flags( FileRecordProperties::NONE );
      if( (*iter)->document()->isModified() ) flags |= FileRecordProperties::MODIFIED;
      if( active_window )
      {
        flags |= FileRecordProperties::ACTIVE;
        if( *iter == &(*window_iter)->activeDisplay() ) flags |= FileRecordProperties::SELECTED;
      }
      
      // assign flags and store
      records.push_back( record.setFlags( flags ) );
        
    }
    
  }
  
  sort( records.begin(), records.end(), FileRecord::FileFTor() );
  records.erase( unique( records.begin(), records.end(), FileRecord::SameFileFTor() ), records.end() );
  Debug::Throw( "WindowServer::records - done.\n" );
  return records;
  
}

//______________________________________________________
bool WindowServer::closeAll( void )
{ 
  
  Debug::Throw( "WindowServer::closeAll.\n" );

  // retrieve opened files
  FileRecord::List records( WindowServer::records() );
    
  // ask for confirmation if more than one file is opened.
  if( records.size() > 1 )
  {
    CloseFilesDialog dialog( &_activeWindow(), records );
    dialog.centerOnParent();
    if( !dialog.exec() ) return false;
  }
  
  list<QString> files;
  for( FileRecord::List::const_iterator iter = records.begin(); iter != records.end(); iter++ )
  { files.push_back( iter->file() ); }

  return _close( files );
  
}

//______________________________________________________
void WindowServer::readFilesFromArguments( CommandLineArguments arguments )
{
      
  Debug::Throw() << "WindowServer::readFilesFromArguments." << endl;
  
  // retrieve files from arguments
  CommandLineParser parser( Application::commandLineParser( arguments ) );
  QStringList filenames( parser.orphans() );
  
  // close mode
  if( parser.hasFlag( "--close" ) ) 
  {
    _close( filenames );
    _setFirstCall( false );
    return;
  }
  
  // check number of files
  if( filenames.size() > 10 )
  {
    QString buffer;
    QTextStream what( &buffer );
    what << "Do you really want to open " << filenames.size() << " files at the same time ?" << endl;
    what << "This might be very resource intensive and can overload your computer." << endl;
    what << "If you choose No, only the first file will be opened.";
    if( !QuestionDialog( &_activeWindow(), buffer ).exec() )
    { filenames = filenames.mid(0,1); }
    
  }
  
  // check if at least one file is opened
  bool file_opened( false );
  
  // tabbed | diff mode
  bool tabbed( parser.hasFlag( "--tabbed" ) );
  bool diff( parser.hasFlag( "--diff" ) );
  if( ( tabbed || diff ) && filenames.size() > 1 )
  {

    bool first( true );
    for( QStringList::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++ )
    {
    
      if( first )
      {
      
        if( file_opened |= _open( File( qPrintable( *iter ) ).expand() ) ) 
        {
          _applyArguments( _activeWindow().activeDisplay(), arguments );
          first = false;
        }
        
      } else { 
        
        if( file_opened |= _open( File( qPrintable( *iter ) ).expand(), _activeWindow().orientation() ) )
        { _applyArguments( _activeWindow().activeDisplay(), arguments ); }
        
      }
    
    }
    
    if( diff )
    { 
      if( !first && _activeWindow().activeView().independentDisplayCount() == 2 ) _activeWindow().diffAction().trigger();
      else InformationDialog( &_activeWindow(), "invalid number of files selected. <Diff> canceled." ).exec();
    }
    
  } else {
  
    // default mode
    for( QStringList::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++ )
    { 
      
      OpenMode mode( _openMode() );
      if( parser.hasFlag( "--same-window" ) ) mode = ACTIVE_WINDOW;
      else if( parser.hasFlag( "--new-window" ) ) mode = NEW_WINDOW;
      
      bool opened = _open( File( qPrintable( *iter ) ).expand(), mode );
      if( opened ) { _applyArguments( _activeWindow().activeDisplay(), arguments ); }
      file_opened |= opened;
      
    }
     
  }
  
  if( !file_opened )
  {
    if( _firstCall() ) { 
      
      // at first call and if no file was oppened,
      // set the current display as a new document.
      // also force update of the recent files frame
      // and center on desktop
      _activeWindow().activeView().setIsNewDocument(); 
      _activeWindow().navigationFrame().recentFilesFrame().update();
      
    } 
    
    _activeWindow().uniconify();
    
  }
  _setFirstCall( false );
  return;
  
}

//___________________________________________________________
void WindowServer::multipleFileReplace( std::list<File> files, TextSelection selection )
{
  Debug::Throw( "WindowServer::multipleFileReplace.\n" );
    
  // keep track of number of replacements
  unsigned int counts(0);  
  
  // create progressDialog
  ProgressDialog dialog;
  dialog.setAttribute( Qt::WA_DeleteOnClose );
  QtUtil::centerOnWidget( &dialog, qApp->activeWindow() );
  dialog.show();
  
  // loop over files to get relevant displays
  int maximum(0);
  BASE::KeySet<TextDisplay> displays;
  BASE::KeySet<MainWindow> windows( this );
  for( list<File>::iterator iter = files.begin(); iter != files.end(); iter++ )
  {
    
    File& file( *iter );
    
    // find matching window
    BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( file ) );
    assert( iter != windows.end() );
    
    // loop over views
    BASE::KeySet<TextView> views( *iter );
    for( BASE::KeySet<TextView>::iterator view_iter = views.begin(); view_iter != views.end(); view_iter++ )
    {
      if( !(*view_iter)->selectDisplay( file ) ) continue;
      TextDisplay* display( &(*view_iter)->activeDisplay() );
      connect( display, SIGNAL( progressAvailable( int ) ), &dialog, SLOT( setValue( int ) ) );
      maximum += display->toPlainText().size();
      displays.insert( display );
    }
    
  }

  dialog.setMaximum( maximum );
  //QtUtil::centerOnWidget( &dialog, qApp->activeWindow() );
  //dialog.show();

  // loop over displays and perform replacement
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    counts += (*iter)->replaceInWindow( selection, false );
    dialog.setOffset( dialog.value() );
  }
  
  // close progress dialog
  // dialog.close();
  dialog.close();
  
  // popup dialog
  QString buffer;
  QTextStream what( &buffer );
  if( !counts ) what << "string not found.";
  else if( counts == 1 ) what << "1 replacement performed";
  else what << counts << " replacements performed";
  InformationDialog( &_activeWindow(), buffer ).centerOnWidget( qApp->activeWindow() ).exec();
  
  return;
}

//____________________________________________
void WindowServer::_updateConfiguration( void )
{
  
  Debug::Throw( "WindowServer::_updateConfiguration.\n" );
  _setOpenMode( XmlOptions::get().raw( "OPEN_MODE" ) == MULTIPLE_WINDOWS ? NEW_WINDOW:ACTIVE_WINDOW );
  _setOrientation( XmlOptions::get().raw( "ORIENTATION" ) == MainWindow::LEFT_RIGHT ? Qt::Horizontal : Qt::Vertical );
  
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
  saveAllAction().setEnabled( !records( true ).empty() );

}

//_______________________________________________
void WindowServer::_newFile( WindowServer::OpenMode mode )
{
  
  Debug::Throw( "WindowServer::_newFile.\n" );
  
  // retrieve all MainWindows
  BASE::KeySet<MainWindow> windows( this );
    
  // try find empty editor
  TextView* view(0);
  BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
  if( iter != windows.end() ) 
  {
    
    // select the view that contains the empty display
    BASE::KeySet<TextView> views( *iter );
    BASE::KeySet<TextView>::iterator view_iter( find_if( views.begin(), views.end(), MainWindow::EmptyFileFTor() ) );
    assert( view_iter != views.end() );
    (*iter)->setActiveView( **view_iter );
    view = *view_iter;
    
    // uniconify
    (*iter)->uniconify();
    
  }
  
  // if no window found, create a new one
  if( !view ) {
    
    if( mode == NEW_WINDOW )
    {
      
      MainWindow &window( newMainWindow() );
      view = &window.activeView();
      window.show();
      
    } else {
      
      view = &_activeWindow().newTextView();
      _activeWindow().uniconify();
      
    }
    
  }
  
  // mark view as new document
  view->setIsNewDocument();
  
  return;
   
}


//_______________________________________________
void WindowServer::_newFile( Qt::Orientation orientation )
{
  
  Debug::Throw( "WindowServer::_newFile.\n" );
   
  // retrieve active view
  TextView& active_view( _activeWindow().activeView() );
  
  // look for an empty display
  // create a new display if none is found
  BASE::KeySet<TextDisplay> displays( active_view );
  BASE::KeySet<TextDisplay>::iterator iter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
  if( iter == displays.end() ) active_view.splitDisplay( orientation, false );
  
  active_view.setIsNewDocument();
  return;

}
  
//_______________________________________________
bool WindowServer::_open( FileRecord record, WindowServer::OpenMode mode )
{
  
  Debug::Throw() << "WindowServer::_open - file: " << record.file() << "." << endl;
  
  // do nothing if record is empty
  if( record.file().isEmpty() ) return false;
  
  // see if file is directory
  if( record.file().isDirectory() )
  {
    
    QString buffer;
    QTextStream( &buffer ) << "File \"" << record.file() << "\" is a directory. <Open> canceled.";
    InformationDialog( &_activeWindow(), buffer ).exec();
    return false;
    
  }
  
  // retrieve all MainWindows
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
  
  // create file if it does not exist
  if( !( record.file().exists() || _createNewFile( record ) ) ) return false;
    
  // try find empty editor
  TextView* view(0);
  iter = find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
  if( iter != windows.end() ) 
  {
    
    // select the view that contains the empty display
    BASE::KeySet<TextView> views( *iter );
    BASE::KeySet<TextView>::iterator view_iter( find_if( views.begin(), views.end(), MainWindow::EmptyFileFTor() ) );
    assert( view_iter != views.end() );
    (*view_iter)->setFile( record.file() );
    (*iter)->setActiveView( **view_iter );
    view = *view_iter;
          
    // uniconify
    (*iter)->uniconify();
    
  }
  
  // if no window found, create a new one
  if( !view ) {
    
    if( mode == NEW_WINDOW )
    {
      
      MainWindow &window( newMainWindow() );
      view = &window.activeView();
      view->setFile( record.file() );
      window.show();
      
    } else {
      
      // create new view
      view = &_activeWindow().newTextView( record );
            
      // uniconify
      _activeWindow().uniconify();
      
    }
    
  }
  
  // assign file
  //view->setFile( record.file() );
  qApp->processEvents();
  
  return true;
  
}

//_______________________________________________
bool WindowServer::_open( FileRecord record, Qt::Orientation orientation )
{  
  
  Debug::Throw() << "WindowServer::_open - file: " << record.file() << " orientation: " << orientation << endl;
  
  // do nothing if record is empty
  if( record.file().isEmpty() ) return false;

  // create file if it does not exist
  if( !( record.file().exists() || _createNewFile( record ) ) ) return false;
  
  // retrieve active view
  TextView& active_view( _activeWindow().activeView() );
  
  // retrieve all windows and find one matching
  BASE::KeySet<MainWindow> windows( this );
  BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file() ) );
  if( iter != windows.end() )
  {
    
    // find matching view
    BASE::KeySet<TextView> views( *iter );
    BASE::KeySet<TextView>::iterator view_iter = find_if( views.begin(), views.end(), MainWindow::SameFileFTor( record.file() ) );
    assert( view_iter != views.end() );
    
    // check if the found view is the current
    if( *view_iter == &active_view )
    {
      (*iter)->uniconify();
      _setActiveWindow( **iter );
      return true;
    }

    // select found display in TextView
    (*view_iter)->selectDisplay( record.file() );

    QString buffer;
    QTextStream( &buffer )
      << "The file " << record.file() << " is already opened in another window.\n"
      << "Do you want to close the other display and open the file here ?";
    if( !QuestionDialog( &_activeWindow(), buffer ).exec() )
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
    TextDisplay& previous_display( (*view_iter)->activeDisplay() );

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
    { (*view_iter)->closeDisplay( **display_iter ); }
      
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
    QuestionDialog( &active_window_local,
    "Active display has clones in the current window.\n"
    "They will be closed when the display is detached.\n"
    "Continue ?" ).exec() ) ) return;

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

  return;
  
}
  
//_______________________________________________
void WindowServer::_saveAll( void )
{
  Debug::Throw( "WindowServer::_saveAll.\n" );
  _save( records( true ) ); 
  return;
  
}

//_______________________________________________
void WindowServer::_save( FileRecord::List records )
{
  Debug::Throw( "WindowServer::_save.\n" );
  
  // check how many records are modified
  assert( !records.empty() );
  
  // ask for confirmation
  if( records.size() > 1 && !SaveAllDialog( &_activeWindow(), records ).exec() ) return;
  
  // retrieve windows
  BASE::KeySet<MainWindow> windows( this );
  for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); iter++ ) 
  { 
  
    // retrieve displays
    BASE::KeySet<TextDisplay> displays( (*iter)->associatedDisplays() );
    for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
    {
      TextDisplay& display( **display_iter );
      if( !display.document()->isModified() ) continue;
      if( find_if( records.begin(), records.end(), FileRecord::SameFileFTor( display.file() ) ) == records.end() ) continue;
      display.save();
    }
  
  }

}


//_______________________________________________
void WindowServer::_close( QStringList filenames )
{

  FileRecord::List records;
  for( QStringList::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++ )
  { records.push_back( FileRecord( *iter ) ); }
  
  _close( records );
  
}

//_______________________________________________
void WindowServer::_close( FileRecord::List records )
{
  Debug::Throw( "WindowServer::_close.\n" );
  
  // check how many records are modified
  assert( !records.empty() );
  
  // ask for confirmation
  if( records.size() > 1 && !CloseFilesDialog( &_activeWindow(), records ).exec() ) return;
  
  list<QString> files;
  for( FileRecord::List::const_iterator iter = records.begin(); iter != records.end(); iter++ )
  { files.push_back( iter->file() ); }
  
  _close( files );
  
}

//________________________________________________________________
bool WindowServer::_close( const list<QString>& files )
{
 
  int state( AskForSaveDialog::UNKNOWN );

  // need a first loop over associated windows to store modified files
  set<QString> modified_files;
  BASE::KeySet<MainWindow> windows( this );
  for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); iter++ ) 
  { 
    BASE::KeySet<TextDisplay> displays( (*iter)->associatedDisplays() );
    for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
    {
      
      // see if file is in list
      TextDisplay& display( **display_iter );
      if( find( files.begin(), files.end(), display.file() ) == files.end() ) continue;
      if( display.document()->isModified() ) modified_files.insert( display.file() );
      
    }
   
  }
  
  // retrieve windows
  for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); iter++ ) 
  { 
    
    // retrieve views
    BASE::KeySet<TextView> views( *iter );
    for( BASE::KeySet<TextView>::iterator view_iter = views.begin(); view_iter != views.end(); view_iter++ )
    {
      
      TextView& view( **view_iter );
      BASE::KeySet<TextDisplay> displays( &view );
      for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
      {
        TextDisplay& display( **display_iter );
        
        // see if file is in list
        if( find( files.begin(), files.end(), display.file() ) == files.end() ) continue;
        
        Debug::Throw() << "WindowServer::_close - file: " << display.file() << endl;
        
        if( display.document()->isModified() ) 
        {
          
          if( state == AskForSaveDialog::YES_TO_ALL ) display.save();
          else if( state == AskForSaveDialog::NO_TO_ALL ) display.document()->setModified( false );
          else 
          {
            state = display.askForSave( modified_files.size() > 1 );
            if( state == AskForSaveDialog::YES_TO_ALL || state == AskForSaveDialog::YES ) display.save();
            else if( state == AskForSaveDialog::NO_TO_ALL  || state == AskForSaveDialog::YES ) display.document()->setModified( false );   
            else if( state == AskForSaveDialog::CANCEL ) return false;
            modified_files.erase( display.file() );
            view.closeDisplay( display );
          }
          
        }
        
        view.closeDisplay( display );
        
      }
    }
  }
  
  return true;
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
  dialog.setDirectory( QDir( working_directory ) );
  
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return record;
  
  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return record;
  
  File file = File( qPrintable( files.front() ) ).expand();
  
  // assign file to record
  record.setFile( file );
  return record;
  
}

//_______________________________________________
bool WindowServer::_createNewFile( const FileRecord& record )
{
  Debug::Throw( "WindowServer::_createNewFile.\n" );
  
  // check file is valid
  if( record.file().isEmpty() ) return false;
  
  // create NewFileDialog
  int buttons( NewFileDialog::CREATE | NewFileDialog::CANCEL );
  if( records().empty() ) buttons |= NewFileDialog::EXIT;
    
  NewFileDialog dialog( &_activeWindow(), record.file(), buttons );
  dialog.centerOnParent();
  int state = dialog.exec();
    
  Debug::Throw() << "WindowServer::Open - New file dialog state: " << state << endl; 
  switch( state )
  {
    
    case NewFileDialog::CREATE:
    {
      File fullname( record.file().expand() );
      if( !fullname.create() )
      {
        
        QString buffer;
        QTextStream( &buffer ) << "Unable to create file " << record.file() << ".";
        InformationDialog( &_activeWindow(), buffer ).exec();
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
void WindowServer::_applyArguments( TextDisplay& display, CommandLineArguments arguments )
{ 
  Debug::Throw( "WindowServer::_applyArguments.\n" );

  CommandLineParser parser( Application::commandLineParser( arguments ) );
  
  //! see if autospell action is required
  bool autospell( parser.hasFlag( "--autospell" ) );
  
  //! see if autospell filter and dictionary are required
  QString filter = parser.hasOption( "--filter" ) ? parser.option( "--filter" ) : "";
  QString dictionary = parser.hasOption( "--dictionary" ) ? parser.option( "--dictionary" ) : "";
  Debug::Throw() << "WindowServer::_applyArguments -"
    << " filter:" << qPrintable( filter )
    << " dictionary: " << qPrintable( dictionary ) 
    << endl;
  
  if( autospell ) display.autoSpellAction().setChecked( true );
  if( !filter.isEmpty() ) display.selectFilter( filter );
  if( !dictionary.isEmpty() ) display.selectDictionary( dictionary );
  
}

//________________________________________________________________
void WindowServer::_setActiveWindow( MainWindow& window )
{ 
  Debug::Throw() << "WindowServer::setActiveWindow - key: " << window.key() << endl;
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
