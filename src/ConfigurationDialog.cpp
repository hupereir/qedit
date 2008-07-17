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
  \file ConfigurationDialog.cpp
  \brief xMaze configuration dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QGroupBox>

#include "Config.h"
#include "ConfigurationDialog.h"
#include "CustomGridLayout.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "OptionListBox.h"
#include "OptionBrowsedLineEditor.h"
#include "OptionCheckBox.h"
#include "OptionColorDisplay.h"
#include "OptionFontInfo.h"
#include "OptionSpinBox.h"

// forward declaration
void installDefaultOptions( void );
void installSystemOptions( void );

using namespace std;

//_________________________________________________________
ConfigurationDialog::ConfigurationDialog( QWidget* parent ):
  BaseConfigurationDialog( parent )
{
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog.\n" );

  // base configuration
  baseConfiguration();

  // generic objects
  QGroupBox *box;
  OptionCheckBox* checkbox;
  OptionSpinBox* spinbox;
  OptionColorDisplay* color_display;
  
  // document classes 
  QWidget* page = &addPage( "Document classes" );
  
  page->layout()->addWidget( box = new QGroupBox( "Document classes", page ) );  
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);
  
  box->layout()->addWidget( new QLabel( "Configuration files", box ) );
  OptionListBox* listbox = new OptionListBox( box, "PATTERN_FILENAME" );
  listbox->setBrowsable( true );
  box->layout()->addWidget( listbox );
  addOptionWidget( listbox );
  listbox->setToolTip( "input files from which the document classes are read" );
  
  // edition flags
  page->layout()->addWidget( box = new QGroupBox( "Flags", page ) );  
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Show line numbers", box, "SHOW_LINE_NUMBERS" ) );
  checkbox->setToolTip( "Turn on/off line numbers" );
  addOptionWidget( checkbox );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Show block delimiters", box, "SHOW_BLOCK_DELIMITERS" ) );
  checkbox->setToolTip( "Turn on/off block delimiters" );
  addOptionWidget( checkbox );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Highlight syntax", box, "TEXT_HIGHLIGHT" ) );
  checkbox->setToolTip( "Turn on/off syntax highlighting" );
  addOptionWidget( checkbox );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Highlight parenthesis", box, "TEXT_PARENTHESIS" ) );
  checkbox->setToolTip( "Turn on/off highlighting of oppening/closing parenthesis" );
  addOptionWidget( checkbox );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Automatic indentation", box, "TEXT_INDENT" ) );
  checkbox->setToolTip( "Turn on/off automatic text indentation" );
  addOptionWidget( checkbox );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "use document class wrap mode", box, "WRAP_FROM_CLASS" ) );
  checkbox->setToolTip( "Use wrap mode read from document class in place of the one specified in the default configuration." );
  addOptionWidget( checkbox );

  // edition
  page = &addPage( "Edition" );
  textEditConfiguration( page ); 

  // display
  page = &addPage( "Colors" );
  
  // additional colors
  page->layout()->addWidget( box = new QGroupBox( "Colors", page ) );  
   
  CustomGridLayout* grid_layout = new CustomGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );
  
  grid_layout->addWidget( new QLabel( "Parenthesis matching color", box ) ); 
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "PARENTHESIS_COLOR" ) );
  addOptionWidget( color_display );
  checkbox->setToolTip( "Color for matching parenthesis" );

  grid_layout->addWidget( new QLabel( "Tagged paragraphs color", box ) ); 
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "TAGGED_BLOCK_COLOR" ) );
  addOptionWidget( color_display );
  checkbox->setToolTip( "Color for tagged paragraphs" );
  
  grid_layout->addWidget( new QLabel( "Conflict paragraph highlight color", box ) );
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "DIFF_CONFLICT_COLOR" ) );
  addOptionWidget( color_display );
  color_display->setToolTip( "highlight color for diff conflict paragraphs" );

  grid_layout->addWidget( new QLabel( "Added paragraph highlight color", box ) );
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "DIFF_ADDED_COLOR" ) );
  addOptionWidget( color_display );
  color_display->setToolTip( "highlight color for diff added paragraphs" );
  
  grid_layout->addWidget( new QLabel( "Block delimiter foreground", box ) );
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "DELIMITER_FOREGROUND" ) );
  addOptionWidget( color_display );
  color_display->setToolTip( "foreground color for block delimiters and line numbers" );

  grid_layout->addWidget( new QLabel( "Block delimiter background", box ) );
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "DELIMITER_BACKGROUND" ) );
  addOptionWidget( color_display );
  color_display->setToolTip( "background color for block delimiters and line numbers" );

  // multiple views
  page = &addPage( "Multiple views" );
  page->layout()->addWidget( box = new QGroupBox( page ) );  

  grid_layout = new CustomGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );
  
  grid_layout->addWidget( new QLabel( "Default open mode ", box ) );
  OptionComboBox *combobox;
  grid_layout->addWidget( combobox = new OptionComboBox( box, "OPEN_MODE" ) );
  combobox->addItem( "open in new window" );
  combobox->addItem( "open in new view" );
  addOptionWidget( combobox );
  combobox->setToolTip( "Configure how new files should be open." );

  grid_layout->addWidget( new QLabel( "Default orientation ", box ) );
  grid_layout->addWidget( combobox = new OptionComboBox( box, "ORIENTATION" ) );
  combobox->addItem( "top/bottom" );
  combobox->addItem( "left/right" );
  addOptionWidget( combobox );
  combobox->setToolTip( "Configure how new views are organized." );
  grid_layout->setColumnStretch( 1, 1 );
    
  // toolbars
  page = &addPage( "Toolbars" );
  page->layout()->addWidget( box = new QGroupBox( "Toolbars", page ) );  

  grid_layout = new CustomGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "Visibility", box ) );
  grid_layout->addWidget( new QLabel( "Location", box ) );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Main toolbar", box, "FILE_TOOLBAR" ) );
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "FILE_TOOLBAR_LOCATION" ) );
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Edition toolbar", box, "EDITION_TOOLBAR" ));
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EDITION_TOOLBAR_LOCATION" ));
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Tools", box, "EXTRA_TOOLBAR" ));
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EXTRA_TOOLBAR_LOCATION" ));
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Multiple views toolbar", box, "SPLIT_TOOLBAR" ));
  grid_layout->addWidget( new CustomToolBar::LocationComboBox( box, "SPLIT_TOOLBAR_LOCATION" ));
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  // printing
  page = &addPage( "Printing" );
  page->layout()->addWidget( box = new QGroupBox( "Printing", page ) );  

  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);

  box->layout()->addWidget( new QLabel("Printing/editing commands: ", box ) );
  listbox = new OptionListBox( box, "PRINT_COMMAND" );
  listbox->setBrowsable( true );
  addOptionWidget( listbox );
  listbox->setToolTip( "Available command for printing/editing converted files" );
  box->layout()->addWidget( listbox );

  // misc
  page = &addPage( "Misc" );
  page->layout()->addWidget( box = new QGroupBox( "Backup and Autosave", page ) );  
   
  grid_layout = new CustomGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Save backup", box, "BACKUP" ), 0, 0, 1, 2 );
  checkbox->setToolTip( "Make backup of the file prior to saving modifications" );
  addOptionWidget( checkbox );

  grid_layout->addWidget( new QLabel( "autosave interval (seconds)", box ) );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "AUTOSAVE_INTERVAL" ) );
  spinbox->setMinimum( 0 );
  spinbox->setMaximum( 300 );
  spinbox->setToolTip( "interval (seconds) between two autosave. 0 means no autosave." );
  addOptionWidget( spinbox );
  
  OptionBrowsedLineEditor *edit;
  grid_layout->addWidget( new QLabel( "autosave path", box ) );
  grid_layout->addWidget( edit = new OptionBrowsedLineEditor( box, "AUTOSAVE_PATH" ) );
  edit->setToolTip( "directory when autosaved files are stored" );
  addOptionWidget( edit );

  // misc
  page->layout()->addWidget( box = new QGroupBox( "Misc", page ) );  
   
  grid_layout = new CustomGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "main window width", box ) );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "WINDOW_WIDTH" ) );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  addOptionWidget( spinbox );

  grid_layout->addWidget( new QLabel( "main window height", box ) );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "WINDOW_HEIGHT" ) );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  addOptionWidget( spinbox );
  
  // previous file history size
  grid_layout->addWidget( new QLabel( "previous file history size", box ) );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "DB_SIZE" ) );
  spinbox->setMinimum( 0 );
  spinbox->setMaximum( 100 );
  addOptionWidget( spinbox );
  spinbox->setToolTip( "number of previously opened files to appear in the Open Previous menu" );

  // sort previous files by date
  grid_layout->addWidget( checkbox = new OptionCheckBox( "sort previous files by date", box, "SORT_FILES_BY_DATE" ), 3, 0, 1, 2 );
  checkbox->setToolTip( "Sort files by date rather than name in Open Previous menu." );
  addOptionWidget( checkbox );
  new QWidget( box );
    
  // load initial configuration
  _read();
  adjustSize();

}

//________________________________________________________________________________
void ConfigurationDialog::_restoreDefaults( void )
{
  
  Debug::Throw( "ConfigurationDialog::restoreDefaults.\n" );
  
  // reset options
  XmlOptions::get() = Options();
  
  // reinstall default options
  installDefaultOptions();
  installSystemOptions();
  
  // read everything in dialog
  _read();
  
}
