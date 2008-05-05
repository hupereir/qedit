// $Id$
#ifndef EditFrame_h
#define EditFrame_h

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
   \file EditFrame.h
   \brief editor main window
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QAction>
#include <QSplitter>
#include <list>
#include <string>

#include "CustomMainWindow.h"
#include "Config.h"
#include "Counter.h"
#include "FileRecord.h"
#include "Key.h"
#include "QtUtil.h"
#include "TextView.h"
#include "TimeStamp.h"


class LineEditor;
class Menu;
class StatusBar;

//! editor main window
class EditFrame: public CustomMainWindow, public Counter, public BASE::Key
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! creator
  EditFrame( QWidget* parent = 0 );

  //! destructor
  ~EditFrame( void );

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
    bool operator() ( const EditFrame* frame ) const
    { 
      BASE::KeySet<TextView> views( frame );
      return std::find_if( views.begin(), views.end(), TextView::SameFileFTor( file_ ) ) != views.end();
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
    bool operator() ( const EditFrame* frame ) const
    { 
      BASE::KeySet<TextView> views( frame );
      return std::find_if( views.begin(), views.end(), TextView::EmptyFileFTor() ) != views.end();
    }
    
  };

  //! used to select editor with empty, unmodified file
  class IsModifiedFTor
  {
    public:

    //! predicate
    bool operator() ( const EditFrame* frame ) const
    { return frame->isModified(); }
    
  };
  
  //! set file and read
  void setFile( File file );

  //! returns true if there is at least one display modified in this window
  bool isModified( void ) const
  {
    BASE::KeySet<TextView> views( this );
    return std::find_if( views.begin(), views.end(), TextView::ModifiedFTor() ) != views.end();
  }
  
  //! return number of independant displays
  unsigned int independentDisplayCount( void )
  { 
    unsigned int out( 0 );
    BASE::KeySet<TextView> views( this );
    BASE::KeySet<TextDisplay> displays;
    for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
    { 
      // increment if no associated display is found in the already processed displays
      if( std::find_if( displays.begin(), displays.end(), BASE::Key::IsAssociatedFTor( &(*iter)->editor() ) ) == displays.end() ) out++;
      displays.insert( &(*iter)->editor() );
    }
    
    return out;
  }
  
  //! return number of independent modified displays
  unsigned int modifiedDisplayCount( void )
  {
    
    unsigned int out( 0 );
    BASE::KeySet<TextView> views( this );
    BASE::KeySet<TextDisplay> displays;
    for( BASE::KeySet<TextView>::iterator iter = views.begin(); iter != views.end(); iter++ )
    { 
      // increment if no associated display is found in the already processed displays
      // and if current is modified
      if( 
        std::find_if( displays.begin(), displays.end(), BASE::Key::IsAssociatedFTor( &(*iter)->editor() ) ) == displays.end() && 
        (*iter)->editor().document()->isModified() )
      { out++; }
      displays.insert( &(*iter)->editor() );
   }
    
    return out;
    
  }   
  //@}

  //!@name display management
  //@{
  
  //! retrieve active display
  TextView& activeView( void )
  { return *active_view_; }
  
  //! retrieve active display
  const TextView& activeView( void ) const
  { return *active_view_; }

  //! select display from file
  void selectView( const File& file )
  {
    
    BASE::KeySet<TextView> views( this );
    BASE::KeySet<TextView>::iterator iter( std::find_if(
      views.begin(),
      views.end(),
      TextView::SameFileFTor( file ) ) );
    if( iter == views.end() ) return;
    
    // change active display
    setActiveView( **iter );
    (*iter)->editor().setFocus();
    
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
    
    //! new view
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
  void setActiveView( TextView& );
 
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

  //! close view
  QAction& closeViewAction( void ) const
  { return *close_view_action_; }

  //! close view
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
  
  //! split view horizontal
  QAction& splitViewHorizontalAction( void ) const
  { return *split_view_horizontal_action_; }
  
  //! split view vertical
  QAction& splitViewVerticalAction( void ) const
  { return *split_view_vertical_action_; }
  
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
  void enterEvent( QEvent *event );
    
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
  void _splitView( void )
  { _splitView( orientation(), true ); }

  //! clone current file horizontal
  void _splitViewHorizontal( void )
  { _splitView( Qt::Horizontal, true ); }

  //! clone current file horizontal
  void _splitViewVertical( void )
  { _splitView( Qt::Vertical, true ); }

  //! detach current view
  void _detach( void );
  
  //! close
  /*! close window */
  void _closeWindow( void )
  { 
    Debug::Throw( "EditFrame::_closeWindow.\n" );
    close();
  }
  
  //! close 
  /*! close current view if more than two view are open, */
  void _closeView( void )
  { 
    Debug::Throw( "EditFrame::_closeView (SLOT)\n" );
    BASE::KeySet< TextView > views( this );
    if( views.size() > 1 ) _closeView( activeView() );
    else _closeWindow();
  }
  
  //! save
  void _save( void )
  { activeView().editor().save(); }
  
  //! Save As
  void _saveAs( void )
  { activeView().editor().saveAs(); }

  //! Revert to save
  void _revertToSave( void );
  
  //! Print current document
  void _print( void );
  
  //! undo
  void _undo( void )
  { 
    Debug::Throw( "EditFrame::_undo.\n" );
    activeView().editor().undoAction().trigger(); 
  }

  //! redo
  void _redo( void )
  { 
    Debug::Throw( "EditFrame::_redo.\n" );
    activeView().editor().redoAction().trigger(); 
  }

  //! cut
  void _cut( void )
  { 
    Debug::Throw( "EditFrame::_cut.\n" );
    activeView().editor().cutAction().trigger(); 
  }

  //! copy
  void _copy( void )
  { 
    Debug::Throw( "EditFrame::_copy.\n" );
    activeView().editor().copyAction().trigger(); 
  }

  //! paste
  void _paste( void )
  { 
    Debug::Throw( "EditFrame::_paste.\n" );
    activeView().editor().pasteAction().trigger(); 
  }
  
  //! file information
  void _fileInfo( void )
  { activeView().editor().fileInfoAction().trigger(); }

  //! spellcheck
  void _spellcheck( void )
  { activeView().editor().spellcheckAction().trigger(); }
  
  //! diff files
  void _diff( void );
  
  //! update window title, cut, copy, paste buttons, and filename line editor
  /*! \param flags are bitwise or of TextDisplay::UpdateFlags */
  void _update( unsigned int flags );

  //! display cursor position in state frame
  void _updateCursorPosition( void );

  //! display focus changed
  void _displayFocusChanged( TextDisplay* display );
  
  private:

  //! install actions
  void _installActions( void );

  //! Update window title
  void _updateWindowTitle();

  //! new file
  void _newFile( const OpenMode&, const Qt::Orientation& );

  //! open file
  void _open( FileRecord,  const OpenMode&, const Qt::Orientation& );
  
  //! close view
  /*! Ask for save if view is modified */
  void _closeView( TextView& );

  //! split view
  TextView& _splitView( const Qt::Orientation&, const bool& clone );
  
  //! create new splitter
  QSplitter& _newSplitter( const Qt::Orientation&, const bool& clone  );
  
  //! create new TextView
  TextView& _newTextView( QWidget* parent );

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
  TextView* active_view_;
      
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

  //! close view
  QAction* close_view_action_;

  //! close view
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
  
  //! split view horizontal
  QAction* split_view_horizontal_action_;
  
  //! split view vertical
  QAction* split_view_vertical_action_;
  
  //! open horizontal
  QAction* open_horizontal_action_;

  //! open vertical
  QAction* open_vertical_action_;

  //@}
  
  //! default orientation for multiple views
  Qt::Orientation default_orientation_;
  
  //! default open mode
  OpenMode default_open_mode_;
  
  //! position update timer
  QTimer position_timer_;
  
};

#endif
