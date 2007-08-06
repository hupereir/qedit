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
  \file NewFileDialog.cc
  \brief QDialog used to ask if a new file should be created
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <sstream>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "CustomPixmap.h"
#include "NewFileDialog.h"
#include "QtUtil.h"
#include "XmlOptions.h"

using namespace std;

//________________________________________________________
NewFileDialog::NewFileDialog( QWidget* parent, const File& file, const unsigned int& buttons ):
  QDialog( parent ),
  Counter( "NewFileDialog" )
{
  
  Debug::Throw( "NewFileDialog::NewFileDialog.\n" );
  
  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout();
  layout->setSpacing(10);
  layout->setMargin(10);
  setLayout( layout );
  
  // create message
  ostringstream what;
  what << "Can't open " << file << "." << endl;
  what << "No such file or directory";
  
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
  button_layout->setSpacing(5);
  button_layout->setMargin(0);
  layout->addLayout( button_layout );

  // yes button
  QPushButton* button;
  if( buttons & CREATE )
  {
    button_layout->addWidget( button = new QPushButton( "&Create", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _create() ) );
  }
  
  // cancel button. 
  if( buttons & CANCEL )
  {
    button_layout->addWidget( button = new QPushButton( "&Cancel", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _cancel() ) );
  }

  // cancel button. 
  if( buttons & EXIT )
  {
    button_layout->addWidget( button = new QPushButton( "&Exit", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _exit() ) );
  }
  
  adjustSize();
  
}
