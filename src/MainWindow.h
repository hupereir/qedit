// $Id$
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
#include <QSplitter>
#include <QTimer>

#include <list>
#include <string>

#include "CustomMainWindow.h"
#include "Config.h"
#include "Counter.h"
#include "FileRecord.h"
#include "Key.h"
#include "QtUtil.h"
#include "TextView.h"

class LineEditor;
class Menu;
class NavigationFrame;
class StatusBar;

//! editor main window
class MainWindow: public CustomMainWindow, public Counter, public BASE::Key
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! creator
  MainWindow( QWidget* parent = 0 );

  //! destructor
  ~MainWindow( void );

  //!@name file management
  //@{
  
  //! used to select editor with matching filename
  class SameFileFTor: public TextView::SameFileFTor
  {
    
    public:

    //! constructor
    SameFileFTor( const File& file ):
      TextView::SameFileFTor( file.expand() )
    {}

    //! predicate
    bool operator() ( const MainWindow* window ) const
    { return TextView::SameFileFTor::operator() (&window->activeView() ); }

  };

  //! used to select editor with empty, unmodified file
  class EmptyFileFTor: public TextView::EmptyFileFTor
  {
    public:

    //! predicate
    bool operator() ( const MainWindow* window ) const
    { return TextView::EmptyFileFTor::operator()( &window->activeView() ); }
    
  };

  //! used to select editor with empty, unmodified file
  class IsModifiedFTor: public TextView::IsModifiedFTor
  {
    public:

    //! predicate
    bool operator() ( const MainWindow* window ) const
    { return TextView::IsModifiedFTor::operator()( &window->activeView() ); }
    
  };
  
  //! returns true if there is at least one display modified in this window
  bool isModified( void ) const
  { return activeView().isModified(); }
  
  //! return number of independant displays
  unsigned int independentDisplayCount( void )
  { return activeView().independentDisplayCount(); }
  
  //! return number of independent modified displays
  unsigned int modifiedDisplayCount( void )
  { return activeView().modifiedDisplayCount(); }

  //@}

  //! navigation window
  NavigationFrame& navigationFrame( void ) const
  { 
    assert( navigation_frame_ );
    return *navigation_frame_;
  }
  
  //!@name active view/display managment
  //@{
  
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
  void setActiveView( TextView& view )
  { active_view_ = &view; }

  //! active display
  const TextDisplay& activeDisplay( void ) const
  { return activeView().activeDisplay(); }
  
  //! active display
  TextDisplay& activeDisplay( void )
  { return activeView().activeDisplay(); }

  //! select display from file
  bool selectDisplay( const File& file )
  { return activeView().selectDisplay( file ); }
  
  //@}
  
  //!@name configuration
  //@{
   
  //! open mode
  const TextView::OpenMode& openMode( void ) const
  { return default_open_mode_; }
  
  //! open mode
  void setOpenMode( const TextView::OpenMode& mode )
  { default_open_mode_ = mode; }
  
  //! orientation
  const Qt::Orientation& orientation( void ) const
  { return default_orientation_; }
  
  //! orientation
  void setOrientation( const Qt::Orientation orientation )
  { default_orientation_ = orientation; }
  
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
  QAction& fileInfoAction( void ) const
  { return *file_info_action_; }
  
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

  public slots:
 
  //! open file
  void open( FileRecord record = FileRecord() )
  { activeView().open( record, openMode(), orientation() ); }

  //! open file horizontally
  void openHorizontal( FileRecord record = FileRecord() )
  { activeView().open( record, TextView::NEW_VIEW, Qt::Horizontal ); }

  //! open file vertically
  void openVertical( FileRecord record = FileRecord() )
  { activeView().open( record, TextView::NEW_VIEW, Qt::Vertical ); }

  //! save all modified text displays
  void saveAll( void )
  { activeView().saveAll(); }

  //! select class name
  void selectClassName( QString value )
  { activeView().selectClassName( value ); }
  
  //! rehighlight all text displays
  void rehighlight( void )
  { activeView().rehighlight(); }

  //! uniconify
  void uniconify( void )
  { QtUtil::uniconify( this ); }
 
  protected:

  //! close event
  virtual void closeEvent( QCloseEvent* );

  //! enter event handler
  virtual void enterEvent( QEvent* );
      
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
  
  //! new file
  void _newFile( void )
  { activeView().newFile( openMode(), orientation() ); }
  
  //! new file
  void _newHorizontal( void )
  { activeView().newFile( TextView::NEW_VIEW, Qt::Horizontal ); }
  
  //! new file
  void _newVertical( void )
  { activeView().newFile( TextView::NEW_VIEW, Qt::Vertical ); }
  
  //! clone current file
  void _splitDisplay( void )
  { activeView().splitDisplay( orientation(), true ); }

  //! clone current file horizontal
  void _splitDisplayHorizontal( void )
  { activeView().splitDisplay( Qt::Horizontal, true ); }

  //! clone current file horizontal
  void _splitDisplayVertical( void )
  { activeView().splitDisplay( Qt::Vertical, true ); }

  //! detach current display
  void _detach( void );
  
  //! close
  /*! close window */
  void _closeWindow( void )
  { close(); }
  
  //! close 
  /*! close current display if more than two display are open, */
  void _closeDisplay( void )
  { if( !activeView().closeActiveDisplay() ) _closeWindow(); }
  
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
  { activeDisplay().fileInfoAction().trigger(); }

  //! spellcheck
  void _spellcheck( void )
  { activeDisplay().spellcheckAction().trigger(); }
  
  //! diff files
  void _diff( void )
  { activeView().diff(); }
  
  //@}
  
  //! update window title, cut, copy, paste buttons, and filename line editor
  /*! \param flags are bitwise or of TextDisplay::UpdateFlags */
  void _update( unsigned int );

  //! update overwrite mode
  void _updateOverwriteMode( void )
  { _update( TextDisplay::OVERWRITE_MODE ); }
  
  //! update actions based on number of displays in active view
  void _updateDisplayCount( void )
  { _update( TextDisplay::DISPLAY_COUNT ); }
  
  //! display cursor position in state window
  void _updateCursorPosition( void );

  private:

  //! install actions
  void _installActions( void );

  //! Update window title
  void _updateWindowTitle();

  //! create new TextView
  TextView& _newTextView( QWidget* );
  
  /*! it is used to print formatted text to both HTML and PDF */
  QString _htmlString( const int& );
    
  //!@name child widgets
  //@{

  //! menu
  Menu* menu_;

  //! main splitter
  QSplitter* splitter_;
  
  //! navigation window
  NavigationFrame* navigation_frame_;
  
  //! main display widget
  TextView* active_view_;
  
  //! state window
  StatusBar* statusbar_;

  //! file display lineEdit
  LineEditor* file_editor_;
   
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
  QAction* file_info_action_;
  
  //! spellcheck
  QAction* spellcheck_action_;
  
  //! diff files
  QAction* diff_action_;
  
  //! split display horizontal
  QAction* split_display_horizontal_action_;
  
  //! split display vertical
  QAction* split_display_vertical_action_;
  
  //! open horizontal
  QAction* open_horizontal_action_;

  //! open vertical
  QAction* open_vertical_action_;

  //@}
  
  //! default orientation for multiple displays
  Qt::Orientation default_orientation_;
  
  //! default open mode
  TextView::OpenMode default_open_mode_;
    
  //! timer
  QBasicTimer resize_timer_;
  
};

#endif
