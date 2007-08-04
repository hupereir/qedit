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

#include "AutoSave.h"
#include "AutoSaveThread.h"
#include "CustomFileDialog.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "FileSelectionDialog.h"
#include "HtmlUtil.h"
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
using namespace BASE;

// empty line regular expression
const QRegExp TextDisplay::empty_line_regexp_( "(^\\s*$)" );

//___________________________________________________
TextDisplay::TextDisplay( QWidget* parent ):
  CustomTextEdit( parent ),
  file_( "" ),
  working_directory_( Util::workingDirectory() ), 
  class_name_( "" ),
  flags_( 0 ),
  modified_( false ),
  active_( false ),
  highlight_( new TextHighlight( document() ) ),
  indent_( new TextIndent( this ) )
{
  Debug::Throw( "TextDisplay::TextDisplay.\n" );

  // tell frame to delete on exit
  setAttribute( Qt::WA_DeleteOnClose );

  setKeyCompression( true );

  highlight_->setEnabled( false );
  indent_->setEnabled( false );

  // connections
  connect( this, SIGNAL( selectionChanged() ), SLOT( _selectionChanged() ) );
  connect( this, SIGNAL( indent( QTextBlock& ) ), indent_, SLOT( indent( QTextBlock& ) ) );
  
}

//_____________________________________________________
TextDisplay::~TextDisplay( void )
{
  Debug::Throw( "TextDisplay::~TextDisplay.\n" );

  // note: highlight should be deleted automatically as child of this widget
  // but for some reason (maybe because it derives from QSyntaxHighlighter) it is not.
  // needs to be deleted by hand otherwise the code leaks badly
  delete highlight_;

}

//___________________________________________________________________________
void TextDisplay::clone( TextDisplay& display )
{
 
  Debug::Throw( "TextDisplay::Clone\n" );
  
  setFlags( display.flags() );
  updateFlags();

  // highlighting
  textHighlight().setPatterns( display.textHighlight().patterns() );
  
  // indentation
  textIndent().setPatterns( display.textIndent().patterns() );
  textIndent().setBaseIndentation( display.textIndent().baseIndentation() );

  // braces
  _setBraces( display._braces() );
 
  // macros
  _setMacros( display.macros() );

  // file
  setFile( display.file() );
    
  // synchronize text
  // (from base class)
  synchronize( display )
  
}

//____________________________________________
void TextDisplay::openFile( File file )
{
  
  Debug::Throw() << "TextDisplay::openFile " << file << endl;
  
  // expand filename
  file = file.expand();

  // check is there is an "AutoSave" file matching with more recent modification time
  // here, when the Diff is working, I could offer the possibility to show a diff between
  // the saved file and the backup
  bool restore_autosave( false );
  File tmp( file );
  File autosaved( AutoSaveThread::autoSaveName( tmp ) );
  if( autosaved.exist() &&
    ( !tmp.exist() ||
    ( autosaved.LastModified() > tmp.LastModified() && tmp.Diff(autosaved) ) ) )
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
  KeySet<TextDisplay> displays( this );
  displays.insert( this );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    (*iter)->setClassName( getClassName() );
    (*iter)->updateDocumentClass();
  }
  
  // check file and try open.
  ifstream in( tmp.c_str() );
  if( in )
  {
    // retrieve text
    string text;
    while( !(in.rdstate() & ios::failbit ) )
    {
      char c = 0;
      in.get(c);

      // add character to string. Skip null characters.
      if( c && !(in.rdstate() & ios::failbit ) ) text.push_back( c );
    }
 
    // set text to TextDisplay
    for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      (*iter)->setUpdatesEnabled( false );
      (*iter)->setText( text.c_str() );
      (*iter)->setUpdatesEnabled( true );
      (*iter)->updateContents();
      (*iter)->SetModified( false );
      (*iter)->_SetSavedText( text );
      (*iter)->_SetBackupText( text );
    }
    
  }

  // save file if restored from autosaved.
  if( restore_autosave ) Save();
  
  // perform first autosave
  (static_cast<MainFrame*>(qApp))->GetAutoSave().SaveFiles( this );
  
}
  
//____________________________________________
bool TextDisplay::CheckSynchronization( void )
{

  Debug::Throw( "TextDisplay::CheckSynchronization.\n" );
  
  // retrieve associated displays
  KeySet<TextDisplay> displays( this );
  if( displays.empty() ) return true;
  
  // loop over associates and compare text
  string reference_text( (const char*) text() );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { 
    string local_text( (const char*) (*iter)->text() );
    if( local_text != reference_text ) return false;
  }
  
  // all files synchronized, update backup
  BackupText();
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { (*iter)->_SetBackupText( GetBackupText() ); }
  
  return true;
}
  
  
//____________________________________________
bool TextDisplay::FileModified( void )
{  Debug::Throw( "TextDisplay::FileModified.\n" );

  // check file size
  if( !( GetFile().size() && GetFile().Exist() ) ) return false;
  TimeStamp fileModified( GetFile().LastModified() );

  // check if file was modified and contents is changed
  if(
    fileModified.IsValid() &&
    last_save_.IsValid() &&
    fileModified > last_save_ &&
    _ContentsChanged() )
  {
    // update last_save to avoid chain questions
    last_save_ = fileModified;
    return true;
  }

  return false;

}  

//___________________________________________________________________________
void TextDisplay::Save( void )
{
    
  Debug::Throw( "TextDisplay::Save.\n" );
  
  // do nothing if not modified
  if( !Modified() ) return;
  
  // check file name
  if( GetFile().empty() ) return SaveAs();
  
  // check is contents differ from saved file
  if( _ContentsChanged() )
  {
 
    // make backup
    if(  Options::Get<bool>( "BACKUP" ) && GetFile().Exist() ) GetFile().Backup();
    
    // open output file
    ofstream out( GetFile().c_str() );
    if( !out )
    {
      ostringstream what;
      what << "Cannot write to file \"" << GetFile() << "\". <Save> canceled.";
      QtUtil::InfoDialogExclusive( this, what.str() );
      return;
    }

    // retrieve text
    string text( (const char*) this->text() );
    out << text;

    // add end of line if needed
    if( text[text.size()-1] != '\n' ) out << endl;

    // close
    out.close();
    
    // update saved text
    _SetSavedText( text );
    
  }
  
  // update modification state and last_saved time stamp
  SetModified( false );
  _SetLastSaved( GetFile().LastModified() );
  
  // retrieve associated displays, update saved Text, time, and modification state
  KeySet<TextDisplay> displays( this );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    (*iter)->SetModified( false );
    (*iter)->_SetSavedText( _GetSavedText() );
    (*iter)->_SetLastSaved( GetFile().LastModified() );
  }
  
  return;
  
}

//___________________________________________________________________________
void TextDisplay::SaveAs( void )
{
  Debug::Throw( "TextDisplay::SaveAs.\n" );
  
  // define default file
  string default_file( GetFile() );
  if( default_file.empty() ) default_file = File( "document" ).AddPath( working_directory_ );

  // create file dialog
  CustomFileDialog dialog( this, "file dialog", TRUE );
  dialog.setMode( QFileDialog::AnyFile );
  dialog.setSelection( default_file.c_str() );
  QtUtil::CenterOnPointer( &dialog, false );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve filename
  File fullname = File( (const char*) dialog.selectedFile() ).Expand();

  // check if file is directory
  if( fullname.IsDirectory() )
  {
    ostringstream what;
    what << "file \"" << fullname << "\" is a directory. <Save> canceled.";
    QtUtil::InfoDialogExclusive( this, what.str() );
    return;
  }

  // check if file exist
  if( fullname.Exist() )
  {
    if( !fullname.IsWritable() )
    {
      ostringstream what;
      what << "file \"" << fullname << "\" is read-only. <Save> canceled.";
      QtUtil::InfoDialogExclusive( this, what.str() );
      return;
    } else if( !QtUtil::QuestionDialogExclusive( this, "selected file already exist. Overwrite ?" ) )
    return;
  }

  KeySet<TextDisplay> displays( this );
  displays.insert( this );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { 
    
    // update file
    (*iter)->SetFile( fullname );

    // update document class
    // the class name is reset, to allow a document class
    // matching the new filename to get loaded
    (*iter)->SetClassName("");
    (*iter)->UpdateDocumentClass();
    (*iter)->Rehighlight();
  }
  
  // save (using new filename 
  Save();
  
}
  

//___________________________________________________________
void TextDisplay::RevertToSave( const bool& check )
{

  Debug::Throw( "TextDisplay::RevertToSave.\n" );

  // check filename
  if( GetFile().empty() )
  {
    if( check ) QtUtil::InfoDialogExclusive( this, "No filename given. <Revert to save> canceled." );
    return;
  }

  // ask for confirmation
  if( check )
  {
    ostringstream what;
    if( Modified() ) what << "Discard changes to " << GetFile().GetLocalName() << "?";
    else what << "Reload file " << GetFile().GetLocalName() << "?";
    if( !QtUtil::QuestionDialogExclusive( this, what.str() ) ) return;
  }

  SetModified( false );
  OpenFile( GetFile() );

}

//_____________________________________________________________________
void TextDisplay::SetActive( const bool& active )
{ 

  // check if value is changed
  if( isActive() == active ) return;
  active_ = active;   
  
  // update paper
  if( GetFlag( HAS_PAPER ) ) setPaper( active_ ? active_color_:inactive_color_ );
  
  // clear current paragraph background if inactive
  if( !active_ ) GetParagraphHighlight().Clear();
  
}

//___________________________________________________________________________
void TextDisplay::UpdateDocumentClass( void )
{

  Debug::Throw( "TextDisplay::UpdateDocumentClass\n" );
  GetTextHighlight().ClearPatterns();
  GetTextIndent().ClearPatterns();
  GetTextIndent().SetBaseIndentation(0);
  _ClearLocations();
  _ClearBraces();
  _ClearMacros();

  const DocumentClass* document_class( 0 );
  
  // try load document class from class_name
  if( !class_name_.empty() ) 
  { document_class = static_cast<MainFrame*>(qApp)->GetClassManager().Get( GetClassName() ); }

  // try load from file
  if( !(document_class || GetFile().empty() ) )
  { document_class = static_cast<MainFrame*>(qApp)->GetClassManager().Find( GetFile() ); }
      
  // abort if no document class is found
  if( !document_class ) return;
 
  // update class name
  SetClassName( document_class->Name() );
  
  // update Flags
  if( Options::Get<bool>( "WRAP_FROM_CLASS" ) && !GetFlag( HAS_WRAP ) )
  { SetFlag( WRAP, document_class->Wrap() ); }
  
  SetFlag( HAS_BRACES, !document_class->Braces().empty() );
  SetFlag( HAS_HIGHLIGHT, !document_class->HighlightPatterns().empty() );
  SetFlag( HAS_INDENT, !document_class->IndentPatterns().empty() );
  
  // wrapping
  SetWrap( GetFlag( WRAP ) );

  // highlighting
  GetTextHighlight().SetPatterns( document_class->HighlightPatterns() );

  // indentation
  GetTextIndent().SetPatterns( document_class->IndentPatterns() );
  GetTextIndent().SetBaseIndentation( document_class->GetBaseIndentation() );

  // braces
  /* change flag and rehighlight */
  _SetBraces( document_class->Braces() );
  HighlightBraces();
  
  // macros
  _SetMacros( document_class->TextMacros() );

  return;
  
}

//___________________________________________________
bool TextDisplay::UpdateFlags( void )
{
  
  Debug::Throw() << "TextDisplay::UpdateFlags - key: " << GetKey() << endl;
  
  bool changed( false );
  
  GetTextIndent().SetEnabled( GetFlag( INDENT ) );
  changed |= GetTextHighlight().SetEnabled( GetFlag( HIGHLIGHT ) );

  #if WITH_ASPELL
  // change text highlight settings
  changed |= GetTextHighlight().SetAutoSpellEnabled( GetFlag( AUTOSPELL ) );
  GetTextHighlight().SetColor( QColor( Options::Get<string>("AUTOSPELL_COLOR").c_str() ) );
  GetTextHighlight().SetFontFormat( Options::Get<unsigned int>("AUTOSPELL_FONT_FORMAT") );
  GetTextHighlight().UpdateSpellPattern();
  
  if( GetFlag( AUTOSPELL ) ) Rehighlight(); 
  #endif

  // Active/inactive paper color
  QColor default_color( QWidget().colorGroup().base() );
  _SetPaper( true, GetFlag( HAS_PAPER ) ? QColor( Options::Get<string>("ACTIVE_COLOR") ) : default_color );
  _SetPaper( false, GetFlag( HAS_PAPER ) ? QColor( Options::Get<string>("INACTIVE_COLOR") ) : default_color );
  setPaper( isActive() ? active_color_:inactive_color_ );

  // paragraph highlighting
  GetParagraphHighlight().SetColor( QColor( Options::Get<string>("HIGHLIGHT_COLOR").c_str() ) );
  GetParagraphHighlight().SetEnabled( GetFlag( HIGHLIGHT_PARAGRAPH ) && GetFlag( HAS_HIGHLIGHT_PARAGRAPH ) );
  if( GetFlag( HIGHLIGHT_PARAGRAPH ) && GetFlag( HAS_HIGHLIGHT_PARAGRAPH ) )
  {
    TextPosition position( GetPosition() );
    GetParagraphHighlight().Update( position.Paragraph(), position.Index() );
  }
    
  // enable/disable wrapping
  Debug::Throw() << "Wrap" << endl;
  SetWrap( GetFlag( WRAP ) );
  
  // tab emulation
  Debug::Throw() << "Tabs" << endl;
  SetTabEmulation( GetFlag( TAB_EMULATION ), Options::Get<unsigned int>("TAB_SIZE") );
  
  return changed;
  
}

//___________________________________________________
void TextDisplay::ClearParagraphsBackground( void )
{
  
  Debug::Throw( "TextDisplay::ClearParagraphsBackground.\n" );

  // change background color
  for( int i=0; i<paragraphs(); i++ )
  { clearParagraphBackground(i); }
  
  // clear paragraph highlight previous color
  GetParagraphHighlight().ClearPreviousColor();
  
  // rehighlight current paragraph, if needed
  if( isActive() && GetParagraphHighlight().Enabled() )
  { GetParagraphHighlight().Update( GetPosition().Paragraph() ); }
  
}

//___________________________________________________
void TextDisplay::PrintLocations( void ) const
{
  cout << "TextDisplay::PrintLocations" << endl;
  cout << "key: " << GetKey() << endl;
  for( LocationMap::const_iterator iter = locations_.begin(); iter != locations_.end(); iter++ )
  {
    
    const HighlightPattern::LocationSet& locations( iter->second );
    if( locations.empty() ) continue;
    
    cout << "paragraph: " << iter->first;
    cout << " [ ";
    for( HighlightPattern::LocationSet::const_iterator iter = locations.begin(); iter != locations.end(); iter++ )
    { cout << iter->Parent().Id() << " "; }
    cout << "]" << endl;
  }
  
  cout << endl;
}

//___________________________________________________
void TextDisplay::SynchronizeBoxSelection( const TextDisplay& display )
{
  _SetHasBoxSelection( display.HasBoxSelection() );
  _GetBoxSelection().Copy( display._GetBoxSelection() );
  updateContents( _GetBoxSelection().ClearRect() );
}

//___________________________________________________
void TextDisplay::SynchronizeSelection( const TextDisplay& display, const int& selection_number )
{

  if( selection_number == 0 && !display.hasSelectedText() )
  {
    removeSelection();
    return;
  }

  int parFrom( 0 );
  int indexFrom( 0 );
  int parTo( 0 );
  int indexTo( 0 );
  display.getSelection( &parFrom, &indexFrom, &parTo, &indexTo, selection_number );
  setSelection( parFrom, indexFrom, parTo, indexTo, selection_number );

}

//_______________________________________________________
bool TextDisplay::HasLeadingTabs( void ) const
{
  Debug::Throw( "TextDisplay::HasLeadingTabs.\n" );

  // define regexp to perform replacement
  QRegExp wrong_tab_regexp = _GetTabEmulation() ? _NormalTabRegExp():_EmulatedTabRegExp();
  string wrong_tab = _GetTabEmulation() ? NormalTab():EmulatedTab();
  for( int par=0; par<paragraphs(); par++ )
  {
    // look for leading tabs
    if( wrong_tab_regexp.search( text(par) ) ) return true;
  }

  return false;

}

//_______________________________________________________
QDomElement TextDisplay::HtmlNode( QDomDocument& document )
{

  // clear highlight locations and rehighlight
  // locations_.clear();

  QDomElement out = document.createElement( "pre" );

  // loop over paragraphs
  // int active_id = 0;
  for( int par = 0; par < paragraphs(); par++ )
  {
    // retrieve text
    string par_text( (const char*) text( par ) );

    // retrieve location list
    HighlightPattern::LocationSet locations( GetLocations( par ) );

    if( locations.empty() )
    {
      
      QDomElement span = out.appendChild( document.createElement( "span" ) ).toElement();
      span.appendChild( document.createTextNode( par_text.c_str() ) );
    
    } else {
      
      // active_id = locations.ActiveId().second;
      
      QDomElement span;

      // current pattern
      const HighlightPattern *current_pattern = 0;

      // parse text
      ostringstream what;
      unsigned int index = 0;
      while( index < par_text.size() )
      {

        // parse locations
        HighlightPattern::LocationSet::reverse_iterator location_iter = find_if(
          locations.rbegin(),
          locations.rend(),
          HighlightPattern::Location::ContainsFTor( index ) );

        const HighlightPattern* pattern = ( location_iter == locations.rend() ) ? 0:&location_iter->Parent();
        if( pattern != current_pattern || index == 0 )
        {

          // append text to current element and reset stream
          HtmlUtil::TextNode( what.str().c_str(), span, document );
          what.str("");

          // update pattern
          current_pattern = pattern;

          // update current element
          span = out.appendChild( document.createElement( "span" ) ).toElement();
          if( current_pattern )
          {

            // retrieve font format
            const unsigned int& format = current_pattern->Style().Format();
            ostringstream format_stream;
            if( format & FORMAT::UNDERLINE ) format_stream << "text-decoration: underline; ";
            if( format & FORMAT::ITALIC ) format_stream << "font-style: italic; ";
            if( format & FORMAT::BOLD ) format_stream << "font-weight: bold; ";
            if( format & FORMAT::STRIKE ) format_stream << "text-decoration: line-through; ";

            // retrieve color
            const QColor& color = current_pattern->Style().Color();
            if( color.isValid() ) format_stream << "color: " << color.name() << "; ";

            span.setAttribute( "style", format_stream.str().c_str() );

          }
        }

        // convert spaces to non breakable
        what << par_text[index];
        index++;

      }

      span.appendChild( document.createTextNode( what.str().c_str() ) );

    }
    out.appendChild( document.createElement( "br" ) );
  }

  return out;
}

//_______________________________________________________
void TextDisplay::IndentSelection( void )
{
  Debug::Throw( "TextDisplay::IndentSelection.\n" );

  // check activity, indentation and text selection
  if( !indent_->Enabled() ) return;
  if( !hasSelectedText() ) return;

  // first propagate to associated displays
  if( isActive() && Synchronized() )
  {
    // propagate to other displays
    KeySet<TextDisplay> displays( this );
    for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { (*iter)->IndentSelection(); }
  }

  setUpdatesEnabled( false );
  SelectionRange range( GetSelectionRange() );

  // need to remove selection otherwise the first adding of a tab
  // will remove the entire selection.
  removeSelection();
  for( int par = range.first.Paragraph(); par <= range.second.Paragraph(); par++ )
  { emit Indent( par ); }

  // restore selection
  setSelection( range.first.Paragraph(), 0, range.second.Paragraph(), paragraphLength( range.second.Paragraph() ) );
  emit selectionChanged();

  setUpdatesEnabled( true );
  updateContents();

  return;
}

//_____________________________________________
void TextDisplay::ProcessMacro( string name )
{
  
  Debug::Throw( "TextDisplay::ProcessMacro.\n" );

  // retrieve macro that match argument name
  MacroList::const_iterator macro_iter = find_if( macros_.begin(), macros_.end(), TextMacro::SameNameFTor( name ) );
  if( macro_iter == macros_.end() )
  {
    ostringstream what;
    what << "Unable to find macro named " << name;
    QtUtil::InfoDialogExclusive( this, what.str() );
    return;
  }

  // check display
  if( !hasSelectedText() ) return;
  if( !isActive() ) return;

  // disable updates
  setUpdatesEnabled( false );
  TextDisplay::SelectionRange range( GetSelectionRange() );

  // remove last line, if range is at beginning
  if( range.second.Index() == 0 )
  {
    range.second.Paragraph()--;
    range.second.Index() = paragraphLength( range.second.Paragraph() );
    setSelection( range.first.Paragraph(), range.first.Index(), range.second.Paragraph(), range.second.Index() );
    emit selectionChanged();
  }

  Str text( (const char*) selectedText() );
  unsigned int n_lines_orig( text.Contains( Str("\n") ) );
  unsigned int last_line_size_orig = text.rfind( "\n" );
  last_line_size_orig = text.size() - (last_line_size_orig == string::npos ? 0:last_line_size_orig);
  if( (*macro_iter)->ProcessText( text ) )
  {
    // replace selected text
    insert( text.c_str() );

    // update selection range
    unsigned int n_lines_new( text.Contains( Str("\n") ) );
    range.second.Paragraph() += n_lines_new - n_lines_orig;

    unsigned int last_line_size_new = text.rfind( "\n" );
    last_line_size_new = text.size() - (last_line_size_new == string::npos ? 0:last_line_size_new);

    if( n_lines_new == n_lines_orig ) range.second.Index() += last_line_size_new - last_line_size_orig;
    else range.second.Index() = last_line_size_new;

    // reinstall selection
    setSelection( range.first.Paragraph(), range.first.Index(), range.second.Paragraph(), range.second.Index() );
    emit selectionChanged();
  }

  // enable updates
  setUpdatesEnabled( true );
  updateContents();
  return;
}

//_______________________________________________________
void TextDisplay::ReplaceLeadingTabs( const bool& confirm )
{
  Debug::Throw( "TextDisplay::ReplaceLeadingTabs.\n" );

  // ask for confirmation
  if( confirm && !QtUtil::QuestionDialogExclusive( this, "Replace all leating tabs with space characters ?" ) ) return;

  // disable updates
  setUpdatesEnabled( false );

  // define regexp to perform replacement
  QRegExp wrong_tab_regexp = _GetTabEmulation() ? _NormalTabRegExp():_EmulatedTabRegExp();
  string wrong_tab = _GetTabEmulation() ? NormalTab():EmulatedTab();
  for( int par=0; par<paragraphs(); par++ )
  {

    // look for leading tabs
    if( wrong_tab_regexp.search( text(par) ) < 0 ) continue;
    setSelection( par, 0, par, wrong_tab_regexp.matchedLength() );
    emit selectionChanged();

    // create replacement string and insert.
    ostringstream what;
    for( int i=0; i< int(wrong_tab_regexp.matchedLength()/wrong_tab.size()); i++ )
    { what << Tab(); }
    insert( what.str().c_str() );

  }

  // enable updates
  setUpdatesEnabled( true );
  updateContents();
  return;
}

//_____________________________________________________________________________
void TextDisplay::cut( void )
{
  Debug::Throw() << "TextDisplay::cut." << endl;
  
  if( !( Synchronized() && isActive() ) ) return CustomTextEdit::cut();

  KeySet<TextDisplay> displays( this );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { 
    (*iter)->setUpdatesEnabled( false );
    (*iter)->CustomTextEdit::cut();
  }

  CustomTextEdit::cut();

  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { 
    (*iter)->setUpdatesEnabled( true );
    (*iter)->updateContents();
  }

  return;  
}

//_____________________________________________________________________________
void TextDisplay::paste( void )
{

  // check if there is some synchronization to maintain
  if( !( Synchronized() && isActive() ) ) 
  {
    // make sure current paragraph is not highlighted when pasting
    if( GetParagraphHighlight().Enabled() )
    {
      
      GetParagraphHighlight().Update( -1 );
      CustomTextEdit::paste();
      GetParagraphHighlight().Update( GetPosition().Paragraph() );
      
    } else CustomTextEdit::paste();
    
    return;
    
  }
  
  KeySet<TextDisplay> displays( this );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { 
    (*iter)->setUpdatesEnabled( false );
    (*iter)->CustomTextEdit::paste();
  }
  
  // make sure current paragraph is not highlighted when pasting
  if( GetParagraphHighlight().Enabled() )
  {
    
    GetParagraphHighlight().Update( -1 );
    CustomTextEdit::paste();
    GetParagraphHighlight().Update( GetPosition().Paragraph() );
  
  } else CustomTextEdit::paste();
  
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { 
    (*iter)->setUpdatesEnabled( true ); 
    (*iter)->updateContents();
  }

  return;
  
}

//_____________________________________________________________________________
void TextDisplay::undo( void )
{
  
  // do nothing if synchronized and not active
  /* because the "base class" undo is called indirectly from the main associated display */
  if( Synchronized() && !isActive() ) return;
  
  // retrieve associated displays
  /* 
    it is not possible to use the synchronize key 
    for the active widget because it is temporarely disabled 
    in the keyPressedEvent method
  */
  KeySet<TextDisplay> displays( this );

  // no associated displays
  if( displays.empty() )
  {
    
    // clear paragraph highlighting first, if needed
    if( GetParagraphHighlight().Enabled() )
    {
      GetParagraphHighlight().Update( -1 );
      CustomTextEdit::undo();
      GetParagraphHighlight().Update( GetPosition().Paragraph() );
    } else CustomTextEdit::undo();
    
    if( isActive() ) emit UndoCalled();
    return;
    
  }
  
  // there are associated displays but this one is not active
  if( !( isActive() && isUndoAvailable() ) ) return;
  
  // check if associated displays have undo history
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { if( !(*iter)->isUndoAvailable() ) return; }

  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { 
    (*iter)->setUpdatesEnabled( false );
    (*iter)->CustomTextEdit::undo(); 
  }
  
  // clear paragraph highlighting first, if needed
  if( GetParagraphHighlight().Enabled() )
  {
    GetParagraphHighlight().Update( -1 );
    CustomTextEdit::undo();
    GetParagraphHighlight().Update( GetPosition().Paragraph() );
  } else CustomTextEdit::undo();
  
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { (*iter)->setUpdatesEnabled( true ); }
  
  emit UndoCalled();
  
}

//_____________________________________________________________________________
void TextDisplay::redo( void )
{
  
  // do nothing if synchronized and not active
  /* because the "base class" undo is called indirectly from the main associated display */
  if( Synchronized() && !isActive() ) return;
  
  // retrieve associated displays
  /* 
    it is not possible to use the synchronize key 
    for the active widget because it is temporarely disabled 
    in the keyPressedEvent method
  */
  KeySet<TextDisplay> displays( this );

  // no associated displays
  if( displays.empty() )
  {
    
    if( GetParagraphHighlight().Enabled() )
    {
      GetParagraphHighlight().Update( -1 );
      CustomTextEdit::redo();
      GetParagraphHighlight().Update( GetPosition().Paragraph() );
    } else CustomTextEdit::redo();    
    
    if( isActive() ) emit RedoCalled();
    return;
    
  }
  
  // there are associated displays but this one is not active
  if( !( isActive() && isRedoAvailable() ) ) return;
  
  // check if associated displays have redo history
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { if( !(*iter)->isRedoAvailable() ) return; }

  // undo all associated displays
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { 
    (*iter)->setUpdatesEnabled( false );
    (*iter)->CustomTextEdit::redo(); 
  }
  
  if( GetParagraphHighlight().Enabled() )
  {
    GetParagraphHighlight().Update( -1 );
    CustomTextEdit::redo();
    GetParagraphHighlight().Update( GetPosition().Paragraph() );
  } else CustomTextEdit::redo();
  
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { (*iter)->setUpdatesEnabled( true ); }

  emit RedoCalled();
  
}

//_______________________________________________________
void TextDisplay::insert( const QString & text, uint flags )
{
    
  if( isActive() && Synchronized() )
  {
    Debug::Throw( "TextDisplay::insert - synchronize.\n" );

    // propagate to other displays
    KeySet<TextDisplay> displays( this );
    for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      (*iter)->setUpdatesEnabled( false );
      (*iter)->SynchronizeBoxSelection( *this );
      (*iter)->SynchronizeSelection( *this, 0 );
      (*iter)->SynchronizeSelection( *this, BRACES_SELNUMBER );
      (*iter)->insert( text, flags );
      (*iter)->setUpdatesEnabled( true );
      (*iter)->updateContents();
    }
  }

  if( GetParagraphHighlight().Enabled() )
  {
    GetParagraphHighlight().Update( -1 );
    CustomTextEdit::insert( text, flags );
    GetParagraphHighlight().Update( GetPosition().Paragraph() );
  } else CustomTextEdit::insert( text, flags );
  
  return;

}

//_______________________________________________________
void TextDisplay::keyPressEvent( QKeyEvent* event )
{

  // check if tab key is pressed
  if( event->key() == Key_Tab && indent_->isEnabled() && !textCursor().hasSelection() )
  { emit indent( textCursor().block() ) ); }
  else 
  {

    // indent current paragraph when return is pressed
    if( event->key() == Key_Return && indent_->isEnabled() && !textCursor().hasSelection() )
    { emit indent( textCursor().block() ) ); }
  
    // process key
    CustomTextEdit::keyPressEvent( event );

    // reindent paragraph if needed
    if( indent_->enabled() && ( event->key() == Key_BraceRight || event->key() == Key_BraceLeft ) && !textCursor().hasSelection() )
    { emit indent( textCursor().block() ) ); }

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

//____________________________________________________________
#if WITH_ASPELL
QPopupMenu *TextDisplay::createPopupMenu( const QPoint& point )
{
  Debug::Throw( "TextDisplay::createPopupMenu.\n" );

  int par(0);
  int index( charAt( point, &par ) );
  TextPosition position( par, index );
  SPELLCHECK::Word word( highlight_->Misspelled( position ) );
  if( word.empty() ) return CustomTextEdit::createPopupMenu( point );

  // select range
  _Select( TextPosition(par, word.position_), word.size() );

  // retrieve menu
  QPopupMenu* menu = new SPELLCHECK::SuggestionMenu( this, word );
  connect( menu, SIGNAL( SuggestionSelected( const std::string& ) ), this, SLOT( _ReplaceSelection( const std::string& ) ) );
  connect( menu, SIGNAL( IgnoreWord( const std::string& ) ), this, SLOT( _IgnoreWord( const std::string& ) ) );
  connect( menu, SIGNAL( needUpdate( void ) ), this, SLOT( Rehighlight() ) );
  return menu;

}
#endif

//_____________________________________________________________________
void TextDisplay::_CreateReplaceDialog( void )
{
  Debug::Throw( "TextDisplay::_CreateReplaceDialog.\n" );
  CustomTextEdit::_CreateReplaceDialog();
  ReplaceDialog &dialog( _ReplaceDialog() );

  // insert multiple file buttons
  QPushButton* button = new QPushButton( "&Files", &dialog );
  QtUtil::FixSize( button );
  connect( button, SIGNAL( clicked() ), this, SLOT( _MultipleFileReplace() ) );
  button->setAutoDefault( false );
  QToolTip::add( button, "replace all occurence of the search string in the selected files" );
  dialog.AddDisabledButton( button );
  dialog.LocationLayout().addWidget( button );

}

//_______________________________________________________
void TextDisplay::_RemoveBoxSelectedText()
{
  Debug::Throw( "TextDisplay::_RemoveBoxSelectedText.\n" );
  if( isActive() && Synchronized() )
  {

    Debug::Throw( "TextDisplay::_RemoveBoxSelectedText - synchronize.\n" );

    // propagate to other displays
    KeySet<TextDisplay> displays( this );
    for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      (*iter)->setUpdatesEnabled( false );

      // make sure current box selections are synchronized
      (*iter)->SynchronizeBoxSelection( *this );
      (*iter)->_RemoveBoxSelectedText();
      (*iter)->setUpdatesEnabled( true );
      (*iter)->updateContents();
    }
  }

  CustomTextEdit::_RemoveBoxSelectedText();
}

//_______________________________________________________
void TextDisplay::_InsertBoxSelection( const BoxSelection& box )
{
  Debug::Throw( "TextDisplay::_InsertBoxSelection.\n" );
  if( isActive() && Synchronized() )
  {

    Debug::Throw( "TextDisplay::_InsertBoxSelection - synchronize.\n" );

    // propagate to other displays
    KeySet<TextDisplay> displays( this );
    for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      (*iter)->setUpdatesEnabled( false );

      // make sure current box selections are synchronized
      (*iter)->SynchronizeBoxSelection( *this );

      // insert argument box selection
      (*iter)->_InsertBoxSelection( box );
      (*iter)->setUpdatesEnabled( true );
      (*iter)->updateContents();
    }
  }

  CustomTextEdit::_InsertBoxSelection( box );
}

//_______________________________________________________
void TextDisplay::_ReplaceBoxSelection( const BoxSelection& box )
{
  Debug::Throw( "TextDisplay::_ReplaceBoxSelection.\n" );
  if( isActive() && Synchronized() )
  {

    Debug::Throw( "TextDisplay::_ReplaceBoxSelection - synchronize.\n" );

    // propagate to other displays
    KeySet<TextDisplay> displays( this );
    for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    {
      (*iter)->setUpdatesEnabled( false );

      // make sure current box selections are synchronized
      (*iter)->SynchronizeBoxSelection( *this );

      // insert argument box selection
      (*iter)->_ReplaceBoxSelection( box );
      (*iter)->setUpdatesEnabled( true );
      (*iter)->updateContents();
    }
  }

  CustomTextEdit::_ReplaceBoxSelection( box );
}

//_______________________________________________________
void TextDisplay::_SetBraces( const TextDisplay::BracesList& braces )
{
  Debug::Throw( "TextDisplay::_SetBraces.\n" );
  braces_ = braces;
  braces_set_.clear();
  for( BracesList::const_iterator iter = braces_.begin(); iter != braces_.end(); iter++ )
  {
    braces_set_.insert( (*iter)->First() );
    braces_set_.insert( (*iter)->Second() );
  }
}

//_______________________________________________________
void TextDisplay::_MultipleFileReplace( void )
{
  Debug::Throw( "TextDisplay::_MultipleFileReplace.\n" );
  TextSelection selection( _ReplaceDialog().GetSelection( false ) );

  // retrieve selection from replace dialog
  FileSelectionDialog dialog( this, selection );
  connect( &dialog, SIGNAL( FileSelected( const File&, TextSelection ) ), qApp, SLOT( MultipleFileReplace( const File&, TextSelection ) ) );
  dialog.exec();
  return;
}

//_______________________________________________________
void TextDisplay::_ReplaceSelection( const std::string& word )
{
  #if WITH_ASPELL
  Debug::Throw( "TextDisplay::_ReplaceSelection.\n" );
  SelectionRange range( GetSelectionRange() );
  SPELLCHECK::Word misspelled_word( highlight_->Misspelled( range.first ) );
  if( misspelled_word.empty() )
    throw runtime_error( DESCRIPTION( "invalid selection" ) );

  // update selection
  insert( word );
  #endif
}

//_______________________________________________________
void TextDisplay::_IgnoreWord( const std::string& word )
{
  #if WITH_ASPELL
  highlight_->IgnoreWord( word );
  #endif
}

//_______________________________________________________
void TextDisplay::_IndentCurrentParagraph( void )
{
  if( !indent_->Enabled() ) return;
  int paragraph(0), index(0);
  getCursorPosition( &paragraph, &index );
  emit Indent( paragraph );

}
  
//_______________________________________________________
void TextDisplay::_HighlightBraces( int paragraph, int index )
{
  // clear previously highlighted braces
  removeSelection( BRACES_SELNUMBER );

  // check flags
  if( !GetFlag( BRACES ) || braces_set_.empty() ) return;

  // check if paragraph is to be ignored
  if( IgnoreParagraph( paragraph ) ) return;

  // retrieve current character
  /*
    index is decremented by 1 because the braces are to be
    marked when the cursor is after it
  */
  if( index == 0 ) return;
  index--;
  char c = ((const char*)text(paragraph))[index];

  // check if character is in braces_set
  if( braces_set_.find( c ) == braces_set_.end() ) return;

  // check against opening braces
  BracesList::const_iterator braces = find_if(
    braces_.begin(),
    braces_.end(),
    TextBraces::FirstElementFTor(c) );
  if( braces != braces_.end() )
  {
    unsigned int increment(0);
    bool found(false);
    int local_index(0);
    while( paragraph < paragraphs() && !found )
    {
      if( !IgnoreParagraph( paragraph ) )
      {
        string local_text( (const char*)text(paragraph) );
        for( local_index = index; local_index < (int)local_text.size() && !found; local_index++ )
        {
          if( local_text[local_index] == (*braces)->First() ) increment++;
          if( local_text[local_index] == (*braces)->Second() ) increment--;
          if( increment == 0 ) {
            found = true;
            break;
          }
        }
      }

      if( !found )
      {
        paragraph++;
        index = 0;
      }
    }

    if( found )
    {
      setUpdatesEnabled( false );
      setSelection( paragraph, local_index, paragraph, local_index+1, BRACES_SELNUMBER );
      setUpdatesEnabled( true );
      updateContents();
    }

    return;
  }

  // check against closing braces
  braces = find_if(
    braces_.begin(),
    braces_.end(),
    TextBraces::LastElementFTor(c) );
  if( braces != braces_.end() )
  {
    unsigned int increment(0);
    bool found(false);
    int local_index(0);
    while( paragraph>=0 && !found )
    {
      if( !IgnoreParagraph( paragraph ) )
      {
        string local_text( (const char*)text(paragraph) );
        for( local_index = index; local_index >=0 && !found; local_index-- )
        {
          if( local_text[local_index] == (*braces)->Second() ) increment++;
          if( local_text[local_index] == (*braces)->First() ) increment--;
          if( increment == 0 ) {
            found = true;
            break;
          }
        }
      }

      if( !found )
      {
        paragraph--;
        index = (paragraph>=0) ? paragraphLength( paragraph )-1:0;
      }
    }

    if( found )
    {
      setUpdatesEnabled( false );
      setSelection( paragraph, local_index, paragraph, local_index+1, BRACES_SELNUMBER );
      setUpdatesEnabled( true );
      updateContents();
    }

    return;

  }

}

//_______________________________________________________
void TextDisplay::_HighlightParagraph( int paragraph, int index )
{
  
  Debug::Throw( "TextDisplay::_HighlightParagraph.\n" );
  
  // see if paragraph highlighting is enabled
  if( !GetParagraphHighlight().Enabled() ) return;
  
  // update
  GetParagraphHighlight().Update( paragraph, index );
  
  // redraw box selection if any
  if( HasBoxSelection() ) updateContents( _GetBoxSelection().Rect() );

}


//_______________________________________________________
void TextDisplay::_SynchronizeCursor( int paragraph, int index )
{
  if( !(Synchronized() && isActive()) ) return;
  KeySet<TextDisplay> displays( this );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    (*iter)->setUpdatesEnabled( false );
    (*iter)->setCursorPosition( paragraph, index );
    (*iter)->setUpdatesEnabled( true );
    (*iter)->updateContents();
  }
}

//_______________________________________________________
void TextDisplay::_SynchronizeSelection( void )
{
  if( !(Synchronized() && isActive()) ) return;

  KeySet<TextDisplay> displays( this );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    (*iter)->setUpdatesEnabled( false );
    (*iter)->SynchronizeSelection( *this, 0 );
    (*iter)->SynchronizeSelection( *this, BRACES_SELNUMBER );
    (*iter)->setUpdatesEnabled( true );
    (*iter)->updateContents();
  }
}

//_______________________________________________________
void TextDisplay::_SynchronizeBoxSelection( void )
{
  if( !(Synchronized() && isActive()) ) return;

  KeySet<TextDisplay> displays( this );
  for( KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    (*iter)->setUpdatesEnabled( false );
    (*iter)->SynchronizeBoxSelection( *this );
    (*iter)->setUpdatesEnabled( true );
    (*iter)->updateContents();
  }

}

//_____________________________________________________________
bool TextDisplay::_ContentsChanged( void ) const
{
  // check file
  if( GetFile().empty() ) return true;

  // open file
  ifstream in( GetFile().c_str() );
  if( !in ) return true;

  // dump file into character string
  string file_text;
  int par_id(0);
  while( !(in.rdstate() & ios::failbit ) )
  {
    char c;
    in.get(c);

    // check validity
    if(in.rdstate() & ios::failbit ) break;

    // check if end of line
    if( c == '\n' )
    {

      // check if corresponding paragraph exists
      if( par_id >= paragraphs() ) return true;

      // retrieve corresponding paragraph
      // remove last character
      string text( (const char*) this->text( par_id ) );
      if( text.size() ) text = text.substr( 0, text.size()-1);

      // compare to current text
      if( text != file_text ) return true;

      // reset text, increment paragraph
      file_text = "";
      par_id++;

    } else file_text.push_back( c );
  }

  in.close();

  // check last paragraph
  if( par_id+1 != paragraphs() ) return true;
  string text( (const char*) this->text( par_id ) );
  if( text.size() ) text = text.substr( 0, text.size()-1);

  // compare to current text
  if( text != file_text ) return true;

  // check if there are more paragraphs
  if( paragraphs() > par_id +1 ) return true;

  // files are identical
  return false;

}
