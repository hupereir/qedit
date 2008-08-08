#ifndef TextView_h
#define TextView_h

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
*******************************************************************************/

/*!
  \file TextView.h
  \brief handle multiple text views
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <algorithm>
#include <QSplitter>
#include <QTimer>
#include <QWidget>

#include "Counter.h"
#include "FileRecord.h"
#include "Key.h"
#include "TextDisplay.h"

class RecentFilesMenu;

//! handles multiple views
class TextView: public QWidget, public Counter, public BASE::Key
{
  
  //! Qt meta object declaration
  Q_OBJECT
    
  public:
    
  //! constructor
  TextView( QWidget* parent );

  //! destructor
  virtual ~TextView( void );
 
  //!@name display management
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
    bool operator() ( const TextView* view ) const
    { 
      BASE::KeySet<TextDisplay> displays( view );
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
    bool operator() ( const TextView* view ) const
    { 
      BASE::KeySet<TextDisplay> displays( view );
      return std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) != displays.end();
    }
    
  };

  //! used to select editor with empty, unmodified file
  class IsModifiedFTor
  {
    public:

    //! predicate
    bool operator() ( const TextView* view ) const
    { return view->isModified(); }
    
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
  
  //! change active display manualy
  void setActiveDisplay( TextDisplay& );

  //! close display
  bool closeDisplay( void )
  {
    BASE::KeySet< TextDisplay > displays( this );
    if( displays.size() > 1 ) 
    {
    
      closeDisplay( activeDisplay() );
      return true;
    
    } else return false;
  
  }
  
  //! close display
  /*! Ask for save if display is modified */
  void closeDisplay( TextDisplay& );
  
  //@}

  //! default open mode
  enum OpenMode
  {
    //! new window
    NEW_WINDOW,
    
    //! new display
    NEW_VIEW
  };

  //! new file
  void newFile( const OpenMode&, const Qt::Orientation& );

  //! open file
  void open( FileRecord,  const OpenMode&, const Qt::Orientation& );

  //! split display
  TextDisplay& splitDisplay( const Qt::Orientation&, const bool& clone );
  
  //! save all displays
  void saveAll( void );
  
  //! select class name
  void selectClassName( QString );
  
  //! rehighlight all displays
  void rehighlight( void );
  
  //! diff files
  void diff( void );
  
  //! position timer
  QTimer& positionTimer( void )
  { return position_timer_; }
  
  signals:
  
  //! emitted when parent window must be update
  void needUpdate( unsigned int );
     
  //! current display overwrite mode changed
  void overwriteModeChanged( void );
  
  //! independent display count changed
  void displayCountChanged( void );
  
  //! current display undo is available
  void undoAvailable( bool );
 
  //! current display redo is available
  void redoAvailable( bool );
  
  private slots:
  
  //! display focus changed
  void _displayFocusChanged( TextEditor* );
  
  private:
  
  //! assiciated recent files menu
  bool _hasRecentFilesMenu( void ) const;
  
  //! open previous menu
  RecentFilesMenu& _recentFilesMenu( void ) const;
 
  //! create new splitter
  QSplitter& _newSplitter( const Qt::Orientation&, const bool& clone  );
  
  //! create new TextDisplay
  TextDisplay& _newTextDisplay( QWidget* );
  
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
 
  //! text display with focus
  TextDisplay* active_display_;
 
  //! position update timer
  QTimer position_timer_;
  
};

#endif
