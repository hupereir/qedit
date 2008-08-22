// $Id$
#ifndef NavigationFrame_h
#define NavigationFrame_h

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
   \file NavigationFrame.h
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <assert.h>
#include <map>
#include <QAction>
#include <QShowEvent>
#include <QStackedWidget>
#include <QToolButton>

#include "Counter.h"
#include "FileList.h"
#include "FileRecord.h"

class SessionFilesFrame;
class RecentFilesFrame;
class FileSystemFrame;

//! editor windows navigator
/*!
  displays an up-to-date list of recent files
  as well as files opened in current session
*/
class NavigationFrame: public QStackedWidget, public Counter
{

  public:

  //! creator
  NavigationFrame( QWidget* parent, FileList&  );

  //! default size
  void setDefaultWidth( const int& );
    
  //! default width
  const int& defaultWidth( void ) const
  { return default_width_; }
    
  //! size
  QSize sizeHint( void ) const;  
       
  //! session files 
  SessionFilesFrame& sessionFilesFrame( void ) const
  {
    assert( session_files_frame_ );
    return *session_files_frame_;
  }
 
  
  //! recent files 
  RecentFilesFrame& recentFilesFrame( void ) const
  {
    assert( recent_files_frame_ );
    return *recent_files_frame_;
  }

  //! file system
  FileSystemFrame& fileSystemFrame( void ) const
  { 
    assert( file_system_frame_ );
    return *file_system_frame_;
  }
  
  //!@name actions
  //@{
  
  //! visibility
  QAction& visibilityAction( void ) const
  { return *visibility_action_; }
  
  //@}
          
  private:
  
  //! install actions
  void _installActions( void );
 
  //! default width;
  int default_width_;
  
  //! session files
  SessionFilesFrame *session_files_frame_;
  
  //! recent files
  RecentFilesFrame *recent_files_frame_;

  //! file system
  FileSystemFrame* file_system_frame_;
  
  //@}
  
  //!@name actions
  //@{
  
  //! visibility
  QAction* visibility_action_;
      
  //@}
  
};

#endif
