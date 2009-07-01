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

/*!
   \file MainWindow.h
   \brief editor main window
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QAction>
#include <QBasicTimer>
#include <QCloseEvent>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimerEvent>
#include <list>


#include "BaseMainWindow.h"
#include "Config.h"
#include "Counter.h"
#include "FileRecord.h"
#include "Key.h"
#include "TextView.h"

class AnimatedLineEditor;
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

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //!@name orientation strings
  static const QString LEFT_RIGHT;
  static const QString TOP_BOTTOM;
    
  //! creator
  MainWindow( QWidget* parent = 0 );

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
    assert( navigation_frame_ );
    return *navigation_frame_;
  }
  
  //!@name active view/display managment
  //@{

  //! create new TextView
  TextView& newTextView( FileRecord record = FileRecord() );

  //! active view
  TextView& activeView( void )
  {
    assert( active_view_ );
    return *active_view_; 
  }
    
  //! active view
  const TextView& activeView( void ) const
  {
    assert( active_view_ );
    return *active_view_; 
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
  { return *new_file_action_; }
   
  //! clone display
  QAction& cloneAction( void ) const
  { return *clone_action_; } 
  
  //! detach action
  QAction& detachAction( void ) const
  { return *detach_action_; } 
  
  //! open file
  QAction& openAction( void ) const
  { return *open_action_; }

  //! close display
  QAction& closeDisplayAction( void ) const
  { return *close_display_action_; }

  //! close display
  QAction& closeWindowAction( void ) const
  { return *close_window_action_; }

  //! save
  QAction& saveAction( void ) const
  { return *save_action_; }

  //! save as
  QAction& saveAsAction( void ) const
  { return *save_as_action_; }
  
  //! revert to saved
  QAction& revertToSaveAction( void ) const
  { return *revert_to_save_action_; }
  
  //! undo 
  QAction& undoAction( void ) const
  { return *undo_action_; }
  
  //! redo
  QAction& redoAction( void ) const
  { return *redo_action_; }
  
  //! cut
  QAction& cutAction( void ) const
  { return *cut_action_; }
  
  //! copy
  QAction& copyAction( void ) const
  { return *copy_action_; }
  
  //! paste
  QAction& pasteAction( void ) const
  { return *paste_action_; }
  
  //! print
  QAction& printAction( void ) const
  { return *print_action_; }
  
  //! file info
  QAction& filePropertiesAction( void ) const
  { return *file_properties_action_; }
  
  //! spellcheck 
  QAction& spellcheckAction( void ) const
  { return *spellcheck_action_; }
  
  //! diff files action 
  QAction& diffAction( void ) const
  { return *diff_action_; }
  
  //! split display horizontal
  QAction& splitDisplayHorizontalAction( void ) const
  { return *split_display_horizontal_action_; }
  
  //! split display vertical
  QAction& splitDisplayVerticalAction( void ) const
  { return *split_display_vertical_action_; }
  
  //! open horizontal
  QAction& openHorizontalAction( void ) const
  { return *open_horizontal_action_; }

  //! open vertical
  QAction& openVerticalAction( void ) const
  { return *open_vertical_action_; }
  
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
  void _updateModifiers( void )
  { _update( TextDisplay::MODIFIERS ); }
  
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
    assert( find_dialog_ );
    return *find_dialog_;
  }

  //! replace dialog
  virtual ReplaceDialog& _replaceDialog( void )
  {
    assert( replace_dialog_ );
    return *replace_dialog_;
  }
   
  //! replace dialog
  bool _hasReplaceDialog( void ) const
  { return bool( replace_dialog_ ); }
    
  //! navigation frame
  bool _hasNavigationFrame( void ) const
  { return bool( navigation_frame_ ); }
  
  //! status bar
  bool _hasStatusBar( void ) const
  { return bool( statusbar_ ); }
  
  StatusBar& _statusBar( void ) const
  { return *statusbar_; }
  
  //! document class toolbar
  bool _hasDocumentClassToolBar( void ) const
  { return bool( document_class_toolbar_ ); }
  
  //! document class toolbar
  DocumentClassToolBar& _documentClassToolBar( void ) const
  { return *document_class_toolbar_; }
  
  //! file editor
  bool _hasFileEditor( void ) const
  { return bool( file_editor_ ); }
  
  //! file editor
  AnimatedLineEditor& _fileEditor( void ) const
  { return *file_editor_; }
  
  //! make connection between this window and child text view
  void _connectView( TextView& view );
  
  //! Update window title
  void _updateWindowTitle();
  
  /*! it is used to print formatted text to both HTML and PDF */
  QString _htmlString( const int& );
  
  //! stack windget
  QStackedWidget& _stack( void ) const
  { return *stack_; }
    
  //! transition widget
  TransitionWidget& _transitionWidget( void ) const
  { return *transition_widget_; }
  
  //!@name child widgets
  //@{

  //! menu
  Menu* menu_;
  
  //! stack widget
  QStackedWidget* stack_;
  
  //! transition widget
  TransitionWidget* transition_widget_;
  
  //! navigation window
  NavigationFrame* navigation_frame_;
  
  //! main display widget
  TextView* active_view_;
  
  //! state window
  StatusBar* statusbar_;

  //! file display lineEdit
  AnimatedLineEditor* file_editor_;
     
  //! document class toolbar
  DocumentClassToolBar* document_class_toolbar_; 
  
  //@}
    
  //!@name dialogs (re-implemented from TextEditor)
  //@{
  
  //! find dialog
  BaseFindDialog* find_dialog_;

  //! find dialog
  ReplaceDialog* replace_dialog_;
  
  //! line number dialog
  SelectLineDialog* select_line_dialog_;
  
  //@}
  
  //!@name actions
  //@{
  
  //! new file
  QAction* new_file_action_;
   
  //! clone display
  QAction* clone_action_; 
  
  //! detach action
  QAction* detach_action_; 
  
  //! open file
  QAction* open_action_;

  //! open horizontal
  QAction* open_horizontal_action_;

  //! open vertical
  QAction* open_vertical_action_;

  //! close display
  QAction* close_display_action_;

  //! close display
  QAction* close_window_action_;

  //! save
  QAction* save_action_;

  //! save as
  QAction* save_as_action_;
  
  //! revert to saved
  QAction* revert_to_save_action_;
 
  //! print
  QAction* print_action_;
   
  //! undo 
  QAction* undo_action_;
  
  //! redo
  QAction* redo_action_;
  
  //! cut
  QAction* cut_action_;
  
  //! copy
  QAction* copy_action_;
  
  //! paste
  QAction* paste_action_;
 
  //! file info
  QAction* file_properties_action_;
  
  //! spellcheck
  QAction* spellcheck_action_;
  
  //! diff files
  QAction* diff_action_;
  
  //! split display horizontal
  QAction* split_display_horizontal_action_;
  
  //! split display vertical
  QAction* split_display_vertical_action_;
  
  //@}
  
  //! timer
  QBasicTimer resize_timer_;
  
};

#endif
