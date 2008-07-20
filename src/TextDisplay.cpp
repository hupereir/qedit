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
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QTextLayout>

#include "AutoSave.h"
#include "AutoSaveThread.h"
#include "BlockDelimiterDisplay.h"
#include "CustomFileDialog.h"
#include "CustomTextDocument.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "FileInfoDialog.h"
#include "FileModifiedDialog.h"
#include "FileRemovedDialog.h"
#include "FileSelectionDialog.h"
#include "HtmlUtil.h"
#include "HighlightBlockData.h"
#include "HighlightBlockFlags.h"
#include "IconEngine.h"
#include "Icons.h"
#include "LineNumberDisplay.h"
#include "MainFrame.h"
#include "OpenPreviousMenu.h"
#include "QtUtil.h"
#include "ReplaceDialog.h"
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

// empty line regular expression
const QRegExp TextDisplay::empty_line_regexp_( "(^\\s*$)" );

//___________________________________________________
TextDisplay::TextDisplay( QWidget* parent ):
  TextEditor( parent ),
  file_( "" ),
  working_directory_( Util::workingDirectory() ),
  class_name_( "" ),
  left_margin_( 0 ),
  ignore_warnings_( false ),
  show_line_number_action_( 0 ),
  show_block_delimiter_action_( 0 ),
  open_previous_menu_( 0 ),
  line_number_display_( 0 ),
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
  
  // line number
  line_number_display_ = new LineNumberDisplay( this );

  // connections
  connect( this, SIGNAL( selectionChanged() ), SLOT( _selectionChanged() ) );
  connect( this, SIGNAL( cursorPositionChanged() ), SLOT( _highlightParenthesis() ) );
  connect( this, SIGNAL( indent( QTextBlock ) ), indent_, SLOT( indent( QTextBlock ) ) );
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
  connect( TextDisplay::document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged( int ) ) );

  // track configuration modifications
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( spellCheckConfigurationChanged() ), SLOT( _updateSpellCheckConfiguration() ) );
  connect( qApp, SIGNAL( documentClassesChanged() ), SLOT( updateDocumentClass() ) );
  _updateConfiguration();
  _updateSpellCheckConfiguration();

  Debug::Throw( "TextDisplay::TextDisplay - done.\n" );

}

//_____________________________________________________
TextDisplay::~TextDisplay( void )
{ Debug::Throw() << "TextDisplay::~TextDisplay - key: " << key() << endl; }

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
  if( isActive() )
  emit needUpdate( WINDOW_TITLE | UNDO_REDO );

}

//_____________________________________________________
void TextDisplay::setReadOnly( const bool& value )
{
  Debug::Throw() << "TextDisplay::setReadOnly - value: " << value << endl;
  TextEditor::setReadOnly( value );
  emit needUpdate( WINDOW_TITLE | CUT | PASTE | UNDO_REDO | SAVE );
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
  TextEditor::installContextMenuActions( menu, all_actions );
  
  // add specific actions
  menu.insertAction( &wrapModeAction(), &showLineNumberAction() );
  menu.insertAction( &wrapModeAction(), &showBlockDelimiterAction() );
  menu.addSeparator();

  menu.addAction( &tagBlockAction() );
  tagBlockAction().setEnabled( has_selection );
    
  menu.addAction( &nextTagAction() );
  nextTagAction().setEnabled( has_tags );

  menu.addAction( &previousTagAction() );
  previousTagAction().setEnabled( has_tags );

  menu.addAction( &clearTagAction() );
  clearTagAction().setEnabled( current_block_tagged );

  menu.addAction( &clearAllTagsAction() );
  clearAllTagsAction().setEnabled( has_tags );

  return;
}

//___________________________________________________________________________
void TextDisplay::synchronize( TextDisplay* display )
{

  Debug::Throw( "TextDisplay::synchronize.\n" );

  // synchronize text
  // (from base class)
  TextEditor::synchronize( display );
  text_highlight_ = &display->textHighlight();

  // restore connection with document
  // track contents changed for syntax highlighting
  connect( TextDisplay::document(), SIGNAL( contentsChange( int, int, int ) ), SLOT( _setBlockModified( int, int, int ) ) );
  connect( TextDisplay::document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( void ) ) );
  connect( TextDisplay::document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged( int ) ) );

  // indentation
  textIndent().setPatterns( display->textIndent().patterns() );
  textIndent().setBaseIndentation( display->textIndent().baseIndentation() );

  // block delimiters and line numbers
  blockDelimiterDisplay().synchronize( &display->blockDelimiterDisplay() );
  lineNumberDisplay().synchronize( &display->lineNumberDisplay() );
  
  textIndentAction().setChecked( display->textIndentAction().isChecked() );
  textHighlightAction().setChecked( display->textHighlightAction().isChecked() );
  parenthesisHighlightAction().setChecked( display->parenthesisHighlightAction().isChecked() );
  // showLineNumberAction().setChecked( display->showLineNumberAction().isChecked() );
  showBlockDelimiterAction().setChecked( display->showBlockDelimiterAction().isChecked() );

  _setMacros( display->macros() );

  // file
  setFile( display->file() );

}

//____________________________________________
void TextDisplay::openFile( File file, bool check_autosave )
{

  Debug::Throw() << "TextDisplay::openFile " << file << endl;

  // reset class name
  QString class_name( openPreviousMenu().add( file ).information("class_name").c_str() );
  setClassName( class_name );

  // expand filename
  file = file.expand();

  // check is there is an "AutoSave" file matching with more recent modification time
  // here, when the diff is working, I could offer the possibility to show a diff between
  // the saved file and the backup
  bool restore_autosave( false );
  File tmp( file );

  if(1) 
  {
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
      if( QtUtil::questionDialog( this, what.str() ) )
      {
        restore_autosave = true;
        tmp = autosaved;
      }
    }
    
    // retrieve display and associated
    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      (*iter)->setFile( file );
      (*iter)->setClassName( className() );
      (*iter)->updateDocumentClass();
    }
    
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
  
  // save file if restored from autosaved.
  if( restore_autosave && !isReadOnly() ) save();

  // perform first autosave
  (dynamic_cast<MainFrame*>(qApp))->autoSave().saveFiles( this );

  // update openPrevious menu
  if( !TextDisplay::file().empty() )
  { openPreviousMenu().get( TextDisplay::file() ).addInformation( "class_name", qPrintable( className() ) ); }

  //Debug::Throw( "TextDisplay::openFile - done.\n" );

}

//_______________________________________________________
void TextDisplay::setFile( const File& file )
{

  Debug::Throw() << "TextDisplay::setFile - file: " << file << endl;
  file_ = file;
  if( file.exists() )
  {
    _setLastSaved( file.lastModified() );
    _setWorkingDirectory( file.path() );
    _setIgnoreWarnings( false );
  }

  // check if file is read-only
  checkFileReadOnly();

  if( isActive() ) emit needUpdate( WINDOW_TITLE | FILE_NAME );

}

//___________________________________________________________________________
FileRemovedDialog::ReturnCode TextDisplay::checkFileRemoved( void )
{
  Debug::Throw( "TextDisplay::checkFileRemoved.\n" );

  if( _ignoreWarnings() || !_fileRemoved() ) return FileRemovedDialog::IGNORE;

  // disable check
  FileRemovedDialog dialog( this, file() );
  QtUtil::centerOnParent( &dialog );
  int state( dialog.exec() );

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
  QtUtil::centerOnParent( &dialog );
  int state( dialog.exec() );
  if( state == FileModifiedDialog::RESAVE ) { save(); }
  else if( state == FileModifiedDialog::SAVE_AS ) { saveAs(); }
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
  if( enable_all ) flags |=  AskForSaveDialog::ALL;

  AskForSaveDialog dialog( this, file(), flags );
  QtUtil::centerOnParent( &dialog );
  int state( dialog.exec() );
  if( state == AskForSaveDialog::YES ||  state == AskForSaveDialog::ALL ) save();
  else if( state == AskForSaveDialog::NO ) setModified( false );

  return AskForSaveDialog::ReturnCode(state);

}

//___________________________________________________________________________
void TextDisplay::save( void )
{
   Debug::Throw( "TextDisplay::save.\n" );

  // do nothing if not modified
  if( !document()->isModified() ) return;

  // check file name
  if( file().empty() ) return saveAs();

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
      QtUtil::infoDialog( this, what.str() );
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
  { openPreviousMenu().get( file() ).addInformation( "class_name", qPrintable( className() ) ); }

  return;

}

//___________________________________________________________________________
void TextDisplay::saveAs( void )
{
  Debug::Throw( "TextDisplay::saveAs.\n" );

  // define default file
  File default_file( file() );
  if( default_file.empty() ) default_file = File( "document" ).addPath( workingDirectory() );

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
    QtUtil::infoDialog( this, what.str() );
    return;
  }

  // check if file exists
  if( file.exists() )
  {
    if( !file.isWritable() )
    {
      ostringstream what;
      what << "File \"" << file << "\" is read-only. <Save> canceled.";
      QtUtil::infoDialog( this, what.str() );
      return;
    } else if( !QtUtil::questionDialog( this, "Selected file already exists. Overwrite ?" ) )
    { return; }
  }

  // update filename and document class for this and associates
  BASE::KeySet<TextDisplay> displays( this );
  displays.insert( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {

    // update file
    (*iter)->setFile( file );

    // update document class
    // the class name is reset, to allow a document class
    // matching the new filename to get loaded
    (*iter)->setClassName("");
    (*iter)->updateDocumentClass();

  }

  // set document as modified and save using new filename
  setModified( true );
  save();

  // rehighlight
  rehighlight();
  if( !TextDisplay::file().empty() )
  { openPreviousMenu().get( TextDisplay::file() ).addInformation( "class_name", qPrintable( className() ) ); }

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
  openFile( file(), false );

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

  // loop over text blocks
  for( QTextBlock block = TextDisplay::document()->begin(); block.isValid(); block = block.next() )
  {

    PatternLocationSet locations;

    // try retrieve highlightBlockData
    HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( data ) locations = data->locations();

    // retrieve text
    QString text( block.text() );

    // current pattern
    QDomElement span;
    const HighlightPattern *current_pattern = 0;
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

      const HighlightPattern* pattern = ( location_iter == locations.rend() ) ? 0:&location_iter->parent();
      if( pattern != current_pattern || index == 0 || line_break )
      {

        // append text to current element and reset stream
        if( !buffer.isEmpty() )
        {
          if( span.isNull() ) span  = out.appendChild( document.createElement( "span" ) ).toElement();
          HtmlUtil::textNode( buffer, span, document );
          if( line_break )
          {
            out.appendChild( document.createElement( "br" ) );
            line_break = false;
            line_index = 0;
          }
          buffer = "";
        }  

        // update pattern
        current_pattern = pattern;

        // update current element
        span = out.appendChild( document.createElement( "span" ) ).toElement();
        if( current_pattern )
        {

          // retrieve font format
          const unsigned int& format( current_pattern->style().fontFormat() );
          ostringstream format_stream;
          if( format & FORMAT::UNDERLINE ) format_stream << "text-decoration: underline; ";
          if( format & FORMAT::ITALIC ) format_stream << "font-style: italic; ";
          if( format & FORMAT::BOLD ) format_stream << "font-weight: bold; ";
          if( format & FORMAT::STRIKE ) format_stream << "text-decoration: line-through; ";

          // retrieve color
          const QColor& color = current_pattern->style().color();
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

    default:
    { throw logic_error( DESCRIPTION( "invalid tag" ) ); }

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
void TextDisplay::updateDocumentClass( void )
{

  Debug::Throw( "TextDisplay::updateDocumentClass\n" );
  textHighlight().clear();
  textIndent().clear();
  textIndent().setBaseIndentation(0);
  _clearMacros();

  // default document class is empty
  DocumentClass document_class;

  // try load document class from class_name
  if( !className().isEmpty() )
  { document_class = dynamic_cast<MainFrame*>(qApp)->classManager().get( className() ); }

  // try load from file
  if( document_class.name().isEmpty() && !file().empty() )
  { document_class = dynamic_cast<MainFrame*>(qApp)->classManager().find( file() ); }

  // update class name
  setClassName( document_class.name() );

  // wrap mode
  if( XmlOptions::get().get<bool>( "WRAP_FROM_CLASS" ) ) wrapModeAction().setChecked( document_class.wrap() );

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
  if( !file().empty() )
  { 
    FileRecord& record( openPreviousMenu().get( file() ) );
    record.addInformation( "class_name", qPrintable( className() ) ); 
    if( !document_class.icon().isEmpty() ) record.addInformation( "icon", qPrintable( document_class.icon() ) );
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
    QtUtil::infoDialog( this, what.str() );
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
  _replaceLeadingTabs( false );

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
  if( !file().empty() )
  {
    FileRecord& record( openPreviousMenu().get( file() ) );
    record.addInformation( "filter", interface.filter() );
  }

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
  if( !file().empty() )
  {
    FileRecord& record( openPreviousMenu().get( file() ) );
    record.addInformation( "dictionary", interface.dictionary() );
  }

  // rehighlight if needed
  if( textHighlight().spellParser().isEnabled() ) rehighlight();
  #endif

  return;

}

//_______________________________________________________
bool TextDisplay::event( QEvent* event )
{
  
  bool has_block_delimiters( hasBlockDelimiterDisplay() && hasBlockDelimiterAction() && showBlockDelimiterAction().isVisible() && showBlockDelimiterAction().isChecked() );
  bool has_line_numbers( hasLineNumberDisplay() && hasLineNumberAction() && showLineNumberAction().isVisible() && showLineNumberAction().isChecked() );

  
  // check that all needed widgets/actions are valid and checked.
  switch (event->type()) 
  {
    
    case QEvent::Paint:
    if( has_block_delimiters || has_line_numbers ) 
    {
      // block delimiter
      QPainter painter( this );
      
      int height( TextDisplay::height() - 2*frameWidth() );
      if( horizontalScrollBar()->isVisible() ) { height -= horizontalScrollBar()->height() + 2; }
      
      painter.translate( frameWidth(),  frameWidth() );
      painter.setClipRect( 0, 0, left_margin_, height );
      
      painter.setBrush( margin_background_color_ );
      painter.setPen( Qt::NoPen );
      painter.drawRect( 0, 0, left_margin_, height );
      
      int y_offset = verticalScrollBar()->value();      
      painter.translate( 0, -y_offset );

      if( has_line_numbers ) lineNumberDisplay().paint( painter ); 
      if( has_block_delimiters ) blockDelimiterDisplay().paint( painter ); 
      painter.end();
    }
    break;
      
    case QEvent::MouseButtonPress:
    if( has_block_delimiters ) blockDelimiterDisplay().mousePressEvent( static_cast<QMouseEvent*>( event ) );
    break;
    
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
  { emit indent( textCursor().block() ); }
  else
  {
    
    // process key
    TextEditor::keyPressEvent( event );
    
    // indent current paragraph when return is pressed
    if( indent_->isEnabled() && event->key() == Qt::Key_Return && !textCursor().hasSelection() )
    { emit indent( textCursor().block() ); }
    
    // reindent paragraph if needed
    /* remark: this is c++ specific. The list of keys should be set in the document class */
    if( indent_->isEnabled() && ( event->key() == Qt::Key_BraceRight || event->key() == Qt::Key_BraceLeft ) && !textCursor().hasSelection() )
    { emit indent( textCursor().block() ); }

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
  Debug::Throw( "TextEditor::paintEvent.\n" );  
  TextEditor::paintEvent( event );
  
  // handle block background
  QRect rect = event->rect();
  QTextBlock first( cursorForPosition( rect.topLeft() ).block() );
  QTextBlock last( cursorForPosition( rect.bottomRight() ).block() );

  // translate rect from widget to viewport coordinates
  rect.translate( scrollbarPosition() );

  // create painter and translate from widget to viewport coordinates
  QPainter painter( viewport() );
  painter.translate( -scrollbarPosition() );
  if( margin_foreground_color_.isValid() ) painter.setPen( margin_foreground_color_ );
  
  // loop over found blocks  
  for( QTextBlock block( first ); block != last.next() && block.isValid(); block = block.next() )
  {
    if( !block.blockFormat().boolProperty( TextBlock::Collapsed ) ) continue;
    
    QRectF block_rect( document()->documentLayout()->blockBoundingRect( block ) );
    block_rect.setWidth( viewport()->width() + scrollbarPosition().x() );
    painter.drawLine( block_rect.bottomLeft() - QPoint( 2, 0 ), block_rect.bottomRight() );
  }
  painter.end();

  // this is needed to force update of the block delimiter widget
  if( 
    ( showBlockDelimiterAction().isVisible() && showBlockDelimiterAction().isChecked() ) ||
    ( showLineNumberAction().isVisible() && showLineNumberAction().isChecked() ) )
  { QFrame::update(); }
  
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
  addAction( text_indent_action_ = new QAction( "&Indent text", this ) );
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

  addAction( show_line_number_action_ =new QAction( "Show line numbers", this ) );
  show_line_number_action_->setToolTip( "Show/hide line numbers" );
  show_line_number_action_->setCheckable( true );
  show_line_number_action_->setShortcut( Qt::Key_F11 );
  show_line_number_action_->setShortcutContext( Qt::WidgetShortcut );
  connect( show_line_number_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleShowLineNumbers( bool ) ) );

  addAction( show_block_delimiter_action_ =new QAction( "Show block delimiters", this ) );
  show_block_delimiter_action_->setToolTip( "Show/hide block delimiters" );
  show_block_delimiter_action_->setCheckable( true );
  show_block_delimiter_action_->setShortcut( Qt::Key_F9 );
  show_block_delimiter_action_->setShortcutContext( Qt::WidgetShortcut );
  connect( show_block_delimiter_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleShowBlockDelimiters( bool ) ) );

  // autospell
  addAction( autospell_action_ = new QAction( "&Automatic spell-check", this ) );
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
  addAction( file_info_action_ = new QAction( IconEngine::get( ICONS::INFO ), "&File information", this ) );
  connect( file_info_action_, SIGNAL( triggered() ), SLOT( _showFileInfo() ) );

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

//_____________________________________________________________________
void TextDisplay::_createReplaceDialog( void )
{
  Debug::Throw( "TextDisplay::_CreateReplaceDialog.\n" );
  TextEditor::_createReplaceDialog();
  ReplaceDialog &dialog( _replaceDialog() );

  // insert multiple file buttons
  QPushButton* button = new QPushButton( "&Files", &dialog );
  connect( button, SIGNAL( clicked() ), SLOT( _multipleFileReplace() ) );
  button->setToolTip( "replace all occurence of the search string in the selected files" );
  dialog.addDisabledButton( button );
  dialog.locationLayout().addWidget( button );

}

//_____________________________________________________________
bool TextDisplay::_contentsChanged( void ) const
{

  Debug::Throw( "TextDisplay::_contentsChanged.\n" );

  // check file
  if( file().empty() ) return true;

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
  return (!file().empty() && last_save_.isValid() && !file().exists() );
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
  int left_margin( 0 );
  
  if( showLineNumberAction().isChecked() && showLineNumberAction().isVisible() )
  { left_margin += lineNumberDisplay().width(); }

  blockDelimiterDisplay().setOffset( left_margin );
  if( showBlockDelimiterAction().isChecked() && showBlockDelimiterAction().isVisible() )
  { left_margin += fontMetrics().lineSpacing(); }
  
  if( left_margin_ == left_margin ) return false;
  
  left_margin_ = left_margin;
  setViewportMargins( left_margin, 0, 0, 0 );
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
  showLineNumberAction().setChecked( XmlOptions::get().get<bool>( "SHOW_LINE_NUMBERS" ) );
  showBlockDelimiterAction().setChecked( XmlOptions::get().get<bool>( "SHOW_BLOCK_DELIMITERS" ) );
  margin_foreground_color_ = QColor( XmlOptions::get().get<string>("DELIMITER_FOREGROUND").c_str() );
  margin_background_color_ = QColor( XmlOptions::get().get<string>("DELIMITER_BACKGROUND").c_str() );
  
  // retrieve diff colors
  diff_conflict_color_ = QColor( XmlOptions::get().get<string>("DIFF_CONFLICT_COLOR").c_str() );
  diff_added_color_ = QColor( XmlOptions::get().get<string>("DIFF_ADDED_COLOR").c_str() );
  user_tag_color_ = QColor( XmlOptions::get().get<string>("TAGGED_BLOCK_COLOR").c_str() );

  // update paragraph tags
  _updateTaggedBlocks();

}

//___________________________________________________________________________
void TextDisplay::_updateSpellCheckConfiguration( void )
{
  Debug::Throw( "TextDisplay::_updateSpellCheckConfiguration.\n" );

  #if WITH_ASPELL

  // spellcheck configuration
  bool changed( false );
  changed |= textHighlight().spellParser().setColor( QColor( XmlOptions::get().get<string>("AUTOSPELL_COLOR").c_str() ) );
  changed |= textHighlight().spellParser().setFontFormat( XmlOptions::get().get<unsigned int>("AUTOSPELL_FONT_FORMAT") );
  textHighlight().updateSpellPattern();
  autoSpellAction().setChecked( XmlOptions::get().get<bool>("AUTOSPELL") );
  autoSpellAction().setEnabled( textHighlight().spellParser().color().isValid() );

  // store local reference to spell interface
  SPELLCHECK::SpellInterface& interface( textHighlight().spellParser().interface() );

  // load default filter and dictionaries
  string filter( XmlOptions::get().raw("DICTIONARY_FILTER") );
  string dictionary( XmlOptions::get().raw("DICTIONARY") );

  // overwrite with file record
  if( !file().empty() )
  {
    FileRecord& record( openPreviousMenu().get( file() ) );
    if( record.hasInformation( "filter" ) && interface.hasFilter( record.information( "filter" ) ) )
    { filter = record.information( "filter" ); }

    if( record.hasInformation( "dictionary" ) && interface.hasDictionary( record.information( "dictionary" ) ) )
    { dictionary = record.information( "dictionary" ); }

  }

  // see if one should/can change the dictionary and filter
  if( filter != interface.filter() && interface.setFilter( filter ) )
  {
    _filterMenu().select( filter.c_str() );
    changed = true;
  }

  if( dictionary != interface.dictionary() && interface.setDictionary( dictionary ) )
  {
    _dictionaryMenu().select( dictionary.c_str() );
    changed = true;
  }

  // rehighlight if needed
  if( changed && autoSpellAction().isChecked() && autoSpellAction().isEnabled() ) rehighlight();

  #endif

}

//_______________________________________________________
void TextDisplay::_indentCurrentParagraph( void )
{
  Debug::Throw( "TextDisplay::_indentCurrentParagraph.\n" );
  if( !indent_->isEnabled() ) return;
  emit indent( textCursor().block() );
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
void TextDisplay::_toggleShowLineNumbers( bool state )
{

  _updateMargins();
  
  // propagate to other displays
  if( isSynchronized() )
  {
    // temporarely disable synchronization
    // to avoid infinite loop
    setSynchronized( false );

    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { if( (*iter)->isSynchronized() ) (*iter)->showLineNumberAction().setChecked( state ); }
    setSynchronized( true );

  }

  return;
}

//_______________________________________________________
void TextDisplay::_toggleShowBlockDelimiters( bool state )
{
    
  _updateMargins();
  
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


//________________________________________________________
void TextDisplay::_blockCountChanged( int count )
{
  
  Debug::Throw( "TextDisplay::_blockCountChanged.\n" );
  if( !( hasLineNumberDisplay() && lineNumberDisplay().updateWidth( count ) ) ) return;
  if( !( hasLineNumberAction() && showLineNumberAction().isChecked() && showLineNumberAction().isVisible() ) ) return;
  _updateMargins();
  update();
  
}

//_______________________________________________________
void TextDisplay::_multipleFileReplace( void )
{
  Debug::Throw( "TextDisplay::_multipleFileReplace.\n" );
  TextSelection selection( _replaceDialog().selection( false ) );

  // retrieve selection from replace dialog
  FileSelectionDialog dialog( this, selection );
  connect( &dialog, SIGNAL( fileSelected( std::list<File>, TextSelection ) ), qApp, SLOT( multipleFileReplace( std::list<File>, TextSelection ) ) );
  QtUtil::centerOnParent( &dialog );
  dialog.exec();
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
  string default_dictionary( XmlOptions::get().raw("DICTIONARY") );

  // try overwrite with file record
  if( !file().empty() )
  {

    FileRecord& record( openPreviousMenu().get( file() ) );
    if( !( record.hasInformation( "filter" ) && dialog.setFilter( record.information( "filter" ) ) ) )
    { dialog.setFilter( default_filter ); }

    if( !( record.hasInformation( "dictionary" ) && dialog.setDictionary( record.information( "dictionary" ) ) ) )
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
  if( !file().empty() )
  {
    FileRecord& record( openPreviousMenu().get( file() ) );
    record.addInformation( "filter", dialog.filter() );
    record.addInformation( "dictionary", dialog.dictionary() );
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
    if( !QtUtil::questionDialog( this, what.str() ) ) return;

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
void TextDisplay::_showFileInfo( void )
{
  Debug::Throw( "TextDisplay::_showFileInfo.\n" );
  FileInfoDialog dialog( this );
  QtUtil::centerOnParent( &dialog );
  dialog.exec();
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
  if( isActive() ) emit needUpdate( WINDOW_TITLE );
  
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

  if( !textHighlight().isParenthesisEnabled() ) return;

  // clear previous parenthesis
  parenthesisHighlight().clear();

  // retrieve TextCursor
  QTextCursor cursor( textCursor() );
  if( cursor.atBlockStart() ) return;

  // retrieve block
  QTextBlock block( cursor.block() );
  //if( ignoreBlock( block ) ) return;

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
  { return QtUtil::infoDialog( this, "No tagged block found." ); }

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
  { return QtUtil::infoDialog( this, "No tagged block found." ); }

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
