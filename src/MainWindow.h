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


#include <QAction>
#include <QBasicTimer>
#include <QCloseEvent>
#include <QLabel>
#include <QStackedWidget>
#include <QTimerEvent>

class BaseStatusBar;
class DocumentClassToolBar;
class ElidedLabel;
class MenuBar;
class SidePanelWidget;
class PrintHelper;
class ReplaceWidget;
class SelectLineWidget;

//* main window
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
    bool isModified() const
    { return IsModifiedFTor()(this); }

    //@}

    //* menu
    MenuBar& menuBar() const
    { return *menuBar_; }

    //* navigation window
    SidePanelWidget& sidePanelWidget() const
    { return *sidePanelWidget_; }

    //*@name active view/display managment
    //@{

    //* create new TextView
    TextView& newTextView( FileRecord record = FileRecord() );

    //* active view
    TextView& activeView()
    { return *activeView_; }

    //* active view
    const TextView& activeView() const
    { return *activeView_; }

    //* change active display manualy
    void setActiveView( TextView& view );


    //* active display
    const TextDisplay& activeDisplay() const
    { return activeView_->activeDisplay(); }

    //* active display
    bool isActiveDisplay( const TextDisplay& display ) const
    { return activeView_->isActiveDisplay( display ); }

    //* active display
    TextDisplay& activeDisplay()
    { return activeView_->activeDisplay(); }

    //* get set of all displays associated to this window
    /**
    this is a convenient function that loops over all associated views
    and merge their associated displays into a single list
    */
    Base::KeySet<TextDisplay> associatedDisplays() const;

    //* select display from file
    bool selectDisplay( const File& );

    //* save all modified text displays
    void saveAll();

    //* ignore all text display modifications
    void ignoreAll();

    //@}

    //*@name configuration
    //@{

    //@}

    //*@name actions
    //@{

    //* new file
    QAction& newFileAction() const
    { return *newFileAction_; }

    //* clone display
    QAction& cloneAction() const
    { return *cloneAction_; }

    //* detach action
    QAction& detachAction() const
    { return *detachAction_; }

    //* open file
    QAction& openAction() const
    { return *openAction_; }

    //* close display
    QAction& closeDisplayAction() const
    { return *closeDisplayAction_; }

    //* close display
    QAction& closeWindowAction() const
    { return *closeWindowAction_; }

    //* save
    QAction& saveAction() const
    { return *saveAction_; }

    //* save as
    QAction& saveAsAction() const
    { return *saveAsAction_; }

    //* revert to saved
    QAction& revertToSaveAction() const
    { return *revertToSaveAction_; }

    //* undo
    QAction& undoAction() const
    { return *undoAction_; }

    //* redo
    QAction& redoAction() const
    { return *redoAction_; }

    //* cut
    QAction& cutAction() const
    { return *cutAction_; }

    //* copy
    QAction& copyAction() const
    { return *copyAction_; }

    //* paste
    QAction& pasteAction() const
    { return *pasteAction_; }

    //* print
    QAction& printAction() const
    { return *printAction_; }

    //* print
    QAction& printPreviewAction() const
    { return *printPreviewAction_; }

    //* export
    QAction& htmlAction() const
    { return *htmlAction_; }

    //* file info
    QAction& filePropertiesAction() const
    { return *filePropertiesAction_; }

    //* spellcheck
    QAction& spellcheckAction() const
    { return *spellcheckAction_; }

    //* diff files action
    QAction& diffAction() const
    { return *diffAction_; }

    //* split display horizontal
    QAction& splitDisplayHorizontalAction() const
    { return *splitDisplayHorizontalAction_; }

    //* split display vertical
    QAction& splitDisplayVerticalAction() const
    { return *splitDisplayVerticalAction_; }

    //* open horizontal
    QAction& openHorizontalAction() const
    { return *openHorizontalAction_; }

    //* open vertical
    QAction& openVerticalAction() const
    { return *openVerticalAction_; }

    //@}

    Q_SIGNALS:

    //* emitted when window is activated
    void activated( MainWindow* );

    //* emitted when the document modification state of an editor is changed
    void modificationChanged();

    //*@name re-implemented from text editor
    //@{

    //* emitted from TextDisplay when no match is found for find/replace request
    void noMatchFound();

    //* emitted from TextDisplay when no match is found for find/replace request
    void matchFound();

    //* emitted when selected line is not found
    void lineNotFound();

    //* emitted when selected line is found
    void lineFound();

    //* busy
    void busy( int );

    //* progressAvailable
    void progressAvailable( int );

    //* idle
    void idle();

    //* scratch files
    void scratchFileCreated( const File& );

    //@}


    public Q_SLOTS:

    //* select class name
    void selectClassName( QString value )
    { activeView_->selectClassName( value ); }

    //* rehighlight all text displays
    void rehighlight()
    { activeView_->rehighlight(); }

    //*@name reimplemented from TextEditor
    //@{

    //* find text from dialog
    void findFromDialog();

    //* replace text from dialog
    void replaceFromDialog();

    //* select line from dialog
    void selectLineFromDialog();

    //@}

    protected:

    //* generic event
    bool event( QEvent* ) override;

    //* close event
    void closeEvent( QCloseEvent* ) override;

    //* timer event
    void timerEvent( QTimerEvent* ) override;

    private Q_SLOTS:

    //* update configuration
    void _updateConfiguration();

    //* toggle navigation frame visibility
    void _toggleSidePanelWidget( bool );

    //* splitter moved
    void _splitterMoved();

    //* active view changed
    /**
    this is triggered by StackedWidget::currentWidgetChanged
    */
    void _activeViewChanged();

    //* select display from file
    /**
    this is triggered by changing the selection in the sessionFilesWidget
    of the navigation window. This ensures that the selected display
    is always the active one, as long as it belongs to this window
    */
    void _selectDisplay( FileRecord record )
    { selectDisplay( record.file() ); }

    //* clone current file
    void _splitDisplay();

    //* clone current file horizontal
    void _splitDisplayHorizontal()
    { activeView_->splitDisplay( Qt::Horizontal, true ); }

    //* clone current file horizontal
    void _splitDisplayVertical()
    { activeView_->splitDisplay( Qt::Vertical, true ); }

    //* close
    /** close window */
    void _closeWindow()
    { close(); }

    //* close
    /** close current display if more than two display are open, */
    void _closeDisplay()
    { activeView_->closeActiveDisplay(); }

    //* save
    void _save()
    { activeDisplay().save(); }

    //* Save As
    void _saveAs()
    { activeDisplay().saveAs(); }

    //* Revert to save
    void _revertToSave();

    //* Print current document
    void _print();

    //* Print current document
    void _print( PrintHelper& );

    //* Print preview current document
    void _printPreview();

    //* export to html
    void _toHtml();

    //*@name forwarded slots
    //@{

    //* undo
    void _undo()
    { activeDisplay().undoAction().trigger(); }

    //* redo
    void _redo()
    { activeDisplay().redoAction().trigger(); }

    //* cut
    void _cut()
    { activeDisplay().cutAction().trigger(); }

    //* copy
    void _copy()
    { activeDisplay().copyAction().trigger(); }

    //* paste
    void _paste()
    { activeDisplay().pasteAction().trigger(); }

    //* file information
    void _fileInfo()
    { activeDisplay().filePropertiesAction().trigger(); }

    //* spellcheck
    void _spellcheck()
    { activeDisplay().spellcheckAction().trigger(); }

    //* diff files
    void _diff()
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
    void _multipleFileReplace();

    //* restore focus on active display, when closing embedded dialog
    void _restoreFocus()
    { activeDisplay().setFocus(); }

    //@}

    //* update window title, cut, copy, paste buttons, and filename line editor
    /** \param flags are bitwise or of TextDisplay::UpdateFlags */
    void _update( TextDisplay::UpdateFlags );

    //* update replace in selection action
    void _updateReplaceInSelection();

    //* update modifiers
    void _updateModifiers();

    //* update actions based on number of displays in active view
    void _updateDisplayCount()
    { _update( TextDisplay::DisplayCount ); }

    //* display cursor position in state window
    void _updateCursorPosition();

    private:

    //* install actions
    void _installActions();

    //* install toolbars
    void _installToolbars();

    //* create find dialog
    void _createFindWidget();

    //* create replace dialog
    void _createReplaceWidget();

    //* create select line widget
    void _createSelectLineWidget();

    //* make connection between this window and child text view
    void _connectView( TextView& view );

    //* Update window title
    void _updateWindowTitle();


    //*@name child widgets
    //@{

    //* menuBar
    MenuBar* menuBar_ = nullptr;

    //* right container
    QWidget* rightContainer_ = nullptr;

    //* stack widget
    QStackedWidget* stack_ = nullptr;

    //* navigation window
    SidePanelWidget* sidePanelWidget_ = nullptr;

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
