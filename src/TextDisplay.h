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

#include "AskForSaveDialog.h"
#include "BlockDelimiter.h"
#include "FileList.h"
#include "Debug.h"
#include "FileCheck.h"
#include "FileCheckData.h"
#include "FileRecord.h"
#include "FileModifiedWidget.h"
#include "FileRemovedWidget.h"
#include "Functors.h"
#include "HighlightBlockFlags.h"
#include "HighlightPattern.h"
#include "NewDocumentNameServer.h"
#include "ParenthesisHighlight.h"
#include "TextEditor.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "TimeStamp.h"

#if WITH_ASPELL
#include "DictionaryMenu.h"
#include "FilterMenu.h"
#endif

#include <QRegularExpression>
#include <QTimer>
#include <QAction>

// forward declaration
class BlockDelimiterDisplay;
class BaseContextMenu;
class DocumentClass;
class HighlightBlockData;
class TextEncodingMenu;
class TextHighlight;

//* text display window
class TextDisplay: public TextEditor
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit TextDisplay( QWidget* parent );

    //* destructor
    ~TextDisplay() override;

    //*@name accessors
    //@{

    //* number of block associated to argument
    /** reimplemented from base class to account for collapsed blocks */
    int blockCount( const QTextBlock& ) const override;

    // true if widget is to be deleted
    bool isClosed() const
    { return closed_; }

    //* is new document
    bool isNewDocument() const
    { return isNewDocument_; }

    //* use compression
    bool useCompression() const
    { return useCompression_; }

    //* file
    const File& file() const
    { return file_; }

    //* working directory
    const File& workingDirectory() const
    { return workingDirectory_; }

    //* last saved time stamp
    const TimeStamp& lastSaved() const
    { return lastSaved_; }

    //* file check data
    const FileCheckData& fileCheckData() const
    { return fileCheckData_; }

    //* class name
    const QString& className() const
    { return className_; }

    const QByteArray& textEncoding() const
    { return textEncoding_; }

    //* list of macros
    const TextMacro::List& macros() const
    { return macros_; }

    //* text highlight
    bool hasTextHighlight() const
    { return textHighlight_; }

    //* text highlight
    const TextHighlight& textHighlight() const
    { return *textHighlight_; }

    //* block delimiter display
    bool hasBlockDelimiterDisplay() const
    { return blockDelimiterDisplay_; }

    //* returns true if current text has leading tabs of wrong type
    bool hasLeadingTabs() const;

    //* convert to plain text
    QString toPlainText() const;

    // return true if block is an empty line
    bool isEmptyBlock( const QTextBlock& block ) const override;

    //* return true is block is to be ignored from indentation scheme
    bool ignoreBlock( const QTextBlock& block ) const override;

    //* true if current blocks (or selection) has tag
    bool isCurrentBlockTagged() const;

    //* true if some blocks have tags
    bool hasTaggedBlocks() const;

    //@}

    //*@name modifiers
    //@{

    //* clone display configuration and document
    using TextEditor::synchronize;
    void synchronize( TextDisplay* );

    //* check if current entry has been modified or not
    void setModified( bool value = true );

    //* read-only
    void setReadOnly( bool ) override;

    //* install actions in context menu
    void installContextMenuActions( BaseContextMenu* menu, bool = true ) override;

    //* draw margins
    void paintMargin( QPainter& ) override;

    //* update flags (to be passed to TextEditor to change button status)
    enum UpdateFlag
    {

        //* file name (in bottom status bar and navigation frame)
        FileName = 1<<0,

        //* document class
        DocumentClassFlag = 1<<1,

        //* read only
        ReadOnly = 1<<2,

        //* modified
        Modified = 1<<3,

        //* cut availability
        Cut = 1<<4,

        //* copy availability
        Copy = 1<<5,

        //* paster availability
        Paste = 1<<6,

        //* undo/redo availability
        UndoRedo = 1<<7,

        //* overwrite mode
        SpellCheck = 1<<8,

        //* keyword modifiers
        Modifiers = 1<<9,

        //* display count
        DisplayCount = 1<<10,

        //* active file changed
        ActiveDisplayChanged = FileName|DocumentClassFlag|ReadOnly|Cut|Copy|Paste|UndoRedo|SpellCheck|Modifiers,

        //* active file changed
        ActiveViewChanged = FileName|DocumentClassFlag|ReadOnly|Cut|Copy|Paste|UndoRedo|SpellCheck|Modifiers|DisplayCount,

        //* all the above
        All = FileName|Modified|ReadOnly|Cut|Copy|Paste|UndoRedo|SpellCheck|Modifiers|DisplayCount

    };

    Q_DECLARE_FLAGS( UpdateFlags, UpdateFlag )

    // set to true if widget is to be deleted
    void setIsClosed( bool value )
    { closed_ = value; }

    //* file
    void setFile( File file, bool checkAutoSave = true );

    //* define as new document
    void setIsNewDocument();

    //* new document name server
    static NewDocumentNameServer& newDocumentNameServer();

    //* file check data
    void setFileCheckData( const FileCheckData &);

    //* clear file check data
    void clearFileCheckData();

    //* ask for save if modified
    AskForSaveDialog::ReturnCode askForSave( bool enableAll = false );

    //* check if file has been removed externally
    void checkFileRemoved();

    //* check if file has been modified externally
    void checkFileModified();

    //* check if file read-only state has changed
    void checkFileReadOnly();

    //* Save file
    void save();

    //* Save file with new name
    void saveAs();

    //* Revert to save
    void revertToSave();

    //* class name
    void setClassName( const QString &name )
    { className_ = name; }

    //* clear class name
    void clearClassName()
    { className_.clear(); }

    //* text highlight
    TextHighlight& textHighlight()
    { return *textHighlight_; }

    //* text indent
    TextIndent& textIndent() const
    { return *textIndent_; }

    //* block delimiter display
    BlockDelimiterDisplay& blockDelimiterDisplay() const
    { return *blockDelimiterDisplay_; }

    //* return parenthesis highlight object
    ParenthesisHighlight& parenthesisHighlight() const
    { return *parenthesisHighlight_; }

    //* tag block (with diff flag)
    void tagBlock( QTextBlock, int tag );

    //* clear block tags if match argument
    void clearTag( const QTextBlock&, int tags );

    //* close file removed dialogs
    void hideFileRemovedWidgets();

    //* close file modified dialogs
    void hideFileModifiedWidgets();

    //* close file modified dialogs
    void hideFileReadOnlyWidgets();

    //@}

    //*@name actions
    //@{

    //* toggle indentation
    QAction& textIndentAction() const
    { return *textIndentAction_; }

    //* toggle text highlighting
    QAction& textHighlightAction() const
    { return *textHighlightAction_; }

    //* toggle parenthesis highlighting
    QAction& parenthesisHighlightAction() const
    { return *parenthesisHighlightAction_; }

    //* disable automatic macros
    QAction& noAutomaticMacrosAction() const
    { return *noAutomaticMacrosAction_; }

    //* autospell action
    QAction& autoSpellAction() const
    { return *autoSpellAction_; }

    //* show block delimiters
    bool hasBlockDelimiterAction() const
    { return showBlockDelimiterAction_; }

    //* show block delimiters
    QAction& showBlockDelimiterAction() const
    { return *showBlockDelimiterAction_; }

    //* spellcheck action
    QAction& spellcheckAction() const
    { return *spellcheckAction_; }

    //* indent selection
    QAction& indentSelectionAction() const
    { return *indentSelectionAction_; }

    //* indent selection
    QAction& baseIndentAction() const
    { return *baseIndentAction_; }

    //* replace leading tab actions
    QAction& leadingTabsAction() const
    { return *leadingTabsAction_; }

    //* file information
    QAction& filePropertiesAction() const
    { return *filePropertiesAction_; }

    #if WITH_ASPELL

    //* spellcheck dictionary selection
    QAction& dictionaryMenuAction() const
    { return *dictionaryMenuAction_; }

    //* spellcheck filter selection
    QAction& filterMenuAction() const
    { return *filterMenuAction_; }

    #endif

    //* text encoding action
    QAction &textEncodingAction() const
    { return *textEncodingAction_; }

    //* text encoding menu action
    QAction &textEncodingMenuAction() const
    { return *textEncodingMenuAction_; }

    //* tag block action
    QAction &tagBlockAction() const
    { return *tagBlockAction_; }

    //* next tag action
    QAction &nextTagAction() const
    { return *nextTagAction_; }

    //* previous tag action
    QAction &previousTagAction() const
    { return *previousTagAction_; }

    //* clear current block tags action
    QAction &clearTagAction() const
    { return *clearTagAction_; }

    //* clear all tags action
    QAction &clearAllTagsAction() const
    { return *clearAllTagsAction_; }

    //@}


    //* used to select editor with matching filename
    using SameFileFTor = Base::Functor::Unary<TextDisplay, const File&, &TextDisplay::file>;

    //* used to select editor with empty, unmodified file
    class EmptyFileFTor
    {
        public:

        //* destructor
        virtual ~EmptyFileFTor() = default;

        //* predicate
        bool operator() ( const TextDisplay* display ) const
        { return display->file().isEmpty(); }

    };

    //* set document class
    void updateDocumentClass()
    { _updateDocumentClass( file_, isNewDocument_ ); }

    //* process macro by name
    void processMacro( const QString &);

    //* rehighlight
    void rehighlight();

    //* clear all blocks
    void clearAllTags()
    { clearAllTags( TextBlock::All ); }

    //* clear all blocks if match argument
    void clearAllTags( int );

    //* change autospell filter
    void selectFilter( const QString& );

    //* change autospell dictionary
    void selectDictionary( const QString& );

    //* select class name
    void selectClassName( const QString &);

    //* set focus, delayed
    void setFocusDelayed();

    Q_SIGNALS:

    //* emitted when indentation several blocks is required
    void indent( QTextBlock, QTextBlock );

    //* emitted when indentation of one block is required
    void indent( QTextBlock, bool  );

    //* emitted whenever mainwindow toolbar, window title or file name editor needs update
    /* \param flags, bitwise or of UpdateFlags */
    void needUpdate( TextDisplay::UpdateFlags );

    //* request display close
    void requestClose( File );

    protected:

    //*@name event handlers
    //@{

    //* generic event
    bool event( QEvent* ) override;

    //* keypress event [overloaded]
    void keyPressEvent( QKeyEvent* ) override;

    //* context menu event [overloaded]
    void contextMenuEvent( QContextMenuEvent* ) override;

    //* paint event
    void paintEvent( QPaintEvent* ) override;

    //* change event
    void changeEvent( QEvent* ) override;

    //* raise autospell context menu
    /** returns true if autospell context menu is used */
    bool _autoSpellContextEvent( QContextMenuEvent* );

    //@}

    /** returns true if changed */
    bool _toggleWrapMode( bool ) override;

    private:

    //* update configuration
    void _updateConfiguration();

    //* spellcheck configuration
    void _updateSpellCheckConfiguration()
    { _updateSpellCheckConfiguration( File() ); }

    void _updateSpellCheckConfiguration( File );

    //* indent paragraph (when return or tab is pressed)
    void _indentCurrentParagraph();

    //* selection changed
    void _selectionChanged()
    { if( isActive() ) emit needUpdate( UpdateFlags(Cut|Copy) ); }

    //* toggle text indentation
    void _toggleTextIndent( bool state );

    //* toggle text highlight
    void _toggleTextHighlight( bool state );

    //* toggle parenthesis
    void _toggleParenthesisHighlight( bool state );

    //* toggle block delimiters display
    void _toggleShowBlockDelimiters( bool state );

    //* toggle automatic macros
    void _toggleIgnoreAutomaticMacros( bool state );

    //* autospell
    void _toggleAutoSpell( bool state );

    //* run spellcheck
    void _spellcheck();

    //* change text encoding
    void _textEncoding();

    //* set text encoding
    void _setTextEncoding( const QByteArray& );

    //* indent selection
    void _indentSelection();

    //* add base indentation
    void _addBaseIndentation();

    //* replace all leading tabs in text when tab emulation is active
    void _replaceLeadingTabs( bool confirm = true );

    //* show file info
    void _fileProperties();

    //* track text modifications for syntax highlighting
    void _setBlockModified( int, int, int );

    //* update action status
    void _updateSelectionActions( bool state ) override
    {
        TextEditor::_updateSelectionActions( state );
        emit needUpdate( UpdateFlags( Cut|Copy ) );
    }

    //* update paste action
    /** depends on clipboard status and editability */
    void _updatePasteAction() override
    {
        TextEditor::_updatePasteAction();
        emit needUpdate( Paste );
    }

    //* text changed
    void _textModified();

    //* ignore current misspelled word
    /** this method does nothing if not compiled against aspell */
    void _ignoreMisspelledWord( const QString &);

    //* replace current selection with spell-checked suggestion
    /** this method does nothing if not compiled against aspell */
    void _replaceMisspelledSelection( const QString &);

    //* highlight parenthesis
    void _highlightParenthesis();

    //* tag current block
    void _tagBlock();

    //* find next tagged block, starting from current
    void _nextTag();

    //* find previous tagged block, starting from current
    void _previousTag();

    //* clear current block tags
    void _clearTag();

    //* process file removed action
    void _processFileRemovedAction( FileRemovedWidget::ReturnCode );

    //* process file modified action
    void _processFileModifiedAction( FileModifiedWidget::ReturnCode );

    //* true if a block is collapsed
    bool _blockIsCollapsed( const QTextBlock& ) const;

    //* returns collapsed text in a given block, if any
    QString _collapsedText( const QTextBlock& ) const;

    //* returns true if file is on afs
    bool _fileIsAfs() const;

    //* recent files
    FileList& _recentFiles() const;

    //* update document class
    /** first parameter is file name, second tells if document is a new untitled document or not */
    void _updateDocumentClass( const File&, bool );

    //* set file name
    void _setFile( const File& file );

    //* is new document
    void _setIsNewDocument( bool value )
    { isNewDocument_ = value; }

    //* set use compression
    void _setUseCompression( bool value )
    { useCompression_ = value; }

    //* clear macros
    void _clearMacros()
    { macros_.clear(); }

    //* macros
    void _setMacros( const TextMacro::List& macros)
    { macros_ = macros; }

    //* last save time stamp
    void _setLastSaved( const TimeStamp& stamp )
    { lastSaved_ = stamp; }

    //* working directory
    void _setWorkingDirectory( const File& file )
    { workingDirectory_ = file; }

    //* if true file is not checked on enter event
    bool _ignoreWarnings() const
    { return ignoreWarnings_; }

    //* if true file is not checked on enter event
    void _setIgnoreWarnings( bool value )
    { ignoreWarnings_ = value; }

    //* true if macros list contains automatic macros
    bool _hasAutomaticMacros() const;

    //* process macro
    void _processMacro( const TextMacro& );

    //* returns true if text contents differs from file contents
    bool _contentsChanged() const;

    //* returns true if file was removed
    bool _fileRemoved() const;

    //* returns true if file was modified by external application
    bool _fileModified();

    //* track text modifications for syntax highlighting
    void _setBlockModified( const QTextBlock& );

    //* update tagged block colors
    void _updateTaggedBlocks();

    //* update margins
    bool _updateMargin() override;

    //* actions
    void _installActions();

    //* file
    File file_;

    //* working directory
    File workingDirectory_;

    //* compression
    bool useCompression_ = false;

    //*@name property ids
    //@{
    FileRecord::PropertyId::Id classNamePropertyId_;
    FileRecord::PropertyId::Id iconPropertyId_;
    FileRecord::PropertyId::Id wrapPropertyId_;
    FileRecord::PropertyId::Id dictionaryPropertyId_;
    FileRecord::PropertyId::Id filterPropertyId_;
    //@}

    //* text encoding (needed for conversions
    QByteArray textEncoding_ = "UTF-8";

    //* true if display is to be deleted
    bool closed_ = false;

    //* true if display corresponds to a new document
    bool isNewDocument_ = false;

    //* associated document class name
    QString className_;

    //* filesystem check data
    FileCheckData fileCheckData_;

    //* diff conflict color
    QColor diffConflictColor_;

    //* diff added color
    QColor diffAddedColor_;

    //* diff added color
    QColor userTagColor_;

    //* last save timeStamp
    TimeStamp lastSaved_;

    //* if true, _checkFile is disabled
    bool ignoreWarnings_ = false;

    //*@name document classes specific members
    //@{

    //* text indent
    TextIndent* textIndent_ = nullptr;

    //* text macro
    TextMacro::List macros_;

    //@}

    //*@name actions
    //@{

    //* toggle indentation
    QAction* textIndentAction_ = nullptr;

    //* toggle text highlighting
    QAction* textHighlightAction_ = nullptr;

    //* toggle text highlighting
    QAction* parenthesisHighlightAction_ = nullptr;

    //* disable automatic macros
    QAction* noAutomaticMacrosAction_ = nullptr;

    //* toggle autospell
    QAction* autoSpellAction_ = nullptr;

    //* block delimiter
    QAction* showBlockDelimiterAction_ = nullptr;

    //* run spell checker
    QAction* spellcheckAction_ = nullptr;

    //* indent selection
    QAction* indentSelectionAction_ = nullptr;

    //* add base indentation
    QAction* baseIndentAction_ = nullptr;

    //* replace leading tabs
    QAction* leadingTabsAction_ = nullptr;

    //* toggle text highlighting
    QAction* filePropertiesAction_ = nullptr;

    #if WITH_ASPELL

    //* spellcheck dictionary selection menu
    QAction* dictionaryMenuAction_;

    //* spellcheck filter selection menu
    QAction* filterMenuAction_;

    #endif

    //* text encoding action
    QAction* textEncodingAction_ = nullptr;

    //* text encoding menu action
    QAction* textEncodingMenuAction_ = nullptr;

    //* tag block
    QAction* tagBlockAction_ = nullptr;

    //* goto next tag
    QAction* nextTagAction_ = nullptr;

    //* goto previous tag
    QAction* previousTagAction_ = nullptr;

    //* clear current block tags
    QAction* clearTagAction_ = nullptr;

    //* clear current block tags
    QAction* clearAllTagsAction_ = nullptr;

    //@}

    #if WITH_ASPELL

    //* spellcheck dictionary selection menu
    SpellCheck::DictionaryMenu* dictionaryMenu_ = nullptr;

    //* spellcheck filter selection menu
    SpellCheck::FilterMenu* filterMenu_ = nullptr;

    #endif

    //* text encoding menu
    TextEncodingMenu* textEncodingMenu_ = nullptr;

    //* syntax highlighter
    TextHighlight* textHighlight_ = nullptr;

    //* parenthesis highlight object
    ParenthesisHighlight* parenthesisHighlight_ = nullptr;

    //* block delimiter
    BlockDelimiterDisplay* blockDelimiterDisplay_ = nullptr;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( TextDisplay::UpdateFlags )

#endif
