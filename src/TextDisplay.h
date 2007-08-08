// $Id$
#ifndef TextDisplay_h
#define TextDisplay_h

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
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
\file TextDisplay.h
\brief text display window
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <string>
#include <QRegExp>
#include <QAction>

#include "AskForSaveDialog.h"
#include "FileModifiedDialog.h"
#include "FileRemovedDialog.h"
#include "Config.h"
#include "CustomTextEdit.h"
#include "Debug.h"
#include "File.h"
#include "HighlightPattern.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "TimeStamp.h"

#if WITH_ASPELL
#include "DictionaryMenu.h"
#include "FilterMenu.h"
#endif

// forward declaration
class DocumentClass;
class HighlightBlockData;
class OpenPreviousMenu;

//! text display window
class TextDisplay: public CustomTextEdit
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! constructor
  TextDisplay( QWidget* parent );

  //! destructor
  virtual ~TextDisplay();
  
  //! clone display configuration and document
  virtual void synchronize( TextDisplay* display );

  //! set openPrevious menu
  void setMenu( OpenPreviousMenu* menu )
  { menu_ = menu; }
  
  //! open previous menu
  OpenPreviousMenu& menu( void )
  { 
    Exception::checkPointer( menu_, DESCRIPTION( "menu_ not initialized" ) );
    return *menu_;
  }
  
  //! check if current entry has been modified or not
  void setModified( const bool& value = true )
  {
    
    // do nothing if state is unchanged
    if( value == document()->isModified() ) return;
    document()->setModified( value );
    
    // ask for update in the parent frame
    if( isActive() ) emit needUpdate( WINDOW_TITLE | UNDO_REDO );
  }  

  //! read-only
  virtual void setReadOnly( bool readonly )
  {
    Debug::Throw( "TextDisplay::setReadOnly.\n" );
    CustomTextEdit::setReadOnly( readonly );
    emit needUpdate( WINDOW_TITLE | CUT | PASTE | UNDO_REDO ); 
  }

  //! update flags (to be passed to TextEditor to change button status)
  enum UpdateFlags
  {
    
    //! window title
    WINDOW_TITLE = 1<<0, 
    
    //! file name (in bottom status bar)
    FILE_NAME = 1<<1,
    
    //! cut availability
    CUT = 1<<2,

    //! copy availability
    COPY = 1<<3,
    
    //! paster availability
    PASTE = 1<<4,
    
    //! undo/redo availability
    UNDO_REDO = 1<<5,
    
    //! spell checking 
    SPELLCHECK = 1<<6,
    
    //! all the above
    ALL = WINDOW_TITLE|FILE_NAME|CUT|COPY|PASTE|UNDO_REDO|SPELLCHECK
    
  };
  
  //!@ name file management
  //@{
  
  //! open file
  void openFile( File file );
  
  //! set file name
  void setFile( const File& file );
  
  //! file
  const File& file( void ) const
  { return file_; }
  
  //! working directory
  const File& workingDirectory() const
  { return working_directory_; }
  
  //! used to select editor with matching filename
  class SameFileFTor
  {
    public:

    //! constructor
    SameFileFTor( const File& file ):
      file_( file.expand() )
    {}

    //! predicate
    bool operator() ( const TextDisplay* display ) const
    { return display->file() == file_; }

    private:

    //! predicted file
    const File file_;

  };

  //! used to select editor with empty, unmodified file
  class EmptyFileFTor
  {
    public:

    //! predicate
    bool operator() ( const TextDisplay* display ) const
    { return display->file().empty() && !display->document()->isModified(); }

  };

  //! used to select editor modified files
  class ModifiedFTor
  {
    public:

    //! predicate
    bool operator() ( const TextDisplay* display ) const
    { return display->document()->isModified(); }

  };
  
  //! ask for save if modified
  AskForSaveDialog::ReturnCode askForSave( const bool& enable_all = false );
  
  //! check if file has been removed externally
  FileRemovedDialog::ReturnCode checkFileRemoved( void );

  //! check if file has been modified externally
  FileModifiedDialog::ReturnCode checkFileModified( void );
  
  //! Save file
  void save( void );
  
  //! Save file with new name
  void saveAs( void );

  //! Revert to save
  void revertToSave( void );
  
  //@}
  
  //! activity
  void setActive( const bool& value );

  //! activity
  const bool& isActive( void ) const
  { return active_; }

  //!@name document class
  //@{
  
  //! class name
  void setClassName( const std::string& name )
  { class_name_ = name; }
  
  //! class name
  const std::string& className( void ) const 
  { return class_name_; }
  
  //@}
  
  //!@name macro
  //@{
  
  //! list of macros
  const TextMacro::List& macros( void ) const
  { return macros_; }
  
  //@}
  
  //! paper color for active/inactive views
  const QColor& paper( const bool& active ) const
  { return active ? active_color_:inactive_color_;}
 
  //! returns true if paragraph is to be ignored when identing/parsing braces
  bool ignoreParagraph( const QTextBlock& paragraph );

  //! text highlight
  TextHighlight& textHighlight( void )
  {
    BASE::KeySet<TextHighlight> highlights( dynamic_cast<Key*>( document() ) );
    Exception::check( highlights.size() == 1, "invalid association to TextHighlight.\n" );
    return **highlights.begin();
  }

  //! text highlight
  const TextHighlight& textHighlight( void ) const
  {
    BASE::KeySet<TextHighlight> highlights( dynamic_cast<BASE::Key*>(document()) );
    Exception::check( highlights.size() == 1, "invalid association to text-highlight" );
    return **highlights.begin();
  }

  //! text indent
  const TextIndent& textIndent( void ) const
  {
    Exception::checkPointer( indent_, DESCRIPTION( "indent_ not initialized." ) );
    return *indent_;
  }
  
  //! text indent
  TextIndent& textIndent( void )
  {
    Exception::checkPointer( indent_, DESCRIPTION( "indent_ not initialized." ) );
    return *indent_;
  }

  //! returns true if current text has leading tabs of wrong type
  bool hasLeadingTabs( void ) const;

  //! Get HTML formated text
  QDomElement htmlNode( QDomDocument& document );

  //!@name actions
  //@{
    
  //! toggle indentation
  QAction* textIndentAction( void ) const
  { return text_indent_action_; }
  
  //! toggle text highlighting
  QAction* textHighlightAction( void ) const
  { return text_highlight_action_; }
 
  //! toggle braces highlighting
  QAction* bracesHighlightAction( void ) const
  { return braces_highlight_action_; }

  //! autospell action
  QAction* autoSpellAction( void ) const
  { return autospell_action_; }
  
  //! spellcheck action
  QAction* spellcheckAction( void ) const
  { return spellcheck_action_; }
  
  //! file information
  QAction* fileInfoAction( void ) const
  { 
    file_info_action_->setEnabled( !file().empty() );
    return file_info_action_; 
  }

  #if WITH_ASPELL
  
  //! spellcheck dictionary selection
  QAction* dictionaryMenuAction( void ) const
  { 
    dictionary_menu_action_->setEnabled( autoSpellAction()->isChecked() );
    return dictionary_menu_action_;
  }
  
  //! spellcheck filter selection
  QAction* filterMenuAction( void ) const
  { 
    filter_menu_action_->setEnabled( autoSpellAction()->isChecked() );
    return filter_menu_action_;
  }
  
  #endif
  
  //@}
    
  // return true if block is an empty line
  bool isEmptyBlock( const QTextBlock& block ) const
  { return empty_line_regexp_.indexIn( block.text() ) >= 0; }
  
  // return true is block is to be ignored from indentation scheme
  bool ignoreBlock( const QTextBlock& block ) const;
  
  signals:

  //! emmited when indentation of current paragraph is required
  void indent( QTextBlock );

  //! emmited when recieve focus
  void hasFocus( TextDisplay* );

  //! emmited whenever editframe toolbar, window title or file name editor needs update
  /* \param flags, bitwise or of UpdateFlags */
  void needUpdate( unsigned int flags );
  
  public slots:

  //! update configuration
  void updateConfiguration( void );
  
  //! set document class
  void updateDocumentClass( void );

  //! spellcheck configuration
  void updateSpellCheckConfiguration( void );
  
  //! indent selection
  void indentSelection( void );

  //! process macro by name
  void processMacro( std::string );

  //! replace all leading tabs in text when tab emulation is active
  void replaceLeadingTabs( const bool& confirm = true );

  //! rehighlight
  void rehighlight( void );

  protected:

  //! keypress event [overloaded]
  virtual void keyPressEvent( QKeyEvent* );

  //! focus event [overloaded]
  virtual void focusInEvent( QFocusEvent* );

  //! context menu event [overloaded]
  virtual void contextMenuEvent( QContextMenuEvent* );

  //! create replace dialog
  virtual void _createReplaceDialog( void );
    
  //! clear macros
  void _clearMacros( void )
  { macros_.clear(); }

  //! macros
  void _setMacros( const TextMacro::List& macros)
  { macros_ = macros; }  
  
  //! paper color for active/inactive views
  void _setPaper( const bool& active, const QColor& color )
  { 
    if( !color.isValid() ) return;
    if( active ) active_color_ = color;
    else inactive_color_ = color;
  }
  
  //! set background color
  void _setPaper( const QColor& color );
  
  //! last save time stamp
  void _setLastSaved( const TimeStamp& stamp )
  { last_save_ = stamp; }
  
  //! working directory
  void _setWorkingDirectory( const File& file )
  { working_directory_ = file; }

   //! if true file is not checked on enter event
  const bool& _ignoreWarnings() const
  { return ignore_warnings_; }
  
  //! if true file is not checked on enter event
  void _setIgnoreWarnings( const bool& value )
  { ignore_warnings_ = value; }
  
  //! returns true if text contents differs from file contents
  bool _contentsChanged( void ) const;

  //! returns true if file was removed
  bool _fileRemoved( void ) const;
   
  //! returns true if file was modified by external application
  bool _fileModified( void );
  
  //! track text modifications for syntax highlighting
  void _setBlockModified( const QTextBlock& );
  
  #if WITH_ASPELL
  
  //! dictionary menu
  SPELLCHECK::DictionaryMenu& _dictionaryMenu( void )
  { return *dictionary_menu_; }

  //! filter menu
  SPELLCHECK::FilterMenu& _filterMenu( void )
  { return *filter_menu_; }
  
  #endif

  //!@name braces
  //@{
  
  //! braces enabled
  bool _isBracesEnabled( void ) const
  { return bracesHighlightAction()->isChecked() && !braces_.empty(); }
  
  //! braces
  const TextBraces::List& _braces( void ) const
  { return braces_; }
  
  //! set braces
  void _setBraces( const TextBraces::List& );
  
  //@}
  
  protected slots:

  //! highlight current block
  /*! overloaded from CustomTextEdit. It allows to store the data of the current block */
  virtual void _highlightCurrentBlock( void );
 
  //! indent paragraph (when return or tab is pressed)
  void _indentCurrentParagraph( void );

  //! selection changed
  void _selectionChanged( void )
  { if( isActive() ) emit needUpdate( CUT|COPY ); }
  
  //! replace selection in multiple files
  void _multipleFileReplace( void );
  
  //! toggle text indentation
  void _toggleTextIndent( bool state );
    
  //! toggle text highlight
  void _toggleTextHighlight( bool state );
  
  //! toggle braces
  void _toggleBracesHighlight( bool state );
  
  //!@name spell check
  //@{
  
  //! autospell
  void _toggleAutoSpell( bool state );
  
  //! run spellcheck
  void _spellcheck( void );
  
  //! change autospell filter
  void _selectFilter( const std::string& );
  
  //! change autospell dictionary
  void _selectDictionary( const std::string& );
  
  //@}
  
  //! show file info
  void _showFileInfo( void );
  
  //! track text modifications for syntax highlighting
  void _setBlockModified( int, int, int );
  
  //! update action status
  virtual void _updateSelectionActions( bool state )
  { 
    CustomTextEdit::_updateSelectionActions( state );
    emit needUpdate( CUT|COPY );
  }
  
  //! update paste action 
  /*! depends on clipboard status and editability */
  virtual void _updatePasteAction( void )
  {
    CustomTextEdit::_updatePasteAction();
    emit needUpdate( PASTE );
  }
  
  //! text changed
  virtual void _textModified( void )
  { if( isActive() ) emit needUpdate( WINDOW_TITLE ); }
  
  //! ignore current misspelled word
  /*! this method does nothing if not compiled against aspell */
  void _ignoreMisspelledWord( std::string );
  
  //! replace current selection with spell-checked suggestion
  /*! this method does nothing if not compiled against aspell */
  void _replaceMisspelledSelection( std::string );
  
  //! highlight braces
  void _highlightBraces( void );
  
  private:
  
  //! file
  File file_;

  //! working directory
  File working_directory_;
  
  //! associated document class name
  std::string class_name_;
  
  //! display flags
  unsigned int flags_;
  
  //! paper color or active views
  QColor active_color_;
  
  //! paper color for inactive views
  QColor inactive_color_;
  
  //! last save timeStamp
  TimeStamp last_save_;

  //! if true, _checkFile is disabled
  bool ignore_warnings_;
  
  //! true if this display is the active display
  bool active_;

  //!@name document classes specific members
  //@{

  //! text indent
  TextIndent* indent_;

  //! text macro
  TextMacro::List macros_;

  //@}

  //!@name actions
  //@{
  
  //! toggle indentation
  QAction* text_indent_action_;
  
  //! toggle text highlighting
  QAction* text_highlight_action_;
 
  //! toggle text highlighting
  QAction* braces_highlight_action_;
 
  //! toggle autospell
  QAction* autospell_action_;
  
  //! run spell checker
  QAction* spellcheck_action_;
  
  //! toggle text highlighting
  QAction* file_info_action_;
  
  #if WITH_ASPELL

  //! spellcheck dictionary selection menu
  QAction* dictionary_menu_action_;
  
  //! spellcheck filter selection menu
  QAction* filter_menu_action_;
  
  #endif
  //@}
  
  #if WITH_ASPELL
  
  //! spellcheck dictionary selection menu
  SPELLCHECK::DictionaryMenu* dictionary_menu_;
  
  //! spellcheck filter selection menu
  SPELLCHECK::FilterMenu* filter_menu_;
  
  #endif
  
  //! parent OpenPrevious menu
  OpenPreviousMenu* menu_;
  
  //! current block data
  // HighlightBlockData* current_block_data_;
  
  //!@name text braces
  //@{
    
  //! text braces
  TextBraces::List braces_;

  //! keep track of all braces in a single set for fast access
  TextBraces::Set braces_set_;
  
  //@}
  
  //! empty line
  static const QRegExp empty_line_regexp_;

};

#endif
