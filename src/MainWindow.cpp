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
#include "CustomToolButton.h"
#include "LineEditor.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "Diff.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "FileSelectionDialog.h"
#include "FileSystemFrame.h"
#include "FindDialog.h"
#include "HighlightBlockFlags.h"
#include "IconEngine.h"
#include "Icons.h"
#include "MainWindow.h"
#include "Menu.h"
#include "NavigationFrame.h"
#include "NavigationToolBar.h"
#include "NewFileDialog.h"
#include "PixmapEngine.h"
#include "PrintDialog.h"
#include "QtUtil.h"
#include "InformationDialog.h"
#include "QuestionDialog.h"
#include "RecentFilesFrame.h"
#include "ReplaceDialog.h"
#include "SelectLineDialog.h"
#include "SessionFilesFrame.h"
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
const std::string MainWindow::LEFT_RIGHT = "left/right";
const std::string MainWindow::TOP_BOTTOM = "top/bottom";

//_____________________________________________________
MainWindow::MainWindow(  QWidget* parent ):
  CustomMainWindow( parent ),
  Counter( "MainWindow" ),
  menu_( 0 ),
  statusbar_( 0 ),
  file_editor_( 0 ),
  find_dialog_( 0 ),
  replace_dialog_( 0 ),
  select_line_dialog_( 0 ),
  default_orientation_( Qt::Horizontal )
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
  QSplitter* splitter = new QSplitter( this );
  splitter->setOrientation( Qt::Horizontal );
  setCentralWidget( splitter );
  
  // insert navigationFrame
  navigation_frame_ = new NavigationFrame(0, static_cast<Application*>(qApp)->recentFiles() );
  navigationFrame().setDefaultWidth( XmlOptions::get().get<int>( "NAVIGATION_FRAME_WIDTH" ) );
  splitter->addWidget( &navigationFrame() );

  connect( &navigationFrame().visibilityAction(), SIGNAL( toggled( bool ) ), SLOT( _toggleNavigationFrame( bool ) ) );
    
  // need to add navigationFrame visibility action to this list 
  // to enable shortcut event if the frame is hidden
  addAction( &navigationFrame().visibilityAction() );
  connect( &navigationFrame().sessionFilesFrame(), SIGNAL( fileSelected( FileRecord ) ), SLOT( _selectDisplay( FileRecord ) ) );
  connect( &navigationFrame().recentFilesFrame(), SIGNAL( fileSelected( FileRecord ) ), SLOT( _selectDisplay( FileRecord ) ) );
  
  // insert stack widget
  splitter->addWidget( stack_ = new QStackedWidget(0) );
  stack_->layout()->setMargin(0);
  
  connect( &_stack(), SIGNAL( widgetRemoved( int ) ), SLOT( _activeViewChanged() ) );
  
  // create first text view
  newTextView();
  
  // assign stretch factors
  splitter->setStretchFactor( 0, 0 );
  splitter->setStretchFactor( 1, 1 );
  
  connect( splitter, SIGNAL( splitterMoved( int, int ) ), SLOT( _splitterMoved( void ) ) );
  
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
  
  // navigation toolbar
  NavigationToolBar* navigation_toolbar = new NavigationToolBar( this ); 
  navigation_toolbar->connect( navigationFrame() );
  
  //! configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( saveConfiguration() ), SLOT( _saveConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();
  
  // update buttons
  Debug::Throw( "MainWindow::MainWindow - done.\n" );
 
}

//___________________________________________________________
MainWindow::~MainWindow( void )
{ Debug::Throw( "MainWindow::~MainWindow.\n" ); }

//___________________________________________________________
TextView& MainWindow::newTextView( void )
{ 
  Debug::Throw( "MainWindow::newTextView.\n" );

  // create new view and add to this file
  TextView* view = new TextView( this );
  BASE::Key::associate( this, view );

  // add to stack and set active
  _stack().addWidget( view );
  setActiveView( *view );
  
  // connections
  _connectView( *view );
  
  return *view;
  
}

//_____________________________________________________________________
void MainWindow::setActiveView( TextView& view )
{ 

  Debug::Throw() << "MainWindow::setActiveView - key: " << view.key() << endl;

  // do nothing if active view did not change
  if( active_view_ == &view ) return;
  
  // this check is needed because the active view passed as argument
  // might be closing and have no associated display
  if( BASE::KeySet<TextDisplay>( &view ).empty() ) return;

  // store active view
  active_view_ = &view; 
  activeView().activeDisplay().setFocus();
  
  // update stack if needed
  if( _stack().currentWidget() !=  &activeView() ) 
  { _stack().setCurrentWidget( &activeView() ); }

  // update displays, actions, etc.
  if( activeView().activeDisplay().file().size() || activeView().activeDisplay().isNewDocument() )
  { _update( TextDisplay::ACTIVE_DISPLAY_CHANGED ); }

}


//_____________________________________________________________________
BASE::KeySet<TextDisplay> MainWindow::associatedDisplays( void ) const
{
  BASE::KeySet<TextDisplay> displays;
  BASE::KeySet<TextView> views( this );
  for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
  { 
    BASE::KeySet<TextDisplay> view_displays(*iter);
    displays.insert( view_displays.begin(), view_displays.end() );
  }
  
  return displays;
  
}

//_____________________________________________________________________
bool MainWindow::selectDisplay( const File& file )
{
    
  Debug::Throw() << "MainWindow::selectDisplay - file: " << file << endl;
  
  // do nothing if already selected
  if( activeView().activeDisplay().file() == file ) return true;
  
  BASE::KeySet<TextView> views( this );
  for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
  { 
    
    if( (*iter)->selectDisplay( file ) ) 
    {
      // make sure selected view is visible
      if( _stack().currentWidget() != *iter )
      { setActiveView( **iter ); }
      return true; 
    }
    
  }
  
  return false;
}
  
//_____________________________________________________________________
void MainWindow::saveAll( void )
{
  Debug::Throw( "MainWindow::saveAll.\n" );
  BASE::KeySet<TextView> views( this );
  for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
  { (*iter)->saveAll(); }
}

//_____________________________________________________________________
void MainWindow::ignoreAll( void )
{
  Debug::Throw( "MainWindow::ignoreAll.\n" );
  BASE::KeySet<TextView> views( this );
  for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
  { (*iter)->ignoreAll(); }
}

//_____________________________________________________________________
void MainWindow::findFromDialog( void )
{
  Debug::Throw( "MainWindow::findFromDialog.\n" );

  // create
  if( !find_dialog_ ) _createFindDialog();

  // enable/disable regexp
  _findDialog().enableRegExp( true );

  // raise dialog
  _findDialog().centerOnParent().show();

  /*
    setting the default text values
    must be done after the dialog is shown
    otherwise it may be automatically resized
    to very large sizes due to the input text
  */

  // set default string to find
  _findDialog().synchronize();
  _findDialog().clearLabel();

  // set default text
  QString text;
  if( !( text = qApp->clipboard()->text( QClipboard::Selection) ).isEmpty() ) _findDialog().setText( text );
  else if( activeDisplay().textCursor().hasSelection() ) _findDialog().setText( activeDisplay().textCursor().selectedText() );
  else if( !( text = TextDisplay::lastSelection().text() ).isEmpty() ) _findDialog().setText( text );

  // changes focus
  _findDialog().activateWindow();
  _findDialog().editor().setFocus();

  return;
}

//_____________________________________________________________________
void MainWindow::replaceFromDialog( void )
{
  Debug::Throw( "MainWindow::replaceFromDialog.\n" );

  // create
  if( !replace_dialog_ ) _createReplaceDialog();

  // raise dialog
  _replaceDialog().centerOnParent().show();

  /*
    setting the default text values
    must be done after the dialog is shown
    otherwise it may be automatically resized
    to very large sizes due to the input text
  */

  // synchronize combo-boxes
  _replaceDialog().synchronize();
  _replaceDialog().clearLabel();

  // update find text
  QString text;
  if( !( text = qApp->clipboard()->text( QClipboard::Selection) ).isEmpty() ) _replaceDialog().setText( text );
  else if( activeDisplay().textCursor().hasSelection() ) _replaceDialog().setText( activeDisplay().textCursor().selectedText() );
  else if( !( text = TextDisplay::lastSelection().text() ).isEmpty() ) _replaceDialog().setText( text );

  // update replace text
  if( !TextDisplay::lastSelection().replaceText().isEmpty() ) _replaceDialog().setReplaceText( TextDisplay::lastSelection().replaceText() );

  // changes focus
  _replaceDialog().activateWindow();
  _replaceDialog().editor().setFocus();

  return;
}

//________________________________________________
void MainWindow::selectLineFromDialog( void )
{

  Debug::Throw( "TextEditor::selectLineFromDialog.\n" );
  if( !select_line_dialog_ )
  {
    select_line_dialog_ = new SelectLineDialog( this );
    connect( select_line_dialog_, SIGNAL( lineSelected( int ) ), SLOT( _selectLine( int ) ) );
  }

  select_line_dialog_->editor().clear();
  select_line_dialog_->centerOnParent().show();
  select_line_dialog_->activateWindow();
  select_line_dialog_->editor().setFocus();

}

//___________________________________________________________
void MainWindow::_revertToSave( void )
{
  
  Debug::Throw( "MainWindow::_revertToSave.\n" );

  // check filename
  if( activeDisplay().file().empty() || activeDisplay().isNewDocument() )
  {
    InformationDialog( this, "No filename given. <Revert to save> canceled." ).exec();
    return;
  }

  // ask for confirmation
  ostringstream what;
  if( activeDisplay().document()->isModified() ) what << "Discard changes to " << activeDisplay().file().localName() << "?";
  else what << "Reload file " << activeDisplay().file().localName() << "?";
  if( !QuestionDialog( this, what.str().c_str() ).exec() ) return;

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
    InformationDialog( this, "File is not valid for printing. <print> canceled." ).exec();
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
  if( !dialog.centerOnParent().exec() ) return;

  // store options
  XmlOptions::get().set<string>( "PRINT_MODE", dialog.mode() == PrintDialog::PDF ? "PDF":"HTML" );
  XmlOptions::get().set<int>("PRINT_LINE_SIZE", dialog.maximumLineSize() );
  XmlOptions::get().set<bool>( "USE_PRINT_COMMAND", dialog.useCommand() );
  list<string> new_commands( dialog.commands() );
  for( list<string>::iterator iter = new_commands.begin(); iter != new_commands.end(); iter++ )
  { 
    
    if( std::find( commands.begin(), commands.end(), *iter ) == commands.end() ) 
    { XmlOptions::get().add( "PRINT_COMMAND", *iter ); }
    
  }
  
  Debug::Throw( "MainWindow::_print - options saved.\n" );
  
  // try open output file
  File fullname = File( qPrintable( dialog.destinationFile() ) ).expand();
 
  // check if file is directory
  if( fullname.isDirectory() )
  {
    ostringstream what;
    what << "file \"" << fullname << "\" is a directory. <Print> canceled.";
    InformationDialog( this, what.str().c_str(), BaseDialog::CENTER_ON_PARENT ).exec();
    return;
  }


  // check if file exists
  if( fullname.exists() )
  {
    if( !fullname.isWritable() )
    {
      ostringstream what;
      what << "file \"" << fullname << "\" is read-only. <Print> canceled.";
      InformationDialog( this, what.str().c_str(), BaseDialog::CENTER_ON_PARENT ).exec();
      return;
    } else if( !QuestionDialog( this, "selected file already exists. Overwrite ?", BaseDialog::CENTER_ON_PARENT ).exec() )
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
      InformationDialog( this, what.str().c_str() ).exec();
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

//_______________________________________________________
bool MainWindow::event( QEvent* event )
{
    
  // check that all needed widgets/actions are valid and checked.
  switch (event->type()) 
  {
          
    case QEvent::WindowActivate:
    emit activated( this );
    break;
    
    default: break;
  }
  
  return CustomMainWindow::event( event );
  
}

//____________________________________________
void MainWindow::closeEvent( QCloseEvent* event )
{
  Debug::Throw( "MainWindow::closeEvent.\n" );

  // accept event
  event->accept();
      
  // look over TextDisplays
  if( !isModified() ) return;
  
  // loop over TextViews
  unsigned int modified_displays(0);
  BASE::KeySet<TextDisplay> displays;
  BASE::KeySet<TextView> views( this );
  for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
  {
    
    // update the number of modified displays
    modified_displays += (*iter)->modifiedDisplayCount();

    // store associated textDisplays in main set
    BASE::KeySet<TextDisplay> view_displays( *iter );
    displays.insert( view_displays.begin(), view_displays.end() );
    
  }
  
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {

    // get local reference to display
    TextDisplay& display( **iter );
        
    // check if this display is modified
    if( !display.document()->isModified() ) continue;
    
    // this trick allow to run  only once per set of associated displays
    if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( &display ) ) != iter ) continue;
    
    // ask for save
    int state( display.askForSave( modified_displays > 1 ) );
    if( state == AskForSaveDialog::YES_TO_ALL ) saveAll();
    else if( state == AskForSaveDialog::NO_TO_ALL ) ignoreAll();
    else if( state == AskForSaveDialog::CANCEL ) {
      
      event->ignore();
      return;
      
    }

  }
  
  return;
}

//_______________________________________________________
void MainWindow::timerEvent( QTimerEvent* event )
{

  if( event->timerId() == resize_timer_.timerId() )
  {
    
    // stop timer
    resize_timer_.stop();
    
    // save size
    if( navigationFrame().visibilityAction().isChecked() )
    { XmlOptions::get().set<int>( "NAVIGATION_FRAME_WIDTH", navigationFrame().width() ); }
  
  } else return CustomMainWindow::timerEvent( event );
  
}
  
//________________________________________________________
void MainWindow::_updateConfiguration( void )
{
  
  Debug::Throw( "MainWindow::_updateConfiguration.\n" );
      
  resize( sizeHint() );
  
  // navigation frame visibility
  navigationFrame().visibilityAction().setChecked( XmlOptions::get().get<bool>("SHOW_NAVIGATION_FRAME") );
    
  // assign icons to file in open previous menu based on class manager
  FileList& recent_files( static_cast<Application*>(qApp)->recentFiles() );
  DocumentClassManager& class_manager(static_cast<Application*>(qApp)->classManager()); 
  FileRecord::List records( recent_files.records() );
  for( FileRecord::List::iterator iter = records.begin(); iter != records.end(); iter++ )
  {
    
    FileRecord& record( *iter ); 
    if( !record.hasProperty( FileRecordProperties::CLASS_NAME ) ) continue; 
    DocumentClass document_class( class_manager.get( record.property( FileRecordProperties::CLASS_NAME ).c_str() ) );
    if( document_class.icon().isEmpty() ) continue;
    
    // set icon property and store in recent_files list
    recent_files.get( record.file() ).addProperty( FileRecordProperties::ICON, qPrintable( document_class.icon() ) );
  
  }
    
  // default orientation
  _setOrientation( XmlOptions::get().raw( "ORIENTATION" ) == LEFT_RIGHT ? Qt::Horizontal : Qt::Vertical );
  
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
void MainWindow::_splitterMoved( void )
{
  Debug::Throw( "MainWindow::_splitterMoved.\n" );
  resize_timer_.start( 200, this );  
}

//________________________________________________________
void MainWindow::_activeViewChanged( void )
{
  
  Debug::Throw() << "MainWindow::_activeViewChanged" << endl;

  QWidget *widget( _stack().currentWidget() );
  if( !widget ) close();
  else setActiveView( *static_cast<TextView*>( widget ) );

}

//_______________________________________________________
void MainWindow::_multipleFileReplace( void )
{
  Debug::Throw( "MainWindow::_multipleFileReplace.\n" );
  TextSelection selection( _replaceDialog().selection( false ) );

  // retrieve selection from replace dialog
  FileSelectionDialog dialog( this, selection );
  connect( &dialog, SIGNAL( fileSelected( std::list<File>, TextSelection ) ), &static_cast<Application*>(qApp)->windowServer(), SLOT( multipleFileReplace( std::list<File>, TextSelection ) ) );
  QtUtil::centerOnParent( &dialog );
  dialog.exec();
  return;
}

//_______________________________________________________
void MainWindow::_update( unsigned int flags )
{

  Debug::Throw() << "MainWindow::_update - flags: " << flags << endl;

  if( flags & ( TextDisplay::FILE_NAME | TextDisplay::READ_ONLY | TextDisplay::MODIFIED ) )
  { _updateWindowTitle(); }
    
  if( flags & TextDisplay::MODIFIED )
  { emit modificationChanged(); }

  if( flags & TextDisplay::FILE_NAME )
  { 
    
    // update file editor
    if( file_editor_ )
    {
      file_editor_->setText( activeDisplay().file().c_str() ); 
      fileInfoAction().setEnabled( !( activeDisplay().file().empty() || activeDisplay().isNewDocument() ) );
    }
    
    // update session file frame
    if( navigation_frame_ )
    { 
      navigationFrame().sessionFilesFrame().select( activeDisplay().file() ); 
      navigationFrame().recentFilesFrame().select( activeDisplay().file() ); 
      navigationFrame().fileSystemFrame().setHome( activeDisplay().workingDirectory() );
    }
    
    // cursor position
    if( statusbar_ ) _updateCursorPosition();
    
  }

  if( flags & (TextDisplay::CUT|TextDisplay::READ_ONLY) )
  { cutAction().setEnabled( activeDisplay().cutAction().isEnabled() ); }

  if( flags & TextDisplay::COPY )
  { copyAction().setEnabled( activeDisplay().copyAction().isEnabled() ); }

  if( flags & (TextDisplay::CUT|TextDisplay::READ_ONLY) )
  { pasteAction().setEnabled( activeDisplay().pasteAction().isEnabled() ); }

  if( flags & (TextDisplay::UNDO_REDO|TextDisplay::READ_ONLY) )
  {
    undoAction().setEnabled( activeDisplay().undoAction().isEnabled() );
    redoAction().setEnabled( activeDisplay().redoAction().isEnabled() );
  }
 
  if( flags & TextDisplay::READ_ONLY )
  { saveAction().setEnabled( !activeDisplay().isReadOnly() ); }

  if( statusbar_ && flags & TextDisplay::OVERWRITE_MODE )
  { statusbar_->label(0).setText( activeDisplay().overwriteMode() ? "INS":"" ); }
  
  if( flags & TextDisplay::DISPLAY_COUNT )
  {
    int display_count = activeView().independentDisplayCount();
    int view_count = BASE::KeySet<TextView>( this ).size();

    // update detach action
    detachAction().setEnabled( display_count > 1 || view_count > 1 );
    
    // update diff action
    diffAction().setEnabled( display_count == 2 );    
    
  }
  
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

  addAction( clone_action_ = new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), "&Clone", this ) );
  clone_action_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_N );
  clone_action_->setToolTip( "Clone current display" );
  connect( clone_action_, SIGNAL( triggered() ), SLOT( _splitDisplay() ) );

  addAction( detach_action_ = new QAction( IconEngine::get( ICONS::VIEW_DETACH ), "&Detach", this ) );
  detach_action_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_O );
  detach_action_->setToolTip( "Detach current display" );
  detach_action_->setEnabled( false );

  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open", this ) );
  open_action_->setShortcut( Qt::CTRL+Qt::Key_O );
  open_action_->setToolTip( "Open an existing file" );
  
  addAction( open_horizontal_action_ =new QAction( IconEngine::get( ICONS::VIEW_BOTTOM ), "Clone display top/bottom", this ) );
  open_horizontal_action_->setToolTip( "Open a new display vertically" );

  addAction( open_vertical_action_ =new QAction( IconEngine::get( ICONS::VIEW_RIGHT ), "Open display left/right", this ) );
  open_vertical_action_->setToolTip( "Open a new display horizontally" );
  
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
  
}

//______________________________________________________________________
void MainWindow::_createFindDialog( void )
{

  Debug::Throw( "MainWindow::_createFindDialog.\n" );
  if( !find_dialog_ )
  {

    find_dialog_ = new FindDialog( this );
    find_dialog_->polish();

    connect( find_dialog_, SIGNAL( find( TextSelection ) ), SLOT( _find( TextSelection ) ) );
    connect( this, SIGNAL( noMatchFound() ), find_dialog_, SLOT( noMatchFound() ) );
    connect( this, SIGNAL( matchFound() ), find_dialog_, SLOT( clearLabel() ) );

  }

  return;

}

//_____________________________________________________________________
void MainWindow::_createReplaceDialog( void )
{
  Debug::Throw( "MainWindow::_CreateReplaceDialog.\n" );
  if( !replace_dialog_ )
  {

    replace_dialog_ = new ReplaceDialog( this );
    replace_dialog_->polish();

    connect( replace_dialog_, SIGNAL( find( TextSelection ) ), SLOT( _find( TextSelection ) ) );
    connect( replace_dialog_, SIGNAL( replace( TextSelection ) ), SLOT( _replace( TextSelection ) ) );
    connect( replace_dialog_, SIGNAL( replaceInWindow( TextSelection ) ), SLOT( _replaceInWindow( TextSelection ) ) );
    connect( replace_dialog_, SIGNAL( replaceInSelection( TextSelection ) ), SLOT( _replaceInSelection( TextSelection ) ) );
    connect( this, SIGNAL( noMatchFound() ), replace_dialog_, SLOT( noMatchFound() ) );
    connect( this, SIGNAL( matchFound() ), replace_dialog_, SLOT( clearLabel() ) );

    // insert multiple file buttons
    QPushButton* button = new QPushButton( "&Files", replace_dialog_ );
    connect( button, SIGNAL( clicked() ), SLOT( _multipleFileReplace() ) );
    button->setToolTip( "replace all occurence of the search string in the selected files" );
    replace_dialog_->addDisabledButton( button );
    replace_dialog_->locationLayout().addWidget( button );

  }
  
}

//_________________________________________________________________
void MainWindow::_connectView( TextView& view )
{
  Debug::Throw( "MainWindow::_connectView.\n" );
  connect( &view, SIGNAL( overwriteModeChanged() ), SLOT( _updateOverwriteMode() ) );
  connect( &view, SIGNAL( needUpdate( unsigned int ) ), SLOT( _update( unsigned int ) ) );
  connect( &view, SIGNAL( displayCountChanged( void ) ), SLOT( _updateDisplayCount( void ) ) );
  connect( &view, SIGNAL( displayCountChanged( void ) ), &static_cast<Application*>(qApp)->windowServer(), SIGNAL( sessionFilesChanged( void ) ) );
  connect( &view, SIGNAL( undoAvailable( bool ) ), &undoAction(), SLOT( setEnabled( bool ) ) );
  connect( &view, SIGNAL( redoAvailable( bool ) ), &redoAction(), SLOT( setEnabled( bool ) ) ); 
  connect( &view.positionTimer(), SIGNAL( timeout() ), SLOT( _updateCursorPosition() ) );  
}

//___________________________________________________________
void MainWindow::_updateWindowTitle()
{ 
  Debug::Throw( "MainWindow::_updateWindowTitle.\n" );
  
  {
    bool readonly( activeDisplay().isReadOnly() );
    bool modified( activeDisplay().document()->isModified() );
    Debug::Throw() << "MainWindow::_updateWindowTitle -"
      << " file: " << activeDisplay().file()
      << " readonly: " << readonly 
      << " modified: " << modified
      << endl;
  }
  
  setWindowTitle( WindowTitle( activeDisplay().file() )
    .setReadOnly( activeDisplay().isReadOnly() )
    .setModified( activeDisplay().document()->isModified() )
    );
  
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
  meta.setAttribute( "content", "text/html; charset=iso-8859-1" );
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
  //QString html_string( document.toString(0) );
  QString html_string( document.toString(0) );
  html_string = html_string.replace( "</span>\n", "</span>" );
  html_string = html_string.replace( "<br/>", "" );
  return html_string;
}
