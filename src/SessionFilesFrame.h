// $Id$
#ifndef SessionFilesFrame_h
#define SessionFilesFrame_h

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
   \file SessionFilesFrame.h
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <assert.h>
#include <map>
#include <QAction>
#include <QPaintEvent>

#include "Counter.h"
#include "SessionFilesModel.h"

class TreeView;

//! editor windows navigator
/*!
  displays an up-to-date list of recent files
  as well as files opened in current session
*/
class SessionFilesFrame: public QWidget, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! creator
  SessionFilesFrame( QWidget* parent );

  //! destructor
  ~SessionFilesFrame( void );

  //! list
  TreeView& list( void ) const
  { 
    assert( list_ );  
    return *list_;
  }

  //! select file in list
  void select( const File& );
  
  public slots:
   
  //! update session files
  void update( void );
  
  signals:

  //! signal emitted when a file is selected
  void fileSelected( FileRecord );  

  //! signal emitted when a file is selected
  void fileActivated( FileRecord );  
      
  //! signal emitted when file is asked to be closed
  void filesClosed( FileRecord::List );
  
  //! signal emitted when file is asked to be saved
  void filesSaved( FileRecord::List );
  
  protected:
    
  //! model
  SessionFilesModel& _model( void ) 
  { return model_; }
        
  private slots:

  //! update configuration
  void _updateConfiguration( void );
     
  //! update session files
  void _updateActions( void );
  
  //! open
  void _open( void );

  //! save
  void _save( void );
  
  //! close
  void _close( void );
  
  //! sessionFilesItem selected
  void _itemSelected( const QModelIndex& index );

  //! sessionFilesItem selected
  void _itemActivated( const QModelIndex& index );

  //! session files sort order
  void _storeSortMethod( int, Qt::SortOrder );
    
  private:
  
  //! install actions
  void _installActions( void );
  
  //!@name actions
  //@{
  
  //! open action
  QAction& _openAction( void ) const
  { return *open_action_; }
  
  //! save action
  QAction& _saveAction( void ) const
  { return *save_action_; }
  
  //! close action
  QAction& _closeAction( void ) const
  { return *close_action_; }
  
  //@}
  
  //! true when list selection updates are enabled
  void _setEnabled( bool value )
  { enabled_ = value; }
  
  //! true when list selection updates are enabled
  bool _enabled( void ) const
  { return enabled_; }
  
  //! model
  SessionFilesModel model_;
  
  //! list
  TreeView* list_;
    
  //! true when list selection updates are enabled
  bool enabled_;
  
  //!@name actions
  //@{
  
  //! open action
  QAction* open_action_;
  
  //! save action
  QAction* save_action_;
  
  //! close action
  QAction *close_action_;
      
  //@}
  
};

#endif
