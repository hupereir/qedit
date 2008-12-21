// $Id$

/******************************************************************************
*
* this is free software; you can redistribute it and/or modify it under the
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
*******************************************************************************/

/*!
  \file TextDisplay.cpp
  \brief text display window
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QMenu>
#include <QScrollBar>
#include <QTextLayout>

#include "Application.h"
#include "AutoSave.h"
#include "AutoSaveThread.h"
#include "BlockDelimiterDisplay.h"
#include "CustomFileDialog.h"
#include "CustomTextDocument.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "FileInformationDialog.h"
#include "FileModifiedDialog.h"
#include "FileRecordProperties.h"
#include "FileRemovedDialog.h"
#include "GridLayout.h"
#include "HtmlTextNode.h"
#include "HighlightBlockData.h"
#include "HighlightBlockFlags.h"
#include "IconEngine.h"
#include "Icons.h"
#include "InformationDialog.h"
#include "LineNumberDisplay.h"
#include "QuestionDialog.h"
#include "QtUtil.h"
#include "Singleton.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "TextSeparator.h"
#include "Util.h"
#include "XmlOptions.h"

#include "Config.h"

#if WITH_ASPELL
#include "SpellDialog.h"
#include "SuggestionMenu.h"
#endif

using namespace std;

//___________________________________________________
NewDocumentNameServer& TextDisplay::newDocumentNameServer( void )
{
  static NewDocumentNameServer server;
  return server;
}

//___________________________________________________
QRegExp& TextDisplay::_emptyLineRegExp( void )
{
  static QRegExp regexp( "(^\\s*$)" );
  return regexp;
}

//___________________________________________________
TextDisplay::TextDisplay( QWidget* parent ):
  TextEditor( parent ),
  file_( "" ),
  working_directory_( Util::workingDirectory() ),
  
  // store property ids associated to property names
  // this is used to speed-up fileRecord access
  class_name_property_id_( FileRecord::PropertyId::get( FileRecordProperties::CLASS_NAME ) ),
  icon_property_id_( FileRecord::PropertyId::get( FileRecordProperties::ICON ) ),
  wrap_property_id_( FileRecord::PropertyId::get( FileRecordProperties::WRAPPED ) ),
  dictionary_property_id_( FileRecord::PropertyId::get( FileRecordProperties::DICTIONARY ) ),
  filter_property_id_( FileRecord::PropertyId::get( FileRecordProperties::FILTER ) ),
  
  is_new_document_( false ),
  class_name_( "" ),
  ignore_warnings_( false ),
  show_block_delimiter_action_( 0 ),
  text_highlight_( 0 ),
  block_delimiter_display_( 0 )
{

  Debug::Throw("TextDisplay::TextDisplay.\n" );

  // disable rich text
  setAcceptRichText( false );

  // text highlight
  text_highlight_ = new TextHighlight( document() );

  // parenthesis highlight
  parenthesis_highlight_ = new ParenthesisHighlight( this );

  // text indent
  indent_ = new TextIndent( this );

  // block delimiter
  block_delimiter_display_ = new BlockDelimiterDisplay( this );
  
  // connections
  connect( this, SIGNAL( selectionChanged() ), SLOT( _selectionChanged() ) );
  connect( this, SIGNAL( cursorPositionChanged() ), SLOT( _highlightParenthesis() ) );
  connect( this, SIGNAL( indent( QTextBlock, bool ) ), indent_, SLOT( indent( QTextBlock, bool ) ) );
  connect( this, SIGNAL( indent( QTextBlock, QTextBlock ) ), indent_, SLOT( indent( QTextBlock, QTextBlock ) ) );

  #if WITH_ASPELL
  
  // install menus
  filter_menu_ = new SPELLCHECK::FilterMenu( this );
  dictionary_menu_ = new SPELLCHECK::DictionaryMenu( this );
  
  #endif

  // actions
  _installActions();

  // connections
  // track contents changed for syntax highlighting
  connect( TextDisplay::document(), SIGNAL( contentsChange( int, int, int ) ), SLOT( _setBlockModified( int, int, int ) ) );
  connect( TextDisplay::document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( void ) ) );

  // track configuration modifications
  connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( Singleton::get().application(), SIGNAL( spellCheckConfigurationChanged() ), SLOT( _updateSpellCheckConfiguration() ) );
  connect( Singleton::get().application(), SIGNAL( documentClassesChanged() ), SLOT( updateDocumentClass() ) );
  _updateConfiguration();
  _updateSpellCheckConfiguration();

  Debug::Throw( "TextDisplay::TextDisplay - done.\n" );

}

//_____________________________________________________
TextDisplay::~TextDisplay( void )
{ Debug::Throw() << "TextDisplay::~TextDisplay - key: " << key() << endl; }

//_____________________________________________________
int TextDisplay::blockCount( const QTextBlock& block ) const
{

  QTextBlockFormat block_format( block.blockFormat() );
  if( block_format.boolProperty( TextBlock::Collapsed ) && block_format.hasProperty( TextBlock::CollapsedData ) )
  {  return block_format.property( TextBlock::CollapsedData ).value<CollapsedBlockData>().blockCount(); }
  else return TextEditor::blockCount( block );
  
}

//_____________________________________________________
void TextDisplay::setModified( const bool& value )
{

  Debug::Throw() << "TextDisplay::setModified - value: " << value << endl;

  // do nothing if state is unchanged
  if( value == document()->isModified() )
  {
    Debug::Throw( "TextDisplay::setModified - unchanged.\n" );
    return;
  }

  if( value && isReadOnly() )
  {
    Debug::Throw( "TextDisplay::setModified - rejected.\n" );
    return;
  }

  document()->setModified( value );

  // ask for update in the parent frame
  if( isActive() && ( file().size() || isNewDocument() ) ) emit needUpdate( MODIFIED );

}

//_____________________________________________________
void TextDisplay::setReadOnly( const bool& value )
{
  Debug::Throw() << "TextDisplay::setReadOnly - value: " << value << endl;
  
  bool changed = (value != isReadOnly() );
  TextEditor::setReadOnly( value );
  
  if( changed && isActive() ) emit needUpdate( READ_ONLY );
}


//______________________________________________________________________________
void TextDisplay::installContextMenuActions( QMenu& menu, const bool& all_actions )
{
  
  Debug::Throw( "TextDisplay::installContextMenuActions.\n" );

  // see if tagged blocks are present
  bool has_tags( hasTaggedBlocks() );
  bool has_selection( textCursor().hasSelection() );
  bool current_block_tagged( has_tags && isCurrentBlockTagged() );

  // retrieve default context menu
  // second argument is to remove un-necessary actions
  // TextEditor::installContextMenuActions( menu, all_actions );
  TextEditor::installContextMenuActions( menu, false );
  
  // add specific actions
  menu.insertAction( &wrapModeAction(), &showBlockDelimiterAction() );
  menu.addSeparator();
  
  // tags submenu
  QMenu* submenu = menu.addMenu( "&Tags" );
    
  submenu->addAction( &tagBlockAction() );    
  submenu->addAction( &nextTagAction() );    
  submenu->addAction( &previousTagAction() );
  submenu->addAction( &clearTagAction() );
  submenu->addAction( &clearAllTagsAction() );
  
  tagBlockAction().setEnabled( has_selection );
  nextTagAction().setEnabled( has_tags );
  previousTagAction().setEnabled( has_tags );
  clearTagAction().setEnabled( current_block_tagged );
  clearAllTagsAction().setEnabled( has_tags );
  
  return;
}

//___________________________________________________________________________
void TextDisplay::synchronize( TextDisplay* display )
{

  Debug::Throw( "TextDisplay::synchronize.\n" );

  // replace base class syntax highlighter prior to calling base class synchronization
  text_highlight_ = &display->textHighlight();

  /* this avoids calling to invalid block of memory which the textHighlight gets deleted
  when changing the document */

  // base class synchronization
  TextEditor::synchronize( display );

  // restore connection with document
  // track contents changed for syntax highlighting
  connect( TextDisplay::document(), SIGNAL( contentsChange( int, int, int ) ), SLOT( _setBlockModified( int, int, int ) ) );
  connect( TextDisplay::document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( void ) ) );

  // indentation
  textIndent().setPatterns( display->textIndent().patterns() );
  textIndent().setBaseIndentation( display->textIndent().baseIndentation() );

  // parenthesis
  parenthesisHighlight().synchronize( display->parenthesisHighlight() );

  // block delimiters and line numbers
  blockDelimiterDisplay().synchronize( &display->blockDelimiterDisplay() );
  
  // actions
  textIndentAction().setChecked( display->textIndentAction().isChecked() );
  textHighlightAction().setChecked( display->textHighlightAction().isChecked() );
  parenthesisHighlightAction().setChecked( display->parenthesisHighlightAction().isChecked() );
  showLineNumberAction().setChecked( display->showLineNumberAction().isChecked() );
  showBlockDelimiterAction().setChecked( display->showBlockDelimiterAction().isChecked() );

  // macros
  _setMacros( display->macros() );

  // file
  _setFile( display->file() );
  _setIsNewDocument( display->isNewDocument() );
}

//____________________________________________
void TextDisplay::setIsNewDocument( void )
{

  Debug::Throw( "TextDisplay::setIsNewDocument.\n" );
  
  // do nothing if already set
  if( isNewDocument() ) return;
  
  // generate filename
  File file( NewDocumentNameServer().get() );
  
  // retrieve display and associated
  BASE::KeySet<TextDisplay> displays( this );
  displays.insert( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    
    (*iter)->_setIsNewDocument( true );
    (*iter)->setClassName( className() );
    (*iter)->_updateDocumentClass( File(), true );
    (*iter)->_updateSpellCheckConfiguration();
    (*iter)->_setFile( file );

    // disable file info action
    (*iter)->filePropertiesAction().setEnabled( false );

  }
      
  // perform first autosave
  Application& application( *Singleton::get().application<Application>() );
  application.autoSave().saveFiles( this );
  
}

//____________________________________________
void TextDisplay::setFile( File file, bool check_autosave )
{

  Debug::Throw() << "TextDisplay::setFile " << file << endl;
  assert( !file.empty() );

  // reset class name
  QString class_name( _recentFiles().add( file ).property(class_name_property_id_).c_str() );
  setClassName( class_name );

  // expand filename
  file = file.expand();

  // check is there is an "AutoSave" file matching with more recent modification time
  // here, when the diff is working, I could offer the possibility to show a diff between
  // the saved file and the backup
  bool restore_autosave( false );
  File tmp( file );

  File autosaved( AutoSaveThread::autoSaveName( tmp ) );
  if( check_autosave && autosaved.exists() &&
    ( !tmp.exists() ||
    ( autosaved.lastModified() > tmp.lastModified() && tmp.diff(autosaved) ) ) )
  {
    ostringstream what;
    what << "A more recent version of file " << file << endl;
    what << "was found at " << autosaved << "." << endl;
    what << "This probably means that the application crashed the last time ";
    what << "the file was edited." << endl;
    what << "Use autosaved version ?";
    if( QuestionDialog( this, what.str().c_str() ).exec() )
    {
      restore_autosave = true;
      tmp = autosaved;
    }
  }
   
  // remove new document version from name server
  if( isNewDocument() ) { NewDocumentNameServer().remove( TextDisplay::file() ); }

  // retrieve display and associated, update document class
  // this is needed to avoid highlight glitch when oppening file
  BASE::KeySet<TextDisplay> displays( this );
  displays.insert( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    
    (*iter)->setClassName( className() );
    (*iter)->_updateDocumentClass( file, false );    
    (*iter)->_updateSpellCheckConfiguration( file );
    
  }
  
  // check file and try open.
  QFile in( tmp.c_str() );
  if( in.open( QIODevice::ReadOnly ) )
  {

    setPlainText( in.readAll() );
    in.close();

    // update flags
    setModified( false );
    _setIgnoreWarnings( false );

  }

  // finally set file. This is needed to be done _after_ the text is loaded in the display
  // in order to minimize the amount of slots that are sent
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    (*iter)->_setIsNewDocument( false );
    (*iter)->_setFile( file );
    (*iter)->filePropertiesAction().setEnabled( true );
  }
    
  // save file if restored from autosaved.
  if( restore_autosave && !isReadOnly() ) save();

  // perform first autosave
  Application& application( *Singleton::get().application<Application>() );
  application.autoSave().saveFiles( this );
  
}

//_______________________________________________________
void TextDisplay::_setFile( const File& file )
{

  Debug::Throw() << "TextDisplay::_setFile - file: " << file << endl;
  
  file_ = file;
  if( !isNewDocument() && file.exists() )
  {
    _setLastSaved( file.lastModified() );
    _setWorkingDirectory( file.path() );
    _setIgnoreWarnings( false );
  }

  // check if file is read-only
  checkFileReadOnly();

  if( isActive() ) emit needUpdate( FILE_NAME );

}

//___________________________________________________________________________
FileRemovedDialog::ReturnCode TextDisplay::checkFileRemoved( void )
{
  Debug::Throw( "TextDisplay::checkFileRemoved.\n" );

  if( _ignoreWarnings() || !_fileRemoved() ) return FileRemovedDialog::IGNORE;

  // disable check
  FileRemovedDialog dialog( this, file() );
  int state( dialog.centerOnParent().exec() );

  if( state == FileRemovedDialog::RESAVE )
  {

    // set document as modified (to force the file to be saved) and save
    setModified( true );
    save();

  } else if( state == FileRemovedDialog::SAVE_AS ) { saveAs(); }
  else if( state == FileRemovedDialog::IGNORE ) {

    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      (*iter)->_setIgnoreWarnings( true );
      (*iter)->setModified( false );
    }

  } else if( state == FileRemovedDialog::IGNORE ) {

    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { (*iter)->_setIgnoreWarnings( true ); }

  }

  return FileRemovedDialog::ReturnCode( state );

}


//___________________________________________________________________________
FileModifiedDialog::ReturnCode TextDisplay::checkFileModified( void )
{
  Debug::Throw( "TextDisplay::checkFileModified.\n" );

  if( _ignoreWarnings() || !_fileModified() ) return FileModifiedDialog::IGNORE;

  FileModifiedDialog dialog( this, file() );
  int state( dialog.centerOnParent().exec() );
  if( state == FileModifiedDialog::RESAVE ) 
  {
    document()->setModified( true );
    save(); 
  } else if( state == FileModifiedDialog::SAVE_AS ) { saveAs(); }
  else if( state == FileModifiedDialog::RELOAD ) {

    setModified( false );
    revertToSave();

  } else if( state == FileModifiedDialog::IGNORE ) {

    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { (*iter)->_setIgnoreWarnings( true ); }

  }

  return FileModifiedDialog::ReturnCode( state );

}

//___________________________________________________________________________
void TextDisplay::checkFileReadOnly( void )
{
  Debug::Throw( "TextDisplay::checkFileReadOnly.\n" );
  setReadOnly( file().exists() && !file().isWritable() );
}

//___________________________________________________________________________
AskForSaveDialog::ReturnCode TextDisplay::askForSave( const bool& enable_all )
{
  Debug::Throw( "TextDisplay::askForSave.\n" );

  if( !( document()->isModified() && _contentsChanged() ) ) return AskForSaveDialog::YES;

  int flags( AskForSaveDialog::YES | AskForSaveDialog::NO | AskForSaveDialog::CANCEL );
  if( enable_all ) flags |=  AskForSaveDialog::YES_TO_ALL | AskForSaveDialog::NO_TO_ALL;

  AskForSaveDialog dialog( this, file(), flags );
  int state( dialog.centerOnParent().exec() );
  if( state == AskForSaveDialog::YES ||  state == AskForSaveDialog::YES_TO_ALL ) save();
  else if( state == AskForSaveDialog::NO ||  state == AskForSaveDialog::NO_TO_ALL ) setModified( false );

  return AskForSaveDialog::ReturnCode(state);

}

//___________________________________________________________________________
void TextDisplay::save( void )
{
   Debug::Throw( "TextDisplay::save.\n" );

  // do nothing if not modified
  if( !document()->isModified() ) return;

  // check file name
  if( file().empty() || isNewDocument() ) return saveAs();

  // check is contents differ from saved file
  if( _contentsChanged() )
  {

    // make backup
    if( XmlOptions::get().get<bool>( "BACKUP" ) && file().exists() ) file().backup();

    // open output file
    QFile out( file().c_str() );
    if( !out.open( QIODevice::WriteOnly ) )
    {
      ostringstream what;
      what << "Cannot write to file \"" << file() << "\". <Save> canceled.";
      InformationDialog( this, what.str().c_str() ).exec();
      return;
    }

    // write file
    // make sure that last line ends with "end of line"
    QString text( toPlainText() );
    out.write( text.toAscii() );
    if( !text.isEmpty() && text[text.size()-1] != '\n' ) out.write( "\n" );

    // close
    out.close();

  }

  // update modification state and last_saved time stamp
  setModified( false );
  _setLastSaved( file().lastModified() );
  _setIgnoreWarnings( false );

  // retrieve associated displays, update saved time
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { (*iter)->_setLastSaved( file().lastModified() ); }

  // add file to menu
  if( !file().empty() )
  { _recentFiles().get( file() ).addProperty( class_name_property_id_, qPrintable( className() ) ); }

  return;

}

//___________________________________________________________________________
void TextDisplay::saveAs( void )
{
  Debug::Throw( "TextDisplay::saveAs.\n" );

  // define default file
  File default_file( file() );
  if( default_file.empty() || isNewDocument() ) default_file = File( "document" ).addPath( workingDirectory() );

  // create file dialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::AnyFile );
  dialog.setDirectory( QDir( default_file.path().c_str() ) );
  dialog.selectFile( default_file.localName().c_str() );
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve filename
  // retrieve filename
  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return;

  File file = File( qPrintable( files.front() ) ).expand();

  // check if file is directory
  if( file.isDirectory() )
  {
    ostringstream what;
    what << "File \"" << file << "\" is a directory. <Save> canceled.";
    InformationDialog( this, what.str().c_str() ).exec();
    return;
  }

  // check if file exists
  if( file.exists() )
  {
    
    if( !file.isWritable() )
    {
      ostringstream what;
      what << "File \"" << file << "\" is read-only. <Save> canceled.";
      InformationDialog( this, what.str().c_str() ).exec();
      return;
    } else if( !QuestionDialog( this, "Selected file already exists. Overwrite ?" ).exec() ) return;
    
  }

  // remove new document version from name server
  if( isNewDocument() ) { NewDocumentNameServer().remove( TextDisplay::file() ); }

  // update filename and document class for this and associates
  // the class name is reset, to allow a document class
  // matching the new filename to get loaded
  setClassName( "" );
  
  BASE::KeySet<TextDisplay> displays( this );
  displays.insert( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {

    // update file
    (*iter)->_setIsNewDocument( false );
    (*iter)->setClassName( className() );
    (*iter)->_updateDocumentClass( file, false );
    (*iter)->_setFile( file ); 

    // enable file info action
    (*iter)->filePropertiesAction().setEnabled( true );

  }

  // set document as modified and save using new filename
  setModified( true );
  save();

  // rehighlight
  rehighlight();

}

//___________________________________________________________
void TextDisplay::revertToSave( void )
{

  Debug::Throw( "TextDisplay::revertToSave.\n" );

  // store scrollbar positions
  int x( horizontalScrollBar()->value() );
  int y( verticalScrollBar()->value() );

  // store cursor position but remove selection
  int position( textCursor().position() );

  setUpdatesEnabled( false );
  setModified( false );
  setFile( file(), false );

  // restore
  horizontalScrollBar()->setValue( x );
  verticalScrollBar()->setValue( y );

  // adjust cursor postion
  position = min( position, toPlainText().size() );

  // restore cursor
  QTextCursor cursor( textCursor() );
  cursor.setPosition( position );
  setTextCursor( cursor );
  setUpdatesEnabled( true );

}

//_______________________________________________________
bool TextDisplay::hasLeadingTabs( void ) const
{
  Debug::Throw( "TextDisplay::hasLeadingTabs.\n" );

  // define regexp to perform replacement
  QRegExp wrong_tab_regexp( _hasTabEmulation() ? _normalTabRegExp():_emulatedTabRegExp() );
  for( QTextBlock block( document()->begin() ); block.isValid(); block = block.next() )
  { if( wrong_tab_regexp.indexIn( block.text() ) >= 0 ) return true; }

  return false;

}

//_______________________________________________________
QString TextDisplay::toPlainText( void ) const
{
  
  Debug::Throw( "TextDisplay::toPlainText.\n" );
  
  // check blockDelimiterAction
  if( !( showBlockDelimiterAction().isEnabled() && showBlockDelimiterAction().isChecked() ) ) 
  { return TextEditor::toPlainText(); }
  
  // output string
  QString out;
  
  // loop over blocks
  for( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
  {
    
    // add current block
    out += block.text();
    if( block.next().isValid() ) out += "\n";
    
    // retrieve associated block format
    QTextBlockFormat block_format( block.blockFormat() );
    if( block_format.boolProperty( TextBlock::Collapsed ) && block_format.hasProperty( TextBlock::CollapsedData ) )
    {
      
      CollapsedBlockData collapsed_data( block_format.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );
      for( CollapsedBlockData::List::const_iterator iter = collapsed_data.children().begin(); iter != collapsed_data.children().end(); iter++ )
      { out += iter->toPlainText(); } 
    }
    
  }
  
  return out;
      
}

//_______________________________________________________
QDomElement TextDisplay::htmlNode( QDomDocument& document, const int& max_line_size )
{

  // clear highlight locations and rehighlight
  QDomElement out = document.createElement( "pre" );

  int active_id( 0 ); 
  
  // loop over text blocks
  for( QTextBlock block = TextDisplay::document()->begin(); block.isValid(); block = block.next() )
  {

    // need to redo highlighting rather that us HighlightBlockData
    // because the latter do not store autospell patterns.
    PatternLocationSet locations;
    if( textHighlight().isHighlightEnabled() ) 
    { 
      locations = textHighlight().locationSet( block.text(), active_id );
      active_id = locations.activeId().second;
    }
    
    // retrieve text
    QString text( block.text() );

    // current pattern
    QDomElement span;
    int current_pattern_id = -1;
    bool line_break( false );
    int line_index( 0 );

    // parse text
    QString buffer("");
    for( int index = 0; index < text.size(); index++, line_index++ )
    {

      // parse locations
      PatternLocationSet::reverse_iterator location_iter = find_if(
        locations.rbegin(),
        locations.rend(),
        PatternLocation::ContainsFTor( index ) );

      int pattern_id( ( location_iter == locations.rend() ) ? -1:location_iter->id() );
      if( pattern_id != current_pattern_id || index == 0 || line_break )
      {

        // append text to current element and reset stream
        if( !buffer.isEmpty() )
        {
          if( span.isNull() ) span  = out.appendChild( document.createElement( "span" ) ).toElement();
          HtmlTextNode( buffer, span, document );
          if( line_break )
          {
            out.appendChild( document.createElement( "br" ) );
            line_break = false;
            line_index = 0;
          }
          buffer = "";
        }  

        // update pattern
        current_pattern_id = pattern_id;

        // update current element
        span = out.appendChild( document.createElement( "span" ) ).toElement();
        if( location_iter !=  locations.rend() )
        {
                    
          // retrieve font format
          const unsigned int& format( location_iter->fontFormat() );
          ostringstream format_stream;
          if( format & FORMAT::UNDERLINE ) format_stream << "text-decoration: underline; ";
          if( format & FORMAT::ITALIC ) format_stream << "font-style: italic; ";
          if( format & FORMAT::BOLD ) format_stream << "font-weight: bold; ";
          if( format & FORMAT::STRIKE ) format_stream << "text-decoration: line-through; ";

          // retrieve color
          const QColor& color = location_iter->color();
          if( color.isValid() ) format_stream << "color: " << qPrintable( color.name() ) << "; ";

          span.setAttribute( "style", format_stream.str().c_str() );

        }
      }

      buffer += text[index];

      // check for line-break
      if( max_line_size > 0  && TextSeparator::get().all().find( text[index] ) != TextSeparator::get().all().end() )
      {

        // look for next separator in string
        int next( -1 );
        for( TextSeparator::SeparatorSet::const_iterator iter = TextSeparator::get().all().begin(); iter != TextSeparator::get().all().end(); iter++ )
        {
          int position( text.indexOf( *iter, index+1 ) );
          if( position >= 0 && ( next < 0 || position < next ) ) next = position;
        }

        if( line_index + next - index > max_line_size ) line_break = true;
      }

    }

    if( !buffer.isEmpty() )
    {
      if( span.isNull() ) span  = out.appendChild( document.createElement( "span" ) ).toElement();
      span.appendChild( document.createTextNode( buffer ) );
    }
    
    out.appendChild( document.createElement( "br" ) );
    
  }

  return out;
}

//___________________________________________________________________________
bool TextDisplay::ignoreBlock( const QTextBlock& block ) const
{

  // first check if block text match empty line
  if( isEmptyBlock( block ) ) return true;

  // try retrieve highlight data
  HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
  return ( data && data->ignoreBlock() );

}

//___________________________________________________________________________
void TextDisplay::tagBlock( QTextBlock block, const unsigned int& tag )
{
  Debug::Throw( "TextDisplay::tagBlock.\n" );

  HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
  if( !data ) block.setUserData( data = new HighlightBlockData() );

  switch( tag )
  {
    case TextBlock::DIFF_ADDED:
    {
      data->setFlag( TextBlock::DIFF_ADDED, true );
      setBackground( block, diff_added_color_ );
      break;
    }

    case TextBlock::DIFF_CONFLICT:
    {
      data->setFlag( TextBlock::DIFF_CONFLICT, true );
      setBackground( block, diff_conflict_color_ );
      break;
    }

    case TextBlock::USER_TAG:
    {
      data->setFlag( TextBlock::USER_TAG, true );
      setBackground( block, user_tag_color_ );
      break;
    }

    default: break;

  }

  return;

}

//___________________________________________________________________________
void TextDisplay::clearTag( QTextBlock block, const int& tags )
{
  Debug::Throw() << "TextDisplay::clearTag - key: " << key() << endl;
  TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
  if( !data ) return;

  if( tags & TextBlock::DIFF_ADDED )
  {
    data->setFlag( TextBlock::DIFF_ADDED, false );
    clearBackground( block );
  }

  if( tags & TextBlock::DIFF_CONFLICT )
  {
    data->setFlag( TextBlock::DIFF_CONFLICT, false );
    clearBackground( block );
  }

  if( tags & TextBlock::USER_TAG )
  {
    data->setFlag( TextBlock::USER_TAG, false );
    clearBackground( block );
  }
  Debug::Throw( "TextDisplay::clearTag - done.\n" );

}


//_____________________________________________________________
bool TextDisplay::isCurrentBlockTagged( void )
{

  Debug::Throw( "TextDisplay::isCurrentBlockTagged.\n" );

  vector<QTextBlock> blocks;
  QTextCursor cursor( textCursor() );
  if( cursor.hasSelection() )
  {

    QTextBlock first( document()->findBlock( min( cursor.position(), cursor.anchor() ) ) );
    QTextBlock last( document()->findBlock( max( cursor.position(), cursor.anchor() ) ) );
    for( QTextBlock block( first ); block.isValid() && block != last;  block = block.next() )
    { blocks.push_back( block ); }
    if( last.isValid() ) blocks.push_back( last );

  } else blocks.push_back( cursor.block() );

  for( vector<QTextBlock>::iterator iter = blocks.begin(); iter != blocks.end(); iter++ )
  {
    TextBlockData *data( static_cast<TextBlockData*>( iter->userData() ) );
    if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) return true;
  }

  return false;

}

//_____________________________________________________________
bool TextDisplay::hasTaggedBlocks( void )
{

  Debug::Throw( "TextDisplay::hasTaggedBlocks.\n" );

  // loop over block
  for( QTextBlock block( document()->begin() ); block.isValid(); block = block.next() )
  {
    TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
    if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) return true;
  }

  return false;
}

//___________________________________________________________________________
void TextDisplay::_updateDocumentClass( File file, bool new_document )
{

  Debug::Throw( "TextDisplay::_updateDocumentClass\n" );
  
  textHighlight().clear();
  textIndent().clear();
  textIndent().setBaseIndentation(0);
  _clearMacros();
  
  // default document class is empty
  DocumentClass document_class;
  Application& application( *Singleton::get().application<Application>() );
  
  // try load document class from class_name
  if( !className().isEmpty() )
  { 
    Debug::Throw( "TextDisplay::updateDocumentClass - try use className().\n" );
    document_class = application.classManager().get( className() ); 
  }

  // try load from file
  if( document_class.name().isEmpty() && !( file.empty() || new_document ) )
  { 
    Debug::Throw( "TextDisplay::updateDocumentClass - try use filename.\n" );
    document_class = application.classManager().find( file ); 
  }

  // use default
  if( document_class.name().isEmpty() )
  { 
    Debug::Throw( "TextDisplay::updateDocumentClass - using default.\n" );
    document_class = application.classManager().defaultClass(); 
  }
  
  // update class name
  setClassName( document_class.name() );

  // wrap mode
  if( !( file.empty() || new_document ) )
  { 
    
    FileRecord& record( _recentFiles().get( file ) );
    if( record.hasProperty( wrap_property_id_ ) ) wrapModeAction().setChecked( Str( record.property( wrap_property_id_ ) ).get<bool>() );
    else if( XmlOptions::get().get<bool>( "WRAP_FROM_CLASS" ) ) wrapModeAction().setChecked( document_class.wrap() );

  } else if( XmlOptions::get().get<bool>( "WRAP_FROM_CLASS" ) ) wrapModeAction().setChecked( document_class.wrap() );
  
  // tab emulation
  if( XmlOptions::get().get<bool>( "EMULATE_TABS_FROM_CLASS" ) ) tabEmulationAction().setChecked( document_class.emulateTabs() );

  // need to update tab size here because at the time it was set in _updateConfiguration,
  // the font might not have been right
  _setTabSize( XmlOptions::get().get<int>("TAB_SIZE") );

  // enable actions consequently
  parenthesisHighlightAction().setVisible( !document_class.parenthesis().empty() );
  textHighlightAction().setVisible( !document_class.highlightPatterns().empty() );
  textIndentAction().setVisible( !document_class.indentPatterns().empty() );
  baseIndentAction().setVisible( document_class.baseIndentation() );

  // store into class members
  textHighlight().setPatterns( document_class.highlightPatterns() );
  textHighlight().setParenthesis( document_class.parenthesis() );
  textHighlight().setBlockDelimiters( document_class.blockDelimiters() );
      
  textIndent().setPatterns( document_class.indentPatterns() );
  textIndent().setBaseIndentation( document_class.baseIndentation() );
  _setMacros( document_class.textMacros() );

  // update block delimiters
  if( blockDelimiterDisplay().expandAllAction().isEnabled() ) blockDelimiterDisplay().expandAllAction().trigger();   
  if( blockDelimiterDisplay().setBlockDelimiters( document_class.blockDelimiters() ) ) update();
  showBlockDelimiterAction().setVisible( !document_class.blockDelimiters().empty() );
  _updateMargins();
  
  // update enability for parenthesis matching
  textHighlight().setParenthesisEnabled(
    textHighlightAction().isChecked() &&
    textHighlight().parenthesisHighlightColor().isValid() &&
    !textHighlight().parenthesis().empty() );

  parenthesisHighlight().setEnabled(
    textHighlightAction().isChecked() &&
    textHighlight().parenthesisHighlightColor().isValid() &&
    !textHighlight().parenthesis().empty() );

  // add information to Menu
  if( !( file.empty() || new_document ) )
  { 
    FileRecord& record( _recentFiles().get( file ) );
    record.addProperty( class_name_property_id_, qPrintable( className() ) ); 
    record.addProperty( wrap_property_id_, Str().assign<bool>( wrapModeAction().isChecked() ) ); 
    if( !document_class.icon().isEmpty() ) record.addProperty( icon_property_id_, qPrintable( document_class.icon() ) );
  }

  // rehighlight text entirely
  // because Pattern Ids may have changed even if the className has not changed.
  #if WITH_ASPELL
  if( textHighlight().isHighlightEnabled() && !textHighlight().spellParser().isEnabled() ) rehighlight();
  #else
  if( textHighlight().isHighlightEnabled() ) rehighlight();
  #endif

  return;

}

//_____________________________________________
void TextDisplay::processMacro( QString name )
{

  Debug::Throw() << "TextDisplay::processMacro - " << qPrintable( name ) << endl;

  // retrieve macro that match argument name
  TextMacro::List::const_iterator macro_iter = find_if( macros_.begin(), macros_.end(), TextMacro::SameNameFTor( name ) );
  if( macro_iter == macros_.end() )
  {
    ostringstream what;
    what << "Unable to find macro named " << qPrintable( name );
    InformationDialog( this, what.str().c_str() ).exec();
    return;
  }

  // check display
  if( !isActive() ) return;

  // check if readonly
  if( isReadOnly() ) return;

  // retrieve text cursor
  QTextCursor cursor( textCursor() );
  if( !cursor.hasSelection() ) return;

  // retrieve blocks
  int position_begin( min( cursor.position(), cursor.anchor() ) );
  int position_end( max( cursor.position(), cursor.anchor() ) );
  QTextBlock begin( document()->findBlock( position_begin ) );
  QTextBlock end( document()->findBlock( position_end ) );

  // enlarge selection so that it matches begin and end of blocks
  position_begin = begin.position();
  if( position_end == end.position() )
  {
    position_end--;
    end = end.previous();
  } else {  position_end = end.position() + end.length() - 1; }

  // prepare text from selected blocks
  QString text;
  if( begin == end ) text = begin.text().mid( position_begin - begin.position(), position_end-position_begin );
  else {
    text = begin.text().mid( position_begin - begin.position() ) + "\n";
    for( QTextBlock block = begin.next(); block.isValid() && block!= end; block = block.next() )
    { text += block.text() + "\n"; }
    text += end.text().left( position_end - end.position() );
  }

  // process macro
  if( !macro_iter->processText( text ) ) return;

  // update selection
  cursor.setPosition( position_begin );
  cursor.setPosition( position_end, QTextCursor::KeepAnchor );

  // insert new text
  cursor.insertText( text );

  // restore selection
  cursor.setPosition( position_begin );
  cursor.setPosition( position_begin + text.size(), QTextCursor::KeepAnchor );
  setTextCursor( cursor );

  // replace leading tabs in selection
  if( !_hasTabEmulation() ) { _replaceLeadingTabs( false ); }

  return;

}

//_______________________________________________________
void TextDisplay::rehighlight( void )
{
  Debug::Throw( "TextDisplay::rehighlight.\n" );

  // set all block to modified
  for( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
  { _setBlockModified( block ); }

  textHighlight().setDocument( document() );
  Debug::Throw( "TextDisplay::rehighlight. done.\n" );

}


//___________________________________________________________________________
void TextDisplay::clearAllTags( const int& flags )
{

  Debug::Throw( "TextEditor::clearAllTags.\n" );
  for( QTextBlock block( document()->begin() ); block.isValid(); block = block.next() )
  { clearTag( block, flags ); }

}

//_______________________________________
void TextDisplay::selectFilter( const QString& filter )
{
  Debug::Throw( "TextDisplay::selectFilter.\n" );

  #if WITH_ASPELL

  // local reference to interface
  SPELLCHECK::SpellInterface& interface( textHighlight().spellParser().interface() );

  if( filter == interface.filter().c_str() || !interface.hasFilter( qPrintable( filter ) ) ) return;

  // update interface
  interface.setFilter( qPrintable( filter ) );
  _filterMenu().select( qPrintable( filter ) );

  // update file record
  if( !( file().empty() || isNewDocument() ) )
  { _recentFiles().get( file() ).addProperty( filter_property_id_, interface.filter() ); }

  // rehighlight if needed
  if( textHighlight().spellParser().isEnabled() ) rehighlight();

  #endif

  return;

}

//_______________________________________
void TextDisplay::selectDictionary( const QString& dictionary )
{
  Debug::Throw( "TextDisplay::selectDictionary.\n" );

  #if WITH_ASPELL
  // local reference to interface
  SPELLCHECK::SpellInterface& interface( textHighlight().spellParser().interface() );

  if( dictionary == interface.dictionary().c_str() || !interface.hasDictionary( qPrintable( dictionary ) ) ) return;

  // update interface
  interface.setDictionary( qPrintable( dictionary ) );
  _dictionaryMenu().select( qPrintable( dictionary ) );

  // update file record
  if( !( file().empty() || isNewDocument() ) )
  { _recentFiles().get( file() ).addProperty( dictionary_property_id_, interface.dictionary() ); }

  // rehighlight if needed
  if( textHighlight().spellParser().isEnabled() ) rehighlight();
  #endif

  return;

}

//_______________________________________________________
bool TextDisplay::event( QEvent* event )
{
  
  bool has_block_delimiters( hasBlockDelimiterDisplay() && hasBlockDelimiterAction() && showBlockDelimiterAction().isVisible() && showBlockDelimiterAction().isChecked() );
  
  // check that all needed widgets/actions are valid and checked.
  switch (event->type()) 
  {
          
    case QEvent::MouseButtonPress:
    if( has_block_delimiters ) blockDelimiterDisplay().mousePressEvent( static_cast<QMouseEvent*>( event ) );
    break;
    
    case QEvent::Wheel:
    {
      QWheelEvent *wheel_event( static_cast<QWheelEvent*>(event) );
      if( QRect( frameWidth(),  frameWidth(), _leftMargin(), height() ).contains( wheel_event->pos() ) )
      { qApp->sendEvent( viewport(), event ); }
      return false;
    }
    
    default: break;
  }
  
  return TextEditor::event( event );
  
}

//_______________________________________________________
void TextDisplay::keyPressEvent( QKeyEvent* event )
{
  
  // check if tab key is pressed
  if(
    event->key() == Qt::Key_Tab &&
    indent_->isEnabled() &&
    !( textCursor().hasSelection() || _boxSelection().state() == BoxSelection::FINISHED ) )
  { emit indent( textCursor().block(), false ); }
  else
  {
    
    // process key
    TextEditor::keyPressEvent( event );
    
    // indent current paragraph when return is pressed
    if( indent_->isEnabled() && event->key() == Qt::Key_Return && !textCursor().hasSelection() )
    { emit indent( textCursor().block(), true ); }
    
    // reindent paragraph if needed
    /* remark: this is c++ specific. The list of keys should be set in the document class */
    if( indent_->isEnabled() && ( event->key() == Qt::Key_BraceRight || event->key() == Qt::Key_BraceLeft ) && !textCursor().hasSelection() )
    { emit indent( textCursor().block(), false ); }

  }

  return;
}

//________________________________________________
void TextDisplay::contextMenuEvent( QContextMenuEvent* event )
{

  Debug::Throw( "TextEditor::contextMenuEvent.\n" );

  if( _autoSpellContextEvent( event ) ) return;
  else {
    
    TextEditor::contextMenuEvent( event );
    return;
    
  }
  
}

//________________________________________________
void TextDisplay::paintEvent( QPaintEvent* event )
{
  TextEditor::paintEvent( event );
  
  // handle block background
  QTextBlock first( cursorForPosition( event->rect().topLeft() ).block() );
  QTextBlock last( cursorForPosition( event->rect().bottomRight() ).block() );

  // create painter and translate from widget to viewport coordinates
  QPainter painter( viewport() );
  painter.setClipRect( event->rect() );
  painter.translate( -scrollbarPosition() );
  if( _marginForegroundColor().isValid() ) painter.setPen( _marginForegroundColor() );
  
  // loop over found blocks  
  for( QTextBlock block( first ); block != last.next() && block.isValid(); block = block.next() )
  {
    if( !block.blockFormat().boolProperty( TextBlock::Collapsed ) ) continue;
    
    QRectF block_rect( document()->documentLayout()->blockBoundingRect( block ) );
    block_rect.setWidth( viewport()->width() + scrollbarPosition().x() );
    painter.drawLine( block_rect.bottomLeft() - QPoint( 2, 0 ), block_rect.bottomRight() );
  }
  painter.end();
  
}

//________________________________________________
bool TextDisplay::_autoSpellContextEvent( QContextMenuEvent* event )
{
  Debug::Throw( "TextEditor::_autoSpellContextEvent.\n" );

  #if WITH_ASPELL

  // check autospell enability
  if( !textHighlight().spellParser().isEnabled() ) return false;

  // block and cursor
  QTextCursor cursor( cursorForPosition( event->pos() ) );
  QTextBlock block( cursor.block() );

  // block data
  HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
  if( !data ) return false;

  // try retrieve misspelled word
  SPELLCHECK::Word word( data->misspelledWord( cursor.position() - block.position() ) );
  if( word.empty() || textHighlight().spellParser().interface().isWordIgnored( word ) )
  { return false; }

  // change selection to misspelled word
  cursor.setPosition( word.position() + block.position(), QTextCursor::MoveAnchor );
  cursor.setPosition( word.position() + word.size() + block.position(), QTextCursor::KeepAnchor );
  setTextCursor( cursor );

  // create suggestion menu
  SPELLCHECK::SuggestionMenu menu( this, word.c_str(), isReadOnly() );
  menu.interface().setFilter( textHighlight().spellParser().interface().filter() );
  menu.interface().setDictionary( textHighlight().spellParser().interface().dictionary() );

  // set connections
  connect( &menu, SIGNAL( ignoreWord( QString ) ), SLOT( _ignoreMisspelledWord( QString ) ) );
  connect( &menu, SIGNAL( suggestionSelected( QString ) ), SLOT( _replaceMisspelledSelection( QString ) ) );

  // execute
  menu.exec( event->globalPos() );
  return true;

  #else

  return false;
  #endif

}

//_____________________________________________________________________
void TextDisplay::_installActions( void )
{

  Debug::Throw( "TextDisplay::_installActions.\n" );

  // actions
  addAction( text_indent_action_ = new QAction( IconEngine::get( ICONS::INDENT ), "&Indent text", this ) );
  text_indent_action_->setCheckable( true );
  text_indent_action_->setChecked( textIndent().isEnabled() );
  connect( text_indent_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleTextIndent( bool ) ) );

  addAction( text_highlight_action_ = new QAction( "&Highlight text", this ) );
  text_highlight_action_->setCheckable( true );
  text_highlight_action_->setChecked( textHighlight().isHighlightEnabled() );
  text_highlight_action_->setShortcut( Qt::Key_F8 );
  text_highlight_action_->setShortcutContext( Qt::WidgetShortcut );
  connect( text_highlight_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleTextHighlight( bool ) ) );

  addAction( parenthesis_highlight_action_ = new QAction( "&Highlight parenthesis", this ) );
  parenthesis_highlight_action_->setCheckable( true );
  parenthesis_highlight_action_->setChecked( parenthesisHighlight().isEnabled() );
  connect( parenthesis_highlight_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleParenthesisHighlight( bool ) ) );

  addAction( show_block_delimiter_action_ =new QAction( "Show block delimiters", this ) );
  show_block_delimiter_action_->setToolTip( "Show/hide block delimiters" );
  show_block_delimiter_action_->setCheckable( true );
  show_block_delimiter_action_->setShortcut( Qt::Key_F9 );
  show_block_delimiter_action_->setShortcutContext( Qt::WidgetShortcut );
  connect( show_block_delimiter_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleShowBlockDelimiters( bool ) ) );

  // autospell
  addAction( autospell_action_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), "&Automatic spell-check", this ) );
  autospell_action_->setShortcut( Qt::Key_F6 );
  autospell_action_->setShortcutContext( Qt::WidgetShortcut );
  autospell_action_->setCheckable( true );

  #if WITH_ASPELL
  autospell_action_->setChecked( textHighlight().spellParser().isEnabled() );
  connect( autospell_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleAutoSpell( bool ) ) );
  #else
  autospell_action_->setVisible( false );
  #endif

  // spell checking
  addAction( spellcheck_action_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), "&Spell check", this ) );
  #if WITH_ASPELL
  connect( spellcheck_action_, SIGNAL( triggered( void ) ), SLOT( _spellcheck( void ) ) );
  #else
  spellcheck_action_->setVisible( false );
  #endif

  // indent selection
  addAction( indent_selection_action_ = new QAction( IconEngine::get( ICONS::INDENT ), "&Indent selection", this ) );
  indent_selection_action_->setShortcut( Qt::CTRL + Qt::Key_I );
  indent_selection_action_->setShortcutContext( Qt::WidgetShortcut );
  connect( indent_selection_action_, SIGNAL( triggered( void ) ), SLOT( _indentSelection( void ) ) );

  // base indentation
  addAction( base_indent_action_ = new QAction( IconEngine::get( ICONS::INDENT ), "&Add base indentation", this ) );
  base_indent_action_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_I );
  connect( base_indent_action_, SIGNAL( triggered( void ) ), SLOT( _addBaseIndentation( void ) ) );

  // replace leading tabs
  addAction( leading_tabs_action_ = new QAction( "&Replace leading tabs", this ) );
  connect( leading_tabs_action_, SIGNAL( triggered( void ) ), SLOT( _replaceLeadingTabs( void ) ) );

  // file information
  addAction( file_properties_action_ = new QAction( IconEngine::get( ICONS::INFO ), "&File properties", this ) );
  file_properties_action_->setShortcut( Qt::ALT + Qt::Key_Return );
  file_properties_action_->setToolTip( "Display current file properties" );
  connect( file_properties_action_, SIGNAL( triggered() ), SLOT( _fileProperties() ) );

  #if WITH_ASPELL

  filter_menu_action_ = _filterMenu().menuAction();
  dictionary_menu_action_ = _dictionaryMenu().menuAction();

  connect( &_filterMenu(), SIGNAL( selectionChanged( const QString& ) ), SLOT( selectFilter( const QString& ) ) );
  connect( &_dictionaryMenu(), SIGNAL( selectionChanged( const QString& ) ), SLOT( selectDictionary( const QString& ) ) );

  #endif

  // tag block action
  addAction( tag_block_action_ = new QAction( IconEngine::get( ICONS::TAG ), "&Tag selected blocks", this ) );
  connect( tag_block_action_, SIGNAL( triggered() ), SLOT( _tagBlock( void ) ) );

  // clear current block tags
  addAction( clear_tag_action_ = new QAction( "Clear current tags", this ) );
  connect( clear_tag_action_, SIGNAL( triggered() ), SLOT( _clearTag( void ) ) );

  // clear all tags
  addAction( clear_all_tags_action_ = new QAction( "Clear all tags", this ) );
  connect( clear_all_tags_action_, SIGNAL( triggered() ), SLOT( clearAllTags( void ) ) );

  // next tag action
  addAction( next_tag_action_ = new QAction( IconEngine::get( ICONS::DOWN ), "Goto next tagged block", this ) );
  connect( next_tag_action_, SIGNAL( triggered() ), SLOT( _nextTag( void ) ) );
  next_tag_action_->setShortcut( Qt::ALT + Qt::Key_Down );
  next_tag_action_->setShortcutContext( Qt::WidgetShortcut );

  // previous tag action
  addAction( previous_tag_action_ = new QAction( IconEngine::get( ICONS::UP ), "Goto previous tagged block", this ) );
  connect( previous_tag_action_, SIGNAL( triggered() ), SLOT( _previousTag( void ) ) );
  previous_tag_action_->setShortcut( Qt::ALT + Qt::Key_Up );
  previous_tag_action_->setShortcutContext( Qt::WidgetShortcut );

}

//_____________________________________________________________
FileList& TextDisplay::_recentFiles( void ) const
{ return Singleton::get().application<Application>()->recentFiles(); }

//_____________________________________________________________
bool TextDisplay::_contentsChanged( void ) const
{

  Debug::Throw( "TextDisplay::_contentsChanged.\n" );

  // check file
  if( file().empty() || isNewDocument() ) return true;

  // open file
  QFile in( file().c_str() );
  if( !in.open( QIODevice::ReadOnly ) ) return true;

  // dump file into character string
  QString file_text( in.readAll() );
  QString text( toPlainText() );
  return (text.size() != file_text.size() || text != file_text );

}

//____________________________________________
bool TextDisplay::_fileRemoved( void ) const
{
  Debug::Throw( "TextDisplay::_fileRemoved.\n" );
  return ( !( file().empty() || isNewDocument() ) && last_save_.isValid() && !file().exists() );
}

//____________________________________________
bool TextDisplay::_fileModified( void )
{

  Debug::Throw( "TextDisplay::_fileModified.\n" );

  // check file size
  if( !( file().size() && file().exists() ) ) return false;
  TimeStamp fileModified( file().lastModified() );

  // check if file was modified and contents is changed
  if(
    fileModified.isValid() &&
    last_save_.isValid() &&
    fileModified > last_save_ &&
    _contentsChanged() )
  {
    // update last_save to avoid chain questions
    last_save_ = fileModified;
    return true;
  }

  return false;

}

//_____________________________________________________________
void TextDisplay::_setBlockModified( const QTextBlock& block )
{
  // check if highlight is enabled.
  if( !textHighlight().isHighlightEnabled() ) return;

  // retrieve associated block data if any
  // set block as modified so that its highlight content gets reprocessed.
  TextBlockData* data( static_cast<TextBlockData*>( block.userData() ) );
  if( data ) data->setFlag( TextBlock::MODIFIED, true );

}

//_____________________________________________________________
void TextDisplay::_updateTaggedBlocks( void )
{

  Debug::Throw( "TextDisplay::_updateTaggedBlocks.\n" );

  // loop over block
  for( QTextBlock block( document()->begin() ); block.isValid(); block = block.next() )
  {

    TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
    if( !( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) ) continue;

    if( data->hasFlag( TextBlock::DIFF_ADDED ) ) setBackground( block, diff_added_color_ );
    if( data->hasFlag( TextBlock::DIFF_CONFLICT ) ) setBackground( block, diff_conflict_color_ );
    if( data->hasFlag( TextBlock::USER_TAG ) ) setBackground( block, user_tag_color_ );

  }

}

//___________________________________________________________________________
bool TextDisplay::_updateMargins( void )
{
  Debug::Throw( "TextDisplay::_updateMargins.\n" );

  TextEditor::_updateMargins();
  int left_margin( _leftMargin() );
  
  blockDelimiterDisplay().setOffset( left_margin );
  if( showBlockDelimiterAction().isChecked() && showBlockDelimiterAction().isVisible() )
  { left_margin += blockDelimiterDisplay().width(); }
  
  return _setLeftMargin( left_margin );
  
}


//__________________________________________________________
void TextDisplay::_drawMargins( QPainter& painter )
{
  TextEditor::_drawMargins( painter );
  bool has_block_delimiters( hasBlockDelimiterDisplay() && hasBlockDelimiterAction() && showBlockDelimiterAction().isVisible() && showBlockDelimiterAction().isChecked() );
  if( has_block_delimiters ) blockDelimiterDisplay().paint( painter ); 
}

//___________________________________________________________________________
bool TextDisplay::_toggleWrapMode( bool state )
{
  
  Debug::Throw() << "TextDisplay::_toggleWrapMode - " << (state ? "true":"false") << endl;
  if( !TextEditor::_toggleWrapMode( state ) ) return false;
  
  if( !( file().empty() || isNewDocument() ) )
  { _recentFiles().get( file() ).addProperty( wrap_property_id_, Str().assign<bool>(state) ); }
    
  return true;
  
}

//___________________________________________________________________________
void TextDisplay::_updateConfiguration( void )
{
   Debug::Throw( "TextDisplay::_updateConfiguration.\n" );

  // indentation
  textIndentAction().setChecked( XmlOptions::get().get<bool>( "TEXT_INDENT" ) );

  // syntax highlighting
  textHighlightAction().setChecked( XmlOptions::get().get<bool>( "TEXT_HIGHLIGHT" ) );

  // parenthesis highlight
  textHighlight().setParenthesisHighlightColor( QColor( XmlOptions::get().raw( "PARENTHESIS_COLOR" ).c_str() ) );
  parenthesisHighlightAction().setChecked( XmlOptions::get().get<bool>( "TEXT_PARENTHESIS" ) );

  // block delimiters, line numbers and margin
  showBlockDelimiterAction().setChecked( XmlOptions::get().get<bool>( "SHOW_BLOCK_DELIMITERS" ) );
  blockDelimiterDisplay().setWidth( fontMetrics().lineSpacing() );
  blockDelimiterDisplay().setCustomSymbols( XmlOptions::get().get<bool>( "CUSTOM_BLOCK_DELIMITERS" ) );
  _updateMargins();
  
  // retrieve diff colors
  diff_conflict_color_ = QColor( XmlOptions::get().get<string>("DIFF_CONFLICT_COLOR").c_str() );
  diff_added_color_ = QColor( XmlOptions::get().get<string>("DIFF_ADDED_COLOR").c_str() );
  user_tag_color_ = QColor( XmlOptions::get().get<string>("TAGGED_BLOCK_COLOR").c_str() );

  // update paragraph tags
  _updateTaggedBlocks();

}

//___________________________________________________________________________
void TextDisplay::_updateSpellCheckConfiguration( File file )
{
  Debug::Throw( "TextDisplay::_updateSpellCheckConfiguration.\n" );

  #if WITH_ASPELL

  // spellcheck configuration
  bool changed( false );
  changed |= textHighlight().spellParser().setColor( QColor( XmlOptions::get().get<string>("AUTOSPELL_COLOR").c_str() ) );
  changed |= textHighlight().spellParser().setFontFormat( XmlOptions::get().get<unsigned int>("AUTOSPELL_FONT_FORMAT") );
  textHighlight().updateSpellPattern();
  autoSpellAction().setEnabled( textHighlight().spellParser().color().isValid() );

  // store local reference to spell interface
  SPELLCHECK::SpellInterface& interface( textHighlight().spellParser().interface() );

  // load default filter and dictionaries
  string filter( XmlOptions::get().raw("DICTIONARY_FILTER") );
  string dictionary( XmlOptions::get().raw( "DICTIONARY" ) );

  // overwrite with file record
  if( file.empty() ) file = TextDisplay::file();
  if( !( file.empty() || isNewDocument() ) )
  {
    FileRecord& record( _recentFiles().get( file ) );
    if( record.hasProperty( filter_property_id_ ) && interface.hasFilter( record.property( filter_property_id_ ) ) )
    { filter = record.property( filter_property_id_ ); }

    if( record.hasProperty( dictionary_property_id_ ) && interface.hasDictionary( record.property( dictionary_property_id_ ) ) )
    { dictionary = record.property( dictionary_property_id_ ); }

  }

  // see if one should/can change the dictionary and filter
  if( filter == interface.filter() || interface.setFilter( filter ) )
  {
    _filterMenu().select( filter.c_str() );
    changed = true;
  }

  if( dictionary == interface.dictionary() || interface.setDictionary( dictionary ) )
  {
    _dictionaryMenu().select( dictionary.c_str() );
    changed = true;
  }

  // rehighlight if needed
  if( changed && autoSpellAction().isChecked() && autoSpellAction().isEnabled() ) 
  { rehighlight(); }

  #endif

}

//_______________________________________________________
void TextDisplay::_indentCurrentParagraph( void )
{
  Debug::Throw( "TextDisplay::_indentCurrentParagraph.\n" );
  if( !indent_->isEnabled() ) return;
  emit indent( textCursor().block(), false );
}

//_______________________________________________________
void TextDisplay::_toggleTextIndent( bool state )
{

  Debug::Throw( "TextDisplay::_toggleTextIndent.\n" );

  // update text indent
  textIndent().setEnabled( textIndentAction().isEnabled() && state );

  // propagate to other displays
  if( isSynchronized() )
  {
    // temporarely disable synchronization
    // to avoid infinite loop
    setSynchronized( false );

    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { if( (*iter)->isSynchronized() ) (*iter)->textIndentAction().setChecked( state ); }
    setSynchronized( true );

  }

}


//_______________________________________________________
void TextDisplay::_toggleTextHighlight( bool state )
{

  Debug::Throw( "TextDisplay::_toggleTextHighlight.\n" );
  if( textHighlight().setHighlightEnabled( textHighlightAction().isEnabled() && state ) )
  { rehighlight(); }

  // propagate to other displays
  if( isSynchronized() )
  {
    // temporarely disable synchronization
    // to avoid infinite loop
    setSynchronized( false );

    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { if( (*iter)->isSynchronized() ) (*iter)->textHighlightAction().setChecked( state ); }
    setSynchronized( true );

  }

}

//_______________________________________________________
void TextDisplay::_toggleParenthesisHighlight( bool state )
{

  Debug::Throw() << "TextDisplay::_toggleParenthesisHighlight -"
    << " state: " << state
    << " color: " << textHighlight().parenthesisHighlightColor().isValid()
    << " parenthesis: " << textHighlight().parenthesis().empty()
    << endl;

  // propagate to textHighlight
  textHighlight().setParenthesisEnabled(
    state &&
    textHighlight().parenthesisHighlightColor().isValid() &&
    !textHighlight().parenthesis().empty() );

  parenthesisHighlight().setEnabled(
    state &&
    textHighlight().parenthesisHighlightColor().isValid() &&
    !textHighlight().parenthesis().empty() );

  // propagate to other displays
  if( isSynchronized() )
  {
    // temporarely disable synchronization
    // to avoid infinite loop
    setSynchronized( false );

    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )

    { if( (*iter)->isSynchronized() ) (*iter)->parenthesisHighlightAction().setChecked( state ); }
    setSynchronized( true );

  }

  return;
}

//_______________________________________________________
void TextDisplay::_toggleAutoSpell( bool state )
{
  #if WITH_ASPELL
  Debug::Throw( "TextDisplay::_toggleAutoSpell.\n" );

  // enable menus
  // dictionaryMenuAction().setEnabled( state );
  // filterMenuAction().setEnabled( state );
  
  // propagate to textHighlight
  textHighlight().spellParser().setEnabled( state );
  rehighlight();

  // propagate to other displays
  if( isSynchronized() )
  {
    // temporarely disable synchronization
    // to avoid infinite loop
    setSynchronized( false );

    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { if( (*iter)->isSynchronized() ) (*iter)->autoSpellAction().setChecked( state ); }
    setSynchronized( true );

  }

  return;
  #endif
}

//_______________________________________________________
void TextDisplay::_toggleShowBlockDelimiters( bool state )
{
    
  _updateMargins();
  
  // need to expand all blocks if block delimiters is being hidden
  if( !state && hasBlockDelimiterDisplay() && blockDelimiterDisplay().expandAllAction().isEnabled() )
  { blockDelimiterDisplay().expandAllAction().trigger(); }
  
  // update options
  XmlOptions::get().set<bool>( "SHOW_BLOCK_DELIMITERS", state );

  // propagate to other displays
  if( isSynchronized() )
  {
    
    // temporarely disable synchronization
    // to avoid infinite loop
    setSynchronized( false );

    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { if( (*iter)->isSynchronized() ) (*iter)->showBlockDelimiterAction().setChecked( state ); }
    setSynchronized( true );

  }

  return;
}

//_______________________________________________________
void TextDisplay::_spellcheck( void )
{
  Debug::Throw( "TextDisplay::_spellcheck.\n" );

  #if WITH_ASPELL
  // create dialog
  SPELLCHECK::SpellDialog dialog( this );
  dialog.interface().setIgnoredWords( textHighlight().spellParser().interface().ignoredWords() );

  // default dictionary from XmlOptions
  string default_filter( XmlOptions::get().raw("DICTIONARY_FILTER") );
  string default_dictionary( XmlOptions::get().raw( "DICTIONARY" ) );

  // try overwrite with file record
  if( !( file().empty()  || isNewDocument() ) )
  {

    FileRecord& record( _recentFiles().get( file() ) );
    if( !( record.hasProperty( filter_property_id_ ) && dialog.setFilter( record.property( filter_property_id_ ) ) ) )
    { dialog.setFilter( default_filter ); }

    if( !( record.hasProperty( dictionary_property_id_ ) && dialog.setDictionary( record.property( dictionary_property_id_ ) ) ) )
    { dialog.setDictionary( default_dictionary ); }

  }  else {

    dialog.setFilter( default_filter );
    dialog.setDictionary( default_dictionary );

  }

  // connections
  connect( &dialog, SIGNAL( filterChanged( const QString& ) ), SLOT( selectFilter( const QString& ) ) );
  connect( &dialog, SIGNAL( dictionaryChanged( const QString& ) ), SLOT( selectDictionary( const QString& ) ) );

  dialog.nextWord();
  dialog.exec();

  // try overwrite with file record
  if( !( file().empty() || isNewDocument() ) )
  {
    _recentFiles().get( file() )
      .addProperty( filter_property_id_, dialog.filter() )
      .addProperty( dictionary_property_id_, dialog.dictionary() );
  }

  textHighlight().spellParser().interface().mergeIgnoredWords( dialog.interface().ignoredWords() );

  #endif

}

//_______________________________________________________
void TextDisplay::_indentSelection( void )
{
  Debug::Throw( "TextDisplay::_indentSelection.\n" );

  // check activity, indentation and text selection
  if( !indent_->isEnabled() ) return;

  // retrieve text cursor
  QTextCursor cursor( textCursor() );
  if( !cursor.hasSelection() ) return;

  // retrieve blocks
  QTextBlock begin( document()->findBlock( min( cursor.position(), cursor.anchor() ) ) );
  QTextBlock end( document()->findBlock( max( cursor.position(), cursor.anchor() ) ) );

  // need to remove selection otherwise the first adding of a tab
  // will remove the entire selection.

  cursor.clearSelection();
  emit indent( begin, end );

  // select all indented blocks
  cursor.setPosition( begin.position(), QTextCursor::MoveAnchor );
  cursor.setPosition( end.position()+end.length()-1, QTextCursor::KeepAnchor );
  setTextCursor( cursor );

  return;
}

//_______________________________________________________
void TextDisplay::_addBaseIndentation( void )
{
  Debug::Throw( "TextDisplay::_addBaseIndentation.\n" );

  // check activity, indentation and text selection
  if( !indent_->baseIndentation() ) return;

  // define regexp to perform replacement
  QRegExp leading_space_regexp( "^\\s*" );
  QString replacement( indent_->baseIndentation(), ' ' );

  // define blocks to process
  QTextBlock begin;
  QTextBlock end;

  // retrieve cursor
  // retrieve text cursor
  QTextCursor cursor( textCursor() );
  if( !cursor.hasSelection() ) return;

  int position_begin( min( cursor.position(), cursor.anchor() ) );
  int position_end( max( cursor.position(), cursor.anchor() ) );
  begin = document()->findBlock( position_begin );
  end = document()->findBlock( position_end );

  // store blocks
  vector<QTextBlock> blocks;
  for( QTextBlock block = begin; block.isValid() && block != end; block = block.next() )
  { blocks.push_back( block ); }
  blocks.push_back( end );

  // loop over blocks
  for( vector<QTextBlock>::iterator iter = blocks.begin(); iter != blocks.end(); iter++ )
  {
    // check block
    if( !iter->isValid() ) continue;

    // retrieve text
    QString text( iter->text() );

    // look for leading tabs
    if( leading_space_regexp.indexIn( text ) < 0 ) continue;

    // select with cursor
    QTextCursor cursor( *iter );
    cursor.movePosition( QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );
    cursor.setPosition( cursor.position() + leading_space_regexp.matchedLength(), QTextCursor::KeepAnchor );

    cursor.insertText( replacement );


  }

  // indent
  emit indent( begin, end );

  // enable updates
  setUpdatesEnabled( true );

  return;
}

//_______________________________________________________
void TextDisplay::_replaceLeadingTabs( const bool& confirm )
{
  Debug::Throw( "TextDisplay::_replaceLeadingTabs.\n" );

  // ask for confirmation
  if( confirm )
  {

    ostringstream what;
    if( _hasTabEmulation() ) what << "Replace all leading tabs with space characters ?";
    else what << "Replace all leading spaces with tab characters ?";
    if( !QuestionDialog( this, what.str().c_str() ).exec() ) return;

  }

  // disable updates
  setUpdatesEnabled( false );

  // define regexp to perform replacement
  QRegExp wrong_tab_regexp( _hasTabEmulation() ? _normalTabRegExp():_emulatedTabRegExp() );
  QString wrong_tab( _hasTabEmulation() ? normalTabCharacter():emulatedTabCharacter() );

  // define blocks to process
  QTextBlock begin;
  QTextBlock end;

  // retrieve cursor
  QTextCursor cursor( textCursor() );
  if( cursor.hasSelection() )
  {

    int position_begin( min( cursor.position(), cursor.anchor() ) );
    int position_end( max( cursor.position(), cursor.anchor() ) );
    begin = document()->findBlock( position_begin );
    end = document()->findBlock( position_end );

  } else {

    begin = document()->begin();
    end = document()->end();

  }

  // store blocks
  vector<QTextBlock> blocks;
  for( QTextBlock block = begin; block.isValid() && block != end; block = block.next() )
  { blocks.push_back( block ); }
  blocks.push_back( end );

  // loop over blocks
  for( vector<QTextBlock>::iterator iter = blocks.begin(); iter != blocks.end(); iter++ )
  {
    // check block
    if( !iter->isValid() ) continue;

    // retrieve text
    QString text( iter->text() );

    // look for leading tabs
    if( wrong_tab_regexp.indexIn( text ) < 0 ) continue;

    // select with cursor
    QTextCursor cursor( *iter );
    cursor.movePosition( QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );
    cursor.setPosition( cursor.position() + wrong_tab_regexp.matchedLength(), QTextCursor::KeepAnchor );

    // create replacement string and insert.
    QString buffer;
    for( int i=0; i< int(wrong_tab_regexp.matchedLength()/wrong_tab.size()); i++ )
    { buffer += tabCharacter(); }
    cursor.insertText( buffer );

  }

  // enable updates
  setUpdatesEnabled( true );

  Debug::Throw( "TextDisplay::_replaceLeadingTabs - done.\n" );
  return;
}


//_______________________________________________________
void TextDisplay::_fileProperties( void )
{
  Debug::Throw( "TextDisplay::_fileProperties.\n" );
  if( file().empty() || isNewDocument() ) return;

  // prior to showing the dialog 
  // one should add needed tab for misc information
  const FileRecord& record(  _recentFiles().get( file() ) );
  FileInformationDialog dialog( this, record );
  
  // add additional informations frame
  QWidget* box( new QWidget() );
  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing( 5 );
  box->setLayout( layout );
  Debug::Throw( "FileInformationDialog::FileInformationDialog - Miscellaneous tab booked.\n" );
  
  GridLayout* grid_layout = new GridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing( 5 );
  grid_layout->setMaxCount( 2 );
  layout->addLayout( grid_layout );
  
  // number of characters
  grid_layout->addWidget( new QLabel( "number of characters: ", box ) );
  grid_layout->addWidget( new QLabel( Str().assign<int>(toPlainText().size()).c_str(), box ) );
  
  // number of lines
  grid_layout->addWidget( new QLabel( "number of lines: ", box ) );
  grid_layout->addWidget( new QLabel( Str().assign<int>(TextEditor::blockCount()).c_str(), box ) );
  
  grid_layout->addWidget( new QLabel( "Current paragraph highlighting: ", box ) );
  grid_layout->addWidget( new QLabel( (blockHighlightAction().isChecked() ? "true":"false" ), box ) );
  
  grid_layout->addWidget( new QLabel( "Text highlighting: ", box ) );
  grid_layout->addWidget( new QLabel( (textHighlight().isHighlightEnabled() ? "true":"false" ), box ) );
  
  grid_layout->addWidget( new QLabel( "Matching parenthesis highlighting: ", box ) );
  grid_layout->addWidget( new QLabel( (textHighlight().isParenthesisEnabled() ? "true":"false" ), box ) );
  
  grid_layout->addWidget( new QLabel( "Text indent: ", box ) );
  grid_layout->addWidget( new QLabel( (textIndent().isEnabled() ? "true":"false" ), box ) );
  
  grid_layout->addWidget( new QLabel( "Text wrapping: ", box ) );
  grid_layout->addWidget( new QLabel( (wrapModeAction().isChecked() ? "true":"false" ), box ) );
  
  grid_layout->addWidget( new QLabel( "Tab emulation: ", box ) );
  grid_layout->addWidget( new QLabel( (tabEmulationAction().isChecked() ? "true":"false" ), box ) );
  
  grid_layout->setColumnStretch( 1, 1 );
  
  // autosave
  layout->addWidget( new QLabel( "Auto-save filename: ", box ) );
  layout->addWidget( new QLabel( AutoSaveThread::autoSaveName( file() ).c_str(), box ) );
  
  layout->addStretch();
  
  dialog.tabWidget().addTab( box, "&Miscellaneous" );
  
  // execute dialog
  dialog.centerOnWidget( window() ).exec();
  
}

//_____________________________________________________________
void TextDisplay::_setBlockModified( int position, int, int added )
{
  QTextBlock begin( document()->findBlock( position ) );
  QTextBlock end(  document()->findBlock( position + added ) );

  for( QTextBlock block = begin; block.isValid() && block != end; block = block.next() )
  { _setBlockModified( block ); }

  _setBlockModified( end );

}

//__________________________________________________
void TextDisplay::_textModified( void )
{
  Debug::Throw( "TextDisplay::_textModified.\n" );

  // document should never appear modified
  // for readonly displays
  if( document()->isModified() && isReadOnly() ) document()->setModified( false );
  if( isActive() && ( file().size() || isNewDocument() ) ) emit needUpdate( MODIFIED );
  
}

//__________________________________________________
void TextDisplay::_ignoreMisspelledWord( QString word )
{
  Debug::Throw() << "TextDisplay::_ignoreMisspelledWord - word: " << qPrintable( word ) << endl;
  #if WITH_ASPELL
  textHighlight().spellParser().interface().ignoreWord( qPrintable( word ) );
  rehighlight();
  #endif
  return;

}

//__________________________________________________
void TextDisplay::_replaceMisspelledSelection( QString word )
{

  #if WITH_ASPELL
  Debug::Throw() << "TextDisplay::_replaceMisspelledSelection - word: " << qPrintable( word ) << endl;
  QTextCursor cursor( textCursor() );
  cursor.insertText( word );
  #endif
  return;

}

//__________________________________________________
void TextDisplay::_highlightParenthesis( void )
{

  if( !( hasTextHighlight() && textHighlight().isParenthesisEnabled() ) ) return;

  // clear previous parenthesis
  parenthesisHighlight().clear();

  // retrieve TextCursor
  QTextCursor cursor( textCursor() );
  if( cursor.atBlockStart() ) return;

  // retrieve block
  QTextBlock block( cursor.block() );

  // store local position in block
  int position(cursor.position()-block.position());

  // retrieve text block data
  HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
  if( !data ) return;  

  QString text( block.text() );
  const TextParenthesis::List& parenthesis( textHighlight().parenthesis() );
     
  // check against opening parenthesis
  bool found( false );
  TextParenthesis::List::const_iterator iter( find_if( 
    parenthesis.begin(), parenthesis.end(), 
    TextParenthesis::FirstElementFTor( text.left( position ) ) ) );
  
  if( iter != parenthesis.end() )
  {
    int increment( 0 );
    while( block.isValid() && !found )
    {
      // retrieve block text
      QString text( block.text() );

      // parse text
      while( (position = text.indexOf( iter->regexp(), position ) ) >= 0 )
      {
        if( text.mid(position, iter->regexp().matchedLength() ) == iter->second() ) increment--;
        else if( text.mid(position, iter->regexp().matchedLength() ) == iter->first() ) increment++;

        if( increment < 0 )
        {
          found = true;
          break;
        }

        position++;

      }

      if( !found )
      {
        block = block.next();
        position = 0;
      }
    }
  }

  // if not found, check against closing parenthesis
  if( !( found || (iter = 
    find_if( 
    parenthesis.begin(), parenthesis.end(), 
    TextParenthesis::SecondElementFTor( text.left( position ) ) )) == parenthesis.end()  ) )
  {
    int increment( 0 );
    position -= (iter->second().size() + 1 );
    while( block.isValid() && !found )
    {
      // retrieve block text
      QString text( block.text() );

      // parse text
      while( position >= 0 && (position = text.lastIndexOf( iter->regexp(), position ) ) >= 0 )
      {
        
        if( text.mid(position, iter->regexp().matchedLength() ) == iter->first() ) increment--;
        else if( text.mid(position, iter->regexp().matchedLength() ) == iter->second() ) increment++;

        if( increment < 0 )
        {
          found = true;
          break;
        }

        position--;

      }

      if( !found )
      {
        block = block.previous();
        if( block.isValid() ) position = block.text().length() - 1;
      }
    }
  }

  if( found ) parenthesisHighlight().highlight( position + block.position(), iter->regexp().matchedLength() );
  
  return;

}

//__________________________________________________
void TextDisplay::_tagBlock( void )
{

  Debug::Throw( "TextDisplay::_tagBlock.\n" );
  vector<QTextBlock> blocks;
  QTextCursor cursor( textCursor() );
  if( cursor.hasSelection() )
  {

    QTextBlock first( document()->findBlock( min( cursor.position(), cursor.anchor() ) ) );
    QTextBlock last( document()->findBlock( max( cursor.position(), cursor.anchor() ) ) );
    for( QTextBlock block( first ); block.isValid() && block != last;  block = block.next() )
    { blocks.push_back( block ); }
    if( last.isValid() ) blocks.push_back( last );

  } else {

    // add previous blocks and current
    for( QTextBlock block( cursor.block() ); block.isValid(); block = block.previous() )
    {
      TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
      if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) blocks.push_back( block );
      else break;
    }

     // add previous blocks and current
    for( QTextBlock block( cursor.block().next() ); block.isValid(); block = block.next() )
    {
      TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
      if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) blocks.push_back( block );
      else break;
    }

  }

  // clear background for selected blocks
  for( vector<QTextBlock>::iterator iter = blocks.begin(); iter != blocks.end(); iter++ )
  { tagBlock( *iter, TextBlock::USER_TAG ); }

}

//__________________________________________________
void TextDisplay::_nextTag( void )
{
  Debug::Throw( "TextDisplay::_nextTag.\n" );
  QTextCursor cursor( textCursor() );
  QTextBlock block( cursor.block() );
  TextBlockData* data;

  // first skipp blocks that have tags if the first one has
  while(
    block.isValid() &&
    (data = static_cast<TextBlockData*>( block.userData() ) ) &&
    data->hasFlag( TextBlock::ALL_TAGS ) )
  { block = block.next(); }

  // skip blocks with no tag
  while(
    block.isValid() &&
    !((data = static_cast<TextBlockData*>( block.userData() ) ) &&
    data->hasFlag( TextBlock::ALL_TAGS ) ) )
  { block = block.next(); }

  if( !block.isValid() )
  { 
    InformationDialog( this, "No tagged block found." ).exec();
    return;
  }

  // update cursor
  cursor.setPosition( block.position() );
  setTextCursor( cursor );
  return;

}

//__________________________________________________
void TextDisplay::_previousTag( void )
{
  Debug::Throw( "TextDisplay::_previousTag.\n" );
  QTextCursor cursor( textCursor() );
  QTextBlock block( cursor.block() );
  TextBlockData* data;

  // first skipp blocks that have tags if the first one has
  while(
    block.isValid() &&
    (data = static_cast<TextBlockData*>( block.userData() ) ) &&
    data->hasFlag( TextBlock::ALL_TAGS ) )
  { block = block.previous(); }

  // skip blocks with no tag
  while(
    block.isValid() &&
    !((data = static_cast<TextBlockData*>( block.userData() ) ) &&
    data->hasFlag( TextBlock::ALL_TAGS ) ) )
  { block = block.previous(); }

  if( !block.isValid() )
  { 
    InformationDialog( this, "No tagged block found." ).exec();
    return;
  }

  // update cursor
  cursor.setPosition( block.position() );
  setTextCursor( cursor );
  return;

}

//___________________________________________________________________________
void TextDisplay::_clearTag( void )
{

  Debug::Throw( "TextEditor::_clearTag.\n" );

  vector<QTextBlock> blocks;
  QTextCursor cursor( textCursor() );
  if( cursor.hasSelection() )
  {

    QTextBlock first( document()->findBlock( min( cursor.position(), cursor.anchor() ) ) );
    QTextBlock last( document()->findBlock( max( cursor.position(), cursor.anchor() ) ) );
    for( QTextBlock block( first ); block.isValid() && block != last;  block = block.next() )
    { blocks.push_back( block ); }
    if( last.isValid() ) blocks.push_back( last );

  } else {

    // add previous blocks and current
    for( QTextBlock block( cursor.block() ); block.isValid(); block = block.previous() )
    {
      TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
      if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) blocks.push_back( block );
      else break;
    }

     // add previous blocks and current
    for( QTextBlock block( cursor.block().next() ); block.isValid(); block = block.next() )
    {
      TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
      if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) blocks.push_back( block );
      else break;
    }

  }

  // clear background for selected blocks
  for( vector<QTextBlock>::iterator iter = blocks.begin(); iter != blocks.end(); iter++ )
  { clearTag( *iter, TextBlock::ALL_TAGS ); }
  
}
