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
  \file SaveAllDialog.cpp
  \brief QDialog used to save all modified files
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>

#include "Icons.h"
#include "PixmapEngine.h"
#include "Debug.h"
#include "SaveAllDialog.h"

using namespace std;

//__________________________________________________
SaveAllDialog::SaveAllDialog( QWidget* parent, FileRecord::List files ):
  CustomDialog( parent, OK_BUTTON | CANCEL_BUTTON )
{
  
  Debug::Throw( "SaveAllDialog::SaveAllDialog.\n" );

  QHBoxLayout *h_layout( new QHBoxLayout() );
  h_layout->setSpacing(5); 
  h_layout->setMargin( 0 ); 
  mainLayout().addLayout( h_layout );
  
  // add icon
  QLabel *label( new QLabel( this ) );
  label->setPixmap( PixmapEngine::get( ICONS::WARNING ) );
  h_layout->addWidget( label, 0, Qt::AlignHCenter );

  // create label text
  static const unsigned int max_line_size( 50 );
  unsigned int current_line( 0 );
  ostringstream what;
  what << "Modified: ";

  unsigned int index = 0;
  for( FileRecord::List::const_iterator iter = files.begin(); iter != files.end(); iter++, index++ )
  {
    what << iter->file().localName();
    if( index < files.size()-2 ) what << ", ";
    else if( index == files.size()-2 ) what << " and ";
    else what << ".";
    
    if( what.str().size() >= (current_line+1)*max_line_size )
    {
      what << endl;
      current_line++;
    }
    
  }
    
  what << endl << "Save ?";
  h_layout->addWidget( new QLabel( what.str().c_str(), this ), 1, Qt::AlignHCenter );

  // rename buttons
  okButton().setText( "&Yes" );
  cancelButton().setText( "&No" );
  adjustSize();
}
  
