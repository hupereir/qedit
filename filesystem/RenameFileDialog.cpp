
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
 * ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or         
 * FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License         
 * for more details.                    
 *                         
 * You should have received a copy of the GNU General Public License along with 
 * software; if not, write to the Free Software Foundation, Inc., 59 Temple     
 * Place, Suite 330, Boston, MA   02111-1307 USA                          
 *                        
 *                        
 *******************************************************************************/

/*!
   \file RenameFileDialog.cpp
   \brief Edit/create keyword popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLabel>

#include "Debug.h"
#include "LineEditor.h"
#include "RenameFileDialog.h"

using namespace std;

//_____________________________________________________
RenameFileDialog::RenameFileDialog( QWidget* parent, FileRecord record ):
  CustomDialog( parent )
{
  
  Debug::Throw( "RenameFileDialog::RenameFileDialog.\n" );
  
  QString buffer;
  QTextStream( &buffer ) << "Rename the item '" << record.file() << "' to:"; 
  mainLayout().addWidget( new QLabel( buffer, this ) );
  mainLayout().addWidget( editor_ = new LineEditor( this ) );

  // set editor text
  _editor().setText( record.file() );
  _editor().setFocus();

  // get short name and select
  File short_file( record.file().truncatedName() );
  if( !short_file.isEmpty() ) _editor().setSelection( 0, short_file.size() );
  else _editor().selectAll();
  connect( &_editor(), SIGNAL( textChanged( const QString& ) ), SLOT( _updateButtons() ) );
  
  // rename buttons
  okButton().setText( "&Rename" );
  
  setMinimumSize( QSize( 320, 0 ) );
  
}

//_____________________________________________________
File RenameFileDialog::file( void ) const
{
  
  QString text( _editor().text() );
  return ( text.isEmpty() || text.isNull() ) ? File():File( qPrintable( text ) );
  
}

//_____________________________________________________
void RenameFileDialog::_updateButtons( void )
{
  
  Debug::Throw( "RenameFileDialog::_updateButtons.\n" );
  okButton().setEnabled( !_editor().text().isEmpty() ); 
  
}
