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
  \file DocumentClassModifiedDialog.cpp
  \brief QDialog used when a file has been removed from disk
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "DocumentClassModifiedDialog.h"
#include "PixmapEngine.h"
#include "DocumentClassIcons.h"
#include "IconEngine.h"
#include "QtUtil.h"

using namespace std;

//________________________________________________________
DocumentClassModifiedDialog::DocumentClassModifiedDialog( QWidget* parent ):
  BaseDialog( parent ),
  Counter( "DocumentClassModifiedDialog" )
{

  Debug::Throw( "DocumentClassModifiedDialog::DocumentClassModifiedDialog.\n" );

  setWindowTitle( "Document Classes Modified - qedit" );
  
  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(10);
  setLayout( layout );
  
  // create message
  QString buffer( "Document classes have been modified.\nSave ?" );

  //! try load Question icon
  QPixmap question_pixmap = PixmapEngine::get( ICONS::WARNING );
  
  // insert main vertical box
  if( question_pixmap.isNull() )
  { layout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter ); }
  else
  {
    
    QHBoxLayout *h_layout( new QHBoxLayout() );
    layout->addLayout( h_layout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( question_pixmap );
    h_layout->addWidget( label, 0, Qt::AlignHCenter );
    h_layout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter );
    
  }

  // horizontal separator
  QFrame* frame( new QFrame( this ) );
  frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  layout->addWidget( frame );

  // button layout
  QHBoxLayout *button_layout = new QHBoxLayout();     
  button_layout->setSpacing( 5 );
  layout->addLayout( button_layout );
  button_layout->addStretch(1);
  
  // resave button
  QPushButton* button;
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_OK ), "&Yes", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _yes() ) );
  button->setToolTip( "Save document classes modifications" );
  
  
  // save as button
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&No", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _no() ) );
  button->setToolTip( "Ignore document classes modifications" );

  // ignore button.
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CANCEL ), "&Cancel", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _cancel() ) );
  button->setToolTip( "Cancel current action" );
  
  adjustSize();

}
