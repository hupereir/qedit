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

#include "Application.h"
#include "Config.h"
#include "ConfigurationDialog.h"
#include "FileList.h"
#include "GridLayout.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "MainWindow.h"
#include "OptionListBox.h"
#include "OptionBrowsedLineEditor.h"
#include "OptionCheckBox.h"
#include "OptionColorDisplay.h"
#include "OptionFontInfo.h"
#include "OptionSpinBox.h"
#include "RecentFilesConfiguration.h"
#include "RecentFilesFrame.h"
#include "ServerConfiguration.h"
#include "SessionFilesFrame.h"
#include "Singleton.h"
#include "TreeView.h"
#include "TreeViewConfiguration.h"
#include "WindowServer.h"

using namespace std;

//_________________________________________________________
ConfigurationDialog::ConfigurationDialog( QWidget* parent ):
  BaseConfigurationDialog( parent )
{
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog.\n" );
  setWindowTitle( "Configuration - qedit" );
  
  // base configuration
  baseConfiguration();

  // generic objects
  QGroupBox *box;
  OptionCheckBox* checkbox;
  OptionSpinBox* spinbox;
  OptionColorDisplay* color_display;
  
  // document classes 
  QWidget* page = &addPage( "Document classes", "Document classes definitions and flags" );
  
  page->layout()->addWidget( box = new QGroupBox( "Document classes", page ) );  
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);
  
  box->layout()->addWidget( new QLabel( "Configuration files", box ) );
  OptionListBox* listbox = new OptionListBox( box, "PATTERN_FILENAME" );
  listbox->setBrowsable( true );
  box->layout()->addWidget( listbox );
  addOptionWidget( listbox );
  listbox->setToolTip( "Input files from which the document classes are read" );
  
  // edition flags
  page->layout()->addWidget( box = new QGroupBox( "Flags", page ) );  
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);

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

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Use document class wrap mode", box, "WRAP_FROM_CLASS" ) );
  checkbox->setToolTip( "Use wrap mode read from document class in place of the one specified in the default configuration." );
  addOptionWidget( checkbox );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Use document class tab emulation mode", box, "EMULATE_TABS_FROM_CLASS" ) );
  checkbox->setToolTip( "Use tab emulation mode read from document class in place of the one specified in the default configuration." );
  addOptionWidget( checkbox );

  // edition
  // page = &addPage( "Edition" );
  textEditConfiguration(); 

  // display
  page = &addPage( "Colors", "Text edition color settings" );
  
  // additional colors
  page->layout()->addWidget( box = new QGroupBox( "Colors", page ) );  
   
  GridLayout* grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
  box->setLayout( grid_layout );
  
  grid_layout->addWidget( new QLabel( "Parenthesis matching: ", box ) ); 
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "PARENTHESIS_COLOR" ) );
  addOptionWidget( color_display );
  checkbox->setToolTip( "Color for matching parenthesis: " );
  
  grid_layout->addWidget( new QLabel( "Tagged paragraphs: ", box ) ); 
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "TAGGED_BLOCK_COLOR" ) );
  addOptionWidget( color_display );
  checkbox->setToolTip( "Color for tagged paragraphs" );
  
  grid_layout->addWidget( new QLabel( "Conflicting paragraphs: ", box ) );
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "DIFF_CONFLICT_COLOR" ) );
  addOptionWidget( color_display );
  color_display->setToolTip( "Highlight color for diff conflict paragraphs" );

  grid_layout->addWidget( new QLabel( "Added paragraphs: ", box ) );
  grid_layout->addWidget( color_display = new OptionColorDisplay( box, "DIFF_ADDED_COLOR" ) );
  addOptionWidget( color_display );
  color_display->setToolTip( "Highlight color for diff added paragraphs" );
  
  // multiple views
  page = &addPage( "Navigation", "Visible columns in navigation tabs" );
  
  QHBoxLayout *h_layout = new QHBoxLayout();
  h_layout->setSpacing(5); 
  h_layout->setMargin(0);
  page->layout()->addItem( h_layout );
  
  SessionFilesFrame session_frame(0);
  TreeViewConfiguration *listview_config = new TreeViewConfiguration( 
    page, 
    &session_frame.list(), 
    session_frame.list().maskOptionName() );
  h_layout->addWidget( listview_config );
  listview_config->setTitle( "Session files" );
  addOptionWidget( listview_config );

  FileList tmp(0);
  RecentFilesFrame recent_frame(0, tmp );
  listview_config = new TreeViewConfiguration( 
    page, 
    &recent_frame.list(), 
    recent_frame.list().maskOptionName() );
  listview_config->setTitle( "Recent files" );
  h_layout->addWidget( listview_config );
  addOptionWidget( listview_config );

  // multiple views
  page = &addPage( "Multiple views", "Multiple views configuration" );
  page->layout()->addWidget( box = new QGroupBox( page ) );  

  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
  box->setLayout( grid_layout );
  
  OptionComboBox *combobox;
  
  // opening
  grid_layout->addWidget( new QLabel( "Default open mode: ", box ) );
  grid_layout->addWidget( combobox = new OptionComboBox( box, "OPEN_MODE" ) );
  combobox->addItem( WindowServer::MULTIPLE_WINDOWS );
  combobox->addItem( WindowServer::SINGLE_WINDOW );
  
  addOptionWidget( combobox );
  combobox->setToolTip( "Configure how new files are oppened." );
  
  // splitting
  grid_layout->addWidget( new QLabel( "Default splitting orientation: ", box ) );
  grid_layout->addWidget( combobox = new OptionComboBox( box, "ORIENTATION" ) );
  combobox->addItem( MainWindow::TOP_BOTTOM );
  combobox->addItem( MainWindow::LEFT_RIGHT );
  
  addOptionWidget( combobox );
  combobox->setToolTip( "Configure how new views are organized." );
  grid_layout->setColumnStretch( 1, 1 );
    
  // toolbars
  page = &addPage( "Toolbars", "Toolbars visibility and location" );
  page->layout()->addWidget( box = new QGroupBox( "Toolbars", page ) );  

  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "Visibility", box ) );
  grid_layout->addWidget( new QLabel( "Location", box ) );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Main Toolbar", box, "FILE_TOOLBAR" ) );
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "FILE_TOOLBAR_LOCATION" ) );
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  checkbox->setChecked( false );
  combobox->setEnabled( false );
  connect( checkbox, SIGNAL( toggled( bool ) ), combobox, SLOT( setEnabled( bool ) ) );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Edition Toolbar", box, "EDITION_TOOLBAR" ));
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EDITION_TOOLBAR_LOCATION" ));
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  checkbox->setChecked( false );
  combobox->setEnabled( false );
  connect( checkbox, SIGNAL( toggled( bool ) ), combobox, SLOT( setEnabled( bool ) ) );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Tools", box, "EXTRA_TOOLBAR" ));
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EXTRA_TOOLBAR_LOCATION" ));
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  checkbox->setChecked( false );
  combobox->setEnabled( false );
  connect( checkbox, SIGNAL( toggled( bool ) ), combobox, SLOT( setEnabled( bool ) ) );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Multiple views Toolbar", box, "SPLIT_TOOLBAR" ));
  grid_layout->addWidget( new CustomToolBar::LocationComboBox( box, "SPLIT_TOOLBAR_LOCATION" ));
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  checkbox->setChecked( false );
  combobox->setEnabled( false );
  connect( checkbox, SIGNAL( toggled( bool ) ), combobox, SLOT( setEnabled( bool ) ) );

  // printing
  page = &addPage( "Printing", "Commands used for printing" );
  page->layout()->addWidget( box = new QGroupBox( page ) );  

  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);

  listbox = new OptionListBox( box, "PRINT_COMMAND" );
  listbox->setBrowsable( true );
  addOptionWidget( listbox );
  listbox->setToolTip( "Available command for printing/editing converted files" );
  box->layout()->addWidget( listbox );
 
  // recent files
  page = &addPage( "Recent files", "Recent files list settings", true );
  RecentFilesConfiguration* recent_files_configuration = new RecentFilesConfiguration( page, Singleton::get().application<Application>()->recentFiles() );
  page->layout()->addWidget( recent_files_configuration );
  addOptionWidget( recent_files_configuration );

  // misc
  page = &addPage( "Misc", "Additional unsorted settings" );
  
  // server
  SERVER::ServerConfiguration* server_configuration;
  page->layout()->addWidget( server_configuration = new SERVER::ServerConfiguration( page, "Server configuration" ));
  addOptionWidget( server_configuration );

  page->layout()->addWidget( box = new QGroupBox( "Backup and Autosave", page ) );  

  box->setLayout( new QVBoxLayout() );
  box->layout()->setSpacing(5);
  box->layout()->setMargin(5);

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Make backup of files when saving modifications", box, "BACKUP" ) );
  checkbox->setToolTip( "Make backup of the file prior to saving modifications" );
  addOptionWidget( checkbox );
   
  box->layout()->addWidget( checkbox = new OptionCheckBox( "Save files automatically for crash recovery", box, "AUTOSAVE" ) );
  checkbox->setToolTip( 
    "Make automatic copies of edited files in\n"
    "specified directory to allow crash recovery." );
  addOptionWidget( checkbox );

  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(0);
  grid_layout->setMaxCount(2);
  grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
  box->layout()->addItem( grid_layout );

  grid_layout->addWidget( new QLabel( "Autosave interval (seconds): ", box ) );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "AUTOSAVE_INTERVAL" ) );
  spinbox->setMinimum( 1 );
  spinbox->setMaximum( 300 );
  spinbox->setToolTip( "Interval (seconds) between two autosave." );
  addOptionWidget( spinbox );
  
  OptionBrowsedLineEditor *edit;
  grid_layout->addWidget( new QLabel( "Autosave path: ", box ) );
  grid_layout->addWidget( edit = new OptionBrowsedLineEditor( box, "AUTOSAVE_PATH" ) );
  edit->setToolTip( "Directory where autosaved files are stored" );
  addOptionWidget( edit );

  checkbox->setChecked( false );
  spinbox->setEnabled( false );
  edit->setEnabled( false );
  connect( checkbox, SIGNAL( toggled( bool ) ), spinbox, SLOT( setEnabled( bool ) ) );
  connect( checkbox, SIGNAL( toggled( bool ) ), edit, SLOT( setEnabled( bool ) ) );

  // misc
  page->layout()->addWidget( box = new QGroupBox( "Misc", page ) );  

  box->setLayout( new QVBoxLayout() );
  box->layout()->setSpacing(5);
  box->layout()->setMargin(5);

  h_layout = new QHBoxLayout();
  h_layout->setSpacing(5); 
  h_layout->setMargin(0);
  box->layout()->addItem( h_layout );

  QLabel* label;
  h_layout->addWidget( label = new QLabel( "Diff command: ", box ) );
  h_layout->addWidget( edit = new OptionBrowsedLineEditor( box, "DIFF_COMMAND" ) );
  edit->setToolTip( "Command used to diff files" );
  addOptionWidget( edit );
  label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
  
  // load initial configuration
  _read();

}
