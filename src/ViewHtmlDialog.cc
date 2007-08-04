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
   \file ViewHtmlDialog.cc
   \brief file informations
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLabel>
#include <QLayout>

#include "ViewHtmlDialog.h"
#include "CustomGridLayout.h"
#include "Debug.h"
using namespace std;

//_______________________________________________________________________
ViewHtmlDialog::ViewHtmlDialog( QWidget* parent ):
  CustomDialog( parent, OK_BUTTON | CANCEL_BUTTON )
{
  
  Debug::Throw( "ViewHtmlDialog::ViewHtmlDialog.\n" );

  CustomGridLayout* layout = new CustomGridLayout();
  layout->setSpacing( 5 ); 
  layout->setMargin( 0 ); 
  layout->setMaxCount( 2 );
  mainLayout().addLayout( layout );
  
  // output file
  layout->addWidget( new QLabel( "File: ", this ) );
  layout->addWidget( file_ = new BrowsedLineEdit( this ) );
  file_->editor().setToolTip( "output HTML file" );
  
  // use html command
  layout->addWidget( use_command_ = new QCheckBox( "Open with ", this ) );
  file_->setToolTip( "use editor to open HTML file" ); 

  layout->addWidget( command_ = new BrowsedLineEdit( this ) );
  command_->setToolTip( "HTML editor" ); 
  connect( use_command_, SIGNAL( toggled( bool ) ), this, SLOT( _update( bool ) ) ); 
  
}
