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

#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "MainWindow.h"
#include "NewFileDialog.h"
#include "QtUtil.h"
#include "SaveAllDialog.h"
#include "WindowServer.h"

using namespace std;

//________________________________________________________________
WindowServer::WindowServer( QObject* parent ):
  QObject( parent ),
  Counter( "WindowServer" ),
  open_status_( OPEN )
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
  MainWindow* out = new MainWindow();
  BASE::Key::associate( this, out );
  return *out;
}

//______________________________________________________
WindowServer::FileMap WindowServer::files( bool modified_only ) const
{ 
  
  Debug::Throw( "WindowServer::files.\n" );
  
  // output
  FileMap files;
  
  // get associated main windows
  BASE::KeySet<MainWindow> windows( this );
  for( BASE::KeySet<MainWindow>::iterator window_iter = windows.begin(); window_iter != windows.end(); window_iter++ )
  {
      
    // retrieve associated text displays
    BASE::KeySet<TextDisplay> displays( *window_iter );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {

      // check modification status
      if( modified_only && !(*iter)->document()->isModified() ) continue;
        
      // retrieve file
      // store in map if not empty
      const File& file( (*iter)->file() );
      if( file.empty() ) continue;
        
      // insert in map (together with modification status
      files.insert( make_pair( file, (*iter)->document()->isModified() ) );
        
    }
    
  }
  
  return files;
  
}

//______________________________________________________
bool WindowServer::closeAllWindows( void )
{ 
  
  Debug::Throw( "WindowServer::closeAllWindows.\n" );
  
  // try close all windows one by one
  BASE::KeySet<MainWindow> windows( this );
  for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); iter++ )
  {
    
    // if window is not modified, close
    if( !(*iter)->isModified() ) 
    {
      
      // close window
      assert( (*iter)->close() );
      continue;
    
    }
    
    // loop over displays
    bool save_all_enabled = count_if( iter, windows.end(), MainWindow::IsModifiedFTor() ) > 1;
    BASE::KeySet<TextDisplay> displays( *iter );
    for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
    {
      
      if( !(*display_iter)->document()->isModified() ) continue;
      save_all_enabled |= (*iter)->modifiedDisplayCount() > 1;
      int state( (*display_iter)->askForSave( save_all_enabled ) );
      
      if( state == AskForSaveDialog::YES_TO_ALL ) 
      {
        
        // save all displays for this window, starting from the current
        for(; display_iter != displays.end(); display_iter++ ) 
        { if( (*display_iter)->document()->isModified() ) (*display_iter)->save(); }
       
        // save all mainwindows starting from the next to this one
        BASE::KeySet<MainWindow>::iterator sub_iter = iter; 
        for( sub_iter++; sub_iter != windows.end(); sub_iter++ ) (*sub_iter)->saveAll();
        
        // break loop since everybody has been saved
        break;
        
      } else if( state == AskForSaveDialog::NO_TO_ALL ) {
        
        // save all displays for this window, starting from the current
        for(; display_iter != displays.end(); display_iter++ ) 
        { if( (*display_iter)->document()->isModified() ) (*display_iter)->setModified( false ); }
       
        // save all mainwindows starting from the next to this one
        BASE::KeySet<MainWindow>::iterator sub_iter = iter; 
        for( sub_iter++; sub_iter != windows.end(); sub_iter++ )
        {
         
          BASE::KeySet<TextDisplay> sub_displays( *sub_iter );
          for( BASE::KeySet<TextDisplay>::iterator sub_display_iter = sub_displays.begin(); sub_display_iter != sub_displays.end(); sub_display_iter++ ) 
          { if( (*sub_display_iter)->document()->isModified() ) (*sub_display_iter)->setModified( false ); }

        }
        
        // break loop since everybody has been saved
        break;
        
      } else if( state == AskForSaveDialog::CANCEL ) return false;

    }
    
    // try close. Should succeed, otherwise it means there is a flow in the algorithm above
    assert( (*iter)->close() );
    
  }

  return true;
  
}

//______________________________________________________
void WindowServer::readFilesFromArguments( ArgList args )
{
  
  Debug::Throw( "WindowServer::readFilesFromArguments.\n" );
    
  // retrieve files from arguments
  ArgList::Arg last_arg( args.get().back() );
  
  // check number of files
  if( last_arg.options().size() > 10 )
  {
    ostringstream what;
    what << "Do you really want to open " << last_arg.options().size() << " files at the same time ?" << endl;
    what << "This might be very resource intensive and can overload your computer." << endl;
    what << "If you choose No, only the first file will be opened.";
    if( !QtUtil::questionDialog( qApp->activeWindow(), what.str() ) )
    {
      ArgList::Arg tmp;
      tmp.options().push_back( last_arg.options().front() );
      last_arg = tmp;
    }
  }
  
  // see if tabbed mode
  bool tabbed( args.find( "--tabbed" ) );
  bool diff( args.find( "--diff" ) );
  if( tabbed || diff )
  {
    
    MainWindow *window( 0 );
    
    // try find empty editor
    BASE::KeySet<MainWindow> windows( this );
    
    // retrieve an empty window if any, or create one
    BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
    if( iter != windows.end() ) window = (*iter );
    else window = &newMainWindow();
    
    // loop over files and open in current window
    set<string> files;
    for( list< string >::const_iterator iter = last_arg.options().begin(); iter != last_arg.options().end(); iter++ )
    {
      
      
      File file( *iter );
      if( file.size() ) file = file.expand();
      
      // open in same window, using default orientation
      if( window->orientation() == Qt::Horizontal ) window->openHorizontal( file );
      else window->openVertical( file );
      files.insert( file );
      
    }
    
    // if diff 
    if( diff && window )
    { 
      if( files.size() == 2 ) window->diffAction().trigger(); 
      else if( files.size() > 2 ) QtUtil::infoDialog( qApp->activeWindow(), "too many files selected. <Diff> canceled." );
      else if( files.size() < 2 ) QtUtil::infoDialog( qApp->activeWindow(), "too few files selected. <Diff> canceled." );
    }
    
    window->show();
    return;
  } 
  
  // close mode
  if( args.find( "--close" ) )
  {
  
    // loop over files 
    for( list< string >::const_iterator iter = last_arg.options().begin(); iter != last_arg.options().end(); iter++ )
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
    
    // exit application
    open_status_ = EXIT_APP;
    return;
    
  }
  
  // default mode
  for( list< string >::const_iterator iter = last_arg.options().begin(); iter != last_arg.options().end(); iter++ )
  {
    
    File file( *iter );
    if( !file.empty() ) file = file.expand();
    open( file, args );
    if( open_status_ == EXIT_APP ) return;
    
  }
      
  return;
  
}

//_______________________________________________
MainWindow* WindowServer::open( FileRecord record, ArgList args )
{
  
  Debug::Throw() << "WindowServer::Open - file: " << record.file() << endl;

  //! see if autospell action is required
  bool autospell( args.find( "--autospell" ) );
  
  //! see if autospell filter and dictionary are required
  string filter = ( args.find( "--filter" ) && !args.get( "--filter" ).options().empty() ) ? args.get( "--filter" ).options().front() : "";
  string dictionary = (args.find( "--dictionary" ) && !args.get( "--dictionary" ).options().empty() ) ? args.get( "--dictionary" ).options().front() : "";
  Debug::Throw() << "WindowServer::open - filter:" << filter << " dictionary: " << dictionary << endl;
  
  //! set default status to "open"
  open_status_ = OPEN;
  
  // see if file is directory
  if( record.file().isDirectory() )
  {
    
    ostringstream what;
    what << "File \"" << record.file() << "\" is a directory. <Open> canceled.";
    QtUtil::infoDialog( qApp->activeWindow(), what.str() );
    
    // update open status and exit
    open_status_ = INVALID;
    return 0;
    
  }

  // retrieve all MainWindows
  MainWindow* window( 0 );
  BASE::KeySet<MainWindow> windows( this );
  
  // try find editor with matching name
  BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file() ) );
  if( iter != windows.end() )
  {
    (*iter)->uniconify();
    (*iter)->selectDisplay( record.file() );
  
    // trigger autospell if required
    if( autospell ) (*iter)->activeDisplay().autoSpellAction().setChecked( true );
    if( !filter.empty() ) (*iter)->activeDisplay().selectFilter( filter.c_str() );
    if( !dictionary.empty() ) (*iter)->activeDisplay().selectDictionary( dictionary.c_str() );
    
    // update open status and exit
    open_status_ = OPEN;
    return (*iter);
    
  }
  
  // try find empty editor
  iter = find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
  if( iter != windows.end() ) window = (*iter );

  // if no window found, create a new one
  if( !window ) window = &newMainWindow();
  window->show();

  // check if file exists
  if( record.file().exists() || record.file().empty() ) window->setFile( record.file() );
  else if( !record.file().empty() )
  {
  
    // create NewFileDialog
    int buttons( NewFileDialog::CREATE | NewFileDialog::CANCEL );
    bool enable_exit( BASE::KeySet<MainWindow>(this).size() == 1 );
    if( enable_exit ) buttons |= NewFileDialog::EXIT;
    
    NewFileDialog dialog( window, record.file(), buttons );
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
          QtUtil::infoDialog( window, what.str() );
          window->setFile( File("") );
        } else window->setFile( fullname );
        open_status_ = OPEN;
        break;
      }
  
      case NewFileDialog::CANCEL:
      if( enable_exit ) open_status_ = INVALID;
      else {
        open_status_ = INVALID;
        window->close();
        return 0;
      }
      
      break; 
  
      case NewFileDialog::EXIT:
      {
        open_status_ = INVALID;
        window->close();
        return 0;
      }
      
      default: throw runtime_error( DESCRIPTION( "invalid return code" ) );
    }
  } 
  
  // trigger autospell if required
  if( autospell ) window->activeDisplay().autoSpellAction().setChecked( true );
  if( !filter.empty() ) window->activeDisplay().selectFilter( filter.c_str() );
  if( !dictionary.empty() ) window->activeDisplay().selectDictionary( dictionary.c_str() );

  Debug::Throw( "WindowServer::Open - done.\n" );
    
  return window;

}

//___________________________________________________________
void WindowServer::multipleFileReplace( std::list<File> files, TextSelection selection )
{
  Debug::Throw( "WindowServer::multipleFileRepplace.\n" );
    
  // keep track of number of replacements
  unsigned int counts(0);
  
  // retrieve frame associated with file
  BASE::KeySet<MainWindow> frames( this );
  for( list<File>::iterator iter = files.begin(); iter != files.end(); iter++ )
  {
    File& file( *iter );
    
    BASE::KeySet<MainWindow>::iterator iter = find_if( frames.begin(), frames.end(), MainWindow::SameFileFTor( file ) );
    assert( iter != frames.end() );

    // retrieve TextDisplay that match file
    BASE::KeySet<TextDisplay> displays( *iter );
    BASE::KeySet<TextDisplay>::iterator display_iter( find_if( displays.begin(), displays.end(), TextDisplay::SameFileFTor( file ) ) );
    assert( display_iter != displays.end() );
    
    // need to set display as active so that synchronization is kept with other possible displays
    (*iter)->setActiveDisplay( **display_iter );
    (*display_iter)->setFocus();
    
    // perform replacement
    counts += (*display_iter)->replaceInWindow( selection, false );
  
  }
  
  // popup dialog
  ostringstream what;
  if( !counts ) what << "string not found.";
  else if( counts == 1 ) what << "1 replacement performed";
  else what << counts << " replacements performed";
  QtUtil::infoDialog( qApp->activeWindow(), what.str() );
  
  return;
}

//_______________________________________________
void WindowServer::_saveAll( void )
{
  
  Debug::Throw( "WindowServer::_saveAll.\n" );

  // load files
  FileMap files( WindowServer::files( true ) );
  
  // check how many files are modified
  if( files.empty() )
  {
    QtUtil::infoDialog( qApp->activeWindow(), "No files to save" );
    return;
  }
  
  // ask for confirmation
  if( !SaveAllDialog( qApp->activeWindow(), files ).exec() ) return;
  
  // retrieve windows
  BASE::KeySet<MainWindow> windows( this );
  for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); iter++ )
  { (*iter)->saveAll(); }

  return;
  
}
