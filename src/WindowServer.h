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
  
  //! constructor
  WindowServer( QObject* parent = 0 );
  
  //! destructor
  virtual ~WindowServer( void );
  
  //! create new empty main window
  MainWindow& newMainWindow( void );

  //! map files and modification status
  typedef std::map< File, bool > FileMap;
  
  //! returns list of opened files
  FileMap files( bool modified_only = false ) const;
  
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
  
  public slots:
  
  //! open
  /*! returns 0 if no file is open (force application to quit) */
  MainWindow* open( FileRecord record = FileRecord(), ArgList args = ArgList() );
 
  //! multiple files replace
  void multipleFileReplace( std::list<File>, TextSelection );
  
  private slots:
  
  //! save all edited files
  void _saveAll( void );
  
  private:

  //! open file status
  enum OpenStatus
  {
    
    //! file successfully opened
    OPEN,
    
    //! file is invalid
    INVALID,
    
    //! file invalid and application should be closed
    EXIT_APP
    
  };
    
  //! current open file status
  OpenStatus open_status_;

  //!@name actions
  //@{
 
  //! save all modified files
  QAction* save_all_action_;
  
  //@}
  
};

#endif
