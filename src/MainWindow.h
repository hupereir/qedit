#ifndef MainWindow_h
#define MainWindow_h

// $Id$
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
****************************************************************************/

#include "BaseMainWindow.h"
#include "Config.h"
#include "Counter.h"
#include "FileRecord.h"
#include "Key.h"
#include "TextView.h"

#include <QtCore/QBasicTimer>
#include <QtCore/QTimerEvent>

#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>

#include <list>

class QLabel;
class DocumentClassToolBar;
class Menu;
class NavigationFrame;
class StatusBar;

class BaseFindDialog;
class ReplaceDialog;
class SelectLineDialog;
class TransitionWidget;

//! editor main window
class MainWindow: public BaseMainWindow, public Counter, public BASE::Key
{

    Q_OBJECT

    public:

    //!@name orientation strings
    static const QString LEFT_RIGHT;
    static const QString TOP_BOTTOM;

    //! constructor
    MainWindow( QWidget* = 0 );

    //! destructor
    ~MainWindow( void );

    //!@name file management
    //@{

    //! used to select editor with matching filename
    class SameFileFTor: public TextDisplay::SameFileFTor
    {

        public:

        //! constructor
        SameFileFTor( const File& file ):
            TextDisplay::SameFileFTor( file )
        {}

        //! predicate
        bool operator() ( const MainWindow* window ) const
        {
            BASE::KeySet<TextView> views( window );
            return std::find_if( views.begin(), views.end(), *this ) != views.end();
        }

        //! predicate
        bool operator() ( const TextView* view ) const
        {
            BASE::KeySet<TextDisplay> displays( view );
            return std::find_if( displays.begin(), displays.end(), (TextDisplay::SameFileFTor)*this ) != displays.end();
        }

    };

    //! used to select editor with empty, unmodified file
    class EmptyFileFTor: public TextDisplay::EmptyFileFTor
    {
        public:

        //! predicate
        bool operator() ( const MainWindow* window ) const
        {
            BASE::KeySet<TextView> views( window );
            return std::find_if( views.begin(), views.end(), *this ) != views.end();
        }

        //! predicate
        bool operator() ( const TextView* view ) const
        {
            BASE::KeySet<TextDisplay> displays( view );
            return std::find_if( displays.begin(), displays.end(), (TextDisplay::EmptyFileFTor)*this ) != displays.end();
        }

    };

    //! used to select editor with empty, unmodified file
    class IsModifiedFTor
    {
        public:

        //! predicate
        bool operator() ( const MainWindow* window ) const
        {
            BASE::KeySet<TextView> views( window );
            return std::find_if( views.begin(), views.end(), *this ) != views.end();
        }

        //! predicate
        bool operator() ( const TextView* view ) const
        {
            BASE::KeySet<TextDisplay> displays( view );
            return std::find_if( displays.begin(), displays.end(), *this ) != displays.end();
        }

        //! predicate
        bool operator() ( const TextDisplay* display ) const
        { return display->document()->isModified(); }

    };

    //! returns true if there is at least one display modified in this window
    bool isModified( void ) const
    { return IsModifiedFTor()(this); }

    //@}

    //! menu
    Menu& menu( void ) const
    {
        assert( menu_ );
        return *menu_;
    }

    //! navigation window
    NavigationFrame& navigationFrame( void ) const
    {
        assert( navigationFrame_ );
        return *navigationFrame_;
    }

    //!@name active view/display managment
    //@{

    //! create new TextView
    TextView& newTextView( FileRecord record = FileRecord() );

    //! active view
    TextView& activeView( void )
    {
        assert( activeView_ );
        return *activeView_;
    }

    //! active view
    const TextView& activeView( void ) const
    {
        assert( activeView_ );
        return *activeView_;
    }

    //! change active display manualy
    void setActiveView( TextView& view );


    //! active display
    const TextDisplay& activeDisplay( void ) const
    { return activeView().activeDisplay(); }

    //! active display
    bool isActiveDisplay( const TextDisplay& display ) const
    { return activeView().isActiveDisplay( display ); }

    //! active display
    TextDisplay& activeDisplay( void )
    { return activeView().activeDisplay(); }

    //! get set of all displays associated to this window
    /*!
    this is a convenient function that loops over all associated views
    and merge their associated displays into a single list
    */
    BASE::KeySet<TextDisplay> associatedDisplays( void ) const;

    //! select display from file
    bool selectDisplay( const File& );

    //! save all modified text displays
    void saveAll( void );

    //! ignore all text display modifications
    void ignoreAll( void );

    //@}

    //!@name configuration
    //@{

    //@}

    //!@name actions
    //@{

    //! new file
    QAction& newFileAction( void ) const
    { return *newFileAction_; }

    //! clone display
    QAction& cloneAction( void ) const
    { return *cloneAction_; }

    //! detach action
    QAction& detachAction( void ) const
    { return *detachAction_; }

    //! open file
    QAction& openAction( void ) const
    { return *openAction_; }

    //! close display
    QAction& closeDisplayAction( void ) const
    { return *closeDisplayAction_; }

    //! close display
    QAction& closeWindowAction( void ) const
    { return *closeWindowAction_; }

    //! save
    QAction& saveAction( void ) const
    { return *saveAction_; }

    //! save as
    QAction& saveAsAction( void ) const
    { return *saveAsAction_; }

    //! revert to saved
    QAction& revertToSaveAction( void ) const
    { return *revertToSaveAction_; }

    //! undo
    QAction& undoAction( void ) const
    { return *undoAction_; }

    //! redo
    QAction& redoAction( void ) const
    { return *redoAction_; }

    //! cut
    QAction& cutAction( void ) const
    { return *cutAction_; }

    //! copy
    QAction& copyAction( void ) const
    { return *copyAction_; }

    //! paste
    QAction& pasteAction( void ) const
    { return *pasteAction_; }

    //! print
    QAction& printAction( void ) const
    { return *printAction_; }

    //! print
    QAction& printPreviewAction( void ) const
    { return *printPreviewAction_; }

    //! file info
    QAction& filePropertiesAction( void ) const
    { return *filePropertiesAction_; }

    //! spellcheck
    QAction& spellcheckAction( void ) const
    { return *spellcheckAction_; }

    //! diff files action
    QAction& diffAction( void ) const
    { return *diffAction_; }

    //! split display horizontal
    QAction& splitDisplayHorizontalAction( void ) const
    { return *splitDisplayHorizontalAction_; }

    //! split display vertical
    QAction& splitDisplayVerticalAction( void ) const
    { return *splitDisplayVerticalAction_; }

    //! open horizontal
    QAction& openHorizontalAction( void ) const
    { return *openHorizontalAction_; }

    //! open vertical
    QAction& openVerticalAction( void ) const
    { return *openVerticalAction_; }

    //@}

    signals:

    //! emmited when window is activated
    void activated( MainWindow* );

    //! emmited when the document modification state of an editor is changed
    void modificationChanged( void );

    //!@name re-implemented from text editor
    //@{

    //! emmited from TextDisplay when no match is found for find/replace request
    void noMatchFound( void );

    //! emmited from TextDisplay when no match is found for find/replace request
    void matchFound( void );

    //! busy
    void busy( int );

    //! progressAvailable
    void progressAvailable( int );

    //! idle
    void idle( void );

    //@}


    public slots:

    //! select class name
    void selectClassName( QString value )
    { activeView().selectClassName( value ); }

    //! rehighlight all text displays
    void rehighlight( void )
    { activeView().rehighlight(); }

    //!@name reimplemented from TextEditor
    //@{

    //! find text from dialog
    virtual void findFromDialog( void );

    //! replace text from dialog
    virtual void replaceFromDialog( void );

    //! select line from dialog
    virtual void selectLineFromDialog( void );

    //@}

    protected:

    //! generic event
    virtual bool event( QEvent* );

    //! close event
    virtual void closeEvent( QCloseEvent* );

    //! timer event
    virtual void timerEvent( QTimerEvent* );

    private slots:

    //! update configuration
    void _updateConfiguration( void );

    //! update configuration
    void _saveConfiguration( void );

    //! toggle navigation frame visibility
    void _toggleNavigationFrame( bool );

    //! splitter moved
    void _splitterMoved( void );

    //! active view changed
    /*!
    this is triggered by StackedWidget::currentWidgetChanged
    */
    void _activeViewChanged( void );

    //! select display from file
    /*!
    this is triggered by changing the selection in the sessionFilesFrame
    of the navigation window. This ensures that the selected display
    is always the active one, as long as it belongs to this window
    */
    void _selectDisplay( FileRecord record )
    { selectDisplay( record.file() ); }

    //! clone current file
    void _splitDisplay( void );

    //! clone current file horizontal
    void _splitDisplayHorizontal( void )
    { activeView().splitDisplay( Qt::Horizontal, true ); }

    //! clone current file horizontal
    void _splitDisplayVertical( void )
    { activeView().splitDisplay( Qt::Vertical, true ); }

    //! close
    /*! close window */
    void _closeWindow( void )
    { close(); }

    //! close
    /*! close current display if more than two display are open, */
    void _closeDisplay( void )
    { activeView().closeActiveDisplay(); }

    //! save
    void _save( void )
    { activeDisplay().save(); }

    //! Save As
    void _saveAs( void )
    { activeDisplay().saveAs(); }

    //! Revert to save
    void _revertToSave( void );

    //! Print current document
    void _print( void );

    //! Print preview current document
    void _printPreview( void );

    //!@name forwarded slots
    //@{

    //! undo
    void _undo( void )
    { activeDisplay().undoAction().trigger(); }

    //! redo
    void _redo( void )
    { activeDisplay().redoAction().trigger(); }

    //! cut
    void _cut( void )
    { activeDisplay().cutAction().trigger(); }

    //! copy
    void _copy( void )
    { activeDisplay().copyAction().trigger(); }

    //! paste
    void _paste( void )
    { activeDisplay().pasteAction().trigger(); }

    //! file information
    void _fileInfo( void )
    { activeDisplay().filePropertiesAction().trigger(); }

    //! spellcheck
    void _spellcheck( void )
    { activeDisplay().spellcheckAction().trigger(); }

    //! diff files
    void _diff( void )
    { activeView().diff(); }

    //! find
    void _find( TextSelection selection )
    { activeDisplay().find( selection ); }

    //! find
    void _replace( TextSelection selection )
    { activeDisplay().replace( selection ); }

    //! find
    void _replaceInSelection( TextSelection selection )
    { activeDisplay().replaceInSelection( selection ); }

    //! find
    void _replaceInWindow( TextSelection selection )
    { activeDisplay().replaceInWindow( selection ); }

    //! select line
    void _selectLine( int value )
    { activeDisplay().selectLine( value ); }

    //! replace selection in multiple files
    void _multipleFileReplace( void );

    //@}

    //! update window title, cut, copy, paste buttons, and filename line editor
    /*! \param flags are bitwise or of TextDisplay::UpdateFlags */
    void _update( unsigned int );

    //! update modifiers
    void _updateModifiers( void );

    //! update actions based on number of displays in active view
    void _updateDisplayCount( void )
    { _update( TextDisplay::DISPLAY_COUNT ); }

    //! display cursor position in state window
    void _updateCursorPosition( void );

    //! replace transition widget
    /*! this is needed when transition widget gets deleted via its parent Display, during animation */
    void _replaceTransitionWidget( void );

    //! animations
    void _animationFinished( void );

    private:

    //! install actions
    void _installActions( void );

    //! install toolbars
    void _installToolbars( void );

    //! create find dialog
    void _createBaseFindDialog( void );

    //! create replace dialog
    void _createReplaceDialog( void );

    //! find dialog
    virtual BaseFindDialog& _findDialog( void )
    {
        assert( findDialog_ );
        return *findDialog_;
    }

    //! replace dialog
    virtual ReplaceDialog& _replaceDialog( void )
    {
        assert( replaceDialog_ );
        return *replaceDialog_;
    }

    //! replace dialog
    bool _hasReplaceDialog( void ) const
    { return bool( replaceDialog_ ); }

    //! navigation frame
    bool _hasNavigationFrame( void ) const
    { return bool( navigationFrame_ ); }

    //! status bar
    bool _hasStatusBar( void ) const
    { return bool( statusbar_ ); }

    StatusBar& _statusBar( void ) const
    { return *statusbar_; }

    //! document class toolbar
    bool _hasDocumentClassToolBar( void ) const
    { return bool( documentClassToolbar_ ); }

    //! document class toolbar
    DocumentClassToolBar& _documentClassToolBar( void ) const
    { return *documentClassToolbar_; }

    //! file editor
    bool _hasFileEditor( void ) const
    { return bool( fileEditor_ ); }

    //! file editor
    QLabel& _fileEditor( void ) const
    { return *fileEditor_; }

    //! make connection between this window and child text view
    void _connectView( TextView& view );

    //! Update window title
    void _updateWindowTitle();

    //! stack windget
    QStackedWidget& _stack( void ) const
    { return *stack_; }

    //! transition widget
    TransitionWidget& _transitionWidget( void ) const
    { return *transitionWidget_; }

    //!@name child widgets
    //@{

    //! menu
    Menu* menu_;

    //! stack widget
    QStackedWidget* stack_;

    //! transition widget
    TransitionWidget* transitionWidget_;

    //! navigation window
    NavigationFrame* navigationFrame_;

    //! main display widget
    TextView* activeView_;

    //! state window
    StatusBar* statusbar_;

    //! file display lineEdit
    QLabel* fileEditor_;

    //! document class toolbar
    DocumentClassToolBar* documentClassToolbar_;

    //@}

    //!@name dialogs (re-implemented from TextEditor)
    //@{

    //! find dialog
    BaseFindDialog* findDialog_;

    //! find dialog
    ReplaceDialog* replaceDialog_;

    //! line number dialog
    SelectLineDialog* selectLineDialog_;

    //@}

    //!@name actions
    //@{

    //! new file
    QAction* newFileAction_;

    //! clone display
    QAction* cloneAction_;

    //! detach action
    QAction* detachAction_;

    //! open file
    QAction* openAction_;

    //! open horizontal
    QAction* openHorizontalAction_;

    //! open vertical
    QAction* openVerticalAction_;

    //! close display
    QAction* closeDisplayAction_;

    //! close display
    QAction* closeWindowAction_;

    //! save
    QAction* saveAction_;

    //! save as
    QAction* saveAsAction_;

    //! revert to saved
    QAction* revertToSaveAction_;

    //! print
    QAction* printAction_;

    //! print preview
    QAction* printPreviewAction_;

    //! undo
    QAction* undoAction_;

    //! redo
    QAction* redoAction_;

    //! cut
    QAction* cutAction_;

    //! copy
    QAction* copyAction_;

    //! paste
    QAction* pasteAction_;

    //! file info
    QAction* filePropertiesAction_;

    //! spellcheck
    QAction* spellcheckAction_;

    //! diff files
    QAction* diffAction_;

    //! split display horizontal
    QAction* splitDisplayHorizontalAction_;

    //! split display vertical
    QAction* splitDisplayVerticalAction_;

    //@}

    //! timer
    QBasicTimer resizeTimer_;

};

#endif
