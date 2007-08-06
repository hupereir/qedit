// $Id$

/******************************************************************************
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
*******************************************************************************/


/*!
  \file TextDisplay.cc
  \brief text display window
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QApplication>
#include <QPushButton>
#include <QScrollBar>

#include "AutoSave.h"
#include "AutoSaveThread.h"
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
#include "MainFrame.h"
#include "XmlOptions.h"
#include "QtUtil.h"
#include "ReplaceDialog.h"
#include "TextBraces.h"
#include "TextDisplay.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "Util.h"

using namespace std;
using namespace Qt;

// empty line regular expression
const QRegExp TextDisplay::empty_line_regexp_( "(^\\s*$)" );

//___________________________________________________
TextDisplay::TextDisplay( QWidget* parent ):
  CustomTextEdit( parent ),
  file_( "" ),
  working_directory_( Util::workingDirectory() ), 
  class_name_( "" ),
  flags_( 0 ),
  ignore_warnings_( false ),
  active_( false ),
  indent_( new TextIndent( this ) )
{
  
  Debug::Throw( "TextDisplay::TextDisplay.\n" );

  // tell frame to delete on exit
  // setAttribute( Qt::WA_DeleteOnClose );
  
  // text highlight
  TextHighlight* highlight = new TextHighlight( document() );
  BASE::Key::associate(  dynamic_cast<BASE::Key*>(document()), highlight );
  highlight->setEnabled( false );
  indent_->setEnabled( false );

  // connections
  connect( this, SIGNAL( selectionChanged() ), SLOT( _selectionChanged() ) );
  connect( this, SIGNAL( indent( QTextBlock ) ), indent_, SLOT( indent( QTextBlock ) ) );
  
  // actions
  addAction( text_indent_action_ = new QAction( "&Indent text", this ) );
  text_indent_action_->setCheckable( true );
  text_indent_action_->setChecked( textIndent().isEnabled() );
  connect( text_indent_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleTextIndent( bool ) ) );

  addAction( text_highlight_action_ = new QAction( "&Highlight text", this ) );
  text_highlight_action_->setCheckable( true );
  text_highlight_action_->setChecked( textHighlight().isEnabled() );
  connect( text_highlight_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleTextHighlight( bool ) ) );
  
  addAction( braces_highlight_action_ = new QAction( "&Highlight braces", this ) );
  braces_highlight_action_->setCheckable( true );
  braces_highlight_action_->setChecked( false );
  connect( braces_highlight_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleBracesHighlight( bool ) ) );
  
  addAction( file_info_action_ = new QAction( "&File information", this ) );
  connect( file_info_action_, SIGNAL( triggered() ), SLOT( _showFileInfo() ) );
  
  // connections
  // track contents changed for syntax highlighting
  connect( TextDisplay::document(), SIGNAL( contentsChange( int, int, int ) ), SLOT( _setBlockModified( int ) ) );
  connect( TextDisplay::document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( void ) ) );

  // track configuration modifications
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() ) );
  
  // track document classes modifications
  // connect( qApp, SIGNAL( documentClassesChanged() ), SLOT( updateDocumentClass() ) );
  Debug::Throw( "TextDisplay::TextDisplay - done.\n" );
  
}

//_____________________________________________________
TextDisplay::~TextDisplay( void )
{ Debug::Throw() << "TextDisplay::~TextDisplay - key: " << key() << endl; }

//___________________________________________________________________________
void TextDisplay::synchronize( TextDisplay* display )
{
 
  Debug::Throw( "TextDisplay::synchronize.\n" );
      
  // synchronize text
  // (from base class)
  CustomTextEdit::synchronize( display );

  // update flags
  setFlags( display->flags() );
  updateFlags();

  // indentation
  textIndent().setPatterns( display->textIndent().patterns() );
  textIndent().setBaseIndentation( display->textIndent().baseIndentation() );
  
  textIndentAction()->setChecked( display->textIndentAction()->isChecked() );
  textHighlightAction()->setChecked( display->textHighlightAction()->isChecked() );
  bracesHighlightAction()->setChecked( display->bracesHighlightAction()->isChecked() );
    
  _setBraces( display->_braces() );
  _setMacros( display->macros() );
  _setPaper( true, display->paper( true ) );
  _setPaper( false, display->paper( false ) );
  
  // file
  setFile( display->file() );
  
}

//____________________________________________
void TextDisplay::openFile( File file )
{
  
  Debug::Throw() << "TextDisplay::openFile " << file << endl;
  
  // expand filename
  file = file.expand();

  // check is there is an "AutoSave" file matching with more recent modification time
  // here, when the diff is working, I could offer the possibility to show a diff between
  // the saved file and the backup
  bool restore_autosave( false );
  File tmp( file );
  File autosaved( AutoSaveThread::autoSaveName( tmp ) );
  if( autosaved.exist() &&
    ( !tmp.exist() ||
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
  
  // check file and try open.
  QFile in( tmp.c_str() );
  if( in.open( QIODevice::ReadOnly ) )
  {
    
    setPlainText( in.readAll() );
    in.close();
    setModified( false );
    setIgnoreWarnings( false );

  }

  // save file if restored from autosaved.
  if( restore_autosave ) save();
  
  // perform first autosave
  (static_cast<MainFrame*>(qApp))->autoSave().saveFiles( this );
    
}

//_______________________________________________________
void TextDisplay::setFile( const File& file )
{ 
  file_ = file; 
  if( file.exist() ) 
  {
    _setLastSaved( file.lastModified() );
    _setWorkingDirectory( file.path() );
    setIgnoreWarnings( false );
    setReadOnly( file.exist() && !file.isWritable() );
  }
  
  if( isActive() ) emit needUpdate( WINDOW_TITLE | FILE_NAME ); 
  
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
    if( XmlOptions::get().get<bool>( "BACKUP" ) && file().exist() ) file().backup();
    
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
  setIgnoreWarnings( false );
  
  // retrieve associated displays, update saved time
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { (*iter)->_setLastSaved( file().lastModified() ); }
  
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
  QtUtil::centerOnPointer( &dialog );
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

  // check if file exist
  if( file.exist() )
  {
    if( !file.isWritable() )
    {
      ostringstream what;
      what << "File \"" << file << "\" is read-only. <Save> canceled.";
      QtUtil::infoDialog( this, what.str() );
      return;
    } else if( !QtUtil::questionDialog( this, "Selected file already exist. Overwrite ?" ) )
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

  // save (using new filename)
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
  document()->setModified( false );
  openFile( file() );

  // restore
  horizontalScrollBar()->setValue( x );
  verticalScrollBar()->setValue( y );
  
  QTextCursor cursor( textCursor() );
  cursor.setPosition( position );
  setTextCursor( cursor );
  setUpdatesEnabled( true );
  

}

//____________________________________________
bool TextDisplay::fileRemoved( void ) const
{
  Debug::Throw( "TextDisplay::fileRemoved.\n" );
  return (!file().empty() && last_save_.isValid() && !file().exist() );
}

//____________________________________________
bool TextDisplay::fileModified( void )
{  
  
  Debug::Throw( "TextDisplay::fileModified.\n" );

  // check file size
  if( !( file().size() && file().exist() ) ) return false;
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

//_____________________________________________________________________
void TextDisplay::setActive( const bool& active )
{ 

  // check if value is changed
  if( isActive() == active ) return;
  active_ = active;   
  
  // update paper
  if( flag( HAS_PAPER ) ) _setPaper( active_ ? active_color_:inactive_color_ );
    
}

//_______________________________________________________
bool TextDisplay::hasLeadingTabs( void ) const
{
  Debug::Throw( "TextDisplay::hasLeadingTabs.\n" );

  // define regexp to perform replacement
  QRegExp wrong_tab_regexp( _hasTabEmulation() ? _normalTabRegExp():_emulatedTabRegExp() );
  for( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
  { if( wrong_tab_regexp.indexIn( block.text() ) >= 0 ) return true; }

  return false;

}

//_______________________________________________________
QDomElement TextDisplay::htmlNode( QDomDocument& document )
{

  // clear highlight locations and rehighlight
  QDomElement out = document.createElement( "pre" );

  // loop over text blocks
  for( QTextBlock block = TextDisplay::document()->begin(); block.isValid(); block = block.next() )
  {
    
    HighlightPattern::LocationSet locations;
    
    // try retrieve highlightBlockData
    HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( data ) locations = data->locations();
    
    // retrieve text
    QString text( block.text() );
    if( locations.empty() )
    {
      
      QDomElement span = out.appendChild( document.createElement( "span" ) ).toElement();
      span.appendChild( document.createTextNode( text ) );
    
    } else {
      
      QDomElement span;

      // current pattern
      const HighlightPattern *current_pattern = 0;

      // parse text
      QString buffer("");
      for( int index = 0; index < text.size(); index++ )
      {

        // parse locations
        HighlightPattern::LocationSet::reverse_iterator location_iter = find_if(
          locations.rbegin(),
          locations.rend(),
          HighlightPattern::Location::ContainsFTor( index ) );

        const HighlightPattern* pattern = ( location_iter == locations.rend() ) ? 0:&location_iter->parent();
        if( pattern != current_pattern || index == 0 )
        {

          // append text to current element and reset stream
          HtmlUtil::textNode( buffer, span, document );
          buffer = "";
          
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

      }

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
  if( data ) 
  {
    // retrieve locations
    const HighlightPattern::LocationSet& locations( data->locations() );
    return (!locations.empty()) && locations.begin()->parent().flag( HighlightPattern::NO_INDENT );
    
  }
  
  // all checks passed
  return false;

}

//___________________________________________________________________________
void TextDisplay::updateConfiguration( void )
{
  Debug::Throw( "TextDisplay::updateConfiguration" );

  // base class configuration update
  CustomTextEdit::updateConfiguration();
  
  // local update
  textIndentAction()->setChecked( XmlOptions::get().get<bool>( "TEXT_INDENT" ) );
  textHighlightAction()->setChecked( XmlOptions::get().get<bool>( "TEXT_HIGHLIGHT" ) );
  bracesHighlightAction()->setChecked( XmlOptions::get().get<bool>( "TEXT_BRACES" ) );
  
  // retrieve active/inactive colors
  QColor active_color( XmlOptions::get().get<string>("ACTIVE_COLOR").c_str() );
  QColor inactive_color( XmlOptions::get().get<string>("INACTIVE_COLOR").c_str() );

  // paper background color
  setFlag( TextDisplay::HAS_PAPER, XmlOptions::get().get<bool>( "SHADE_INACTIVE_VIEWS" ) && active_color.isValid() && inactive_color.isValid() );
  
  // update flags
  updateFlags();

}

//___________________________________________________________________________
void TextDisplay::updateDocumentClass( void )
{

  Debug::Throw( "TextDisplay::UpdateDocumentClass\n" );
  textHighlight().clear();
  textIndent().clear();
  textIndent().setBaseIndentation(0);
  _clearBraces();
  _clearMacros();

  // default document class is empty
  const DocumentClass* document_class( 0 );
  
  // try load document class from class_name
  if( !className().empty() ) 
  { document_class = static_cast<MainFrame*>(qApp)->classManager().get( className() ); }

  // try load from file
  if( !(document_class || file().empty() ) )
  { document_class = static_cast<MainFrame*>(qApp)->classManager().find( file() ); }
      
  // abort if no document class is found
  if( !document_class ) return;
 
  // update class name
  setClassName( document_class->name() );
  
  // update Flags
  if( XmlOptions::get().get<bool>( "WRAP_FROM_CLASS" ) && !flag( HAS_WRAP ) )
  { setFlag( WRAP, document_class->wrap() ); }
  
  bracesHighlightAction()->setEnabled( !document_class->braces().empty() );
  textHighlightAction()->setEnabled( !document_class->highlightPatterns().empty() );
  textIndentAction()->setEnabled( !document_class->indentPatterns().empty() );
  
  // wrapping
  _toggleWrapMode( flag( WRAP ) );
  wrapModeAction()->setChecked( flag( WRAP ) );

  // highlighting
  textHighlight().setPatterns( document_class->highlightPatterns() );

  // indentation
  textIndent().setPatterns( document_class->indentPatterns() );
  textIndent().setBaseIndentation( document_class->baseIndentation() );

  // braces
  _setBraces( document_class->braces() );
  
  // macros
  _setMacros( document_class->textMacros() );

  return;
  
}

//___________________________________________________
bool TextDisplay::updateFlags( void )
{
  
  Debug::Throw() << "TextDisplay::UpdateFlags - key: " << key() << endl;
  
  bool changed( false );
  
  textIndent().setEnabled( textIndentAction()->isEnabled() && textIndentAction()->isChecked() );
  changed |= textHighlight().setEnabled( textHighlightAction()->isEnabled() && textHighlightAction()->isChecked() );

  // Active/inactive paper color
  QColor default_color( QWidget().palette().color( QPalette::Base ) );
  _setPaper( true, flag( HAS_PAPER ) ? QColor( XmlOptions::get().get<string>("ACTIVE_COLOR").c_str() ) : default_color );
  _setPaper( false, flag( HAS_PAPER ) ? QColor( XmlOptions::get().get<string>("INACTIVE_COLOR").c_str() ) : default_color );
  _setPaper( isActive() ? active_color_:inactive_color_ );

  // enable/disable wrapping
  _toggleWrapMode( flag( WRAP ) );
  wrapModeAction()->setChecked( flag( WRAP ) );
  
  return changed;
  
}

//_______________________________________________________
void TextDisplay::indentSelection( void )
{
  Debug::Throw( "TextDisplay::IndentSelection.\n" );

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
  setUpdatesEnabled( false );
  cursor.clearSelection(); 
  
  for( QTextBlock block = begin; block != end && block.isValid(); block = block.next() )
  { emit indent( block ); }
  
  // handle last block
  emit indent( end );
  
  // select all indented blocks
  cursor.setPosition( begin.position(), QTextCursor::MoveAnchor );
  cursor.setPosition( end.position()+end.length(), QTextCursor::KeepAnchor );
  setTextCursor( cursor );
  setUpdatesEnabled( true );

  return;
}

//_____________________________________________
void TextDisplay::processMacro( string name )
{
  
  Debug::Throw( "TextDisplay::processMacro.\n" );

  // retrieve macro that match argument name
  MacroList::const_iterator macro_iter = find_if( macros_.begin(), macros_.end(), TextMacro::SameNameFTor( name ) );
  if( macro_iter == macros_.end() )
  {
    ostringstream what;
    what << "Unable to find macro named " << name;
    QtUtil::infoDialog( this, what.str() );
    return;
  }

  // check display
  if( !isActive() ) return;
 
  // retrieve text cursor
  QTextCursor cursor( textCursor() );
  if( !cursor.hasSelection() ) return;
  
  // retrieve blocks
  QTextBlock begin( document()->findBlock( min( cursor.position(), cursor.anchor() ) ) );
  QTextBlock end( document()->findBlock( max( cursor.position(), cursor.anchor() ) ) );

  // need to remove selection otherwise the first adding of a tab
  // will remove the entire selection.
  setUpdatesEnabled( true );
  cursor.clearSelection(); 
  
  for( QTextBlock block = begin; block != end && block.isValid(); block = block.next() )
  { emit indent( block ); }
  
  // handle last block
  emit indent( end );
  
  QString text( cursor.selectedText() );
  if( (*macro_iter)->processText( text ) )
  { cursor.insertText( text ); }
  
  // enable updates
  setUpdatesEnabled( true );
  return;
  
}

//_______________________________________________________
void TextDisplay::replaceLeadingTabs( const bool& confirm )
{
  Debug::Throw( "TextDisplay::replaceLeadingTabs.\n" );

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
  
  // loop over blocks
  for( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
  {

    QString text( block.text() );
    
    // look for leading tabs
    if( wrong_tab_regexp.indexIn( text ) < 0 ) continue;
    
    // select with cursor
    QTextCursor cursor( block );
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
  return;
}

//_______________________________________________________
void TextDisplay::rehighlight( void )
{ 
  Debug::Throw( "TextDisplay::rehighlight.\n" );
  textHighlight().setDocument( document() ); 
}


//_______________________________________________________
void TextDisplay::keyPressEvent( QKeyEvent* event )
{

  // check if tab key is pressed
  if( event->key() == Key_Tab && indent_->isEnabled() && !textCursor().hasSelection() )
  { emit indent( textCursor().block() ); }
  else 
  {
  
    // process key
    CustomTextEdit::keyPressEvent( event );

    // indent current paragraph when return is pressed
    if( event->key() == Key_Return && indent_->isEnabled() && !textCursor().hasSelection() )
    { emit indent( textCursor().block() ); }

    // reindent paragraph if needed
    if( indent_->isEnabled() && ( event->key() == Key_BraceRight || event->key() == Key_BraceLeft ) && !textCursor().hasSelection() )
    { emit indent( textCursor().block() ); }

  }

  return;
}

//_______________________________________________________
void TextDisplay::focusInEvent( QFocusEvent* event )
{
  Debug::Throw() << "TextDisplay::focusInEvent - " << key() << endl;
  emit hasFocus( this );
  CustomTextEdit::focusInEvent( event );
}

//_____________________________________________________________________
void TextDisplay::_createReplaceDialog( void )
{
  Debug::Throw( "TextDisplay::_CreateReplaceDialog.\n" );
  CustomTextEdit::_createReplaceDialog();
  ReplaceDialog &dialog( _replaceDialog() );

  // insert multiple file buttons
  QPushButton* button = new QPushButton( "&Files", &dialog );
  connect( button, SIGNAL( clicked() ), this, SLOT( _multipleFileReplace() ) );
  button->setToolTip( "replace all occurence of the search string in the selected files" );
  dialog.addDisabledButton( button );
  dialog.locationLayout().addWidget( button );

}

//_______________________________________________________
void TextDisplay::_setBraces( const TextDisplay::BracesList& braces )
{
  Debug::Throw( "TextDisplay::_setBraces.\n" );
  braces_ = braces;
  braces_set_.clear();
  for( BracesList::const_iterator iter = braces_.begin(); iter != braces_.end(); iter++ )
  {
    braces_set_.insert( (*iter)->first );
    braces_set_.insert( (*iter)->second );
  }
}

//_______________________________________________________
void TextDisplay::_setPaper( const QColor& color )
{
  
  Debug::Throw( "TextDisplay::_setPaper.\n" );
//   if( !color.isValid() ) return;
//   
//   QPalette palette( TextDisplay::palette() );
//   palette.setColor( QPalette::Base, color );
//   setPalette( palette );

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

//_______________________________________________________
void TextDisplay::_indentCurrentParagraph( void )
{
  if( !indent_->isEnabled() ) return;
  emit indent( textCursor().block() );
}

//_______________________________________________________
void TextDisplay::_toggleTextIndent( bool state )
{

  Debug::Throw( "TextDisplay::_toggleTextIndent.\n" ); 
  
  // update text indent
  textIndent().setEnabled( textIndentAction()->isEnabled() && state );
  
  // propagate to other displays
  if( isSynchronized() )
  {
    // temporarely disable synchronization
    // to avoid infinite loop
    setSynchronized( false );
    
    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { if( (*iter)->isSynchronized() ) (*iter)->textIndentAction()->setChecked( state ); }
    setSynchronized( true );
    
  }
  
}


//_______________________________________________________
void TextDisplay::_toggleTextHighlight( bool state )
{

  Debug::Throw( "TextDisplay::_toggleTextHighlight.\n" ); 
  if( textHighlight().setEnabled( textHighlightAction()->isEnabled() && state ) )
  { rehighlight(); }

  // propagate to other displays
  if( isSynchronized() )
  {
    // temporarely disable synchronization
    // to avoid infinite loop
    setSynchronized( false );
    
    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { if( (*iter)->isSynchronized() ) (*iter)->textHighlightAction()->setChecked( state ); }
    setSynchronized( true );
    
  }

}
  
//_______________________________________________________
void TextDisplay::_toggleBracesHighlight( bool state )
{
  
  Debug::Throw( "TextDisplay::_toggleBracesHighlight.\n" ); 

  // propagate to other displays
  if( isSynchronized() )
  {
    // temporarely disable synchronization
    // to avoid infinite loop
    setSynchronized( false );
    
    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { if( (*iter)->isSynchronized() ) (*iter)->bracesHighlightAction()->setChecked( state ); }
    setSynchronized( true );
    
  }
  
  return; 
}

//_______________________________________________________
void TextDisplay::_multipleFileReplace( void )
{
  Debug::Throw( "TextDisplay::_multipleFileReplace.\n" );
  TextSelection selection( _replaceDialog().selection( false ) );

  // retrieve selection from replace dialog
  FileSelectionDialog dialog( this, selection );
  connect( &dialog, SIGNAL( fileSelected( File, TextSelection ) ), qApp, SLOT( multipleFileReplace( File, TextSelection ) ) );
  dialog.exec();
  return;
}

//_______________________________________________________
void TextDisplay::_showFileInfo( void )
{ FileInfoDialog( this ).exec(); }

//_____________________________________________________________
void TextDisplay::_setBlockModified( int position )
{
  Debug::Throw( "TextDisplay::_setBlockModified.\n" );
  
  // check if highlight is enabled.
  if( !textHighlight().isEnabled() ) return;
  
  // retrieve block matching position
  QTextBlock block( document()->findBlock( position ) );
  
  // retrieve associated block data if any
  // set block as modified so that its highlight content gets reprocessed.
  HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
  if( data ) data->setModified( true );
  
}
