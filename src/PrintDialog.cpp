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
#include <QToolButton>
#include <QButtonGroup> 
#include <QGroupBox> 
#include <QLabel>

#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "CustomFileDialog.h"
#include "GridLayout.h"
#include "PrintDialog.h"
#include "QtUtil.h"

using namespace std;

//__________________________________________________
PrintDialog::PrintDialog( QWidget* parent ):
  CustomDialog( parent, OK_BUTTON | CANCEL_BUTTON )
{
  
  Debug::Throw( "PrintDialog::PrintDialog.\n" );
  setWindowTitle( "QEdit - print" );
  
  setOptionName( "PRINT_DIALOG" );
  
  QButtonGroup* group = new QButtonGroup( this );
  group->setExclusive( true );

  // destination  
  QGroupBox *box;
  box = new QGroupBox( "destination", this );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(10);
  box->layout()->setMargin(5);
  mainLayout().addWidget( box );  
  
  box->layout()->addWidget( pdf_checkbox_ = new QRadioButton( "Print to PDF file", box ) );
  group->addButton( pdf_checkbox_ );
  pdf_checkbox_->setChecked( true );

  box->layout()->addWidget( html_checkbox_ = new QRadioButton( "Print to HTML file", box ) );
  group->addButton( html_checkbox_ );
  html_checkbox_->setChecked( false );
  
  box->layout()->addWidget( new QLabel( "Destination file: ", box ) );
  box->layout()->addWidget( destination_editor_ = new BrowsedLineEditor( box ) );
  _destinationEditor().setMinimumSize( QSize( 150, 0 ) );
  
  // options
  box = new QGroupBox( "options", this );
  GridLayout* grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(10);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );
  mainLayout().addWidget( box );  
    
  grid_layout->addWidget( wrap_checkbox_ = new QCheckBox( "Wrap lines to maximum size:", box ) );
  wrap_checkbox_->setChecked( true );

  grid_layout->addWidget( maximum_line_size_ = new QSpinBox( box ) );
  maximum_line_size_->setMaximum( 1024) ;
  
  grid_layout->addWidget( command_checkbox_ = new QCheckBox( "Open/print with: ", box ) );
  command_checkbox_->setChecked( true );

  QHBoxLayout *h_layout = new QHBoxLayout();
  h_layout->setSpacing(2);
  h_layout->setMargin(0); 
  grid_layout->addLayout( h_layout );
  
  h_layout->addWidget( command_editor_ = new CustomComboBox( box ) );
  _commandEditor().setEditable( true );
  _commandEditor().setEditable( true );
  _commandEditor().setCaseSensitive( Qt::CaseSensitive );
  _commandEditor().setAutoCompletion( true );
  _commandEditor().setMinimumSize( QSize( 150, 0 ) );

  // browse command button associated to the CustomComboBox
  QToolButton* button = new QToolButton( box );
  button->setIcon( IconEngine::get( ICONS::OPEN ) );
  button->setAutoRaise( false );
  h_layout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( _browseCommand() ) );
  
  // connections
  connect( pdf_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateFile() ) );
  connect( html_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateFile() ) );
  connect( wrap_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateCheckBoxes() ) );
  connect( command_checkbox_, SIGNAL( toggled( bool ) ), SLOT( _updateCheckBoxes() ) );
  
  _updateFile();
  _updateCheckBoxes();
  
  // change button text
  okButton().setText( "&Print" );
  okButton().setIcon( IconEngine::get( ICONS::PRINT ));
}

//__________________________________________________
void PrintDialog::setFile( const File& file )
{
  Debug::Throw( "PrintDialog::setFile.\n" );
  _destinationEditor().editor().setText( file );
  _updateFile();
}

//__________________________________________________ 
void PrintDialog::_updateCheckBoxes( void )
{
  
  Debug::Throw( "PrintDialog::_updateCheckBoxes.\n" );
  maximum_line_size_->setEnabled( wrap_checkbox_->isChecked() );
  _commandEditor().setEnabled( command_checkbox_->isChecked() );
  
}

//__________________________________________________ 
void PrintDialog::_updateFile( void )
{
 
  Debug::Throw( "PrintDialog::_updateFile.\n" );
  
  File file( qPrintable( _destinationEditor().editor().text() ) );
  file = file.isEmpty() ? File("document"):file.truncatedName();
  if( pdf_checkbox_->isChecked() ) file += ".pdf";
  else if( html_checkbox_->isChecked() ) file += ".html";
  
  _destinationEditor().editor().setText( file );
  
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
  
  _commandEditor().setEditText( files.front() );
  _commandEditor().addItem( files.front() );
  
  return;

}
