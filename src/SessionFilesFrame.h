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

  //! select file in list
  void selectFile( const File& );
  
  //!@name actions
  //@{
  
  //! visibility
  QAction& updateAction( void ) const
  { return *update_action_; }
  
  //@}
  
  signals:

  //! signal emited when a file is selected
  void fileSelected( FileRecord );  

  //! signal emited when a file is selected
  void fileActivated( FileRecord );  
      
  protected:
  
  //! list
  TreeView& _list( void ) const
  { 
    assert( list_ );  
    return *list_;
  }
  
  //! model
  SessionFilesModel& _model( void ) 
  { return model_; }
        
  private slots:

  //! update configuration
  void _updateConfiguration( void );
     
  //! update session files
  void _update( void );
 
  //! update session files
  void _updateActions( void );
  
  //! check file selection
  void _checkSelection( void );
  
  //! open
  void _open( void );

  //! sessionFilesItem selected
  void _itemSelected( const QModelIndex& index );

  //!@name selections
  //@{

  //! restore selection
  void _storeSelection( void );
  
  //! store selection
  void _restoreSelection( void );

  //@}

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
  
  //@}
  
  //! model
  SessionFilesModel model_;
  
  //! list
  TreeView* list_;
  
  //@}
    
  //!@name actions
  //@{
  
  //! update
  QAction* update_action_;
 
  //! open action
  QAction* open_action_;
      
  //@}
  
};

#endif
