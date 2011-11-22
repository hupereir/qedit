
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

#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>

#include "AnimatedLineEditor.h"
#include "BrowsedLineEditor.h"
#include "GridLayout.h"
#include "DocumentClassConfiguration.h"



//____________________________________________________
DocumentClassConfiguration::DocumentClassConfiguration( QWidget* parent ):
  QWidget( parent ),
  Counter( "DocumentClassConfiguration" )
{
  Debug::Throw( "DocumentClassConfiguration::DocumentClassConfiguration.\n" );

  setLayout( new QVBoxLayout() );
  layout()->setSpacing(5);
  layout()->setMargin(5);

  // flags
  QGroupBox* box;
  layout()->addWidget( box = new QGroupBox( "Class definition", this ) );

  GridLayout* grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
  box->setLayout( grid_layout );

  // file editor
  grid_layout->addWidget( new QLabel( "File: ", this ) );
  grid_layout->addWidget( file_editor_ = new BrowsedLineEditor( this ) );
  file_editor_->setAcceptMode( QFileDialog::AcceptSave );
  file_editor_->setFileMode( QFileDialog::AnyFile );

  // name editor
  grid_layout->addWidget( new QLabel( "Name: ", this ) );
  grid_layout->addWidget( name_editor_ = new AnimatedLineEditor( this ) );

  // icon editor
  grid_layout->addWidget( new QLabel( "Icon: ", this ) );
  grid_layout->addWidget( icon_editor_ = new AnimatedLineEditor( this ) );

  // patterns
  layout()->addWidget( box = new QGroupBox( "Matching patterns", this ) );

  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
  box->setLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "File name pattern: ", this ) );
  grid_layout->addWidget( file_pattern_editor_ = new AnimatedLineEditor( this ) );
  file_pattern_editor_->setToolTip( "Regular expression used to determine document class from file name." );

  grid_layout->addWidget( new QLabel( "First line pattern: ", this ) );
  grid_layout->addWidget( first_line_pattern_editor_ = new AnimatedLineEditor( this ) );
  first_line_pattern_editor_->setToolTip( "Regular expression used to determine document class from the first line of the file." );

  // options
  layout()->addWidget( box = new QGroupBox( "Options", this ) );

  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);

  // default
  box->layout()->addWidget( default_checkbox_ = new QCheckBox( "Default", box ) );
  default_checkbox_->setToolTip( "Use this document class when no other is found that match a given file" );
  default_checkbox_->setChecked( false );

  // wrap mode
  box->layout()->addWidget( wrap_checkbox_ = new QCheckBox( "Wrap", box ) );
  wrap_checkbox_->setChecked( false );

  // tab emulation
  box->layout()->addWidget( tab_emulation_checkbox_ = new QCheckBox( "Emulate tabs", box ) );
  tab_emulation_checkbox_->setChecked( false );

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setMargin(0);
  layout->setSpacing(5);
  box->layout()->addItem( layout );

  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(0);
  grid_layout->setMaxCount(2);
  grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
  layout->addLayout( grid_layout );
  layout->addStretch(1);

  QLabel* label = new QLabel( "Tab size: ", box );
  grid_layout->addWidget( label );

  tab_size_spinbox_ = new QSpinBox( box );
  tab_size_spinbox_->setMinimum( 0 );
  tab_size_spinbox_->setMaximum( 20 );
  tab_size_spinbox_->setToolTip(
    "Tab size (in unit of space characters).\n "
    "When zero value is specified, the default qedit tab size is used." );
  label->setBuddy( tab_size_spinbox_ );
  grid_layout->addWidget( tab_size_spinbox_ );

  // base indentation
  grid_layout->addWidget( new QLabel( "Base indentation: ", box ) );
  grid_layout->addWidget( base_indentation_spinbox_ = new QSpinBox( box ) );
  base_indentation_spinbox_->setMinimum(0);
  base_indentation_spinbox_->setValue(0);

}

//_________________________________________________________________________
void DocumentClassConfiguration::setDocumentClass( const DocumentClass& document_class )
{

  // name
  name_editor_->setText( document_class.name() );

  // icon
  icon_editor_->setText( document_class.icon() );

  // file editor
  file_editor_->editor().setText( document_class.file() );

  // file name matching pattern
  file_pattern_editor_->setText( document_class.fileMatchingPattern().pattern() );

  // first line matching pattern
  first_line_pattern_editor_->setText( document_class.firstLineMatchingPattern().pattern() );

  // base indentation
  base_indentation_spinbox_->setValue( document_class.baseIndentation() );

  // flags
  default_checkbox_->setChecked( document_class.isDefault() );
  wrap_checkbox_->setChecked( document_class.wrap() );
  tab_emulation_checkbox_->setChecked( document_class.emulateTabs() );
  tab_size_spinbox_->setValue( document_class.tabSize() );

}

//_________________________________________________________________________
DocumentClass DocumentClassConfiguration::documentClass( void )
{
  DocumentClass out;
  out.setName( name_editor_->text() );
  out.setIcon( icon_editor_->text() );
  out.setFile( file_editor_->editor().text() );
  out.setFileMatchingPattern( file_pattern_editor_->text() );
  out.setFirstLineMatchingPattern( first_line_pattern_editor_->text() );
  out.setBaseIndentation( base_indentation_spinbox_->value() );
  out.setWrap( wrap_checkbox_->isChecked() );
  out.setEmulateTabs( tab_emulation_checkbox_->isChecked() );
  out.setTabSize( tab_size_spinbox_->value() );
  out.setIsDefault( default_checkbox_->isChecked() );
  return out;
}
