
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
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        
 * for more details.                     
 *                          
 * You should have received a copy of the GNU General Public License along with 
 * software; if not, write to the Free Software Foundation, Inc., 59 Temple     
 * Place, Suite 330, Boston, MA  02111-1307 USA                           
 *                         
 *                         
 *******************************************************************************/

/*!
  \file HighlightStyleDialog.cpp
  \brief Syntax highlighting style editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>
#include <QLabel>

#include "AnimatedLineEditor.h"
#include "HighlightStyleDialog.h"
#include "ColorDisplay.h"
#include "GridLayout.h"
#include "FontInfo.h"

using namespace std;

//________________________________________________________________________
HighlightStyleDialog::HighlightStyleDialog( QWidget* parent ):
  CustomDialog( parent, OkButton|CancelButton|CustomDialog::Separator )
{
  
  Debug::Throw( "HighlightStyleDialog::HighlightStyleDialog.\n" );
  
  GridLayout* grid_layout( new GridLayout() );
  grid_layout->setSpacing( 5 );
  grid_layout->setMargin( 0 );
  grid_layout->setMaxCount( 2 );
  grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
  mainLayout().addLayout( grid_layout );
 
  grid_layout->addWidget( new QLabel( "Name: ", this ) );
  grid_layout->addWidget( editor_ = new AnimatedLineEditor( this ) );
 
  grid_layout->addWidget( new QLabel( "Font color: ", this ) );
  grid_layout->addWidget( color_ = new ColorDisplay( this ) );
  
  grid_layout->addWidget( new QLabel( "Font style: ", this ) );
  grid_layout->addWidget( font_info_ = new FontInfo( this ) );
}

//________________________________________________________________________
void HighlightStyleDialog::setStyle( const HighlightStyle& style )
{
  
  Debug::Throw( "HighlightStyleDialog::setStyle\n" );
  editor_->setText( style.name() );
  color_->setColor( style.color().name() );
  font_info_->setFormat( style.fontFormat() );
  return;
  
}


//________________________________________________________________________
HighlightStyle HighlightStyleDialog::style( void ) const
{
  Debug::Throw( "HighlightStyleDialog::style\n" );
  return HighlightStyle( editor_->text(), font_info_->format(), QColor( color_->colorName() ) );
}
  
