
/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later-
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
   \file NavigationFrame.cpp
   \brief editor windows navigator
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QHeaderView>
#include <QLayout>

#include "Application.h"
#include "Debug.h"
#include "FileSystemFrame.h"
#include "NavigationFrame.h"
#include "RecentFilesFrame.h"
#include "SessionFilesFrame.h"

using namespace std;

//_______________________________________________________________
NavigationFrame::NavigationFrame( QWidget* parent, FileList& files ):
  QStackedWidget( parent ),
  Counter( "NavigationFrame" ),
  default_width_( -1 )
{
  
  Debug::Throw( "NavigationFrame:NavigationFrame.\n" );
  layout()->setMargin(2);
  layout()->setSpacing(2);

  // stack widget
  addWidget( session_files_frame_ = new SessionFilesFrame(0) );  
  addWidget( recent_files_frame_ = new RecentFilesFrame(0, files) );  
  addWidget( file_system_frame_ = new FileSystemFrame(0) );

  setCurrentWidget( &sessionFilesFrame() );
  
  // actions
  _installActions();
    
}

//______________________________________________________________________
void NavigationFrame::setDefaultWidth( const int& value )
{ default_width_ = value; }

//____________________________________________
QSize NavigationFrame::sizeHint( void ) const
{ return (default_width_ ) >= 0 ? QSize( default_width_, 0 ):QWidget::sizeHint(); }

//______________________________________________________________________
void NavigationFrame::_installActions( void )
{
  
  Debug::Throw( "NavigationFrame::_installActions.\n" );
  addAction( visibility_action_ = new QAction( "Show &navigation panel", this ) );
  visibility_action_->setCheckable( true );
  visibility_action_->setChecked( true );
  visibility_action_->setShortcut( Qt::Key_F5 );
  connect( visibility_action_, SIGNAL( toggled( bool ) ), SLOT( setVisible( bool ) ) );
    
}
