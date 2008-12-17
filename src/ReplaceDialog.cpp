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
  \file ReplaceDialog.cpp
  \brief replace_text dialog for text editor widgets
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QPushButton>

#include "Debug.h"
#include "ReplaceDialog.h"
#include "QtUtil.h"

using namespace std;

//________________________________________________________________________
ReplaceDialog::ReplaceDialog( QWidget* parent, Qt::WFlags flags ):
    BaseReplaceDialog( parent, flags )
{
  Debug::Throw( "ReplaceDialog::ReplaceDialog.\n" );
  
  // insert multiple file buttons
  QPushButton* button = new QPushButton( "&Files", this );
  connect( button, SIGNAL( clicked() ), SIGNAL( replaceInFiles() ) );
  button->setToolTip( "replace all occurence of the search string in the selected files" );
  _addDisabledButton( button );
  _locationLayout().addWidget( button );
  button->setAutoDefault( false );

  // tab order
  setTabOrder( &_replaceWindowButton(), button );
  
}
