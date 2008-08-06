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
#include <QBasicTimer>
#include <QResizeEvent>
#include <QSplitter>
#include <QTimer>
#include <QTimerEvent>

#include <list>
#include <string>

#include "CustomMainWindow.h"
#include "Config.h"
#include "Counter.h"
#include "FileRecord.h"
#include "Key.h"
#include "QtUtil.h"
#include "TextDisplay.h"
#include "TimeStamp.h"


class LineEditor;
class Menu;
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
  class SameFileFTor
  {
    
    public:

    //! constructor
    SameFileFTor( const File& file ):
      file_( file.expand() )
    {}

    //! predicate
    bool operator() ( const MainWindow* frame ) const
    { 
      BASE::KeySet<TextDisplay> displays( frame );
      return std::find_if( displays.begin(), displays.end(), TextDisplay::SameFileFTor( file_ ) ) != displays.end();
    }

    private:

    //! predicted file
    const File file_;

  };

  //! used to select editor with empty, unmodified file
  class EmptyFileFTor
  {
    public:

    //! predicate
    bool operator() ( const MainWindow* frame ) const
    { 
      BASE::KeySet<TextDisplay> displays( frame );
      return std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) != displays.end();
    }
    
  };

  //! used to select editor with empty, unmodified file
  class IsModifiedFTor
  {
    public:

    //! predicate
    bool operator() ( const MainWindow* frame ) const
    { return frame->isModified(); }
    
  };
  
  //! set file and read
  void setFile( File file );

  //! returns true if there is at least one display modified in this window
  bool isModified( void ) const
  {
    BASE::KeySet<TextDisplay> displays( this );
    return std::find_if( displays.begin(), displays.end(), TextDisplay::ModifiedFTor() ) != displays.end();
  }
  
  //! return number of independant displays
  unsigned int independentDisplayCount( void )
  { 
    unsigned int out( 0 );
    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { 
      // increment if no associated display is found in the already processed displays
      if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) == iter ) out++;
    }
    
    return out;
  }
  
  //! return number of independent modified displays
  unsigned int modifiedDisplayCount( void )
  {
    
    unsigned int out( 0 );
    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { 
      // increment if no associated display is found in the already processed displays
      // and if current is modified
      if( 
        std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) == iter && 
        (*iter)->document()->isModified() )
      { out++; }
   }
    
    return out;
    
  }   
  //@}

  //!@name display management
  //@{
  
  //! retrieve active display
  TextDisplay& activeDisplay( void )
  { return *active_display_; }
  
  //! retrieve active display
  const TextDisplay& activeDisplay( void ) const
  { return *active_display_; }

  //! select display from file
  void selectDisplay( const File& file )
  {
    
    BASE::KeySet<TextDisplay> displays( this );
    BASE::KeySet<TextDisplay>::iterator iter( std::find_if(
      displays.begin(),
      displays.end(),
      TextDisplay::SameFileFTor( file ) ) );
    if( iter == displays.end() ) return;
    
    // change active display
    setActiveDisplay( **iter );
    (*iter)->setFocus();
    
    return;
    
  }
  
  //@}
  
  //!@name configuration
  //@{
 
  //! default open mode
  enum OpenMode
  {
    //! new window
    NEW_WINDOW,
    
    //! new display
    NEW_VIEW
  };
  
  //! open mode
  const OpenMode& openMode( void ) const
  { return default_open_mode_; }
  
  //! open mode
  void setOpenMode( const OpenMode& mode )
  { default_open_mode_ = mode; }
  
  //! orientation
  const Qt::Orientation& orientation( void ) const
  { return default_orientation_; }
  
  //! orientation
  void setOrientation( const Qt::Orientation orientation )
  { default_orientation_ = orientation; }
  
  //@}
  
  //! change active display manualy
  void setActiveDisplay( TextDisplay& );
 
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
  { _open( record, openMode(), orientation() ); }

  //! open file horizontally
  void openHorizontal( FileRecord record = FileRecord() )
  { _open( record, NEW_VIEW, Qt::Horizontal ); }

  //! open file vertically
  void openVertical( FileRecord record = FileRecord() )
  { _open( record, NEW_VIEW, Qt::Vertical ); }

  //! save all modified text displays
  void saveAll( void );

  //! select class name
  void selectClassName( QString );
  
  //! rehighlight all text displays
  void rehighlight( void );

  //! uniconify
  void uniconify( void )
  { QtUtil::uniconify( this ); }
 
  protected:

  //! close event
  virtual void closeEvent( QCloseEvent* );

  //! enter event handler
  void enterEvent( QEvent* );
    
  //! resize event
  void resizeEvent( QResizeEvent* );
  
  //! timer event
  void timerEvent( QTimerEvent* );
  
  private slots:
  
  //! update configuration
  void _updateConfiguration( void );
  
  //! update configuration
  void _saveConfiguration( void );

  //! new file
  void _newFile( void )
  { _newFile( openMode(), orientation() ); }
  
  //! new file
  void _newHorizontal( void )
  { _newFile( NEW_VIEW, Qt::Horizontal ); }
  
  //! new file
  void _newVertical( void )
  { _newFile( NEW_VIEW, Qt::Vertical ); }
  
  //! clone current file
  void _splitDisplay( void )
  { _splitDisplay( orientation(), true ); }

  //! clone current file horizontal
  void _splitDisplayHorizontal( void )
  { _splitDisplay( Qt::Horizontal, true ); }

  //! clone current file horizontal
  void _splitDisplayVertical( void )
  { _splitDisplay( Qt::Vertical, true ); }

  //! detach current display
  void _detach( void );
  
  //! close
  /*! close window */
  void _closeWindow( void )
  { 
    Debug::Throw( "MainWindow::_closeWindow.\n" );
    close();
  }
  
  //! close 
  /*! close current display if more than two display are open, */
  void _closeDisplay( void )
  { 
    Debug::Throw( "MainWindow::_closeDisplay (SLOT)\n" );
    BASE::KeySet< TextDisplay > displays( this );
    if( displays.size() > 1 ) _closeDisplay( activeDisplay() );
    else _closeWindow();
  }
  
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
  
  //! undo
  void _undo( void )
  { 
    Debug::Throw( "MainWindow::_undo.\n" );
    activeDisplay().undoAction().trigger(); 
  }

  //! redo
  void _redo( void )
  { 
    Debug::Throw( "MainWindow::_redo.\n" );
    activeDisplay().redoAction().trigger(); 
  }

  //! cut
  void _cut( void )
  { 
    Debug::Throw( "MainWindow::_cut.\n" );
    activeDisplay().cutAction().trigger(); 
  }

  //! copy
  void _copy( void )
  { 
    Debug::Throw( "MainWindow::_copy.\n" );
    activeDisplay().copyAction().trigger(); 
  }

  //! paste
  void _paste( void )
  { 
    Debug::Throw( "MainWindow::_paste.\n" );
    activeDisplay().pasteAction().trigger(); 
  }
  
  //! file information
  void _fileInfo( void )
  { activeDisplay().fileInfoAction().trigger(); }

  //! spellcheck
  void _spellcheck( void )
  { activeDisplay().spellcheckAction().trigger(); }
  
  //! diff files
  void _diff( void );
  
  //! update window title, cut, copy, paste buttons, and filename line editor
  /*! \param flags are bitwise or of TextDisplay::UpdateFlags */
  void _update( unsigned int );

  //! update overwrite mode
  void _updateOverwriteMode( void )
  { _update( TextDisplay::OVERWRITE_MODE ); }
  
  //! display cursor position in state frame
  void _updateCursorPosition( void );

  //! display focus changed
  void _displayFocusChanged( TextEditor* );
  
  private:

  //! install actions
  void _installActions( void );

  //! Update window title
  void _updateWindowTitle();

  //! new file
  void _newFile( const OpenMode&, const Qt::Orientation& );

  //! open file
  void _open( FileRecord,  const OpenMode&, const Qt::Orientation& );
  
  //! close display
  /*! Ask for save if display is modified */
  void _closeDisplay( TextDisplay& );

  //! split display
  TextDisplay& _splitDisplay( const Qt::Orientation&, const bool& clone );
  
  //! create new splitter
  QSplitter& _newSplitter( const Qt::Orientation&, const bool& clone  );
  
  //! create new TextDisplay
  TextDisplay& _newTextDisplay( QWidget* );

  /*! it is used to print formatted text to both HTML and PDF */
  QString _htmlString( const int& );
  
  //! local QSplitter object, derived from Counter
  /*! helps keeping track of how many splitters are created/deleted */
  class LocalSplitter: public QSplitter, public Counter
  {
    
    public:
    
    //! constructor
    LocalSplitter( QWidget* parent ):
      QSplitter( parent ),
      Counter( "LocalSplitter" )
    { Debug::Throw( "LocalSplitter::LocalSplitter.\n" ); }

    //! destructor
    virtual ~LocalSplitter( void )
    { Debug::Throw( "LocalSplitter::~LocalSplitter.\n" ); }
    
  };
  
  //!@name child widgets
  //@{

  //! menu
  Menu* menu_;

  //! main display widget
  QWidget* main_;
  
  //! text display with focus
  TextDisplay* active_display_;
      
  //! state frame
  StatusBar* statusbar_;

  //! file display lineEdit
  LineEditor* file_editor_;
   
  //@}
    
  //! last save timeStamp
  TimeStamp last_save_;
  
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
  OpenMode default_open_mode_;
  
  //! resize timer
  QBasicTimer resize_timer_;

  //! position update timer
  QTimer position_timer_;
  
};

#endif