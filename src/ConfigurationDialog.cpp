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

#include "ConfigurationDialog.h"

#include "Application.h"
#include "Config.h"
#include "FileList.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "GridLayout.h"
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

#include <QtGui/QLabel>
#include <QtGui/QGroupBox>

//_________________________________________________________
ConfigurationDialog::ConfigurationDialog( QWidget* parent ):
    BaseConfigurationDialog( parent )
{
    Debug::Throw( "ConfigurationDialog::ConfigurationDialog.\n" );
    setWindowTitle( "Configuration - Qedit" );

    // base configuration
    baseConfiguration();

    // generic objects
    QGroupBox *box;
    OptionCheckBox* checkbox;
    OptionSpinBox* spinbox;
    OptionColorDisplay* colorDisplay;

    // document classes
    QWidget* page = &addPage( "Document classes", "Document classes definitions and flags" );

    // edition flags
    page->layout()->addWidget( box = new QGroupBox( "Options", page ) );
    box->setLayout( new QVBoxLayout() );

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

    // edition
    page = &addPage( "Text Edition", "Settings for text edition" );

    // tab emulation
    // needs customization with respect to BaseConfigurationDialog in order
    // to disabled relevant widgets depending on whether tab emulation is read
    // from document class or not.
    {

        QGroupBox* box = new QGroupBox( "Tab emulation", page );
        QVBoxLayout* layout = new QVBoxLayout();
        box->setLayout( layout );
        page->layout()->addWidget( box );

        OptionCheckBox* checkbox;
        box->layout()->addWidget( checkbox = new OptionCheckBox( "Use document class tab emulation mode", box, "EMULATE_TABS_FROM_CLASS" ) );
        checkbox->setToolTip( "Use tab emulation mode read from document class in place of the one specified in the default configuration." );

        addOptionWidget( checkbox );

        // need a widget to contains following options,
        // so that they can be disabled all at once
        QWidget* widget = new QWidget( box );
        layout->addWidget( widget );
        widget->setLayout( layout = new QVBoxLayout() );
        layout->setMargin(0);

        // enable/disabled widget based on checkbox state
        connect( checkbox, SIGNAL( toggled( bool ) ), widget, SLOT( setDisabled( bool ) ) );

        // tab emulation
        checkbox = new OptionCheckBox( "Emulate tabs", box, "TAB_EMULATION" );
        checkbox->setToolTip( "Turn on/off tab emulation using space characters" );
        layout->addWidget( checkbox );
        addOptionWidget( checkbox );

        // tab size
        QHBoxLayout* h_layout = new QHBoxLayout();
        h_layout->setMargin(0);
        layout->addLayout( h_layout );

        h_layout->addWidget(new QLabel( "Tab size: ", box ) );
        OptionSpinBox* spinbox = new OptionSpinBox( box, "TAB_SIZE" );
        spinbox->setMinimum( 2 );
        spinbox->setMaximum( 20 );
        spinbox->setToolTip( "Tab size (in unit of space characters)." );
        h_layout->addWidget( spinbox );
        h_layout->addStretch( 1 );
        addOptionWidget( spinbox );

    }

    // default text-edition configuration
    textEditConfiguration( page, ALL_TEXT_EDITION & ~(TAB_EMULATION|TEXT_EDITION_FLAGS) );

    // wrap mode
    // needs customization with respect to BaseConfigurationDialog in order
    // to disabled relevant widgets depending on whether tab emulation is read
    // from document class or not.
    {

        // misc
        QGroupBox* box;
        page->layout()->addWidget( box = new QGroupBox( "Flags", page ) );
        QVBoxLayout* layout = new QVBoxLayout();
        box->setLayout( layout );

        box->layout()->addWidget( checkbox = new OptionCheckBox( "Use document class wrap mode", box, "WRAP_FROM_CLASS" ) );
        checkbox->setToolTip( "Use wrap mode read from document class in place of the one specified in the default configuration." );
        addOptionWidget( checkbox );

        OptionCheckBox* wrap_checkbox = new OptionCheckBox( "Wrap text", box, "WRAP_TEXT" );
        wrap_checkbox->setToolTip( "Turn on/off line wrapping at editor border" );
        layout->addWidget( wrap_checkbox );
        addOptionWidget( wrap_checkbox );

        connect( checkbox, SIGNAL( toggled( bool ) ), wrap_checkbox, SLOT( setDisabled( bool ) ) );

        layout->addWidget( checkbox = new OptionCheckBox( "Show line numbers", box, "SHOW_LINE_NUMBERS" ) );
        checkbox->setToolTip( "Turn on/off line numbers" );
        addOptionWidget( checkbox );

        QHBoxLayout* hLayout = new QHBoxLayout();
        layout->addLayout( hLayout );
        hLayout->setMargin(0);
        hLayout->addWidget( checkbox = new OptionCheckBox( "Auto-hide mouse cursor after ", box, "AUTOHIDE_CURSOR" ) );
        addOptionWidget( checkbox );

        QLabel* label;
        OptionSpinBox* spinbox;
        hLayout->addWidget( spinbox = new OptionSpinBox( box, "AUTOHIDE_CURSOR_DELAY" ) );
        hLayout->addWidget( label = new QLabel( " seconds", box ) );
        addOptionWidget( spinbox );

        spinbox->setMinimum( 0 );
        spinbox->setMaximum( 10 );

        spinbox->setEnabled( false );
        connect( checkbox, SIGNAL( toggled( bool ) ), spinbox, SLOT( setEnabled( bool ) ) );

    }

    // display
    page = &addPage( "Colors", "Text edition color settings" );

    // additional colors
    page->layout()->addWidget( box = new QGroupBox( "Colors", page ) );

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( gridLayout );

    gridLayout->addWidget( new QLabel( "Parenthesis matching: ", box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "PARENTHESIS_COLOR" ) );
    addOptionWidget( colorDisplay );
    checkbox->setToolTip( "Color for matching parenthesis: " );

    gridLayout->addWidget( new QLabel( "Tagged paragraphs: ", box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "TAGGED_BLOCK_COLOR" ) );
    addOptionWidget( colorDisplay );
    checkbox->setToolTip( "Color for tagged paragraphs" );

    gridLayout->addWidget( new QLabel( "Conflicting paragraphs: ", box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "DIFF_CONFLICT_COLOR" ) );
    addOptionWidget( colorDisplay );
    colorDisplay->setToolTip( "Highlight color for diff conflict paragraphs" );

    gridLayout->addWidget( new QLabel( "Added paragraphs: ", box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "DIFF_ADDED_COLOR" ) );
    addOptionWidget( colorDisplay );
    colorDisplay->setToolTip( "Highlight color for diff added paragraphs" );

    // multiple views
    page = &addPage( "Navigation", "Visible columns in navigation tabs" );

    QHBoxLayout *h_layout = new QHBoxLayout();
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
    RecentFilesFrame recentFrame(0, tmp );
    listview_config = new TreeViewConfiguration(
        page,
        &recentFrame.list(),
        recentFrame.list().maskOptionName() );
    listview_config->setTitle( "Recent files" );
    h_layout->addWidget( listview_config );
    addOptionWidget( listview_config );

    // multiple views
    page = &addPage( "Multiple views", "Multiple views configuration" );
    page->layout()->addWidget( box = new QGroupBox( page ) );

    gridLayout = new GridLayout();
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( gridLayout );

    OptionComboBox *combobox;

    // opening
    gridLayout->addWidget( new QLabel( "Default open mode: ", box ) );
    gridLayout->addWidget( combobox = new OptionComboBox( box, "OPEN_MODE" ) );
    combobox->addItem( WindowServer::MULTIPLE_WINDOWS );
    combobox->addItem( WindowServer::SINGLE_WINDOW );

    addOptionWidget( combobox );
    combobox->setToolTip( "Configure how new files are oppened." );

    // splitting
    gridLayout->addWidget( new QLabel( "Default view orientation: ", box ) );
    gridLayout->addWidget( combobox = new OptionComboBox( box, "ORIENTATION" ) );
    combobox->addItem( MainWindow::TOP_BOTTOM );
    combobox->addItem( MainWindow::LEFT_RIGHT );

    addOptionWidget( combobox );
    combobox->setToolTip( "Configure how new views are organized." );

    // splitting
    gridLayout->addWidget( new QLabel( "Default view orientation (diff mode): ", box ) );
    gridLayout->addWidget( combobox = new OptionComboBox( box, "DIFF_ORIENTATION" ) );
    combobox->addItem( MainWindow::TOP_BOTTOM );
    combobox->addItem( MainWindow::LEFT_RIGHT );

    addOptionWidget( combobox );
    combobox->setToolTip( "Configure how new views are organized in <i>diff</i> mode." );

    gridLayout->setColumnStretch( 1, 1 );

    // toolbars
    page = &addPage( "Toolbars", "Toolbars visibility and location" );
    page->layout()->addWidget( box = new QGroupBox( "Toolbars", page ) );

    gridLayout = new GridLayout();
    gridLayout->setMaxCount(2);
    box->setLayout( gridLayout );

    gridLayout->addWidget( new QLabel( "Visibility", box ) );
    gridLayout->addWidget( new QLabel( "Location", box ) );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Main Toolbar", box, "FILE_TOOLBAR" ) );
    gridLayout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "FILE_TOOLBAR_LOCATION" ) );
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    checkbox->setChecked( false );
    combobox->setEnabled( false );
    connect( checkbox, SIGNAL( toggled( bool ) ), combobox, SLOT( setEnabled( bool ) ) );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Edition Toolbar", box, "EDITION_TOOLBAR" ));
    gridLayout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EDITION_TOOLBAR_LOCATION" ));
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    checkbox->setChecked( false );
    combobox->setEnabled( false );
    connect( checkbox, SIGNAL( toggled( bool ) ), combobox, SLOT( setEnabled( bool ) ) );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Tools", box, "EXTRA_TOOLBAR" ));
    gridLayout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EXTRA_TOOLBAR_LOCATION" ));
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    checkbox->setChecked( false );
    combobox->setEnabled( false );
    connect( checkbox, SIGNAL( toggled( bool ) ), combobox, SLOT( setEnabled( bool ) ) );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Multiple views Toolbar", box, "SPLIT_TOOLBAR" ));
    gridLayout->addWidget( new CustomToolBar::LocationComboBox( box, "SPLIT_TOOLBAR_LOCATION" ));
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    checkbox->setChecked( false );
    combobox->setEnabled( false );
    connect( checkbox, SIGNAL( toggled( bool ) ), combobox, SLOT( setEnabled( bool ) ) );

    // printing
    page = &addPage( "Printing", "Commands used for printing" );
    page->layout()->addWidget( box = new QGroupBox( page ) );

    box->setLayout( new QVBoxLayout() );

    OptionListBox* listbox = new OptionListBox( box, "PRINT_COMMAND" );
    listbox->setBrowsable( true );
    addOptionWidget( listbox );
    listbox->setToolTip( "Available command for printing/editing converted files" );
    box->layout()->addWidget( listbox );

    // recent files
    page = &addPage( "Recent files", "Recent files list settings", true );
    RecentFilesConfiguration* recentFiles_configuration = new RecentFilesConfiguration( page, Singleton::get().application<Application>()->recentFiles() );
    page->layout()->addWidget( recentFiles_configuration );
    addOptionWidget( recentFiles_configuration );

    // misc
    page = &addPage( "Misc", "Additional unsorted settings" );

    // server
    SERVER::ServerConfiguration* server_configuration;
    page->layout()->addWidget( server_configuration = new SERVER::ServerConfiguration( page, "Server configuration" ));
    addOptionWidget( server_configuration );

    page->layout()->addWidget( box = new QGroupBox( "Backup and Autosave", page ) );

    box->setLayout( new QVBoxLayout() );

    box->layout()->addWidget( checkbox = new OptionCheckBox( "Make backup of files when saving modifications", box, "BACKUP" ) );
    checkbox->setToolTip( "Make backup of the file prior to saving modifications" );
    addOptionWidget( checkbox );

    box->layout()->addWidget( checkbox = new OptionCheckBox( "Save files automatically for crash recovery", box, "AUTOSAVE" ) );
    checkbox->setToolTip(
        "Make automatic copies of edited files in\n"
        "specified directory to allow crash recovery." );
    addOptionWidget( checkbox );

    gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->layout()->addItem( gridLayout );

    gridLayout->addWidget( new QLabel( "Autosave interval (seconds): ", box ) );
    gridLayout->addWidget( spinbox = new OptionSpinBox( box, "AUTOSAVE_INTERVAL" ) );
    spinbox->setMinimum( 1 );
    spinbox->setMaximum( 300 );
    spinbox->setToolTip( "Interval (seconds) between two autosave." );
    addOptionWidget( spinbox );

    OptionBrowsedLineEditor *edit;
    gridLayout->addWidget( new QLabel( "Autosave path: ", box ) );
    gridLayout->addWidget( edit = new OptionBrowsedLineEditor( box, "AUTOSAVE_PATH" ) );
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

    h_layout = new QHBoxLayout();
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
