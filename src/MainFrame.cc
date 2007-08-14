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
  \file MainFrame.cc
  \brief application singleton object
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QMessageBox>

#include "AskForSaveDialog.h"
#include "AutoSave.h"
#include "Config.h" 
#include "ConfigurationDialog.h"
#include "DocumentClassManager.h"
#include "EditFrame.h"
#include "ErrorHandler.h"
#include "ExitDialog.h"
#include "MainFrame.h"
#include "NewFileDialog.h"
#include "XmlOptions.h"
#include "QtUtil.h"
#include "TextDisplay.h"
#include "Util.h"
#include "XmlDef.h"

#if WITH_ASPELL
#include "SpellCheckConfiguration.h"
#include "AutoSpellConfiguration.h"
#endif

using namespace std;
using namespace SERVER;

//____________________________________________
void MainFrame::usage( void )
{
  cout << "Usage : qedit [options] <file1> <file2> <...>" << endl;
  cout << endl;
  cout << "Options : " << endl;
  cout << "  --help\t\t displays this help and exit" << endl;
  cout << "  --tabbed\t\t opens command line files in same window" << endl;
  cout << "  --diff\t\t opens command line files in same window and perform diff" << endl;
  ApplicationManager::usage();
  return;
}

//____________________________________________
MainFrame::MainFrame( int argc, char*argv[] ) :
  QApplication( argc, argv ),
  Counter( "MainFrame" ),
  open_status_( OPEN ),
  application_manager_( 0 ),
  class_manager_( 0 ),
  autosave_( 0 ),
  args_( argc, argv ),
  realized_( false ),
  startup_timer_( this )
{
  Debug::Throw( "MainFrame::MainFrame.\n" );

  // about to quit connection
  connect( this, SIGNAL( aboutToQuit() ), SLOT( _aboutToQuit() ) );

  startup_timer_.setSingleShot( true );
  connect( &startup_timer_, SIGNAL( timeout() ), SLOT( _readFilesFromArgs() ) );
}

//____________________________________________
MainFrame::~MainFrame( void )
{ 
  Debug::Throw( "MainFrame::~MainFrame.\n" ); 

  XmlOptions::write();
  
  if( application_manager_ ) delete application_manager_;
  if( class_manager_ ) delete class_manager_;
  if( autosave_ ) delete autosave_;
  ErrorHandler::exit();

}

//____________________________________________
void MainFrame::initApplicationManager( void )
{
  Debug::Throw( "MainFrame::initApplicationManager.\n" );

  if( application_manager_ ) return;
  if( args_.find( "--no-server" ) ) 
  {
    realizeWidget();
    return;
  }

  // create application manager
  application_manager_ = new ApplicationManager( this );
  application_manager_->setApplicationName( XmlOptions::get().get<string>( "APP_NAME" ) );
  connect( 
    application_manager_, SIGNAL( stateChanged( SERVER::ApplicationManager::State ) ),
    SLOT( _applicationManagerStateChanged( SERVER::ApplicationManager::State ) ) );
    
  connect( application_manager_, SIGNAL( serverRequest( const ArgList& ) ), SLOT( _processRequest( const ArgList& ) ) );
 
  // retrieve files from arguments and expand if needed
  ArgList::Arg& last( args_.get().back() );
  for( list< string >::iterator iter = last.options().begin(); iter != last.options().end(); iter++ )
  { if( File( *iter ).size() ) (*iter) = File( *iter ).expand(); }

  // pass args to application manager
  application_manager_->init( args_ );

}

//____________________________________________
void MainFrame::realizeWidget( void )
{
  Debug::Throw( "MainFrame::realizeWidget.\n" );

  //! check if the method has already been called.
  if( realized_ ) return;
  realized_ = true;
  
  // class manager
  class_manager_ = new DocumentClassManager();
  
  // autosave
  autosave_ = new AutoSave();
  
  // create first editFrame
  newEditFrame().show(); 
  updateConfiguration();

  // make sure application ends when last window is closed.
  connect( this, SIGNAL( lastWindowClosed() ), SLOT( quit() ) );
  
  // run startup timer to open files after the call to exec() is 
  // performed in the main routine
  startup_timer_.start( 0 );
  Debug::Throw( "MainFrame::realizeWidget - done.\n" ); 

}

//_____________________________________
EditFrame& MainFrame::newEditFrame( void )
{  
  Debug::Throw( "MainFrame::newEditFrame.\n" );
  EditFrame* out = new EditFrame();
  BASE::Key::associate( this, out );
  return *out;
}

//___________________________________________________________ 
void MainFrame::configuration( void )
{
  Debug::Throw( "MainFrame::configuration.\n" );
  ConfigurationDialog dialog( activeWindow() );
  connect( &dialog, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() ) );
  QtUtil::centerOnParent( &dialog );
  dialog.exec();
  Debug::Throw( "MainFrame::configuration - done.\n" );
}

//_______________________________________________
void MainFrame::spellCheckConfiguration( void )
{
  
  #if WITH_ASPELL

  Debug::Throw( "MainFrame::SpellCheckConfiguration.\n" );
  
  // create dialog
  CustomDialog dialog( activeWindow() );
  
  SpellCheckConfiguration* spell_config = new SpellCheckConfiguration( &dialog );
  dialog.mainLayout().addWidget( spell_config );
  spell_config->read();

  AutoSpellConfiguration* autospell_config = new AutoSpellConfiguration( &dialog );
  dialog.mainLayout().addWidget( autospell_config );
  autospell_config->read();
  
  dialog.adjustSize();
  QtUtil::centerOnParent( &dialog );
  
  if( dialog.exec() == QDialog::Rejected ) return;
  spell_config->write();
  autospell_config->write();
  XmlOptions::write();
  
  emit spellCheckConfigurationChanged();
  
  #endif
  
}

//_______________________________________________
EditFrame* MainFrame::open( FileRecord record )
{
  
  Debug::Throw() << "MainFrame::Open - file: " << record.file() << endl;

  //! set default status to "open"
  open_status_ = OPEN;
  
  // see if file is directory
  if( record.file().isDirectory() )
  {
    
    ostringstream what;
    what << "File \"" << record.file() << "\" is a directory. <Open> canceled.";
    QtUtil::infoDialog( 0, what.str() );
    
    // update open status and exit
    open_status_ = INVALID;
    return 0;
    
  }

  // retrieve all EditFrames
  EditFrame* frame( 0 );
  BASE::KeySet<EditFrame> frames( this );
  
  // try find editor with matching name
  BASE::KeySet<EditFrame>::iterator iter = find_if( frames.begin(), frames.end(), EditFrame::SameFileFTor( record.file() ) );
  if( iter != frames.end() )
  {
    (*iter)->uniconify();
    (*iter)->selectDisplay( record.file() );
    
    // update open status and exit
    open_status_ = OPEN;
    return (*iter);
    
  }
  
  // try find empty editor
  iter = find_if( frames.begin(), frames.end(), EditFrame::EmptyFileFTor() );
  if( iter != frames.end() ) frame = (*iter );

  // if no frame found, create a new one
  if( !frame ) frame = &newEditFrame();

  // check if file exists
  if( record.file().exists() ) frame->setFile( record.file() );
  else if( !record.file().empty() )
  {
  
    frame->show();
    
    // create NewFileDialog
    int buttons( NewFileDialog::CREATE | NewFileDialog::CANCEL );
    bool enable_exit( BASE::KeySet<EditFrame>(this).size() == 1 );
    if( enable_exit ) buttons |= NewFileDialog::EXIT;
    
    NewFileDialog dialog( frame, record.file(), buttons );
    QtUtil::centerOnParent( &dialog );
    int state = dialog.exec();
    
    Debug::Throw() << "MainFrame::Open - New file dialog state: " << state << endl; 
    switch( state )
    {
  
      case NewFileDialog::CREATE:
      {
        File fullname( record.file().expand() );
        if( !fullname.create() )
        {
          ostringstream what;
          what << "Unable to create file " << record.file() << ".";
          QtUtil::infoDialog( frame, what.str() );
          frame->setFile( File("") );
        } else frame->setFile( fullname );
        open_status_ = OPEN;
        break;
      }
  
      case NewFileDialog::CANCEL:
      if( enable_exit ) open_status_ = INVALID;
      else {
        open_status_ = INVALID;
        frame->close();
        return 0;
      }
      
      break; 
  
      case NewFileDialog::EXIT:
      {
        open_status_ = INVALID;
        frame->close();
        if( BASE::KeySet<EditFrame>(this).size() ==1 ) exit();
        return 0;
      }
      
      default: throw runtime_error( DESCRIPTION( "invalid return code" ) );
    }
  } 
  // update frame configuration
  frame->show();
  return frame;

}

//_______________________________________________
void MainFrame::exit( void )
{
  
  Debug::Throw( "MainFrame::exit.\n" );

  // retrieve associated EditFrames
  BASE::KeySet<EditFrame> frames( this );
  if( frames.empty() ) quit();

  // retrieve all modified files and store in a map, together with modification state
  std::map< File, bool > files;
  
  for( BASE::KeySet<EditFrame>::iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
  {
      
    // retrieve associated text displays
    BASE::KeySet<TextDisplay> displays( *frame_iter );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {

      // retrieve file
      // store in map if not empty
      const File& file( (*iter)->file() );
      if( !file.empty() ) files.insert( make_pair( file, (*iter)->document()->isModified() ) );
        
    }
    
  }
  
  // ask for confirmation if more than one file is opened.
  if( files.size() > 1 )
  {
    ExitDialog dialog( activeWindow(), files );
    QtUtil::centerOnParent( &dialog );
    if( !dialog.exec() ) return;
  }
  
  // try close all windows one by one
  for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  {
    
    if( !(*iter)->isModified() ) continue;
    
    // loop over displays
    bool save_all_enabled = count_if( iter, frames.end(), EditFrame::IsModifiedFTor() ) > 1;
    BASE::KeySet<TextDisplay> displays( *iter );
    for( BASE::KeySet<TextDisplay>::iterator display_iter = displays.begin(); display_iter != displays.end(); display_iter++ )
    {
      
      if( !(*display_iter)->document()->isModified() ) continue;
      save_all_enabled |= (*iter)->modifiedDisplayCount() > 1;
      int state( (*display_iter)->askForSave( save_all_enabled ) );
      
      if( state == AskForSaveDialog::CANCEL ) return;
      else if( state == AskForSaveDialog::ALL ) 
      {
        
        // save all displays for this frame, starting from the current
        for(; display_iter != displays.end(); display_iter++ ) 
        { if( (*display_iter)->document()->isModified() ) (*display_iter)->save(); }
       
        // save all editframes starting from the next to this one
        BASE::KeySet<EditFrame>::iterator sub_iter = iter; 
        for( sub_iter++; sub_iter != frames.end(); sub_iter++ ) (*sub_iter)->saveAll();
        
        // break loop since everybody has been saved
        break;
        
      }
      
    }
    
    // try close. Should succeed, otherwise it means there is a flow in the algorithm above
    Exception::check( (*iter)->close(), DESCRIPTION( "close failed.\n" ) );
    
  }

  Debug::Throw( "MainFrame::exit - done.\n" );
  quit();

}

//_______________________________________________
void MainFrame::about( void )
{

  Debug::Throw( "MainFrame::about.\n" );
  ostringstream what;
  what << "<b>QEdit</b> version " << VERSION << " (" << BUILD_TIMESTAMP << ")<br>";
  what 
    << "<p>This application was written for personal use only. "
    << "It is not meant to be bug free, although all efforts "
    << "are made so that it remains/becomes so. "
    
    << "<p>Suggestions, comments and bug reports are welcome. "
    << "Please use the following e-mail address:"

    << "<p><a href=\"mailto:hugo.pereira@free.fr\">hugo.pereira@free.fr</a>";

  QMessageBox dialog;
  dialog.setWindowIcon( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).expand().c_str() ) );
  dialog.setIconPixmap( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).expand().c_str() ) );
  dialog.setText( what.str().c_str() );
  dialog.exec();

}

//_________________________________________________
void MainFrame::updateConfiguration( void )
{

  Debug::Throw( "MainFrame::updateConfiguration.\n" );

  // set fonts
  QFont font;
  font.fromString( XmlOptions::get().raw( "FONT_NAME" ).c_str() );
  setFont( font );

  font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ).c_str() );
  setFont( font, "QLineEdit" );
  setFont( font, "QTextEdit" );
  
  Debug::setLevel( XmlOptions::get().get<bool>("DEBUG_LEVEL") );
  setWindowIcon( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).expand().c_str() ) );
  
  // configuration changed
  emit configurationChanged();
  
  // update document classes
  updateDocumentClasses();
  
  return;
}

//____________________________________________________________
void MainFrame::updateDocumentClasses( void )
{
  Debug::Throw( "MainFrame::updateDocumentClasses.\n" );
  
  // clear document classes
  class_manager_->clear();
  
  // load files from options
  ostringstream what;
  list<string> files( XmlOptions::get().specialOptions<string>( "PATTERN_FILENAME" ) );
  for( list<string>::const_iterator iter = files.begin(); iter != files.end(); iter++ )
  { 
    class_manager_->read( *iter ); 
    what << class_manager_->readError();
  }

  if( !what.str().empty() ) QtUtil::infoDialog( 0, what.str() );
  
  // emit configuratino changed to force displays to be updated
  emit documentClassesChanged();
  
  return;
}

//___________________________________________________________
void MainFrame::multipleFileReplace( std::list<File> files, TextSelection selection )
{
  Debug::Throw( "MainFrame::MultipleFileRepplace.\n" );
    
  // keep track of number of replacements
  unsigned int counts(0);
  
  // retrieve frame associated with file
  BASE::KeySet<EditFrame> frames( this );
  for( list<File>::iterator iter = files.begin(); iter != files.end(); iter++ )
  {
    File& file( *iter );
    
    BASE::KeySet<EditFrame>::iterator iter = find_if( frames.begin(), frames.end(), EditFrame::SameFileFTor( file ) );
    Exception::check( iter != frames.end(), DESCRIPTION( "file not found" ) );

    // retrieve TextDisplay that match file
    BASE::KeySet<TextDisplay> displays( *iter );
    BASE::KeySet<TextDisplay>::iterator display_iter( find_if( displays.begin(), displays.end(), TextDisplay::SameFileFTor( file ) ) );
    Exception::check( display_iter != displays.end(), DESCRIPTION( "file not found" ) );
    
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
  QtUtil::infoDialog( 0, what.str() );
  
  return;
}
  
//___________________________________________________________ 
void MainFrame::_readFilesFromArgs( void )
{
  Debug::Throw( "MainFrame::_readFilesFromArgs.\n" );
    
  // see if open should be performed in Tabbed mode
  bool tabbed( args_.find( "--tabbed" ) );
  bool diff( args_.find( "--diff" ) );
  
  // keep track of opened files
  set<File> files;
  
  // retrieve files from arguments
  ArgList::Arg last_arg( args_.get().back() );
  if( tabbed | diff )
  {
    
    EditFrame *frame( 0 );
    
    // try find empty editor
    BASE::KeySet<EditFrame> frames( this );
    
    // retrieve an empty frame if any, or create one
    BASE::KeySet<EditFrame>::iterator iter = find_if( frames.begin(), frames.end(), EditFrame::EmptyFileFTor() );
    if( iter != frames.end() ) frame = (*iter );
    else frame = &newEditFrame();
    
    // loop over files and open in current frame
    for( list< string >::const_iterator iter = last_arg.options().begin(); iter != last_arg.options().end(); iter++ )
    {
      
      
      File file( *iter );
      if( file.size() ) file = file.expand();
      
      // open in same frame, using default orientation
      if( frame->orientation() == Qt::Horizontal ) frame->openHorizontal( file );
      else frame->openVertical( file );
      files.insert( file );
      
    }
    
    // if diff 
    if( diff && frame )
    { 
      if( files.size() == 2 ) frame->diffAction().trigger(); 
      else if( files.size() > 2 ) QtUtil::infoDialog( 0, "too many files selected. <Diff> canceled." );
      else if( files.size() < 2 ) QtUtil::infoDialog( 0, "too few files selected. <Diff> canceled." );
    }
    
    frame->show();
    
  } else {
    
    for( list< string >::const_iterator iter = last_arg.options().begin(); iter != last_arg.options().end(); iter++ )
    {
      
      File file( *iter );
      if( !file.empty() ) file = file.expand();
      open( file );
      if( open_status_ == EXIT_APP ) return;
      
    }
    
  }
  
  return;
  
}

//________________________________________________
void MainFrame::_applicationManagerStateChanged( SERVER::ApplicationManager::State state )
{

  Debug::Throw() << "MainFrame::_ApplicationManagerStateChanged - state=" << state << endl;

  switch ( state ) {
    case ApplicationManager::ALIVE:
    realizeWidget();
    break;

    case ApplicationManager::DEAD:
    quit();
    break;

    default:
    break;
  }

  return;

}

//________________________________________________
void MainFrame::_processRequest( const ArgList& args )
{
  Debug::Throw() << "MainFrame::_ProcessRequest - " << args << endl;

  // copy arguments and try open (via QTimer)
  args_ = args;
  startup_timer_.start( 100 );
  return;
  
}

//_______________________________________________
void MainFrame::_aboutToQuit( void )
{ Debug::Throw( "MainFrame::_aboutToQuit.\n" ); }
