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
#include "DocumentClassManagerDialog.h"
#include "EditFrame.h"
#include "ErrorHandler.h"
#include "ExitDialog.h"
#include "FlatStyle.h"
#include "IconEngine.h"
#include "Icons.h"
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
using namespace Qt;

//____________________________________________
void MainFrame::usage( void )
{
  cout << "Usage : qedit [options] <file1> <file2> <...>" << endl;
  cout << endl;
  cout << "Options : " << endl;
  cout << "  --help\t\t displays this help and exit" << endl;
  cout << "  --tabbed\t\t opens files in same window" << endl;
  cout << "  --diff\t\t opens files in same window and perform diff" << endl;
  cout << "  --autospell\t\t switch autospell on for all files" << endl;
  cout << "  --filter <filter>\t select filter for autospell" << endl;
  cout << "  --dictionary <dict>\t select dictionary for autospell" << endl;
  cout << "  --close\t\t close displays matching file names and exit" << endl;
  SERVER::ApplicationManager::usage();
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
  if( XmlOptions::get().get<bool>( "USE_FLAT_THEME" ) ) setStyle( new FlatStyle() );

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
  application_manager_ = new SERVER::ApplicationManager( this );
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
  
  // actions  
  about_action_ = new QAction( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).c_str() ), "About &QEdit", 0 );
  connect( about_action_, SIGNAL( triggered() ), SLOT( _about() ) );
   
  // path list for icons
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  aboutqt_action_ = new QAction( IconEngine::get( ICONS::ABOUT_QT, path_list ), "About &Qt", 0 );
  connect( aboutqt_action_, SIGNAL( triggered() ), SLOT( aboutQt() ) ); 

  close_action_ = new QAction( IconEngine::get( ICONS::EXIT, path_list ), "E&xit", 0 );
  close_action_->setShortcut( CTRL+Key_Q );
  connect( close_action_, SIGNAL( triggered() ), SLOT( _exit() ) );

  save_all_action_ = new QAction( IconEngine::get( ICONS::SAVE, path_list ), "Save A&ll", 0 );
  connect( save_all_action_, SIGNAL( triggered() ), SLOT( _saveAll() ) );
  
  configuration_action_ = new QAction( IconEngine::get( ICONS::CONFIGURE, path_list ), "Default &Configuration", 0 );
  connect( configuration_action_, SIGNAL( triggered() ), SLOT( _configuration() ) );

  document_class_configuration_action_ = new QAction( IconEngine::get( ICONS::CONFIGURE, path_list ), "Document Class &Configuration", 0 );
  connect( document_class_configuration_action_, SIGNAL( triggered() ), SLOT( _documentClassConfiguration() ) );
  
  spellcheck_configuration_action_ = new QAction( IconEngine::get( ICONS::CONFIGURE, path_list ), "&Spell-check &Configuration", 0 );
  connect( spellcheck_configuration_action_, SIGNAL( triggered() ), SLOT( _spellCheckConfiguration() ) );

  
  // class manager
  class_manager_ = new DocumentClassManager();
  
  // autosave
  autosave_ = new AutoSave();
  
  // create first editFrame
  newEditFrame().show(); 
  _updateConfiguration();

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

//_______________________________________________
EditFrame* MainFrame::open( FileRecord record, ArgList args )
{
  
  Debug::Throw() << "MainFrame::Open - file: " << record.file() << endl;

  //! see if autospell action is required
  bool autospell( args.find( "--autospell" ) );
  
  //! see if autospell filter and dictionary are required
  string filter = ( args.find( "--filter" ) && !args.get( "--filter" ).options().empty() ) ? args.get( "--filter" ).options().front() : "";
  string dictionary = (args.find( "--dictionary" ) && !args.get( "--dictionary" ).options().empty() ) ? args.get( "--dictionary" ).options().front() : "";
  Debug::Throw() << "MainFrame::open - filter:" << filter << " dictionary: " << dictionary << endl;
  
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
  
    // trigger autospell if required
    if( autospell ) (*iter)->activeDisplay().autoSpellAction().setChecked( true );
    if( !filter.empty() ) (*iter)->activeDisplay().selectFilter( filter );
    if( !dictionary.empty() ) (*iter)->activeDisplay().selectDictionary( filter );
    
    // update open status and exit
    open_status_ = OPEN;
    return (*iter);
    
  }
  
  // try find empty editor
  iter = find_if( frames.begin(), frames.end(), EditFrame::EmptyFileFTor() );
  if( iter != frames.end() ) frame = (*iter );

  // if no frame found, create a new one
  if( !frame )
  {
    frame = &newEditFrame();
    processEvents();
    //QtUtil::centerOnWidget( frame, activeWindow() );
  }
  
  frame->show();

  // check if file exists
  if( record.file().exists() ) frame->setFile( record.file() );
  else if( !record.file().empty() )
  {
  
    processEvents();
    
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
        if( BASE::KeySet<EditFrame>(this).size() ==1 ) _exit();
        return 0;
      }
      
      default: throw runtime_error( DESCRIPTION( "invalid return code" ) );
    }
  } 
  
  // trigger autospell if required
  if( autospell ) frame->activeDisplay().autoSpellAction().setChecked( true );
  if( !filter.empty() ) frame->activeDisplay().selectFilter( filter );
  if( !dictionary.empty() ) frame->activeDisplay().selectDictionary( filter );

  Debug::Throw( "MainFrame::Open - done.\n" );
    
  return frame;

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
  
  // emit configuration changed to force displays to be updated
  emit documentClassesChanged();
  
  return;
}

//___________________________________________________________
void MainFrame::multipleFileReplace( std::list<File> files, TextSelection selection )
{
  Debug::Throw( "MainFrame::multipleFileRepplace.\n" );
    
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
  QtUtil::infoDialog( activePopupWidget(), what.str() );
  
  return;
}


//_______________________________________________
void MainFrame::_about( void )
{

  Debug::Throw( "MainFrame::_about.\n" );
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
  dialog.setWindowIcon( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).c_str() ) );
  dialog.setIconPixmap( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).c_str() ) );
  dialog.setText( what.str().c_str() );
  dialog.adjustSize();
  QtUtil::centerOnWidget( &dialog, activeWindow() );
  dialog.exec();

}

//___________________________________________________________ 
void MainFrame::_configuration( void )
{
  Debug::Throw( "MainFrame::_configuration.\n" );
  ConfigurationDialog dialog( 0 );
  connect( &dialog, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  QtUtil::centerOnWidget( &dialog, activeWindow() );
  dialog.exec();
  Debug::Throw( "MainFrame::configuration - done.\n" );
}

//___________________________________________________________ 
void MainFrame::_documentClassConfiguration( void )
{
  Debug::Throw( "MainFrame::_documentClassConfiguration.\n" );
  DocumentClassManagerDialog dialog( activeWindow(), &classManager() );
  connect( &dialog, SIGNAL( updateNeeded() ), SIGNAL( documentClassesChanged() ) );
  dialog.exec();
}

//_______________________________________________
void MainFrame::_spellCheckConfiguration( void )
{
  
  #if WITH_ASPELL

  Debug::Throw( "MainFrame::_spellCheckConfiguration.\n" );
  
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
void MainFrame::_exit( void )
{
  
  Debug::Throw( "MainFrame::_exit.\n" );

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

  Debug::Throw( "MainFrame::_exit - done.\n" );
  quit();

}

//_______________________________________________
void MainFrame::_saveAll( void )
{
  
  Debug::Throw( "MainFrame::_saveAll.\n" );

  // try save all windows one by one
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

  Debug::Throw( "MainFrame::_saveAll - done.\n" );

}

//_________________________________________________
void MainFrame::_updateConfiguration( void )
{

  Debug::Throw( "MainFrame::_updateConfiguration.\n" );

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

//___________________________________________________________ 
void MainFrame::_readFilesFromArgs( void )
{
  Debug::Throw( "MainFrame::_readFilesFromArgs.\n" );
    
  // retrieve files from arguments
  ArgList::Arg last_arg( args_.get().back() );
  
  // see if tabbed mode
  bool tabbed( args_.find( "--tabbed" ) );
  bool diff( args_.find( "--diff" ) );
  if( tabbed || diff )
  {
    
    EditFrame *frame( 0 );
    
    // try find empty editor
    BASE::KeySet<EditFrame> frames( this );
    
    // retrieve an empty frame if any, or create one
    BASE::KeySet<EditFrame>::iterator iter = find_if( frames.begin(), frames.end(), EditFrame::EmptyFileFTor() );
    if( iter != frames.end() ) frame = (*iter );
    else frame = &newEditFrame();
    
    // loop over files and open in current frame
    set<string> files;
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
    return;
  } 
  
  // close mode
  if( args_.find( "--close" ) )
  {
  
    // loop over files 
    for( list< string >::const_iterator iter = last_arg.options().begin(); iter != last_arg.options().end(); iter++ )
    {
      File file( *iter );

      // retrieve all EditFrames
      BASE::KeySet<EditFrame> frames( this );
      BASE::KeySet<EditFrame>::iterator frame_iter = find_if( frames.begin(), frames.end(), EditFrame::SameFileFTor( file ) );
      if( frame_iter == frames.end() ) continue;
      
      // select display and close
      (*frame_iter)->selectDisplay( file );
      (*frame_iter)->closeViewAction().trigger();
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
    open( file, args_ );
    if( open_status_ == EXIT_APP ) return;
    
  }
      
  Debug::Throw( "MainFrame::_readFilesFromArgs - done.\n" );
  return;
  
}

//________________________________________________
void MainFrame::_applicationManagerStateChanged( SERVER::ApplicationManager::State state )
{

  Debug::Throw() << "MainFrame::_ApplicationManagerStateChanged - state=" << state << endl;

  switch ( state ) {
    case SERVER::ApplicationManager::ALIVE:
    realizeWidget();
    break;

    case SERVER::ApplicationManager::DEAD:
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
