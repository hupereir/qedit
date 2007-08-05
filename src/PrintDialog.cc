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
  \file PrintDialog.cc
  \brief print document
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QFrame>
#include <QLayout>
#include <QButtonGroup> 

#include "CustomPixmap.h"
#include "CustomGridLayout.h"
#include "Debug.h"
#include "Icons.h"
#include "PrintDialog.h"
#include "XmlOptions.h"

using namespace std;

//__________________________________________________
PrintDialog::PrintDialog( QWidget* parent ):
  CustomDialog( parent, OK_BUTTON | CANCEL_BUTTON )
{
  
  Debug::Throw( "PrintDialog::PrintDialog.\n" );

  QHBoxLayout *h_layout( new QHBoxLayout() );
  h_layout->setSpacing( 10 ); 
  h_layout->setMargin( 0 ); 
  mainLayout().addLayout( h_layout );
  
  // add icon
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  QLabel *label( new QLabel( this ) );
  label->setPixmap( CustomPixmap().find( ICONS::PRINT, path_list ) );
  h_layout->addWidget( label );
  
  CustomGridLayout* grid_layout = new CustomGridLayout();
  grid_layout->setSpacing( 5 ); 
  grid_layout->setMargin( 0 ); 
  grid_layout->setMaxCount( 2 );
  h_layout->addLayout( grid_layout );
  
  // a2ps
  grid_layout->addWidget( a2ps_checkbox_ = new QCheckBox( "Use a2ps command", this ) );
  grid_layout->addWidget(  a2ps_command_ = new CustomLineEdit( this ) );
  a2ps_checkbox_->setToolTip( "use a2ps to format raw input file" );

  // printer command
  QButtonGroup *group( new QButtonGroup( this ) );
  group->setExclusive( true );
  
  grid_layout->addWidget( printer_checkbox_ = new QRadioButton( "Use printer command", this ) );
  grid_layout->addWidget( print_command_ = new CustomLineEdit( this ) );
  group->addButton( printer_checkbox_ );
  printer_checkbox_->setToolTip( "print to a printer using specified command" );
  
  // file
  grid_layout->addWidget( file_checkbox_ = new QRadioButton( "Print to file", this ) );
  grid_layout->addWidget( ps_file_ = new BrowsedLineEdit( this ) );
  group->addButton( file_checkbox_ );
  file_checkbox_->setToolTip( "print to a postscript file (using a2ps)" );
  
  // stretch
  grid_layout->setColumnStretch( 1, 1 );
  
  // separator
  QFrame* frame( new QFrame( this ) );
  frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  mainLayout().addWidget( frame );
  
  // command
  mainLayout().addWidget( new QLabel( "command: ", this ) );
  mainLayout().addWidget( command_ = new CustomLineEdit( this ) );
  
  // connections
  connect( a2ps_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateCheckBoxes( bool ) ) );
  connect( printer_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateCheckBoxes( bool ) ) ); 
  connect( file_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateCheckBoxes( bool ) ) ); 

  connect( a2ps_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updatePrintCommand() ) );
  connect( printer_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updatePrintCommand() ) ); 
  connect( file_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updatePrintCommand() ) ); 
  
  connect( a2ps_command_, SIGNAL( textChanged( const QString& ) ), SLOT( _UpdatePrintCommand() ) );
  connect( print_command_, SIGNAL( textChanged( const QString& ) ), SLOT( _UpdatePrintCommand() ) );
  connect( &ps_file_->editor(), SIGNAL( textChanged( const QString& ) ), SLOT( _UpdatePrintCommand() ) );
  
  // initial state
  a2ps_checkbox_->setChecked( XmlOptions::get().get<bool>("USE_A2PS") );
  printer_checkbox_->setChecked( true );
  
  a2ps_command_->setText( XmlOptions::get().raw("A2PS_COMMAND").c_str() );  
  print_command_->setText( XmlOptions::get().raw("PRINT_COMMAND").c_str() );
  
}
  
//__________________________________________________
void PrintDialog::setFile( const File& file )
{
  Debug::Throw( "PrintDialog::setFile.\n" );
  file_ = file;
  File ps_file = file.truncatedName() + ".ps";
  ps_file_->editor().setText( ps_file.c_str() );
}

//__________________________________________________ 
void PrintDialog::_updateCheckBoxes( bool state )
{
 
  Debug::Throw( "PrintDialog::_UpdateCheckBoxes.\n" );
  if( a2ps_checkbox_->isChecked() )
  {
    
    file_checkbox_->setEnabled( true );
    ps_file_->setEnabled( true );
    
    printer_checkbox_->setEnabled( true );
    print_command_->setEnabled( true );
    
  } else {
    
    file_checkbox_->setEnabled( false );
    ps_file_->setEnabled( false );
    
    printer_checkbox_->setEnabled( true );
    printer_checkbox_->setChecked( true );
    print_command_->setEnabled( true );
    
  }
  
  a2ps_command_->setEnabled( a2ps_checkbox_->isChecked() );
  print_command_->setEnabled( printer_checkbox_->isChecked() );
  ps_file_->setEnabled( file_checkbox_->isChecked() );
  
}

//__________________________________________________ 
void PrintDialog::_updatePrintCommand( void )
{
 
  Debug::Throw( "PrintDialog::_updatePrintCommand.\n" );
  ostringstream what;
  if( a2ps_checkbox_->isChecked() )
  {
    if( file_checkbox_->isChecked() ) what << qPrintable( a2ps_command_->text() ) << " -o " << qPrintable( ps_file_->editor().text() ) << " " << file_;
    else what << qPrintable( a2ps_command_->text() ) << " -o- " << " " << file_ << " | " << qPrintable( print_command_->text() );
  } else what << qPrintable( print_command_->text() ) << " " << file_;
  
  command_->setText( what.str().c_str() );
  
}
