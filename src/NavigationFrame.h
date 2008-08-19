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
#include <QAbstractButton>
#include <QAction>
#include <QShowEvent>
#include <QStackedWidget>

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
class NavigationFrame: public QWidget, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! creator
  NavigationFrame( QWidget* parent, FileList&  );

  //! destructor
  ~NavigationFrame( void );

  //! default size
  void setDefaultWidth( const int& );
  
  //! default width
  const int& defaultWidth( void ) const
  { return default_width_; }
  
    //! session files 
  SessionFilesFrame& sessionFilesFrame( void ) const
  {
    assert( session_files_frame_ );
    return *session_files_frame_;
  }
  
  //! size
  QSize sizeHint( void ) const;  

  //!@name actions
  //@{
  
  //! visibility
  QAction& visibilityAction( void ) const
  { return *visibility_action_; }
  
  //! update session files action
  QAction& updateSessionFilesAction( void ) const;
  
  //! update recent files action
  QAction& updateRecentFilesAction( void ) const;
  
  //@}
  
  signals:

  //! signal emited when a file is activated
  void fileActivated( FileRecord );  
      
  protected:
     
  //! stack widget
  QStackedWidget& _stack( void ) const
  { 
    assert( stack_ );
    return *stack_;
  }

  //!@name file system
  //@{
   
  //! recent files 
  RecentFilesFrame& _recentFilesFrame( void ) const
  {
    assert( recent_files_frame_ );
    return *recent_files_frame_;
  }
  
  //! file system
  FileSystemFrame& _fileSystemFrame( void ) const
  { 
    assert( file_system_frame_ );
    return *file_system_frame_;
  }
    
  //@}
  
  private slots:
  
  //! display item page
  virtual void _display( QAbstractButton* );

  private:
  
  //! install actions
  void _installActions( void );
 
  //! default width
  int default_width_;

  //! stack widget
  QStackedWidget* stack_;
  
  //! map widget to action
  typedef std::map<QAbstractButton*, QWidget* > ButtonMap;
  
  //! map widget to action in the toolbar
  ButtonMap buttons_;

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
