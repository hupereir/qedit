#ifndef WindowServer_h
#define WindowServer_h

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
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 *******************************************************************************/

/*!
  \file WindowServer.h
  \brief handles opened edition windows
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <list>
#include <map>
#include <QAction>
#include <QObject>

#include "ArgList.h"
#include "Counter.h"
#include "File.h"
#include "FileRecord.h"
#include "Key.h"
#include "TextSelection.h"

class MainWindow;

class WindowServer: public QObject, public Counter, public BASE::Key
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public: 
  
  //!@name Open mode string
  //@{
  
  static const std::string SINGLE_WINDOW;
  
  static const std::string MULTIPLE_WINDOWS;
  
  //@}
  
  //! constructor
  WindowServer( QObject* parent = 0 );
  
  //! destructor
  virtual ~WindowServer( void );
  
  //! create new empty main window
  MainWindow& newMainWindow( void );

  //! returns list of opened files
  FileRecord::List files( bool modified_only = false ) const;
  
  //! close all windows gracefully
  /*! returns false if the opperation was cancelled. */
  bool closeAllWindows( void );
  
  //! read file from arguments and open relevant windows
  void readFilesFromArguments( ArgList );
  
  //!@name actions
  //@{

  //! close
  QAction& saveAllAction( void ) const
  { return *save_all_action_; }

  //@}
  
  signals:
  
  //! emmited whenever the session file list is modified
  void sessionFilesChanged( void );
  
  public slots:
 
  //! multiple files replace
  void multipleFileReplace( std::list<File>, TextSelection );
  
  private slots:
  
  //! update configuration
  void _updateConfiguration( void );
  
  //! active window changed
  void _activeWindowChanged( MainWindow* );
  
  //! update actions
  void _updateActions( void );
 
  //!@ new file methods
  //@{

  //! new file
  void _newFile( void );

  //! new file
  void _newFile( Qt::Orientation );
  
  //@}

  //!@ open methods
  //@{
  
  //! from dialog
  bool _open( void )
  { return _open( _selectFileFromDialog() ); }

  //! open file
  bool _open( FileRecord );

  //! open in active view
  bool _openHorizontal( void )
  { return _open( _selectFileFromDialog(), Qt::Vertical ); }

  //! open in active view
  bool _openVertical( void )
  { return _open( _selectFileFromDialog(), Qt::Horizontal ); }
  
  //@}
  
  //! detach
  /*! this closes the active view and opens it in a separate window */
  void _detach( void );
  
  //! save all edited files
  void _saveAll( void );
  
  private:
    
  //! open file
  bool _open( FileRecord, Qt::Orientation );

  //! close files 
  void _closeFiles( const std::list<std::string>& );
  
  //! select file record from dialog
  /*! return empty record if no file is opened or file is directory */
  FileRecord _selectFileFromDialog( void );
  
  //! returns true if new file should be created
  bool _createNewFile( const FileRecord& );
  
  //! active window
  void _setActiveWindow( MainWindow& );
  
  //! true when active window is valid
  bool _hasActiveWindow( void ) const;
  
  //! active window
  MainWindow& _activeWindow( void )
  { return *active_window_; }
  
  //! active window
  const MainWindow& _activeWindow( void ) const
  { return *active_window_; }

  //! open mode
  enum OpenMode
  {
    
    // single stacked window
    ACTIVE_WINDOW,
    
    // multiple windows
    NEW_WINDOW
    
  };
  
  //! open mode
  const OpenMode& _openMode( void ) const
  { return open_mode_; }
  
  //! open mode
  void _setOpenMode( const OpenMode& mode )
  { open_mode_ = mode; }
  
  //! open mode
  OpenMode open_mode_;
  
  //! active window
  MainWindow* active_window_;

  //!@name actions
  //@{
 
  //! save all modified files
  QAction* save_all_action_;
  
  //@}
  
};

#endif
