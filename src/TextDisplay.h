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


#include <QRegExp>
#include <QAction>

#include "AnimatedTextEditor.h"
#include "AskForSaveDialog.h"
#include "BlockDelimiter.h"
#include "FileList.h"
#include "FileModifiedDialog.h"
#include "FileRemovedDialog.h"
#include "Config.h"
#include "Debug.h"
#include "FileCheck.h"
#include "FileRecord.h"
#include "HighlightBlockFlags.h"
#include "HighlightPattern.h"
#include "NewDocumentNameServer.h"
#include "ParenthesisHighlight.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "TimeStamp.h"

#if WITH_ASPELL
#include "DictionaryMenu.h"
#include "FilterMenu.h"
#endif

// forward declaration
class BlockDelimiterDisplay;
class DocumentClass;
class HighlightBlockData;
class TextHighlight;

//! text display window
class TextDisplay: public AnimatedTextEditor
{
  
  //! Qt meta object declaration
  Q_OBJECT
    
  public:
    
  //! constructor
  TextDisplay( QWidget* parent );
  
  //! destructor
  virtual ~TextDisplay();
  
  //! used to select editor with matching filename
  class SameFileFTor
  {
    public:

    //! constructor
    SameFileFTor( const File& file )
    {
      
      if( file.isAbsolute() ) file_ = file.expand();
      else file_ = file;
    
    }

    //! predicate
    bool operator() ( const TextDisplay* display ) const
    { return display->file() == file_; }

    private:

    //! predicted file
    File file_;

  };

  //! used to select editor with empty, unmodified file
  class EmptyFileFTor
  {
    public:

    //! predicate
    bool operator() ( const TextDisplay* display ) const
    { 
      return 
        display->file().isEmpty() || 
        (display->isNewDocument() && !display->document()->isModified() ); 
      }

  };

  //! number of block associated to argument
  /*! reimplemented from base class to account for collapsed blocks */
  virtual int blockCount( const QTextBlock& ) const;
  
  //! clone display configuration and document
  virtual void synchronize( TextDisplay* display );
  
  //! check if current entry has been modified or not
  void setModified( const bool& value = true );

  //! read-only
  virtual void setReadOnly( const bool& value );

  //! retrieve context menu. Create it on first call
  virtual void installContextMenuActions( QMenu& menu, const bool& all_actions = true );

  //! draw margins
  virtual void paintMargin( QPainter& );

  //! update flags (to be passed to TextEditor to change button status)
  enum UpdateFlags
  {
        
    //! file name (in bottom status bar and navigation frame)
    FILE_NAME = 1<<0,
    
    //! read only
    READ_ONLY = 1<<1,
    
    //! modified
    MODIFIED = 1<<2,
      
    //! cut availability
    CUT = 1<<3,

    //! copy availability
    COPY = 1<<4,
    
    //! paster availability
    PASTE = 1<<5,
    
    //! undo/redo availability
    UNDO_REDO = 1<<6,
    
    //! overwrite mode
    SPELLCHECK = 1<<7,

    //! keyword modifiers
    MODIFIERS = 1<<8,
    
    //! display count
    DISPLAY_COUNT = 1<<9,
    
    //! active file changed
    ACTIVE_DISPLAY_CHANGED = FILE_NAME|READ_ONLY|CUT|COPY|PASTE|UNDO_REDO|SPELLCHECK|MODIFIERS,

    //! active file changed
    ACTIVE_VIEW_CHANGED = FILE_NAME|READ_ONLY|CUT|COPY|PASTE|UNDO_REDO|SPELLCHECK|MODIFIERS|DISPLAY_COUNT,
    
    //! all the above
    ALL = FILE_NAME|MODIFIED|READ_ONLY|CUT|COPY|PASTE|UNDO_REDO|SPELLCHECK|MODIFIERS|DISPLAY_COUNT
    
  };
  
  
  //!@ name file management
  //@{
  
  //! file
  void setFile( File file, bool check_autosave = true );
    
  //! define as new document
  void setIsNewDocument( void );
  
  //! is new document
  const bool& isNewDocument( void ) const
  { return is_new_document_; }

  //! new document name server
  static NewDocumentNameServer& newDocumentNameServer( void );
  
  //! file
  const File& file( void ) const
  { return file_; }
  
  //! working directory
  const File& workingDirectory() const
  { return working_directory_; }

  //! last saved time stamp
  const TimeStamp& lastSaved( void ) const
  { return last_saved_; }
  
  //! clear file check data
  void clearFileCheckData( void );

  //! file check data
  void setFileCheckData( const FileCheck::Data& data );
  
  //! file check data
  const FileCheck::Data& fileCheckData( void ) const
  { return file_check_data_; }
  
  //! ask for save if modified
  AskForSaveDialog::ReturnCode askForSave( const bool& enable_all = false );
  
  //! check if file has been removed externally
  FileRemovedDialog::ReturnCode checkFileRemoved( void );

  //! check if file has been modified externally
  FileModifiedDialog::ReturnCode checkFileModified( void );
  
  //! check if file read-only state has changed
  void checkFileReadOnly( void );
  
  //! Save file
  void save( void );
  
  //! Save file with new name
  void saveAs( void );

  //! Revert to save
  void revertToSave( void );
  
  //@}

  //!@name document class
  //@{
  
  //! class name
  void setClassName( const QString& name )
  { class_name_ = name; }
  
  //! class name
  const QString& className( void ) const 
  { return class_name_; }
  
  //@}
  
  //!@name macro
  //@{
  
  //! list of macros
  const TextMacro::List& macros( void ) const
  { return macros_; }
  
  //@}
   
  //! returns true if paragraph is to be ignored when identing/parsing parenthesis
  bool ignoreParagraph( const QTextBlock& paragraph );

  //! text highlight
  bool hasTextHighlight( void )
  { return text_highlight_; }

  //! text highlight
  TextHighlight& textHighlight( void )
  { return *text_highlight_; }

  //! text highlight
  const TextHighlight& textHighlight( void ) const
  { return *text_highlight_; }
  
  //! text indent
  TextIndent& textIndent( void ) const
  {
    assert( indent_ );
    return *indent_;
  }
 
  //! block delimiter display
  bool hasBlockDelimiterDisplay( void ) const
  { return block_delimiter_display_; }

  //! block delimiter display
  BlockDelimiterDisplay& blockDelimiterDisplay( void ) const
  { return *block_delimiter_display_; }

  //! returns true if current text has leading tabs of wrong type
  bool hasLeadingTabs( void ) const;

  //! convert to plain text
  /*! 
  This method makes sure that full text is obtained even when some blocks are collapsed.
  this should be an overloaded function, but the base class method is not virtual
  however, it is never called via a pointer to the base class, so that it should be fine.
  */
  QString toPlainText( void ) const;
    
  //! Get HTML formated text
  QDomElement htmlNode( QDomDocument& document, const int& max_line_size = 0 );

  //!@name actions
  //@{
    
  //! toggle indentation
  QAction& textIndentAction( void ) const
  { return *text_indent_action_; }
  
  //! toggle text highlighting
  QAction& textHighlightAction( void ) const
  { return *text_highlight_action_; }
 
  //! toggle parenthesis highlighting
  QAction& parenthesisHighlightAction( void ) const
  { return *parenthesis_highlight_action_; }

  //! autospell action
  QAction& autoSpellAction( void ) const
  { return *autospell_action_; }
    
  //! show block delimiters
  bool hasBlockDelimiterAction( void ) const
  { return show_block_delimiter_action_; }
  
  //! show block delimiters
  QAction& showBlockDelimiterAction( void ) const
  { return *show_block_delimiter_action_; }
    
  //! spellcheck action
  QAction& spellcheckAction( void ) const
  { return *spellcheck_action_; }
  
  //! indent selection
  QAction& indentSelectionAction( void ) const
  { return *indent_selection_action_; }
 
  //! indent selection
  QAction& baseIndentAction( void ) const
  { return *base_indent_action_; }
  
  //! replace leading tab actions
  QAction& leadingTabsAction( void ) const
  { return *leading_tabs_action_; }
  
  //! file information
  QAction& filePropertiesAction( void ) const
  { return *file_properties_action_; }

  #if WITH_ASPELL
  
  //! spellcheck dictionary selection
  QAction& dictionaryMenuAction( void ) const
  { return *dictionary_menu_action_; }
  
  //! spellcheck filter selection
  QAction& filterMenuAction( void ) const
  { return *filter_menu_action_; }
 
  #endif
  
  //! tag block action
  QAction &tagBlockAction( void ) const
  { return* tag_block_action_; }
  
  //! next tag action
  QAction &nextTagAction( void ) const
  { return* next_tag_action_; }
  
  //! previous tag action
  QAction &previousTagAction( void ) const
  { return* previous_tag_action_; }
  
  //! clear current block tags action
  QAction &clearTagAction( void ) const
  { return* clear_tag_action_; }
  
  //! clear all tags action
  QAction &clearAllTagsAction( void ) const
  { return* clear_all_tags_action_; }
  
  //@}
    
  //!@name block interface
  //@{
  
  // return true if block is an empty line
  bool isEmptyBlock( const QTextBlock& block ) const
  { return _emptyLineRegExp().indexIn( block.text() ) >= 0; }
  
  //! return true is block is to be ignored from indentation scheme
  bool ignoreBlock( const QTextBlock& block ) const;
  
  //! tag block (with diff flag)
  void tagBlock( QTextBlock, const unsigned int& tag );
  
  //! clear block tags if match argument
  void clearTag( QTextBlock, const int& tags );

  //! true if current blocks (or selection) has tag
  bool isCurrentBlockTagged( void );
  
  //! true if some blocks have tags
  bool hasTaggedBlocks( void );
  
  //@}
  
  //! return parenthesis highlight object
  ParenthesisHighlight& parenthesisHighlight( void ) const
  { return *parenthesis_highlight_; }
  
  signals:

  //! emmited when indentation several blocks is required
  void indent( QTextBlock, QTextBlock );

  //! emmited when indentation of one block is required
  void indent( QTextBlock, bool  );

  //! emmited whenever mainwindow toolbar, window title or file name editor needs update
  /* \param flags, bitwise or of UpdateFlags */
  void needUpdate( unsigned int flags );
  
  public slots:
  
  //! set document class
  void updateDocumentClass( void )
  { _updateDocumentClass( file(), isNewDocument() ); }

  //! process macro by name
  void processMacro( QString );

  //! rehighlight
  void rehighlight( void );
  
  //! clear all blocks if match argument
  void clearAllTags( const int& tags = TextBlock::ALL_TAGS );
  
  //! change autospell filter
  void selectFilter( const QString& );
  
  //! change autospell dictionary
  void selectDictionary( const QString& );
  
  protected:

  //!@name event handlers
  //@{
  
  //! generic event
  virtual bool event( QEvent* );
  
  //! keypress event [overloaded]
  virtual void keyPressEvent( QKeyEvent* );

  //! context menu event [overloaded]
  virtual void contextMenuEvent( QContextMenuEvent* );

  //! paint event
  virtual void paintEvent( QPaintEvent* );

  //! raise autospell context menu
  /*! returns true if autospell context menu is used */
  virtual bool _autoSpellContextEvent( QContextMenuEvent* );
  
  //@}
  
  //! actions
  void _installActions( void );
  
  //! recent files
  FileList& _recentFiles( void ) const;
  
  //! update document class
  /*! first parameter is file name, second tells if document is a new untitled document or not */
  void _updateDocumentClass( File, bool );
  
    //! set file name
  void _setFile( const File& file );
    
  //! is new document
  void _setIsNewDocument( bool value )
  { is_new_document_ = value; }
  
  //! clear macros
  void _clearMacros( void )
  { macros_.clear(); }

  //! macros
  void _setMacros( const TextMacro::List& macros)
  { macros_ = macros; }  
      
  //! last save time stamp
  void _setLastSaved( const TimeStamp& stamp )
  { last_saved_ = stamp; }
  
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
  
  //! update tagged block colors
  void _updateTaggedBlocks( void );
  
  #if WITH_ASPELL
  
  //! dictionary menu
  SPELLCHECK::DictionaryMenu& _dictionaryMenu( void )
  { return *dictionary_menu_; }

  //! filter menu
  SPELLCHECK::FilterMenu& _filterMenu( void )
  { return *filter_menu_; }
  
  #endif
    
  //! update margins
  virtual bool _updateMargin( void );
    
  protected slots:
  
  /*! returns true if changed */
  virtual bool _toggleWrapMode( bool );

  private slots:

  //! update configuration
  void _updateConfiguration( void );

  //! spellcheck configuration
  void _updateSpellCheckConfiguration( File file = File() );
      
  //! indent paragraph (when return or tab is pressed)
  void _indentCurrentParagraph( void );

  //! selection changed
  void _selectionChanged( void )
  { if( isActive() ) emit needUpdate( CUT|COPY ); }
  
  //! toggle text indentation
  void _toggleTextIndent( bool state );
    
  //! toggle text highlight
  void _toggleTextHighlight( bool state );
  
  //! toggle parenthesis
  void _toggleParenthesisHighlight( bool state );

  //! toggle block delimiters display
  void _toggleShowBlockDelimiters( bool state );
  
  //!@name spell check
  //@{
  
  //! autospell
  void _toggleAutoSpell( bool state );
  
  //! run spellcheck
  void _spellcheck( void );
  
  //@}

  //! indent selection
  void _indentSelection( void );

  //! add base indentation
  void _addBaseIndentation( void );
  
  //! replace all leading tabs in text when tab emulation is active
  void _replaceLeadingTabs( const bool& confirm = true );
  
  //! show file info
  void _fileProperties( void );
  
  //! track text modifications for syntax highlighting
  void _setBlockModified( int, int, int );
  
  //! update action status
  virtual void _updateSelectionActions( bool state )
  { 
    AnimatedTextEditor::_updateSelectionActions( state );
    emit needUpdate( CUT|COPY );
  }
  
  //! update paste action 
  /*! depends on clipboard status and editability */
  virtual void _updatePasteAction( void )
  {
    AnimatedTextEditor::_updatePasteAction();
    emit needUpdate( PASTE );
  }
  
  //! text changed
  virtual void _textModified( void );
  
  //! ignore current misspelled word
  /*! this method does nothing if not compiled against aspell */
  void _ignoreMisspelledWord( QString );
  
  //! replace current selection with spell-checked suggestion
  /*! this method does nothing if not compiled against aspell */
  void _replaceMisspelledSelection( QString );
  
  //! highlight parenthesis
  void _highlightParenthesis( void );
  
  //! tag current block
  void _tagBlock( void );
  
  //! find next tagged block, starting from current
  void _nextTag( void );
  
  //! find previous tagged block, starting from current
  void _previousTag( void );
  
  //! clear current block tags
  void _clearTag( void );

  //! returns true if file is on afs
  bool _fileIsAfs( void ) const;
  
  private:
  
  //! empty line
  static QRegExp& _emptyLineRegExp( void );

  //! file
  File file_;

  //! working directory
  File working_directory_;
  
  //!@name property ids
  //@{
  FileRecord::PropertyId::Id class_name_property_id_;
  FileRecord::PropertyId::Id icon_property_id_;
  FileRecord::PropertyId::Id wrap_property_id_;
  FileRecord::PropertyId::Id dictionary_property_id_;
  FileRecord::PropertyId::Id filter_property_id_;
  //@}
  
  //! true if display corresponds to a new document
  bool is_new_document_;
  
  //! associated document class name
  QString class_name_;
  
  //! filesystem check data
  FileCheck::Data file_check_data_;
  
  //! diff conflict color 
  QColor diff_conflict_color_;

  //! diff added color
  QColor diff_added_color_; 
 
  //! diff added color
  QColor user_tag_color_; 
       
  //! last save timeStamp
  TimeStamp last_saved_;

  //! if true, _checkFile is disabled
  bool ignore_warnings_;
  
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
  QAction* parenthesis_highlight_action_;
 
  //! toggle autospell
  QAction* autospell_action_;
    
  //! block delimiter
  QAction* show_block_delimiter_action_;

  //! run spell checker
  QAction* spellcheck_action_;
  
  //! indent selection
  QAction* indent_selection_action_;
  
  //! add base indentation
  QAction* base_indent_action_;
  
  //! replace leading tabs
  QAction* leading_tabs_action_;
  
  //! toggle text highlighting
  QAction* file_properties_action_;
  
  #if WITH_ASPELL

  //! spellcheck dictionary selection menu
  QAction* dictionary_menu_action_;
  
  //! spellcheck filter selection menu
  QAction* filter_menu_action_;
  
  #endif
  
  //! tag block
  QAction* tag_block_action_; 
  
  //! goto next tag
  QAction* next_tag_action_;
  
  //! goto previous tag
  QAction* previous_tag_action_;
  
  //! clear current block tags
  QAction* clear_tag_action_;
   
  //! clear current block tags
  QAction* clear_all_tags_action_;
 
  //@}
  
  #if WITH_ASPELL
  
  //! spellcheck dictionary selection menu
  SPELLCHECK::DictionaryMenu* dictionary_menu_;
  
  //! spellcheck filter selection menu
  SPELLCHECK::FilterMenu* filter_menu_;
  
  #endif
  
  //! syntax highlighter
  TextHighlight* text_highlight_;
        
  //! parenthesis highlight object
  ParenthesisHighlight* parenthesis_highlight_;
  
  //! block delimiter
  BlockDelimiterDisplay* block_delimiter_display_;
  
};

#endif
