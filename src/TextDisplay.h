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

#include "Config.h"
#include "CustomTextEdit.h"
#include "Debug.h"
#include "File.h"
#include "HighlightPattern.h"
#include "TextHighlight.h"
#include "TimeStamp.h"

// forward declaration
class DocumentClass;
class TextBraces;
class TextIndent;
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
  
  //! clone display configuration
  virtual void clone( TextDisplay& display );
  
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
  void setFile( const File& file )
  { 
    file_ = file; 
    if( file.Exist() ) 
    {
      _setLastSaved( file.lastModified() );
      _setWorkingDirectory( file.path() );
      setReadOnly( file.exist() && !file.isWritable() );
    }
    
    if( isActive() ) emit needUpdate( WINDOW_TITLE | FILE_NAME ); 
    
  }
  
  //! file
  const File& file( void ) const
  { return file_; }
  
  //! working directory
  const File& workingDirectory() const
  { return working_directory_; }

  //!@name text backup for synchronization check
  //@{
  
  //! text backup
  void makeBackup( void )
  { backup_text_ = plainText(); }

  //! backup text
  const QString& backupText( void ) const
  { return backup_text_; }

  //@}
 
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
      file_( file.Expand() )
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
    { return display->file().empty() && !display->document().isModified(); }

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
  void revertToSave( const bool& check = true );
  
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
  
  //! set document class
  /*! returns true if matching document class was found */
  void updateDocumentClass( void );

  //@}
  
  //!@ name TextDisplay display flags
  //@{
  
  //! state bits
  enum Flag
  {
      
    //! indentation
    INDENT = 1<<0,
    
    //! syntax highlighting
    HIGHLIGHT = 1<<1,

    //! highlight current paragraph
    HIGHLIGHT_PARAGRAPH = 1<<2,
    
    //! braces highlighting
    BRACES = 1<<3,
    
    //! automatic spellcheck
    AUTOSPELL = 1<<4,

    // additional bits to tell if previous options are available
    //! indentation patterns available
    HAS_INDENT = 1<<5,
    
    //! highlight patterns available
    HAS_HIGHLIGHT = 1<<6,

    //! has active/inactive paper color available
    HAS_PAPER = 1<<7,
    
    //! braces pattern available
    HAS_BRACES = 1<<8,
    
    /*! 
      has wrap is set to true when it is modified
      using the menu. It is then not overwritten
      by the document class setting any more.
    */
    HAS_WRAP = 1<<9,

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
  
  //! update display based on flags
  bool applyFlags( void );
  
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
    Exception::checkPointer( highlight_, DESCRIPTION( "highlight_ not initialized." ) );
    return *highlight_;
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

  signals:

  //! emmited when indentation of current paragraph is required
  void indent();

  //! emmited when recieve focus
  void hasFocus( TextDisplay* );

  //! emmited whenever editframe toolbar, window title or file name editor needs update
  /* \param flags, bitwise or of UpdateFlags */
  void needUpdate( unsigned int flags );
  
  public slots:

  //! check if current entry has been modified or not
  void setModified( const bool& value = true )
  {
    
    // do nothing if state is unchanged
    if( value == document()->isModified() ) return;
    document()->setModified( value );
    
    // ask for update in the parent frame
    if( isActive() ) emit needUpdate( WINDOW_TITLE | UNDO_REDO );
  }
  
  //! indent selection
  void indentSelection( void );

  //! process macro by name
  void processMacro( std::string );

  //! replace all leading tabs in text when tab emulation is active
  void replaceLeadingTabs( const bool& confirm );

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
  
  //!@ name backup text for synchronization
  //@{
  
  //! backup text
  void _setBackupText( const std::string text ) 
  { backup_text_ = text; }
  
  //@}
  
  //! clear macros
  void _clearMacros( void )
  { macros_.clear(); }

  //! macros
  void _setMacros( const MacroList& macros)
  { macros_ = macros; }  
    
  //! paper color for active/inactive views
  void _setPaper( const bool& active, const QColor& color )
  { 
    if( !color.isValid() ) return;
    if( active ) active_color_ = color;
    else inactive_color_ = color;
  }
  
  //! last save time stamp
  void _setLastSaved( const TimeStamp& stamp )
  { last_save_ = stamp; }
  
  //! working directory
  void _setWorkingDirectory( const File& file )
  { working_directory_ = file; }
  
  protected slots:

  //! indent paragraph (when return or tab is pressed)
  void _indentCurrentParagraph( void );

  //! selection changed
  void _selectionChanged( void )
  { if( isActive() ) emit needUpdate( CUT|COPY ); }
  
  //! replace selection in multiple files
  void _multipleFileReplace( void );

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

  //! copy of file text when splitting window
  /*! it is used in case synchronization is lost between views */
  std::string backup_text_;

  //! true if this display is the active display
  bool active_;

  //! true if display should synchronize with others
  bool synchronize_;

  //!@name document classes specific members
  //@{

  //! text highlight
  TextHighlight* highlight_;

  //! text indent
  TextIndent* indent_;

  //! text macro
  MacroList macros_;

  //@}

  //! empty line
  static const QRegExp empty_line_regexp_;

};

#endif
