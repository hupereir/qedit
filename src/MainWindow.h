#ifndef MainWindow_h
#define MainWindow_h
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

#include "BaseMainWindow.h"
#include "Counter.h"
#include "FileRecord.h"
#include "Key.h"
#include "TextView.h"

#include <QBasicTimer>
#include <QTimerEvent>

#include <QAction>
#include <QCloseEvent>
#include <QLabel>
#include <QSplitter>
#include <QStackedWidget>

class BaseStatusBar;
class DocumentClassToolBar;
class ElidedLabel;
class Menu;
class NavigationFrame;
class PrintHelper;
class ReplaceWidget;
class SelectLineWidget;

//* editor main window
class MainWindow: public BaseMainWindow, private Base::Counter<MainWindow>, public Base::Key
{

    Q_OBJECT

    public:

    //* constructor
    explicit MainWindow( QWidget* = nullptr );

    //*@name file management
    //@{

    //* used to select editor with matching filename
    class SameFileFTor: public TextDisplay::SameFileFTor
    {

        public:

        //* constructor
        explicit SameFileFTor( const File& file ):
            TextDisplay::SameFileFTor( file )
        {}

        //* predicate
        bool operator() ( const MainWindow* window ) const
        {
            Base::KeySet<TextView> views( window );
            return std::find_if( views.begin(), views.end(), *this ) != views.end();
        }

        //* predicate
        bool operator() ( const TextView* view ) const
        {
            Base::KeySet<TextDisplay> displays( view );
            return std::find_if( displays.begin(), displays.end(), (TextDisplay::SameFileFTor)*this ) != displays.end();
        }

    };

    //* used to select editor with empty, unmodified file
    class EmptyFileFTor: public TextDisplay::EmptyFileFTor
    {
        public:

        //* predicate
        bool operator() ( const MainWindow* window ) const
        {
            Base::KeySet<TextView> views( window );
            return std::find_if( views.begin(), views.end(), *this ) != views.end();
        }

        //* predicate
        bool operator() ( const TextView* view ) const
        {
            Base::KeySet<TextDisplay> displays( view );
            return std::find_if( displays.begin(), displays.end(), (TextDisplay::EmptyFileFTor)*this ) != displays.end();
        }

    };

    //* used to select editor with empty, unmodified file
    class IsModifiedFTor
    {
        public:

        //* predicate
        bool operator() ( const MainWindow* window ) const
        {
            Base::KeySet<TextView> views( window );
            return std::find_if( views.begin(), views.end(), *this ) != views.end();
        }

        //* predicate
        bool operator() ( const TextView* view ) const
        {
            Base::KeySet<TextDisplay> displays( view );
            return std::find_if( displays.begin(), displays.end(), *this ) != displays.end();
        }

        //* predicate
        bool operator() ( const TextDisplay* display ) const
        { return display->document()->isModified(); }

    };

    //* returns true if there is at least one display modified in this window
    bool isModified( void ) const
    { return IsModifiedFTor()(this); }

    //@}

    //* menu
    Menu& menu( void ) const
    { return *menu_; }

    //* navigation window
    NavigationFrame& navigationFrame( void ) const
    { return *navigationFrame_; }

    //*@name active view/display managment
    //@{

    //* create new TextView
    TextView& newTextView( FileRecord record = FileRecord() );

    //* active view
    TextView& activeView( void )
    { return *activeView_; }

    //* active view
    const TextView& activeView( void ) const
    { return *activeView_; }

    //* change active display manualy
    void setActiveView( TextView& view );


    //* active display
    const TextDisplay& activeDisplay( void ) const
    { return activeView_->activeDisplay(); }

    //* active display
    bool isActiveDisplay( const TextDisplay& display ) const
    { return activeView_->isActiveDisplay( display ); }

    //* active display
    TextDisplay& activeDisplay( void )
    { return activeView_->activeDisplay(); }

    //* get set of all displays associated to this window
    /**
    this is a convenient function that loops over all associated views
    and merge their associated displays into a single list
    */
    Base::KeySet<TextDisplay> associatedDisplays( void ) const;

    //* select display from file
    bool selectDisplay( const File& );

    //* save all modified text displays
    void saveAll( void );

    //* ignore all text display modifications
    void ignoreAll( void );

    //@}

    //*@name configuration
    //@{

    //@}

    //*@name actions
    //@{

    //* new file
    QAction& newFileAction( void ) const
    { return *newFileAction_; }

    //* clone display
    QAction& cloneAction( void ) const
    { return *cloneAction_; }

    //* detach action
    QAction& detachAction( void ) const
    { return *detachAction_; }

    //* open file
    QAction& openAction( void ) const
    { return *openAction_; }

    //* close display
    QAction& closeDisplayAction( void ) const
    { return *closeDisplayAction_; }

    //* close display
    QAction& closeWindowAction( void ) const
    { return *closeWindowAction_; }

    //* save
    QAction& saveAction( void ) const
    { return *saveAction_; }

    //* save as
    QAction& saveAsAction( void ) const
    { return *saveAsAction_; }

    //* revert to saved
    QAction& revertToSaveAction( void ) const
    { return *revertToSaveAction_; }

    //* undo
    QAction& undoAction( void ) const
    { return *undoAction_; }

    //* redo
    QAction& redoAction( void ) const
    { return *redoAction_; }

    //* cut
    QAction& cutAction( void ) const
    { return *cutAction_; }

    //* copy
    QAction& copyAction( void ) const
    { return *copyAction_; }

    //* paste
    QAction& pasteAction( void ) const
    { return *pasteAction_; }

    //* print
    QAction& printAction( void ) const
    { return *printAction_; }

    //* print
    QAction& printPreviewAction( void ) const
    { return *printPreviewAction_; }

    //* export
    QAction& htmlAction( void ) const
    { return *htmlAction_; }

    //* file info
    QAction& filePropertiesAction( void ) const
    { return *filePropertiesAction_; }

    //* spellcheck
    QAction& spellcheckAction( void ) const
    { return *spellcheckAction_; }

    //* diff files action
    QAction& diffAction( void ) const
    { return *diffAction_; }

    //* split display horizontal
    QAction& splitDisplayHorizontalAction( void ) const
    { return *splitDisplayHorizontalAction_; }

    //* split display vertical
    QAction& splitDisplayVerticalAction( void ) const
    { return *splitDisplayVerticalAction_; }

    //* open horizontal
    QAction& openHorizontalAction( void ) const
    { return *openHorizontalAction_; }

    //* open vertical
    QAction& openVerticalAction( void ) const
    { return *openVerticalAction_; }

    //@}

    Q_SIGNALS:

    //* emitted when window is activated
    void activated( MainWindow* );

    //* emitted when the document modification state of an editor is changed
    void modificationChanged( void );

    //*@name re-implemented from text editor
    //@{

    //* emitted from TextDisplay when no match is found for find/replace request
    void noMatchFound( void );

    //* emitted from TextDisplay when no match is found for find/replace request
    void matchFound( void );

    //* emitted when selected line is not found
    void lineNotFound( void );

    //* emitted when selected line is found
    void lineFound( void );

    //* busy
    void busy( int );

    //* progressAvailable
    void progressAvailable( int );

    //* idle
    void idle( void );

    //* scratch files
    void scratchFileCreated( const File& );

    //@}


    public Q_SLOTS:

    //* select class name
    void selectClassName( QString value )
    { activeView_->selectClassName( value ); }

    //* rehighlight all text displays
    void rehighlight( void )
    { activeView_->rehighlight(); }

    //*@name reimplemented from TextEditor
    //@{

    //* find text from dialog
    virtual void findFromDialog( void );

    //* replace text from dialog
    virtual void replaceFromDialog( void );

    //* select line from dialog
    virtual void selectLineFromDialog( void );

    //@}

    protected:

    //* generic event
    virtual bool event( QEvent* );

    //* close event
    virtual void closeEvent( QCloseEvent* );

    //* timer event
    virtual void timerEvent( QTimerEvent* );

    private Q_SLOTS:

    //* update configuration
    void _updateConfiguration( void );

    //* update configuration
    void _saveConfiguration( void );

    //* toggle navigation frame visibility
    void _toggleNavigationFrame( bool );

    //* splitter moved
    void _splitterMoved( void );

    //* active view changed
    /**
    this is triggered by StackedWidget::currentWidgetChanged
    */
    void _activeViewChanged( void );

    //* select display from file
    /**
    this is triggered by changing the selection in the sessionFilesFrame
    of the navigation window. This ensures that the selected display
    is always the active one, as long as it belongs to this window
    */
    void _selectDisplay( FileRecord record )
    { selectDisplay( record.file() ); }

    //* clone current file
    void _splitDisplay( void );

    //* clone current file horizontal
    void _splitDisplayHorizontal( void )
    { activeView_->splitDisplay( Qt::Horizontal, true ); }

    //* clone current file horizontal
    void _splitDisplayVertical( void )
    { activeView_->splitDisplay( Qt::Vertical, true ); }

    //* close
    /** close window */
    void _closeWindow( void )
    { close(); }

    //* close
    /** close current display if more than two display are open, */
    void _closeDisplay( void )
    { activeView_->closeActiveDisplay(); }

    //* save
    void _save( void )
    { activeDisplay().save(); }

    //* Save As
    void _saveAs( void )
    { activeDisplay().saveAs(); }

    //* Revert to save
    void _revertToSave( void );

    //* Print current document
    void _print( void );

    //* Print current document
    void _print( PrintHelper& );

    //* Print preview current document
    void _printPreview( void );

    //* export to html
    void _toHtml( void );

    //*@name forwarded slots
    //@{

    //* undo
    void _undo( void )
    { activeDisplay().undoAction().trigger(); }

    //* redo
    void _redo( void )
    { activeDisplay().redoAction().trigger(); }

    //* cut
    void _cut( void )
    { activeDisplay().cutAction().trigger(); }

    //* copy
    void _copy( void )
    { activeDisplay().copyAction().trigger(); }

    //* paste
    void _paste( void )
    { activeDisplay().pasteAction().trigger(); }

    //* file information
    void _fileInfo( void )
    { activeDisplay().filePropertiesAction().trigger(); }

    //* spellcheck
    void _spellcheck( void )
    { activeDisplay().spellcheckAction().trigger(); }

    //* diff files
    void _diff( void )
    { activeView_->diff(); }

    //* find
    void _find( TextSelection selection )
    { activeDisplay().find( selection ); }

    //* find
    void _replace( TextSelection selection )
    { activeDisplay().replace( selection ); }

    //* find
    void _replaceInSelection( TextSelection selection )
    { activeDisplay().replaceInSelection( selection ); }

    //* find
    void _replaceInWindow( TextSelection selection )
    { activeDisplay().replaceInWindow( selection ); }

    //* select line
    void _selectLine( int value )
    { activeDisplay().selectLine( value ); }

    //* replace selection in multiple files
    void _multipleFileReplace( void );

    //* restore focus on active display, when closing embedded dialog
    void _restoreFocus( void )
    { activeDisplay().setFocus(); }

    //@}

    //* update window title, cut, copy, paste buttons, and filename line editor
    /** \param flags are bitwise or of TextDisplay::UpdateFlags */
    void _update( TextDisplay::UpdateFlags );

    //* update replace in selection action
    void _updateReplaceInSelection( void );

    //* update modifiers
    void _updateModifiers( void );

    //* update actions based on number of displays in active view
    void _updateDisplayCount( void )
    { _update( TextDisplay::DisplayCount ); }

    //* display cursor position in state window
    void _updateCursorPosition( void );

    private:

    //* install actions
    void _installActions( void );

    //* install toolbars
    void _installToolbars( void );

    //* create find dialog
    void _createFindWidget( void );

    //* create replace dialog
    void _createReplaceWidget( void );

    //* create select line widget
    void _createSelectLineWidget( void );

    //* make connection between this window and child text view
    void _connectView( TextView& view );

    //* Update window title
    void _updateWindowTitle();


    //*@name child widgets
    //@{

    //* menu
    Menu* menu_ = nullptr;

    //* right container
    QWidget* rightContainer_ = nullptr;

    //* stack widget
    QStackedWidget* stack_ = nullptr;

    //* navigation window
    NavigationFrame* navigationFrame_ = nullptr;

    //* main display widget
    TextView* activeView_ = nullptr;

    //* state window
    BaseStatusBar* statusbar_ = nullptr;

    //* file display lineEdit
    ElidedLabel* fileEditor_ = nullptr;

    //* document class toolbar
    DocumentClassToolBar* documentClassToolBar_ = nullptr;

    //@}

    //*@name widgets (re-implemented from TextEditor)
    //@{

    //* find widget
    BaseFindWidget* findWidget_ = nullptr;

    //* replace widget
    ReplaceWidget* replaceWidget_ = nullptr;

    //* line number dialog
    SelectLineWidget* selectLineWidget_ = nullptr;

    //@}

    //*@name actions
    //@{

    //* new file
    QAction* newFileAction_ = nullptr;

    //* clone display
    QAction* cloneAction_ = nullptr;

    //* detach action
    QAction* detachAction_ = nullptr;

    //* open file
    QAction* openAction_ = nullptr;

    //* open horizontal
    QAction* openHorizontalAction_ = nullptr;

    //* open vertical
    QAction* openVerticalAction_ = nullptr;

    //* close display
    QAction* closeDisplayAction_ = nullptr;

    //* close display
    QAction* closeWindowAction_ = nullptr;

    //* save
    QAction* saveAction_ = nullptr;

    //* save as
    QAction* saveAsAction_ = nullptr;

    //* revert to saved
    QAction* revertToSaveAction_ = nullptr;

    //* print
    QAction* printAction_ = nullptr;

    //* print preview
    QAction* printPreviewAction_ = nullptr;

    //* html action
    QAction* htmlAction_ = nullptr;

    //* undo
    QAction* undoAction_ = nullptr;

    //* redo
    QAction* redoAction_ = nullptr;

    //* cut
    QAction* cutAction_ = nullptr;

    //* copy
    QAction* copyAction_ = nullptr;

    //* paste
    QAction* pasteAction_ = nullptr;

    //* file info
    QAction* filePropertiesAction_ = nullptr;

    //* spellcheck
    QAction* spellcheckAction_ = nullptr;

    //* diff files
    QAction* diffAction_ = nullptr;

    //* split display horizontal
    QAction* splitDisplayHorizontalAction_ = nullptr;

    //* split display vertical
    QAction* splitDisplayVerticalAction_ = nullptr;

    //@}

    //* timer
    QBasicTimer resizeTimer_;

};

#endif
