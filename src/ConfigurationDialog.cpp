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

#include "ConfigurationDialog.h"

#include "Application.h"
#include "BaseFileInfoConfigurationWidget.h"
#include "Config.h"
#include "FileList.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "GridLayout.h"
#include "IconEngine.h"
#include "Icons.h"
#include "MainWindow.h"
#include "OptionListBox.h"
#include "OptionBrowsedLineEditor.h"
#include "OptionCheckBox.h"
#include "OptionColorDisplay.h"
#include "OptionFontInfo.h"
#include "OptionSpinBox.h"
#include "RecentFilesConfiguration.h"
#include "RecentFilesFrame.h"
#include "SessionFilesFrame.h"
#include "Singleton.h"
#include "TreeView.h"
#include "TreeViewConfiguration.h"
#include "WindowServer.h"

#include <QLabel>
#include <QGroupBox>

//_________________________________________________________
ConfigurationDialog::ConfigurationDialog( QWidget* parent ):
    BaseConfigurationDialog( parent )
{
    Debug::Throw( "ConfigurationDialog::ConfigurationDialog.\n" );
    setWindowTitle( tr( "Configuration - Qedit" ) );

    // base configuration
    baseConfiguration();

    // generic objects
    QWidget *box;
    OptionCheckBox* checkbox;
    OptionSpinBox* spinbox;
    OptionColorDisplay* colorDisplay;

    // document classes
    QWidget* page = &addPage( IconEngine::get( ICONS::PREFERENCE_FILE_TYPES ), tr( "Document Classes" ), tr( "Document classes options" ) );

    // edition flags
    page->layout()->addWidget( box = new QWidget( page ) );
    box->setLayout( new QVBoxLayout() );
    box->layout()->setMargin(0);

    box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Show block delimiters" ), box, "SHOW_BLOCK_DELIMITERS" ) );
    checkbox->setToolTip( tr( "Turn on/off block delimiters" ) );
    addOptionWidget( checkbox );

    box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Highlight syntax" ), box, "TEXT_HIGHLIGHT" ) );
    checkbox->setToolTip( tr( "Turn on/off syntax highlighting" ) );
    addOptionWidget( checkbox );

    box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Highlight parenthesis" ), box, "TEXT_PARENTHESIS" ) );
    checkbox->setToolTip( tr( "Turn on/off highlighting of oppening/closing parenthesis" ) );
    addOptionWidget( checkbox );

    box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Automatic indentation" ), box, "TEXT_INDENT" ) );
    checkbox->setToolTip( tr( "Turn on/off automatic text indentation" ) );
    addOptionWidget( checkbox );

    // edition
    page = &addPage( IconEngine::get( ICONS::PREFERENCE_EDITION ), tr( "Text Edition" ), tr( "Settings for text edition" ) );

    // tab emulation
    // needs customization with respect to BaseConfigurationDialog in order
    // to disabled relevant widgets depending on whether tab emulation is read
    // from document class or not.
    {

        QGroupBox* box = new QGroupBox( tr( "Tab Emulation" ), page );
        QVBoxLayout* layout = new QVBoxLayout();
        box->setLayout( layout );
        page->layout()->addWidget( box );

        OptionCheckBox* checkbox;
        box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Use document class tab emulation mode" ), box, "EMULATE_TABS_FROM_CLASS" ) );
        checkbox->setToolTip( tr( "Use tab emulation mode read from document class in place of the one specified in the default configuration" ) );

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
        checkbox = new OptionCheckBox( tr( "Emulate tabs" ), box, "TAB_EMULATION" );
        checkbox->setToolTip( tr( "Turn on/off tab emulation using space characters" ) );
        layout->addWidget( checkbox );
        addOptionWidget( checkbox );

        // tab size
        QHBoxLayout* hLayout = new QHBoxLayout();
        hLayout->setMargin(0);
        layout->addLayout( hLayout );

        hLayout->addWidget(new QLabel( tr( "Tab size: " ), box ) );
        OptionSpinBox* spinbox = new OptionSpinBox( box, "TAB_SIZE" );
        spinbox->setMinimum( 2 );
        spinbox->setMaximum( 20 );
        spinbox->setToolTip( tr( "Tab size (in unit of space characters)." ) );
        hLayout->addWidget( spinbox );
        hLayout->addStretch( 1 );
        addOptionWidget( spinbox );

    }

    // default text-edition configuration
    textEditConfiguration( page, ALL_TEXT_EDITION & ~(TAB_EMULATION|TEXT_EDITION_FLAGS) );

    // wrap mode
    // needs customization with respect to BaseConfigurationDialog in order
    // to disabled relevant widgets depending on whether tab emulation is read
    // from document class or not.
    {

        QGroupBox* box;
        page->layout()->addWidget( box = new QGroupBox( tr( "Options" ), page ) );
        QVBoxLayout* layout = new QVBoxLayout();
        box->setLayout( layout );

        box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Use document class wrap mode" ), box, "WRAP_FROM_CLASS" ) );
        checkbox->setToolTip( tr( "Use wrap mode read from document class in place of the one specified in the default configuration." ) );
        addOptionWidget( checkbox );

        OptionCheckBox* wrap_checkbox = new OptionCheckBox( tr( "Wrap text" ), box, "WRAP_TEXT" );
        wrap_checkbox->setToolTip( tr( "Turn on/off line wrapping at editor border" ) );
        layout->addWidget( wrap_checkbox );
        addOptionWidget( wrap_checkbox );

        connect( checkbox, SIGNAL( toggled( bool ) ), wrap_checkbox, SLOT( setDisabled( bool ) ) );

        layout->addWidget( checkbox = new OptionCheckBox( tr( "Show line numbers" ), box, "SHOW_LINE_NUMBERS" ) );
        checkbox->setToolTip( tr( "Turn on/off line numbers" ) );
        addOptionWidget( checkbox );

        QHBoxLayout* hLayout = new QHBoxLayout();
        layout->addLayout( hLayout );
        hLayout->setMargin(0);
        hLayout->addWidget( checkbox = new OptionCheckBox( tr( "Auto-hide mouse cursor after " ), box, "AUTOHIDE_CURSOR" ) );
        addOptionWidget( checkbox );

        OptionSpinBox* spinbox;
        hLayout->addWidget( spinbox = new OptionSpinBox( box, "AUTOHIDE_CURSOR_DELAY" ) );
        spinbox->setSuffix( "s" );
        addOptionWidget( spinbox );

        spinbox->setMinimum( 0 );
        spinbox->setMaximum( 10 );

        spinbox->setEnabled( false );
        connect( checkbox, SIGNAL( toggled( bool ) ), spinbox, SLOT( setEnabled( bool ) ) );

    }

    // display
    page = &addPage( IconEngine::get( ICONS::PREFERENCE_COLORS ), tr( "Colors" ), tr( "Text edition color settings" ) );

    // additional colors
    page->layout()->addWidget( box = new QWidget( page ) );

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMaxCount(2);
    gridLayout->setMargin(0);
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( gridLayout );

    gridLayout->addWidget( new QLabel( tr( "Parenthesis matching:" ), box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "PARENTHESIS_COLOR" ) );
    addOptionWidget( colorDisplay );
    checkbox->setToolTip( tr( "Color for matching parenthesis" ) );

    gridLayout->addWidget( new QLabel( tr( "Tagged paragraphs:" ), box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "TAGGED_BLOCK_COLOR" ) );
    addOptionWidget( colorDisplay );
    checkbox->setToolTip( tr( "Color for tagged paragraphs" ) );

    gridLayout->addWidget( new QLabel( tr( "Conflicting paragraphs:" ), box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "DIFF_CONFLICT_COLOR" ) );
    addOptionWidget( colorDisplay );
    colorDisplay->setToolTip( tr( "Highlight color for diff conflict paragraphs" ) );

    gridLayout->addWidget( new QLabel( tr( "Added paragraphs:" ), box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "DIFF_ADDED_COLOR" ) );
    addOptionWidget( colorDisplay );
    colorDisplay->setToolTip( tr( "Highlight color for diff added paragraphs" ) );

    // navigation
    QHBoxLayout *hLayout;

    // multiple views
    page = &addPage( IconEngine::get( ICONS::PREFERENCE_MULTIPLE_VIEWS ), tr( "Multiple Views" ), tr( "Multiple views configuration" ) );
    page->layout()->addWidget( box = new QWidget( page ) );

    gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( gridLayout );

    OptionComboBox *combobox;

    // opening
    gridLayout->addWidget( new QLabel( tr( "Default open mode:" ), box ) );
    gridLayout->addWidget( combobox = new OptionComboBox( box, tr( "OPEN_MODE" ) ) );
    combobox->setUseValue( false );
    combobox->addItem( tr( "Open in Current Windoow" ) );
    combobox->addItem( tr( "Open in New Window" ) );

    addOptionWidget( combobox );
    combobox->setToolTip( tr( "Configure how new files are oppened." ) );

    // splitting
    gridLayout->addWidget( new QLabel( tr( "Default view orientation:" ), box ) );
    gridLayout->addWidget( combobox = new OptionComboBox( box, "ORIENTATION" ) );
    combobox->setUseValue( false );
    combobox->addItem( tr( "Top to Bottom" ) );
    combobox->addItem( tr( "Left to Right" ) );

    addOptionWidget( combobox );
    combobox->setToolTip( tr( "Configure how new views are organized" ) );

    // splitting
    gridLayout->addWidget( new QLabel( tr( "Default view orientation (diff mode):" ), box ) );
    gridLayout->addWidget( combobox = new OptionComboBox( box, "DIFF_ORIENTATION" ) );
    combobox->setUseValue( false );
    combobox->addItem( tr( "Top to Bottom" ) );
    combobox->addItem( tr( "Left to Right" ) );

    addOptionWidget( combobox );
    combobox->setToolTip( tr( "Configure how new views are organized in <i>diff</i> mode" ) );
    gridLayout->setColumnStretch( 1, 1 );

    // recent files
    page = &addPage( IconEngine::get( ICONS::PREFERENCE_RECENT_FILES ), tr( "Recent Files" ), tr( "Recent files list settings" ), true );
    RecentFilesConfiguration* recentFiles_configuration = new RecentFilesConfiguration( page, Singleton::get().application<Application>()->recentFiles() );
    page->layout()->addWidget( recentFiles_configuration );
    addOptionWidget( recentFiles_configuration );

    // tooltips
    page = &addPage( IconEngine::get( ICONS::PREFERENCE_APPEARANCE), tr( "Tooltips" ), tr( "Tooltips appearance" ) );
    page->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Show tooltips" ), page, "SHOW_TOOLTIPS" ) );
    addOptionWidget( checkbox );

    hLayout = new QHBoxLayout();
    hLayout->setMargin(5);
    hLayout->setSpacing(5);
    page->layout()->addItem( hLayout );

    BaseFileInfoConfigurationWidget* configurationWidget;
    hLayout->addSpacing( 20 );
    hLayout->addWidget( configurationWidget = new BaseFileInfoConfigurationWidget( page ) );
    configurationWidget->setEnabled( false );
    connect( checkbox, SIGNAL( toggled( bool ) ), configurationWidget, SLOT( setEnabled( bool ) ) );
    addOptionWidget( configurationWidget );

    // misc
    page = &addPage( IconEngine::get( ICONS::PREFERENCE_UNSORTED ), tr( "Unsorted" ), tr( "Additional unsorted settings" ) );
    page->layout()->addWidget( box = new QGroupBox( tr( "Backup and Autosave" ), page ) );

    box->setLayout( new QVBoxLayout() );

    box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Make backup of files when saving modifications" ), box, "BACKUP" ) );
    checkbox->setToolTip( tr( "Make backup of the file prior to saving modifications" ) );
    addOptionWidget( checkbox );

    box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Save files automatically for crash recovery" ), box, "AUTOSAVE" ) );
    checkbox->setToolTip(
        tr( "Make automatic copies of edited files in\n"
        "specified directory to allow crash recovery." ) );
    addOptionWidget( checkbox );

    gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->layout()->addItem( gridLayout );

    gridLayout->addWidget( new QLabel( tr( "Autosave interval:" ), box ) );
    gridLayout->addWidget( spinbox = new OptionSpinBox( box, "AUTOSAVE_INTERVAL" ) );
    spinbox->setSuffix( "s" );
    spinbox->setMinimum( 1 );
    spinbox->setMaximum( 300 );
    spinbox->setToolTip( tr( "Interval between two autosave" ) );
    addOptionWidget( spinbox );

    OptionBrowsedLineEditor *edit;
    gridLayout->addWidget( new QLabel( tr( "Autosave path:" ), box ) );
    gridLayout->addWidget( edit = new OptionBrowsedLineEditor( box, "AUTOSAVE_PATH" ) );
    edit->setToolTip( tr( "Directory where autosaved files are stored" ) );
    addOptionWidget( edit );

    checkbox->setChecked( false );
    spinbox->setEnabled( false );
    edit->setEnabled( false );
    connect( checkbox, SIGNAL( toggled( bool ) ), spinbox, SLOT( setEnabled( bool ) ) );
    connect( checkbox, SIGNAL( toggled( bool ) ), edit, SLOT( setEnabled( bool ) ) );

    // misc
    page->layout()->addWidget( box = new QGroupBox( tr( "Third-Party Applications" ), page ) );

    box->setLayout( new QVBoxLayout() );

    hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    box->layout()->addItem( hLayout );

    QLabel* label;
    hLayout->addWidget( label = new QLabel( tr( "Diff command:" ), box ) );
    hLayout->addWidget( edit = new OptionBrowsedLineEditor( box, "DIFF_COMMAND" ) );
    edit->setToolTip( tr( "Command used to diff files" ) );
    addOptionWidget( edit );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    // load initial configuration
    read();

}
