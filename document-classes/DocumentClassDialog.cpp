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
  \file DocumentClassDialog.h
  \brief Syntax highlighting style editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QTabWidget>
#include <QPushButton>

#include "CustomDialog.h"
#include "CustomGridLayout.h"
#include "CustomLineEdit.h"
#include "DocumentClassDialog.h"
#include "HighlightStyleList.h"
#include "HighlightPatternList.h"
#include "ListWidget.h"
#include "TreeWidget.h"

using namespace std;

//__________________________________________________________
DocumentClassDialog::DocumentClassDialog( QWidget* parent ):
  CustomDialog( parent )
{
  
  Debug::Throw( "DocumentClassDialog::DocumentClassDialog.\n" );
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(0);
  mainLayout().addLayout( layout );
  
  // name editor
  layout->addWidget( new QLabel( "Name: ", this ) );
  layout->addWidget( name_editor_ = new CustomLineEdit( this ) );
  
  // tab widget
  QTabWidget *tab_widget = new QTabWidget( this );
  mainLayout().addWidget( tab_widget );
  
  // box to display additinal information
  QWidget *box;
  
  // pattern matching
  tab_widget->addTab( box = new QWidget(), "&File pattern" );
  
  QVBoxLayout* v_layout;
  v_layout = new QVBoxLayout();
  v_layout->setSpacing(5);
  v_layout->setMargin(10);
  box->setLayout( v_layout );
  
  v_layout->addWidget( box = new QWidget( box ) );
  v_layout->addStretch(1);
  
  CustomGridLayout* grid_layout = new CustomGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(0);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );
  
  grid_layout->addWidget( new QLabel( "File name matching pattern: ", box ) );
  grid_layout->addWidget( file_pattern_editor_ = new CustomLineEdit( box ) );
  file_pattern_editor_->setToolTip( "Regular expression used to determine document class from file name." );
  
  grid_layout->addWidget( new QLabel( "First line matching pattern: ", box ) );
  grid_layout->addWidget( first_line_pattern_editor_ = new CustomLineEdit( box ) );
  first_line_pattern_editor_->setToolTip( "Regular expression used to determine document class from the first line of the file." );
  
  // need to add options (checkboxes) for "wrap" and "default" 
  
  // highlight styles
  tab_widget->addTab( highlight_style_list_ = new HighlightStyleList(), "&Highlight styles" );

  // highlight patterns
  tab_widget->addTab( highlight_pattern_list_ = new HighlightPatternList(), "&Highlight patterns" );
  
  adjustSize();
  resize( 550, 500 );
  
}

//___________________________________________________________________________________
void DocumentClassDialog::setDocumentClass( const DocumentClass& document_class )
{
  
  Debug::Throw( "DocumentClassDialog::setDocumentClass.\n" );
  
  // name
  name_editor_->setText( document_class.name() );
  
  // file name matching pattern
  file_pattern_editor_->setText( document_class.fileMatchingPattern().pattern() );
  
  // first line matching pattern
  first_line_pattern_editor_->setText( document_class.firstLineMatchingPattern().pattern() );
    
  // highlight styles
  highlight_style_list_->setStyles( document_class.highlightStyles() );
  
  // highlight patterns
  highlight_pattern_list_->setStyles( document_class.highlightStyles() );
  highlight_pattern_list_->setPatterns( document_class.highlightPatterns() );
  
  // indentation patterns
  
}
