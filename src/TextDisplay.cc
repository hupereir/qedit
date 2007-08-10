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
#include <QTextLayout>

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
#include "IconEngine.h"
#include "Icons.h"
#include "MainFrame.h"
#include "OpenPreviousMenu.h"
#include "QtUtil.h"
#include "ReplaceDialog.h"
#include "TextParenthesis.h"
#include "TextDisplay.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "Util.h"
#include "XmlOptions.h"

#include "Config.h"

#if WITH_ASPELL
#include "SpellDialog.h"
#include "SuggestionMenu.h"
#endif

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
  ignore_warnings_( false ),
  active_( false )
{
  
  Debug::Throw("TextDisplay::TextDisplay.\n" );
  
  // disable rich text
  setAcceptRichText( false );
  
  // text highlight
  TextHighlight* highlight = new TextHighlight( document() );
  setTextHighlight( highlight );

  // parenthesis highlight
  parenthesis_highlight_ = new ParenthesisHighlight( this );
  
  // text indent
  indent_ = new TextIndent( this );

  // connections
  connect( this, SIGNAL( selectionChanged() ), SLOT( _selectionChanged() ) );
  connect( this, SIGNAL( cursorPositionChanged() ), SLOT( _highlightParenthesis() ) );
  connect( this, SIGNAL( indent( QTextBlock ) ), indent_, SLOT( indent( QTextBlock ) ) );
  connect( this, SIGNAL( indent( QTextBlock, QTextBlock ) ), indent_, SLOT( indent( QTextBlock, QTextBlock ) ) );
  
  // actions
  addAction( text_indent_action_ = new QAction( "&Indent text", this ) );
  text_indent_action_->setCheckable( true );
  text_indent_action_->setChecked( textIndent().isEnabled() );
  connect( text_indent_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleTextIndent( bool ) ) );

  addAction( text_highlight_action_ = new QAction( "&Highlight text", this ) );
  text_highlight_action_->setCheckable( true );
  text_highlight_action_->setChecked( textHighlight().isHighlightEnabled() );
  connect( text_highlight_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleTextHighlight( bool ) ) );
  
  addAction( parenthesis_highlight_action_ = new QAction( "&Highlight parenthesis", this ) );
  parenthesis_highlight_action_->setCheckable( true );
  parenthesis_highlight_action_->setChecked( parenthesisHighlight().isEnabled() );
  connect( parenthesis_highlight_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleParenthesisHighlight( bool ) ) );

  #if WITH_ASPELL
  
  // install menus
  filter_menu_ = new SPELLCHECK::FilterMenu( this );
  dictionary_menu_ = new SPELLCHECK::DictionaryMenu( this );

  filter_menu_action_ = _filterMenu().menuAction();
  dictionary_menu_action_ = _dictionaryMenu().menuAction();
  
  connect( &_filterMenu(), SIGNAL( selectionChanged( const std::string& ) ), SLOT( _selectFilter( const std::string& ) ) );
  connect( &_dictionaryMenu(), SIGNAL( selectionChanged( const std::string& ) ), SLOT( _selectDictionary( const std::string& ) ) );
  
  #endif
  
  // retrieve pixmap path
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  // autospell
  addAction( autospell_action_ = new QAction( "&Automatic spell-check", this ) );
  autospell_action_->setCheckable( true );
  
  #if WITH_ASPELL
  autospell_action_->setChecked( textHighlight().spellParser().isEnabled() );
  connect( autospell_action_, SIGNAL( toggled( bool ) ), SLOT( _toggleAutoSpell( bool ) ) );
  #else 
  autospell_action_->setVisible( false );
  #endif

  // spell checking
  addAction( spellcheck_action_ = new QAction( IconEngine::get( ICONS::SPELLCHECK, path_list ), "&Spell check", this ) );
  #if WITH_ASPELL
  connect( spellcheck_action_, SIGNAL( triggered() ), SLOT( _spellcheck( void ) ) );
  #else 
  spellcheck_action_->setVisible( false );
  #endif
  
  // file information
  addAction( file_info_action_ = new QAction( IconEngine::get( ICONS::INFO, path_list ), "&File information", this ) );
  connect( file_info_action_, SIGNAL( triggered() ), SLOT( _showFileInfo() ) );
  
  // connections
  // track contents changed for syntax highlighting
  connect( TextDisplay::document(), SIGNAL( contentsChange( int, int, int ) ), SLOT( _setBlockModified( int, int, int ) ) );
  connect( TextDisplay::document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( void ) ) );

  // track configuration modifications
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() ) );
  connect( qApp, SIGNAL( documentClassesChanged() ), SLOT( updateDocumentClass() ) );
  connect( qApp, SIGNAL( spellCheckConfigurationChanged() ), SLOT( updateSpellCheckConfiguration() ) );
  updateConfiguration();
  updateSpellCheckConfiguration();
  
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
  
  document()->setModified( value );
  
  // ask for update in the parent frame
  if( isActive() ) 
  emit needUpdate( WINDOW_TITLE | UNDO_REDO );
  
}  
  
//_____________________________________________________
void TextDisplay::setReadOnly( const bool& value )
{
  Debug::Throw() << "TextDisplay::setReadOnly - value: " << value << endl;
  CustomTextEdit::setReadOnly( value );
  document()->setModified( false );
  emit needUpdate( WINDOW_TITLE | CUT | PASTE | UNDO_REDO ); 
}

//___________________________________________________________________________
void TextDisplay::synchronize( TextDisplay* display )
{
 
  Debug::Throw( "TextDisplay::synchronize.\n" );
      
  // synchronize text
  // (from base class)
  CustomTextEdit::synchronize( display );
  
  // restore connection with document
  // track contents changed for syntax highlighting
  connect( TextDisplay::document(), SIGNAL( contentsChange( int, int, int ) ), SLOT( _setBlockModified( int, int, int ) ) );
  connect( TextDisplay::document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( void ) ) );

  // indentation
  textIndent().setPatterns( display->textIndent().patterns() );
  textIndent().setBaseIndentation( display->textIndent().baseIndentation() );
  
  textIndentAction().setChecked( display->textIndentAction().isChecked() );
  textHighlightAction().setChecked( display->textHighlightAction().isChecked() );
  parenthesisHighlightAction().setChecked( display->parenthesisHighlightAction().isChecked() );
    
  _setMacros( display->macros() );
  _setPaper( true, display->paper( true ) );
  _setPaper( false, display->paper( false ) );
  
  // file
  setFile( display->file() );
  
}

//____________________________________________
void TextDisplay::openFile( File file, bool check_autosave )
{
  
  Debug::Throw() << "TextDisplay::openFile " << file << endl;

  // reset class name
  string class_name( menu().get( file ).information("class_name") );
  setClassName( class_name );

  // expand filename
  file = file.expand();

  // check is there is an "AutoSave" file matching with more recent modification time
  // here, when the diff is working, I could offer the possibility to show a diff between
  // the saved file and the backup
  bool restore_autosave( false );
  File tmp( file );
  
  File autosaved( AutoSaveThread::autoSaveName( tmp ) );
  if( check_autosave && autosaved.exist() &&
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

    // update flags
    setModified( false );
    _setIgnoreWarnings( false );

  }

  // save file if restored from autosaved.
  if( restore_autosave && !isReadOnly() ) save();
  
  // perform first autosave
  if( !isReadOnly() ) (static_cast<MainFrame*>(qApp))->autoSave().saveFiles( this );
  
  // update openPrevious menu
  if( !TextDisplay::file().empty() )
  { menu().get( TextDisplay::file() ).addInformation( "class_name", className() ); }
  
}

//_______________________________________________________
void TextDisplay::setFile( const File& file )
{ 
  Debug::Throw( "TextDisplay::setFile.\n" );
  file_ = file; 
  if( file.exist() ) 
  {
    _setLastSaved( file.lastModified() );
    _setWorkingDirectory( file.path() );
    _setIgnoreWarnings( false );
    setReadOnly( file.exist() && !file.isWritable() );
  }
  
  if( isActive() ) emit needUpdate( WINDOW_TITLE | FILE_NAME ); 
  
}

//___________________________________________________________________________
FileRemovedDialog::ReturnCode TextDisplay::checkFileRemoved( void )
{
  Debug::Throw( "TextDisplay::checkFileRemoved.\n" );
 
  if( _ignoreWarnings() || !_fileRemoved() ) return FileRemovedDialog::IGNORE;
 
  // disable check
  int state( FileRemovedDialog( this, file() ).exec() );
  if( state == FileRemovedDialog::RESAVE ) { save(); }
  else if( state == FileRemovedDialog::SAVE_AS ) { saveAs(); }
  else if( state == FileRemovedDialog::IGNORE ) {
 
    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { 
      (*iter)->_setIgnoreWarnings( true );
      (*iter)->document()->setModified( false );
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
 
  int state( FileModifiedDialog( this, file() ).exec() );
  if( state == FileModifiedDialog::RESAVE ) { save(); }
  else if( state == FileModifiedDialog::SAVE_AS ) { saveAs(); }
  else if( state == FileModifiedDialog::RELOAD ) { 
    
    document()->setModified( false ); 
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
AskForSaveDialog::ReturnCode TextDisplay::askForSave( const bool& enable_all )
{
  Debug::Throw( "TextDisplay::askForSave.\n" );
  
  if( !document()->isModified() ) return AskForSaveDialog::YES;
  
  int flags( AskForSaveDialog::YES | AskForSaveDialog::NO | AskForSaveDialog::CANCEL );
  if( enable_all ) flags |=  AskForSaveDialog::ALL;
  int state( AskForSaveDialog( this, file(), flags ).exec() );
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
  _setIgnoreWarnings( false );
  
  // retrieve associated displays, update saved time
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { (*iter)->_setLastSaved( file().lastModified() ); }
  
  // add file to menu
  if( !file().empty() )
  { menu().get( file() ).addInformation( "class_name", className() ); }
  
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
  if( !TextDisplay::file().empty() )
  { menu().get( TextDisplay::file() ).addInformation( "class_name", className() ); }

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

//_____________________________________________________________________
void TextDisplay::setActive( const bool& active )
{ 

  // check if value is changed
  if( isActive() == active ) return;
  active_ = active;   
  
  // update paper
  _setPaper( active_ ? active_color_:inactive_color_ );
    
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
  Debug::Throw( "TextDisplay::updateConfiguration.\n" );

  // base class configuration update
  CustomTextEdit::updateConfiguration();
  
  // indentation
  textIndentAction().setChecked( XmlOptions::get().get<bool>( "TEXT_INDENT" ) );
  
  // syntax highlighting
  textHighlightAction().setChecked( XmlOptions::get().get<bool>( "TEXT_HIGHLIGHT" ) );

  // parenthesis highlight
  textHighlight().setParenthesisHighlightColor( QColor( XmlOptions::get().raw( "PARENTHESIS_COLOR" ).c_str() ) );
  parenthesisHighlightAction().setChecked( XmlOptions::get().get<bool>( "TEXT_PARENTHESIS" ) );
 
  // retrieve active/inactive colors for activity shading
  QColor default_color( QWidget().palette().color( QPalette::Base ) );
  QColor active_color( XmlOptions::get().get<string>("ACTIVE_COLOR").c_str() );
  QColor inactive_color( XmlOptions::get().get<string>("INACTIVE_COLOR").c_str() );
  bool shade_inactive( XmlOptions::get().get<bool>( "SHADE_INACTIVE_VIEWS" ) );
  _setPaper( true, active_color_.isValid() && shade_inactive ? active_color_ : default_color );
  _setPaper( false, inactive_color.isValid() && shade_inactive ? inactive_color : default_color );
  _setPaper( isActive() ? active_color_:inactive_color_ );
    
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

  Debug::Throw() << "TextDisplay::updateDocumentClass - class name: " << className() << endl;
  
  // wrap mode
  if( XmlOptions::get().get<bool>( "WRAP_FROM_CLASS" ) ) wrapModeAction().setChecked( document_class->wrap() );
  
  // enable actions consequently
  parenthesisHighlightAction().setVisible( !document_class->parenthesis().empty() );
  textHighlightAction().setVisible( !document_class->highlightPatterns().empty() );
  textIndentAction().setVisible( !document_class->indentPatterns().empty() );
 
  // store into class members
  textHighlight().setPatterns( document_class->highlightPatterns() );
  textIndent().setPatterns( document_class->indentPatterns() );
  textIndent().setBaseIndentation( document_class->baseIndentation() );
  _setParenthesis( document_class->parenthesis() );
  _setMacros( document_class->textMacros() );

  // update enability for parenthesis matching
  textHighlight().setParenthesisEnabled( 
    textHighlightAction().isChecked() &&
    textHighlight().parenthesisHighlightColor().isValid() && 
    !parenthesis_set_.empty() );

  parenthesisHighlight().setEnabled( 
    textHighlightAction().isChecked() &&
    textHighlight().parenthesisHighlightColor().isValid() && 
    !parenthesis_set_.empty() );
  
  // add information to Menu
  if( !file().empty() )
  { menu().get( file() ).addInformation( "class_name", className() ); }

  // rehighlight text entirely
  // because Pattern Ids may have changed even if the className has not changed.
  #if WITH_ASPELL
  if( textHighlight().isHighlightEnabled() && !textHighlight().spellParser().isEnabled() ) rehighlight();
  #else
  if( textHighlight().isHighlightEnabled() ) rehighlight();
  #endif
  
  return;
  
}

//___________________________________________________________________________
void TextDisplay::updateSpellCheckConfiguration( void )
{
  Debug::Throw( "TextDisplay::updateSpellCheckConfiguration.\n" );
  
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
    FileRecord& record( menu().get( file() ) ); 
    if( record.hasInformation( "filter" ) && interface.hasFilter( record.information( "filter" ) ) )
    { filter = record.information( "filter" ); }
    
    if( record.hasInformation( "dictionary" ) && interface.hasDictionary( record.information( "dictionary" ) ) )
    { dictionary = record.information( "dictionary" ); }
    
  }
  
  // see if one should/can change the dictionary and filter
  if( filter != interface.filter() && interface.setFilter( filter ) ) 
  {
    _filterMenu().select( filter );
    changed = true;
  }
  
  if( dictionary != interface.dictionary() && interface.setDictionary( dictionary ) ) 
  {
    _dictionaryMenu().select( dictionary );
    changed = true;
  }
  
  // rehighlight if needed
  if( changed && autoSpellAction().isChecked() && autoSpellAction().isEnabled() ) rehighlight();
  
  #endif
  
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
  cursor.clearSelection(); 
  emit indent( begin, end );
  
  // select all indented blocks
  cursor.setPosition( begin.position(), QTextCursor::MoveAnchor );
  cursor.setPosition( end.position()+end.length()-1, QTextCursor::KeepAnchor );
  setTextCursor( cursor );

  return;
}

//_____________________________________________
void TextDisplay::processMacro( string name )
{
  
  Debug::Throw() << "TextDisplay::processMacro - " << name << endl;

  // retrieve macro that match argument name
  TextMacro::List::const_iterator macro_iter = find_if( macros_.begin(), macros_.end(), TextMacro::SameNameFTor( name ) );
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
  replaceLeadingTabs( false );
  
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
  
  // loop over blocks
  for( QTextBlock block = begin; block.isValid() && block != end.next(); block = block.next() )
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
  
  // set all block to modified
  for( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
  { _setBlockModified( block ); }
  
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


//________________________________________________
void TextDisplay::contextMenuEvent( QContextMenuEvent* event )
{
  
  Debug::Throw( "CustomTextEdit::contextMenuEvent.\n" );
  #if WITH_ASPELL
  
  // check autospell enability
  if( !textHighlight().spellParser().isEnabled() ) return CustomTextEdit::contextMenuEvent( event );
  
  // block and cursor
  QTextCursor cursor( cursorForPosition( event->pos() ) );
  QTextBlock block( cursor.block() );
  
  // block data
  HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
  if( !data ) return CustomTextEdit::contextMenuEvent( event );


  // try retrieve misspelled word
  SPELLCHECK::Word word( data->misspelledWord( cursor.position() - block.position() ) );
  if( word.empty() || textHighlight().spellParser().interface().isWordIgnored( word ) ) 
  { return CustomTextEdit::contextMenuEvent( event ); }
  
  // change selection to misspelled word
  cursor.setPosition( word.position() + block.position(), QTextCursor::MoveAnchor );
  cursor.setPosition( word.position() + word.size() + block.position(), QTextCursor::KeepAnchor );
  setTextCursor( cursor );
  
  // create suggestion menu
  SPELLCHECK::SuggestionMenu menu( this, word, isReadOnly() );
  menu.interface().setFilter( textHighlight().spellParser().interface().filter() );
  menu.interface().setDictionary( textHighlight().spellParser().interface().dictionary() );
  
  // set connections
  connect( &menu, SIGNAL( ignoreWord( std::string ) ), SLOT( _ignoreMisspelledWord( std::string ) ) );
  connect( &menu, SIGNAL( suggestionSelected( std::string ) ), SLOT( _replaceMisspelledSelection( std::string ) ) );
  
  // execute
  menu.exec( event->globalPos() );

  #else 
  return CustomTextEdit::contextMenuEvent( event );
  #endif
  
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
void TextDisplay::_setPaper( const QColor& color )
{
  
  Debug::Throw( "TextDisplay::_setPaper.\n" );
  if( !color.isValid() ) return;
  
  QPalette palette( TextDisplay::palette() );
  palette.setColor( QPalette::Base, color );
  setPalette( palette );

}

//____________________________________________
bool TextDisplay::_fileRemoved( void ) const
{
  Debug::Throw( "TextDisplay::_fileRemoved.\n" );
  return (!file().empty() && last_save_.isValid() && !file().exist() );
}

//____________________________________________
bool TextDisplay::_fileModified( void )
{  
  
  Debug::Throw( "TextDisplay::_fileModified.\n" );

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
  Debug::Throw( "TextDisplay::_indentCurrentParagraph.\n" );
  if( !indent_->isEnabled() ) return;
  emit indent( textCursor().block() );
}

//_______________________________________________________
void TextDisplay::_setParenthesis( const TextParenthesis::List& parenthesis )
{
  Debug::Throw( "TextDisplay::setParenthesis.\n" );
  parenthesis_ = parenthesis;
  parenthesis_set_.clear();
  for( TextParenthesis::List::const_iterator iter = parenthesis_.begin(); iter != parenthesis_.end(); iter++ )
  {
    parenthesis_set_.insert( iter->first );
    parenthesis_set_.insert( iter->second );
  }
  
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
    << " parenthesis: " << parenthesis_set_.empty()
    << endl;
  
  // propagate to textHighlight
  textHighlight().setParenthesisEnabled( 
    state && 
    textHighlight().parenthesisHighlightColor().isValid() && 
    !parenthesis_set_.empty() );
  
  parenthesisHighlight().setEnabled( 
    state && 
    textHighlight().parenthesisHighlightColor().isValid() && 
    !parenthesis_set_.empty() );
  
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
    
    FileRecord& record( menu().get( file() ) ); 
    if( !( record.hasInformation( "filter" ) && dialog.setFilter( record.information( "filter" ) ) ) )
    { dialog.setFilter( default_filter ); }

    if( !( record.hasInformation( "dictionary" ) && dialog.setDictionary( record.information( "dictionary" ) ) ) )
    { dialog.setDictionary( default_dictionary ); }
  
  }  else {
    
    dialog.setFilter( default_filter );
    dialog.setDictionary( default_dictionary );
    
  }
  
  // connections

  connect( &dialog, SIGNAL( filterChanged( const std::string& ) ), SLOT( _selectFilter( const std::string& ) ) );
  connect( &dialog, SIGNAL( dictionaryChanged( const std::string& ) ), SLOT( _selectDictionary( const std::string& ) ) );

  dialog.nextWord();
  dialog.exec();

  // try overwrite with file record
  if( !file().empty() )
  {
    FileRecord& record( menu().get( file() ) ); 
    record.addInformation( "filter", dialog.filter() );
    record.addInformation( "dictionary", dialog.dictionary() );
  }
  
  textHighlight().spellParser().interface().mergeIgnoredWords( dialog.interface().ignoredWords() );
  
  #endif
  
}

//_______________________________________  
void TextDisplay::_selectFilter( const std::string& filter ) 
{ 
  Debug::Throw( "TextDisplay::_selectFilter.\n" ); 

  #if WITH_ASPELL

  // local reference to interface
  SPELLCHECK::SpellInterface& interface( textHighlight().spellParser().interface() );

  if( filter == interface.filter() || !interface.hasFilter( filter ) ) return;
  
  // update interface
  interface.setFilter( filter );
  _filterMenu().select( filter );
  
  // update file record
  if( !file().empty() )
  {
    FileRecord& record( menu().get( file() ) ); 
    record.addInformation( "filter", interface.filter() ); 
  }

  // rehighlight if needed
  if( textHighlight().spellParser().isEnabled() ) rehighlight();
 
  #endif
  
  return;
  
}

//_______________________________________  
void TextDisplay::_selectDictionary( const std::string& dictionary ) 
{ 
  Debug::Throw( "TextDisplay::_selectDictionary.\n" ); 

  #if WITH_ASPELL
  // local reference to interface
  SPELLCHECK::SpellInterface& interface( textHighlight().spellParser().interface() );

  if( dictionary == interface.dictionary() || !interface.hasDictionary( dictionary ) ) return;
  
  // update interface
  interface.setDictionary( dictionary );
  _dictionaryMenu().select( dictionary );
  
  // update file record
  if( !file().empty() )
  {
    FileRecord& record( menu().get( file() ) ); 
    record.addInformation( "dictionary", interface.dictionary() ); 
  }
  
  // rehighlight if needed
  if( textHighlight().spellParser().isEnabled() ) rehighlight();
  #endif
  
  return;
  
}

//_______________________________________________________
void TextDisplay::_showFileInfo( void )
{ 
  Debug::Throw( "TextDisplay::_showFileInfo.\n" );
  FileInfoDialog( this ).exec();
}

//_____________________________________________________________
void TextDisplay::_setBlockModified( int position, int removed, int added )
{
  // Debug::Throw() << "TextDisplay::_setBlockModified - [" << position << "," << removed << "," << added << "]" << endl;
  QTextBlock begin( document()->findBlock( position ) );
  QTextBlock end(  document()->findBlock( position + added ) );
  
  for( QTextBlock block = begin; block.isValid() && block != end; block = block.next() )
  { _setBlockModified( block ); }
  
  _setBlockModified( end );
  
}
 
//_____________________________________________________________
void TextDisplay::_setBlockModified( const QTextBlock& block )
{
  // check if highlight is enabled.
  if( !textHighlight().isHighlightEnabled() ) return;
  
  // retrieve associated block data if any
  // set block as modified so that its highlight content gets reprocessed.
  HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
  if( data ) data->setModified( true );

}

//__________________________________________________
void TextDisplay::_ignoreMisspelledWord( std::string word )
{ 
  Debug::Throw() << "TextDisplay::_ignoreMisspelledWord - word: " << word << endl;
  #if WITH_ASPELL
  textHighlight().spellParser().interface().ignoreWord( word );
  rehighlight();
  #endif
  return;
  
}

//__________________________________________________
void TextDisplay::_replaceMisspelledSelection( std::string word )
{ 

  #if WITH_ASPELL
  Debug::Throw() << "TextDisplay::_replaceMisspelledSelection - word: " << word << endl;
  QTextCursor cursor( textCursor() );
  cursor.insertText( word.c_str() );
  #endif
  return; 

}

//__________________________________________________
void TextDisplay::_highlightParenthesis( void )
{ 
  
  if( !_isParenthesisEnabled() ) return;
  
  // clear previous parenthesis
  parenthesisHighlight().clear();
  
  // retrieve TextCursor
  QTextCursor cursor( textCursor() );
  if( cursor.atBlockStart() ) return;
  
  // retrieve block
  QTextBlock block( cursor.block() );
  if( ignoreBlock( block ) ) return;
  
  // store local position in block
  int position(cursor.position()-block.position()-1);
  bool found( false );
  
  // check if character is in parenthesis_set
  QChar c( block.text()[position] );
  if( parenthesis_set_.find( c ) == parenthesis_set_.end() ) 
  { return; }
  
  // check against opening parenthesis
  TextParenthesis::List::const_iterator parenthesis = find_if( parenthesis_.begin(), parenthesis_.end(), TextParenthesis::FirstElementFTor(c) );
  if( parenthesis != parenthesis_.end() )
  {
    int increment( 0 );
    position++;
    while( block.isValid() && !found )
    {
      // retrieve block text
      QString text( block.text() );
      
      // parse text
      while( (position = parenthesis->regexp().indexIn( text, position )) >= 0 )
      {
        if( text[position] == parenthesis->second ) increment--;
        else if( text[position] == parenthesis->first ) increment++;
        
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
  if( !( found || (parenthesis = find_if( parenthesis_.begin(), parenthesis_.end(), TextParenthesis::SecondElementFTor(c) )) == parenthesis_.end()  ) )
  {
    int increment( 0 );
    position--;
    while( block.isValid() && !found )
    {
      // retrieve block text
      QString text( block.text() );

      // parse text
      while( position >= 0 && (position = parenthesis->regexp().lastIndexIn( text, position )) >= 0 )
      { 
        
        if( text[position] == parenthesis->first ) increment--;
        else if( text[position] == parenthesis->second ) increment++;
        
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
  
  if( found ) parenthesisHighlight().highlight( position + block.position() );
    
  return;

}
