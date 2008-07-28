
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
  \file AskForSaveDialog.cpp
  \brief QDialog used to ask if modifications of a file should be saved
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <sstream>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "AskForSaveDialog.h"
#include "BaseIcons.h"
#include "CustomPixmap.h"
#include "IconEngine.h"
#include "QtUtil.h"

using namespace std;

//________________________________________________________
AskForSaveDialog::AskForSaveDialog( QWidget* parent, const File& file, const unsigned int& buttons ):
  QDialog( parent ),
  Counter( "AskForSaveDialog" )
{
  
  Debug::Throw( "AskForSaveDialog::AskForSaveDialog.\n" );
  
  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(10);
  setLayout( layout );
  
  // create message
  ostringstream what;
  what << "File ";
  if( file.size() ) what << "\"" << file.localName() << "\" ";
  what << "has been modified." << endl << "Save ?";
  
  //! try load Question icon
  CustomPixmap question_pixmap = CustomPixmap().find( ICONS::WARNING );
  
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
    h_layout->addWidget( new QLabel( what.str().c_str(), this ), 1, Qt::AlignHCenter );
    
  }
    
  // button layout
  QHBoxLayout *button_layout = new QHBoxLayout();     
  button_layout->setSpacing(5);
  button_layout->setMargin(0);
  layout->addLayout( button_layout );

  // yes button
  QPushButton* button;
  if( buttons & YES )
  {
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_OK ), "&Yes", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _yes() ) );
  }
  
  // no button
  if( buttons & NO )
  {
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&No", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _no() ) );
  }
  
  // yes to all button
  if( buttons & ALL )
  {
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_OK_APPLY ), "Yes to &All", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _all() ) );
  }
  
  
  
  // cancel button
  if( buttons & CANCEL )
  {
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CANCEL ), "&Cancel", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _cancel() ) );
  }
  
  adjustSize();
  
}
