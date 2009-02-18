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
  
  //! return number of independant displays
  unsigned int independentDisplayCount( void );
  
  //! return number of independent modified displays
  unsigned int modifiedDisplayCount( void );
  
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
  bool selectDisplay( const File& file );
  
  //! change active display manualy
  void setActiveDisplay( TextDisplay& );

  //! close display
  void closeActiveDisplay( void );
  
  //! close display
  /*! Ask for save if display is modified */
  void closeDisplay( TextDisplay& );
  
  //@}

  //! set new document
  void setIsNewDocument( void );
  
  //! set file and read
  void setFile( File file );
     
  //! split display
  TextDisplay& splitDisplay( const Qt::Orientation&, const bool& clone );
  
  //! save all displays
  void saveAll( void );
  
  //! ignore all display modifications
  void ignoreAll( void );
  
  //! select class name
  void selectClassName( QString );
  
  //! rehighlight all displays
  void rehighlight( void );
  
  //! check modified displays
  void checkModifiedDisplays( void );
  
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

  protected:
  
  //! enter event handler
  /*! it is used to check for removed/externally modified files */
  virtual void enterEvent( QEvent* );
  
  private slots:
  
  //! check number of displays
  /*! 
  this is triggered by TextDisplay::destroyed()
  when no display is found the entire window is closed
  the active display is updated otherwise
  */
  void _checkDisplays( void );
  
  //! display focus changed
  void _activeDisplayChanged( TextEditor* );
  
  private:
 
  //! create new splitter
  QSplitter& _newSplitter( const Qt::Orientation&, const bool&  );
  
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
