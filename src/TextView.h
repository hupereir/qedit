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
#include <QTimer>
#include <QWidget>

#include "Counter.h"
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
    
      _closeDisplay( activeDisplay() );
      return true;
    
    } else return false;
  
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
   
  signals:
  
  //! emitted when parent window must be update
  void needUpdate( unsigned int );
  
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
   
  private slots:
   
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

  //! new file
  void _newFile( const OpenMode&, const Qt::Orientation& );

  //! open file
  void _open( FileRecord,  const OpenMode&, const Qt::Orientation& );

  private:
  
  //! open previous menu
  RecentFilesMenu& _recentFilesMenu( void ) const;

  //! close display
  /*! Ask for save if display is modified */
  void _closeDisplay( TextDisplay& );

  //! split display
  TextDisplay& _splitDisplay( const Qt::Orientation&, const bool& clone );
  
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
 
  //! default orientation for multiple displays
  Qt::Orientation default_orientation_;
  
  //! default open mode
  OpenMode default_open_mode_;

  //! position update timer
  QTimer position_timer_;
  
};

#endif
