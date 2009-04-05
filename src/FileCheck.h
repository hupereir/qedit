#ifndef FileCheck_h
#define FileCheck_h

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
  \file FileCheck.h
  \brief keep track of external file modifications
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QBasicTimer>
#include <QFileSystemWatcher>
#include <QObject>
#include <QTimerEvent>

#include <set>

#include "Key.h"
#include "TimeStamp.h"

class TextDisplay;

//! handles threads for file auto-save
class FileCheck: public QObject, public BASE::Key, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public:
  
  //! constructor
  FileCheck( QObject* parent = 0 );
  
  //! destructor
  ~FileCheck( void );
  
  //! register new dispay
  void registerDisplay( TextDisplay* );
    
  //! add file
  void addFile( const QString& );

  //! remove file
  void removeFile( const QString& );
 
  //! used to monitor file changes
  class Data
  {
    public:
    
    //! flag
    enum Flag
    {
      NONE,
      REMOVED,
      MODIFIED
    };
    
    //! constructor
    Data( QString file = QString(), Flag flag = NONE, TimeStamp stamp = TimeStamp() ):
      file_( file ),
      flag_( flag ),
      time_stamp_( stamp )
    {}
    
    //! equal to operator
    bool operator == ( const Data& data ) const
    { return file() == data.file(); }
    
    //! less than operator 
    bool operator < ( const Data& data ) const
    { return file() < data.file(); }
    
    //! file
    void setFile( const QString& file )
    { file_ = file; }
    
    //! file
    const QString& file( void ) const
    { return file_; }
    
    //! flag
    void setFlag( const Flag& flag )
    { flag_ = flag; }
    
    //! flag
    const Flag& flag( void ) const
    { return flag_; }
    
    //! timestamp
    void setTimeStamp( const TimeStamp& stamp )
    { time_stamp_ = stamp; }
    
    //! timestamp
    const TimeStamp& timeStamp( void ) const
    { return time_stamp_; }
      
    private:
    
    //! file
    QString file_;
    
    //! flag
    Flag flag_;
    
    //! timestamp
    TimeStamp time_stamp_;
    
  };
  
  //! map data to file
  typedef std::set<Data> DataSet;  
  
  
  //! file system watcher
  const QFileSystemWatcher& fileSystemWatcher( void ) const
  { return file_system_watcher_; }
  
  protected:
  
  //! timer event, to handle multiple file modification at once
  virtual void timerEvent( QTimerEvent* event );
  
  private slots:
  
  void _fileChanged( const QString& ); 
  
  private:
  
  //! file system watcher
  QFileSystemWatcher& _fileSystemWatcher( void ) 
  { return file_system_watcher_; }
  
  //! file system watcher
  QFileSystemWatcher file_system_watcher_;
  
  //! file set
  typedef std::set<QString> FileSet;
  
  //! file set
  FileSet files_;
  
  //! map files and modification data
  DataSet data_;

  //! resize timer
  QBasicTimer timer_;

};

#endif