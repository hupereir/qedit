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
  \file FileRemovedDialog.cpp
  \brief QDialog used when a file has been removed from disk
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "FileRemovedDialog.h"
#include "PixmapEngine.h"
#include "Icons.h"
#include "IconEngine.h"
#include "QtUtil.h"



//________________________________________________________
FileRemovedDialog::FileRemovedDialog( QWidget* parent, const File& file ):
  BaseDialog( parent ),
  Counter( "FileRemovedDialog" )
{

  Debug::Throw( "FileRemovedDialog::FileRemovedDialog.\n" );

  setWindowTitle( "File Removed" );

  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(10);
  setLayout( layout );

  // create message
  QString buffer;
  QTextStream( &buffer ) << file.localName() << " has been removed.";

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
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::SAVE ), "&Save Again", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _reSave() ) );
  button->setToolTip( "Save file again. Disc modifications will be lost" );

  // save as button
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::SAVE_AS ), "&Save As", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _saveAs() ) );
  button->setToolTip( "Save file with a different name" );

  // close button.
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&Close", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _close() ) );
  button->setToolTip( "Close window" );

  // ignore button.
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CANCEL ), "&Ignore", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _ignore() ) );
  button->setToolTip( "Ignore warning" );

  adjustSize();

}
