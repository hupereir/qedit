
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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License        
* for more details.                     
*                          
* You should have received a copy of the GNU General Public License along with 
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     
* Place, Suite 330, Boston, MA  02111-1307 USA                           
*                         
*                         
*******************************************************************************/
 
/*!
  \file DocumentClassConfiguration.cpp
  \brief document class configuration
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/


#include <QHeaderView>
#include <QLabel>
#include <QLayout>

#include "BrowsedLineEditor.h"
#include "CustomGridLayout.h"
#include "DocumentClassConfiguration.h"
#include "LineEditor.h"

using namespace std;

//____________________________________________________
DocumentClassConfiguration::DocumentClassConfiguration( QWidget* parent ):
  QGroupBox( "General", parent ),
  Counter( "DocumentClassConfiguration" )
{
  Debug::Throw( "DocumentClassConfiguration::DocumentClassConfiguration.\n" );

  CustomGridLayout* grid_layout = new CustomGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  setLayout( grid_layout );
  
  // file editor
  grid_layout->addWidget( new QLabel( "File: ", this ) );
  grid_layout->addWidget( file_editor_ = new BrowsedLineEditor( this ) );  

  // name editor
  grid_layout->addWidget( new QLabel( "Name: ", this ) );
  grid_layout->addWidget( name_editor_ = new LineEditor( this ) );  
  
  // icon editor
  grid_layout->addWidget( new QLabel( "Icon: ", this ) );
  grid_layout->addWidget( icon_editor_ = new LineEditor( this ) );  
  
  grid_layout->addWidget( new QLabel( "File name pattern: ", this ) );
  grid_layout->addWidget( file_pattern_editor_ = new LineEditor( this ) );
  file_pattern_editor_->setToolTip( "Regular expression used to determine document class from file name." );
  
  grid_layout->addWidget( new QLabel( "First line pattern: ", this ) );
  grid_layout->addWidget( first_line_pattern_editor_ = new LineEditor( this ) );
  first_line_pattern_editor_->setToolTip( "Regular expression used to determine document class from the first line of the file." );
  
  // base indentation
  grid_layout->addWidget( new QLabel( "Base indentation: ", this ) );
  grid_layout->addWidget( base_indentation_spinbox_ = new QSpinBox( this ) );
  base_indentation_spinbox_->setMinimum(0);
  base_indentation_spinbox_->setValue(0);
  
  // default
  grid_layout->addWidget( default_checkbox_ = new QCheckBox( "Default", this ) );
  default_checkbox_->setToolTip( "use this document class when no other is found that match a given file" );
  default_checkbox_->setChecked( false );

  // wrap mode
  grid_layout->addWidget( wrap_checkbox_ = new QCheckBox( "Wrap", this ) );
  wrap_checkbox_->setChecked( false );

}

//_________________________________________________________________________
void DocumentClassConfiguration::setDocumentClass( const DocumentClass& document_class )
{
  
  document_class_ = document_class;
  
  // name
  name_editor_->setText( document_class.name() );

  // icon
  icon_editor_->setText( document_class.icon() );
  
  // file editor
  file_editor_->editor().setText( document_class.file().c_str() );
  
  // file name matching pattern
  file_pattern_editor_->setText( document_class.fileMatchingPattern().pattern() );
  
  // first line matching pattern
  first_line_pattern_editor_->setText( document_class.firstLineMatchingPattern().pattern() );

  // base indentation
  base_indentation_spinbox_->setValue( document_class.baseIndentation() );
  
  // flags
  default_checkbox_->setChecked( document_class.isDefault() );
  wrap_checkbox_->setChecked( document_class.wrap() );
   
}
  
//_________________________________________________________________________
DocumentClass DocumentClassConfiguration::documentClass( void )
{
  document_class_.setName( name_editor_->text() );
  document_class_.setIcon( icon_editor_->text() );
  document_class_.setFile( File( qPrintable( file_editor_->editor().text() ) ) );
  document_class_.setFileMatchingPattern( file_pattern_editor_->text() );
  document_class_.setFirstLineMatchingPattern( first_line_pattern_editor_->text() );
  document_class_.setBaseIndentation( base_indentation_spinbox_->value() );
  document_class_.setWrap( wrap_checkbox_->isChecked() );
  document_class_.setIsDefault( default_checkbox_->isChecked() );
  return document_class_;
}
