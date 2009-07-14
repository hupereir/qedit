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
  \file DocumentClassPreview.cpp
  \brief document class preview
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "Debug.h"
#include "DocumentClassIcons.h"
#include "DocumentClassPreview.h"
#include "DocumentClassTextEditor.h"
#include "IconEngine.h"

#include <QLayout>

//____________________________________________________________
DocumentClassPreview::DocumentClassPreview( QWidget* parent ):
  QWidget( parent ),
  Counter( "DocumentclassPreview" )
{
  setLayout( new QHBoxLayout() );
  layout()->setSpacing(5);
  layout()->setMargin(0);
  
  layout()->addWidget( editor_ = new DocumentClassTextEditor( this ) );
  
  QVBoxLayout *v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing(5);
  layout()->addItem( v_layout );
  
  v_layout->addWidget( reload_button_ = new QPushButton( IconEngine::get( ICONS::RELOAD ), "&Reload", this ) );
  v_layout->addStretch( 1 );
  reloadButton().setToolTip( "Reload document class and update text editor consistently." );
  
}
