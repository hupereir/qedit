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
  \file TextView.cpp
  \brief compound widget to associate TextDisplay and LineNumberWidget
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>

#include "BlockDelimiterWidget.h"
#include "LineNumberWidget.h"
#include "TextDisplay.h"
#include "TextView.h"

using namespace std;

//__________________________________________________________________
TextView::TextView( QWidget* parent ):
  QFrame( parent ),
  Counter( "TextView" )
{
  
  Debug::Throw( "TextView::TextView.\n" );
  
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout( layout );
  
  setFrameStyle( QFrame::StyledPanel | QFrame::NoFrame );
  
  editor_ = new TextDisplay( this );
  editor_->setFrameStyle( QFrame::NoFrame );
  
  line_number_widget_ = new LineNumberWidget( &editor(), this );
  block_delimiter_widget_ = new BlockDelimiterWidget( &editor(), this );
  
  layout->addWidget( &blockDelimiterWidget(), 0 );
  layout->addWidget( &lineNumberWidget(), 0 );
  layout->addWidget( &editor(), 1 );
  
}
