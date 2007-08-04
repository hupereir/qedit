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

#include "Config.h"
#include "CustomTextEdit.h"
#include "CustomTextDocument.h"
#include "Debug.h"
#include "File.h"
#include "HighlightPattern.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "TimeStamp.h"

// forward declaration
class DocumentClass;
class TextBraces;
class TextMacro;

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

  //! check if current entry has been modified or not
  void setModified( const bool& value = true )
  {
    
    // do nothing if state is unchanged
    if( value == document()->isModified() ) return;
    document()->setModified( value );
    
    // ask for update in the parent frame
    if( isActive() ) emit needUpdate( WINDOW_TITLE | UNDO_REDO );
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

  //! returns true if file was modified by external application
  bool fileModified( void );

  //! returns true if file was removed
  bool fileRemoved( void );
  
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
  
  //! Save file
  void save( void );
  
  //! Save file with new name
  void saveAs( void );

  //! Revert to save
  void revertToSave();
  
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
  
  //!@ name TextDisplay display flags
  //@{
  
  //! state bits
  enum Flag
  {
              
    //! braces highlighting
    BRACES = 1<<3,
        
    //! has active/inactive paper color available
    HAS_PAPER = 1<<7,
        
    /*! 
      has wrap is set to true when it is modified
      using the menu. It is then not overwritten
      by the document class setting any more.
    */
    HAS_WRAP = 1<<9,
    
    //! WRAP mode
    WRAP = 1<<10

  };

  //! Flags (bitwise or of the Flag bits)
  const unsigned int& flags( void ) const
  { return flags_; }

  //! flags (bitwise or of the Flag bits)
  void setFlags( const unsigned int& flags )
  { flags_ = flags; }
  
  //! update flags
  void setFlag( const Flag& bit, const bool& value )
  {
    if( value ) flags_ |= bit;
    else flags_ &= (~bit);
  }
  
  //! flag bit status
  bool flag( const Flag& bit ) const
  { return flags_ & bit; }
  
  //@}

  //!@name macro
  //@{
  
  //! list of macros
  typedef std::list< TextMacro* > MacroList;

  //! list of macros
  const MacroList& macros( void ) const
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
    Exception::checkPointer( highlight_, DESCRIPTION( "highlight_ not initialized." ) );
    return *highlight_;
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
  QAction* textIndentAction( void )
  { return indent_action_; }
  
  //! toggle text highlighting
  QAction* textHighlightAction( void )
  { return text_highlight_action_; }
 
  //! toggle text highlighting
  QAction* bracesHighlightAction( void )
  { return braces_highlight_action_; }
  
  //! file information
  QAction* fileInfoAction( void )
  { 
    file_info_action_->setEnabled( !file().empty() );
    return file_info_action_; 
  }
  
  //@}
  
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
  /*! returns true if matching document class was found */
  void updateDocumentClass( void );
    
  //! update display based on flags
  bool updateFlags( void );
  
  //! indent selection
  void indentSelection( void );

  //! process macro by name
  void processMacro( std::string );

  //! replace all leading tabs in text when tab emulation is active
  void replaceLeadingTabs( const bool& confirm = true );

  //! rehighlight
  void rehighlight( void )
  { highlight_->setDocument( document() ); }

  protected:

  //! keypress event [overloaded]
  void keyPressEvent( QKeyEvent* );

  //! focus event [overloaded]
  void focusInEvent( QFocusEvent* );

  //! create replace dialog
  virtual void _createReplaceDialog( void );
    
  //! clear macros
  void _clearMacros( void )
  { macros_.clear(); }

  //! macros
  void _setMacros( const MacroList& macros)
  { macros_ = macros; }  
  
    //! braces
  typedef std::list< TextBraces* > BracesList;
  
  //! braces
  typedef std::set< char > BracesSet;
  
  //! braces
  const BracesList& _braces( void ) const
  { return braces_; }
  
  //! braces
  void _clearBraces( void )
  {
    braces_.clear();
    braces_set_.clear();
  }

  //! set braces
  void _setBraces( const BracesList& );

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

  //! returns true if text contents differs from file contents
  bool _contentsChanged( void ) const;
  
  //! show file info
  void _showFileInfo( void )
  { FileInfoDialog( this ).exec(); }
  
  protected slots:

  //! indent paragraph (when return or tab is pressed)
  void _indentCurrentParagraph( void );

  //! selection changed
  void _selectionChanged( void )
  { if( isActive() ) emit needUpdate( CUT|COPY ); }
  
  //! replace selection in multiple files
  void _multipleFileReplace( void );
  
  //! toggle text indentation
  void _toggleTextIndent( bool state )
  { textIndent().setEnabled( textIndentAction()->isEnabled() && state ); }
    
  //! toggle text highlight
  void _toggleTextHighlight( bool state )
  { textHighlight().setEnabled( textHighlightAction()->isEnabled() && state ); }
  
  //! toggle braces
  void _toggleBracesHighlight( bool state )
  { return; }

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

  //! true if this display is the active display
  bool active_;

  //!@name document classes specific members
  //@{

  //! text highlight
  TextHighlight* highlight_;

  //! text indent
  TextIndent* indent_;

  //! text braces
  BracesList braces_;

  //! keep track of all braces in a single set for fast access
  BracesSet braces_set_;

  //! text macro
  MacroList macros_;

  //@}

  //!@name actions
  //@{
  
  //! toggle indentation
  QAction* indent_action_;
  
  //! toggle text highlighting
  QAction* text_highlight_action_;
 
  //! toggle text highlighting
  QAction* braces_highlight_action_;
 
  //! toggle text highlighting
  QAction* file_info_action_;
  
  //! 
  
  //! empty line
  static const QRegExp empty_line_regexp_;

};

#endif
