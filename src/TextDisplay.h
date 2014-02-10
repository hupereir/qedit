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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "AnimatedTextEditor.h"
#include "AskForSaveDialog.h"
#include "BlockDelimiter.h"
#include "FileList.h"
#include "FileModifiedDialog.h"
#include "FileRemovedDialog.h"
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

#include <QRegExp>
#include <QTimer>
#include <QAction>

// forward declaration
class BlockDelimiterDisplay;
class BaseContextMenu;
class DocumentClass;
class HighlightBlockData;
class TextEncodingMenu;
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

    //! install actions in context menu
    virtual void installContextMenuActions( BaseContextMenu* menu, const bool& = true );

    //! draw margins
    virtual void paintMargin( QPainter& );

    //! update flags (to be passed to TextEditor to change button status)
    enum UpdateFlag
    {

        //! file name (in bottom status bar and navigation frame)
        FILE_NAME = 1<<0,

        //! document class
        DOCUMENT_CLASS = 1<<1,

        //! read only
        READ_ONLY = 1<<2,

        //! modified
        MODIFIED = 1<<3,

        //! cut availability
        CUT = 1<<4,

        //! copy availability
        COPY = 1<<5,

        //! paster availability
        PASTE = 1<<6,

        //! undo/redo availability
        UNDO_REDO = 1<<7,

        //! overwrite mode
        SPELLCHECK = 1<<8,

        //! keyword modifiers
        MODIFIERS = 1<<9,

        //! display count
        DISPLAY_COUNT = 1<<10,

        //! active file changed
        ACTIVE_DISPLAY_CHANGED = FILE_NAME|DOCUMENT_CLASS|READ_ONLY|CUT|COPY|PASTE|UNDO_REDO|SPELLCHECK|MODIFIERS,

        //! active file changed
        ACTIVE_VIEW_CHANGED = FILE_NAME|DOCUMENT_CLASS|READ_ONLY|CUT|COPY|PASTE|UNDO_REDO|SPELLCHECK|MODIFIERS|DISPLAY_COUNT,

        //! all the above
        ALL = FILE_NAME|MODIFIED|READ_ONLY|CUT|COPY|PASTE|UNDO_REDO|SPELLCHECK|MODIFIERS|DISPLAY_COUNT

    };


    Q_DECLARE_FLAGS( UpdateFlags, UpdateFlag )

    // true if widget is to be deleted
    bool isClosed( void ) const
    { return closed_; }

    // set to true if widget is to be deleted
    void setIsClosed( bool value )
    { closed_ = value; }

    //!@ name file management
    //@{

    //! file
    void setFile( File file, bool checkAutoSave = true );

    //! define as new document
    void setIsNewDocument( void );

    //! is new document
    const bool& isNewDocument( void ) const
    { return isNewDocument_; }

    //! new document name server
    static NewDocumentNameServer& newDocumentNameServer( void );

    //! file
    const File& file( void ) const
    { return file_; }

    //! working directory
    const File& workingDirectory() const
    { return workingDirectory_; }

    //! last saved time stamp
    const TimeStamp& lastSaved( void ) const
    { return lastSaved_; }

    //! clear file check data
    void clearFileCheckData( void );

    //! file check data
    void setFileCheckData( const FileCheck::Data& data );

    //! file check data
    const FileCheck::Data& fileCheckData( void ) const
    { return fileCheckData_; }

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
    { className_ = name; }

    //! class name
    const QString& className( void ) const
    { return className_; }

    //@}

    //! text encoding
    const QByteArray& textEncoding( void ) const
    { return textEncoding_; }

    //!@name macro
    //@{

    //! list of macros
    const TextMacro::List& macros( void ) const
    { return macros_; }

    //@}

    //! text highlight
    bool hasTextHighlight( void )
    { return textHighlight_; }

    //! text highlight
    TextHighlight& textHighlight( void )
    { return *textHighlight_; }

    //! text highlight
    const TextHighlight& textHighlight( void ) const
    { return *textHighlight_; }

    //! text indent
    TextIndent& textIndent( void ) const
    { return *indent_; }

    //! block delimiter display
    bool hasBlockDelimiterDisplay( void ) const
    { return blockDelimiterDisplay_; }

    //! block delimiter display
    BlockDelimiterDisplay& blockDelimiterDisplay( void ) const
    { return *blockDelimiterDisplay_; }

    //! returns true if current text has leading tabs of wrong type
    bool hasLeadingTabs( void ) const;

    //! convert to plain text
    /*!
    This method makes sure that full text is obtained even when some blocks are collapsed.
    this should be an overloaded function, but the base class method is not virtual
    however, it is never called via a pointer to the base class, so that it should be fine.
    */
    QString toPlainText( void ) const;

    //!@name actions
    //@{

    //! toggle indentation
    QAction& textIndentAction( void ) const
    { return *textIndentMacro_; }

    //! toggle text highlighting
    QAction& textHighlightAction( void ) const
    { return *textHighlightAction_; }

    //! toggle parenthesis highlighting
    QAction& parenthesisHighlightAction( void ) const
    { return *parenthesisHighlightAction_; }

    //! disable automatic macros
    QAction& noAutomaticMacrosAction( void ) const
    { return *noAutomaticMacrosAction_; }

    //! autospell action
    QAction& autoSpellAction( void ) const
    { return *autoSpellAction_; }

    //! show block delimiters
    bool hasBlockDelimiterAction( void ) const
    { return showBlockDelimiterAction_; }

    //! show block delimiters
    QAction& showBlockDelimiterAction( void ) const
    { return *showBlockDelimiterAction_; }

    //! spellcheck action
    QAction& spellcheckAction( void ) const
    { return *spellcheckAction_; }

    //! indent selection
    QAction& indentSelectionAction( void ) const
    { return *indentSelectionAction_; }

    //! indent selection
    QAction& baseIndentAction( void ) const
    { return *baseIndentAction_; }

    //! replace leading tab actions
    QAction& leadingTabsAction( void ) const
    { return *leadingTabsAction_; }

    //! file information
    QAction& filePropertiesAction( void ) const
    { return *filePropertiesAction_; }

    #if WITH_ASPELL

    //! spellcheck dictionary selection
    QAction& dictionaryMenuAction( void ) const
    { return *dictionaryMenuAction_; }

    //! spellcheck filter selection
    QAction& filterMenuAction( void ) const
    { return *filterMenuAction_; }

    #endif

    //! text encoding menu action
    QAction &textEncodingMenuAction( void ) const
    { return *textEncodingMenuAction_; }

    //! tag block action
    QAction &tagBlockAction( void ) const
    { return *tagBlockAction_; }

    //! next tag action
    QAction &nextTagAction( void ) const
    { return *nextTagAction_; }

    //! previous tag action
    QAction &previousTagAction( void ) const
    { return *previousTagAction_; }

    //! clear current block tags action
    QAction &clearTagAction( void ) const
    { return *clearTagAction_; }

    //! clear all tags action
    QAction &clearAllTagsAction( void ) const
    { return *clearAllTagsAction_; }

    //@}

    //!@name block interface
    //@{

    // return true if block is an empty line
    virtual bool isEmptyBlock( const QTextBlock& block ) const
    { return _emptyLineRegExp().indexIn( block.text() ) >= 0; }

    //! return true is block is to be ignored from indentation scheme
    virtual bool ignoreBlock( const QTextBlock& block ) const;

    //! tag block (with diff flag)
    void tagBlock( QTextBlock, const unsigned int& tag );

    //! clear block tags if match argument
    void clearTag( QTextBlock, const int& tags );

    //! true if current blocks (or selection) has tag
    bool isCurrentBlockTagged( void ) const;

    //! true if some blocks have tags
    bool hasTaggedBlocks( void ) const;

    //@}

    //! return parenthesis highlight object
    ParenthesisHighlight& parenthesisHighlight( void ) const
    { return *parenthesisHighlight_; }

    Q_SIGNALS:

    //! emmited when indentation several blocks is required
    void indent( QTextBlock, QTextBlock );

    //! emmited when indentation of one block is required
    void indent( QTextBlock, bool  );

    //! emmited whenever mainwindow toolbar, window title or file name editor needs update
    /* \param flags, bitwise or of UpdateFlags */
    void needUpdate( TextDisplay::UpdateFlags );

    public Q_SLOTS:

    //! set document class
    void updateDocumentClass( void )
    { _updateDocumentClass( file_, isNewDocument_ ); }

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

    //! select class name
    void selectClassName( QString );

    //! set focus, delayed
    void setFocusDelayed( void )
    { QTimer::singleShot( 0, this, SLOT( setFocus( void ) ) ); }

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

    //! recent files
    FileList& _recentFiles( void ) const;

    //! update document class
    /*! first parameter is file name, second tells if document is a new untitled document or not */
    void _updateDocumentClass( File, bool );

    //! set file name
    void _setFile( const File& file );

    //! is new document
    void _setIsNewDocument( bool value )
    { isNewDocument_ = value; }

    //! clear macros
    void _clearMacros( void )
    { macros_.clear(); }

    //! macros
    void _setMacros( const TextMacro::List& macros)
    { macros_ = macros; }

    //! last save time stamp
    void _setLastSaved( const TimeStamp& stamp )
    { lastSaved_ = stamp; }

    //! working directory
    void _setWorkingDirectory( const File& file )
    { workingDirectory_ = file; }

    //! if true file is not checked on enter event
    const bool& _ignoreWarnings() const
    { return ignoreWarnings_; }

    //! if true file is not checked on enter event
    void _setIgnoreWarnings( const bool& value )
    { ignoreWarnings_ = value; }

    //! true if macros list contains automatic macros
    bool _hasAutomaticMacros( void ) const;

    //! process macro
    void _processMacro( const TextMacro& );

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

    //! update margins
    virtual bool _updateMargin( void );

    protected Q_SLOTS:

    /*! returns true if changed */
    virtual bool _toggleWrapMode( bool );

    private Q_SLOTS:

    //! update configuration
    void _updateConfiguration( void );

    //! spellcheck configuration
    void _updateSpellCheckConfiguration( File file = File() );

    //! indent paragraph (when return or tab is pressed)
    void _indentCurrentParagraph( void );

    //! selection changed
    void _selectionChanged( void )
    { if( isActive() ) emit needUpdate( UpdateFlags(CUT|COPY) ); }

    //! toggle text indentation
    void _toggleTextIndent( bool state );

    //! toggle text highlight
    void _toggleTextHighlight( bool state );

    //! toggle parenthesis
    void _toggleParenthesisHighlight( bool state );

    //! toggle block delimiters display
    void _toggleShowBlockDelimiters( bool state );

    //! toggle automatic macros
    void _toggleIgnoreAutomaticMacros( bool state );

    //!@name spell check
    //@{

    //! autospell
    void _toggleAutoSpell( bool state );

    //! run spellcheck
    void _spellcheck( void );

    //@}

    //! set text encoding
    void _setTextEncoding( const QByteArray& );

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
        emit needUpdate( UpdateFlags( CUT|COPY ) );
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

    //! true if a block is collapsed
    bool _blockIsCollapsed( const QTextBlock& ) const;

    //! returns collapsed text in a given block, if any
    QString _collapsedText( const QTextBlock& ) const;

    //! returns true if file is on afs
    bool _fileIsAfs( void ) const;

    private:

    //! actions
    void _installActions( void );

    //! empty line
    static QRegExp& _emptyLineRegExp( void );

    //! file
    File file_;

    //! working directory
    File workingDirectory_;

    //!@name property ids
    //@{
    FileRecord::PropertyId::Id classNamePropertyId_;
    FileRecord::PropertyId::Id iconPropertyId_;
    FileRecord::PropertyId::Id wrapPropertyId_;
    FileRecord::PropertyId::Id dictionaryPropertyId_;
    FileRecord::PropertyId::Id filterPropertyId_;
    //@}

    //! text encoding (needed for conversions
    QByteArray textEncoding_;

    //! true if display is to be deleted
    bool closed_;

    //! true if display corresponds to a new document
    bool isNewDocument_;

    //! associated document class name
    QString className_;

    //! filesystem check data
    FileCheck::Data fileCheckData_;

    //! diff conflict color
    QColor diffConflictColor_;

    //! diff added color
    QColor diffAddedColor_;

    //! diff added color
    QColor userTagColor_;

    //! last save timeStamp
    TimeStamp lastSaved_;

    //! if true, _checkFile is disabled
    bool ignoreWarnings_;

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
    QAction* textIndentMacro_;

    //! toggle text highlighting
    QAction* textHighlightAction_;

    //! toggle text highlighting
    QAction* parenthesisHighlightAction_;

    //! disable automatic macros
    QAction* noAutomaticMacrosAction_;

    //! toggle autospell
    QAction* autoSpellAction_;

    //! block delimiter
    QAction* showBlockDelimiterAction_;

    //! run spell checker
    QAction* spellcheckAction_;

    //! indent selection
    QAction* indentSelectionAction_;

    //! add base indentation
    QAction* baseIndentAction_;

    //! replace leading tabs
    QAction* leadingTabsAction_;

    //! toggle text highlighting
    QAction* filePropertiesAction_;

    #if WITH_ASPELL

    //! spellcheck dictionary selection menu
    QAction* dictionaryMenuAction_;

    //! spellcheck filter selection menu
    QAction* filterMenuAction_;

    #endif

    //! text encoding menu action
    QAction* textEncodingMenuAction_;

    //! tag block
    QAction* tagBlockAction_;

    //! goto next tag
    QAction* nextTagAction_;

    //! goto previous tag
    QAction* previousTagAction_;

    //! clear current block tags
    QAction* clearTagAction_;

    //! clear current block tags
    QAction* clearAllTagsAction_;

    //@}

    #if WITH_ASPELL

    //! spellcheck dictionary selection menu
    SpellCheck::DictionaryMenu* dictionaryMenu_;

    //! spellcheck filter selection menu
    SpellCheck::FilterMenu* filterMenu_;

    #endif

    //! text encoding menu
    TextEncodingMenu* textEncodingMenu_;

    //! syntax highlighter
    TextHighlight* textHighlight_;

    //! parenthesis highlight object
    ParenthesisHighlight* parenthesisHighlight_;

    //! block delimiter
    BlockDelimiterDisplay* blockDelimiterDisplay_;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( TextDisplay::UpdateFlags )

#endif
