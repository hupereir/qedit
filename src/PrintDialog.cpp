
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
  \file PrintDialog.cpp
  \brief print document
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QFrame>
#include <QLayout>
#include <QButtonGroup> 
#include <QGroupBox> 

#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "PrintDialog.h"
#include "CustomFileDialog.h"
#include "CustomGridLayout.h"

using namespace std;

//__________________________________________________
PrintDialog::PrintDialog( QWidget* parent ):
  CustomDialog( parent, OK_BUTTON | CANCEL_BUTTON )
{
  
  Debug::Throw( "PrintDialog::PrintDialog.\n" );
  setWindowTitle( "QEdit - print" );
  
  QButtonGroup* group = new QButtonGroup( this );
  group->setExclusive( true );

  // destination  
  QGroupBox *box;
  box = new QGroupBox( "destination", this );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(10);
  box->layout()->setMargin(5);
  mainLayout().addWidget( box );  
  
  QHBoxLayout *h_layout;
  h_layout = new QHBoxLayout();
  h_layout->setSpacing(5);
  h_layout->setMargin(0); 
  box->layout()->addItem( h_layout );

  h_layout->addWidget( pdf_checkbox_ = new QRadioButton( "Print to PDF file", box ) );
  group->addButton( pdf_checkbox_ );
  pdf_checkbox_->setChecked( true );

  h_layout->addWidget( html_checkbox_ = new QRadioButton( "Print to HTML file", box ) );
  group->addButton( html_checkbox_ );
  html_checkbox_->setChecked( false );
  
  box->layout()->addWidget( new QLabel( "Destination file: ", box ) );
  box->layout()->addWidget( destination_ = new BrowsedLineEditor( box ) );
  destination_->setMinimumSize( QSize( 350, 0 ) );
  
  // options
  box = new QGroupBox( "options", this );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(10);
  box->layout()->setMargin(5);
  mainLayout().addWidget( box );  

  h_layout = new QHBoxLayout();
  h_layout->setSpacing(5);
  h_layout->setMargin(0); 
  box->layout()->addItem( h_layout );
  
  h_layout->addWidget( wrap_checkbox_ = new QCheckBox( "Wrap lines to maximum size:", box ) );
  wrap_checkbox_->setChecked( true );

  h_layout->addWidget( maximum_line_size_ = new QSpinBox( box ) );
  maximum_line_size_->setMaximum( 1024) ;
  
  box->layout()->addWidget( command_checkbox_ = new QCheckBox( "Open/Print destination file with command: ", box ) );
  command_checkbox_->setChecked( true );

  h_layout = new QHBoxLayout();
  h_layout->setSpacing(5);
  h_layout->setMargin(0); 
  box->layout()->addItem( h_layout );
  h_layout->addWidget( command_ = new CustomComboBox( box ) );
  command_->setEditable( true );
  command_->setEditable( true );
  command_->setCaseSensitive( Qt::CaseSensitive );
  command_->setAutoCompletion( true );
  command_->setMinimumSize( QSize( 350, 0 ) );

  // browse command button associated to the CustomComboBox
  QPushButton* button = new QPushButton( box );
  button->setIcon( IconEngine::get( ICONS::OPEN ) );
  h_layout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( _browseCommand() ) );
  
  // connections
  connect( pdf_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateFile() ) );
  connect( html_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateFile() ) );
  connect( wrap_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateCheckBoxes() ) );
  connect( command_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateCheckBoxes() ) );
  
  _updateFile();
  _updateCheckBoxes();
  
}

//__________________________________________________
void PrintDialog::setFile( const File& file )
{
  Debug::Throw( "PrintDialog::setFile.\n" );
  destination_->editor().setText( file.c_str() );
  _updateFile();
}

//__________________________________________________ 
void PrintDialog::_updateCheckBoxes( void )
{
  
  Debug::Throw( "PrintDialog::_updateCheckBoxes.\n" );
  maximum_line_size_->setEnabled( wrap_checkbox_->isChecked() );
  command_->setEnabled( command_checkbox_->isChecked() );
  
}

//__________________________________________________ 
void PrintDialog::_updateFile( void )
{
 
  Debug::Throw( "PrintDialog::_updateFile.\n" );
  
  File file( qPrintable( destination_->editor().text() ) );
  file = file.empty() ? File("document"):file.truncatedName();
  if( pdf_checkbox_->isChecked() ) file += ".pdf";
  else if( html_checkbox_->isChecked() ) file += ".html";
  
  destination_->editor().setText( file.c_str() );
  
}

//__________________________________________________ 
void PrintDialog::_browseCommand( void )
{
 
  Debug::Throw( "PrintDialog::_browseCommand.\n" );
  
  // open FileDialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::ExistingFile );
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;
  
  // check selected files
  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return;
  
  command_->setEditText( files.front() );
  command_->addItem( files.front() );
  
  return;

}
