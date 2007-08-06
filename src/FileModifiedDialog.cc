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
  \file FileModifiedDialog.cc
  \brief QDialog used when a file has been modified on disk
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include <sstream>

#include "FileModifiedDialog.h"
#include "CustomPixmap.h"
#include "XmlOptions.h"
#include "QtUtil.h"

using namespace std;

//________________________________________________________
FileModifiedDialog::FileModifiedDialog( QWidget* parent, const File& file ):
  QDialog( parent ),
  Counter( "FileModifiedDialog" )
{

  Debug::Throw( "FileModifiedDialog::FileModifiedDialog.\n" );

  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(10);
  setLayout( layout );
  
  // create message
  ostringstream what;
  what << file.localName() << " has been modified by another application." << endl;

  //! try load Question icon
  static CustomPixmap question_pixmap;
  static bool first( true );
  if( first )
  {
    first = false;
    list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
    question_pixmap.find( "messagebox_warning.png", path_list );    
  }
  
  // insert main vertical box
  if( question_pixmap.isNull() )
  { layout->addWidget( new QLabel( what.str().c_str(), this ), 1, Qt::AlignHCenter ); }
  else
  {
    
    QHBoxLayout *h_layout( new QHBoxLayout() );
    layout->addLayout( h_layout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( question_pixmap );
    h_layout->addWidget( label, 0, Qt::AlignHCenter );
    h_layout->addWidget( new QLabel( what.str().c_str(), this ), 0, Qt::AlignHCenter );
    
  }

  // button layout
  QHBoxLayout *button_layout = new QHBoxLayout();     
  button_layout->setSpacing( 5 );
  layout->addLayout( button_layout );

  // resave button
  QPushButton* button;
  button_layout->addWidget( button = new QPushButton( "&Re-save", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _reSave() ) );
  button->setToolTip( "Save file again. Disc modifications will be lost" );
    
  // save as button
  button_layout->addWidget( button = new QPushButton( "&Save as", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _saveAs() ) );
  button->setToolTip( "Save file with a different name" );

  // reload button.
  button_layout->addWidget( button = new QPushButton( "&Reload", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _reLoad() ) );
  button->setToolTip( "Reload file from disc. Modifications will be lost" );

  // ignore button.
  button_layout->addWidget( button = new QPushButton( "&Ignore", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _ignore() ) );
  button->setToolTip( "Ignore warning" );

}
