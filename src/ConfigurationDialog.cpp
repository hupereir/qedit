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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "ConfigurationDialog.h"

#include "Application.h"
#include "BaseFileInfoConfigurationWidget.h"
#include "Debug.h"
#include "GridLayout.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "OptionComboBox.h"
#include "OptionListBox.h"
#include "OptionBrowsedLineEditor.h"
#include "OptionCheckBox.h"
#include "OptionColorDisplay.h"
#include "OptionSpinBox.h"
#include "RecentFilesConfiguration.h"
#include "Singleton.h"

#include <QLabel>
#include <QGroupBox>
#include <QTextCodec>

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
    QLabel* label;

    // document classes
    QWidget* page = &addPage( IconEngine::get( IconNames::PreferencesFileAssociations ), tr( "Document Classes" ), tr( "Document classes options" ) );

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
    page = &addPage( IconEngine::get( IconNames::PreferencesEdition ), tr( "Text Edition" ), tr( "Settings for text edition" ) );

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
        connect( checkbox, SIGNAL(toggled(bool)), widget, SLOT(setDisabled(bool)) );

        // tab emulation
        checkbox = new OptionCheckBox( tr( "Emulate tabs" ), box, "TAB_EMULATION" );
        checkbox->setToolTip( tr( "Turn on/off tab emulation using space characters" ) );
        layout->addWidget( checkbox );
        addOptionWidget( checkbox );

        // tab size
        QHBoxLayout* hLayout = new QHBoxLayout();
        hLayout->setMargin(0);
        layout->addLayout( hLayout );

        hLayout->addWidget( label = new QLabel( tr( "Tab size: " ), box ) );
        OptionSpinBox* spinbox = new OptionSpinBox( box, "TAB_SIZE" );
        spinbox->setMinimum( 2 );
        spinbox->setMaximum( 20 );
        spinbox->setToolTip( tr( "Tab size (in unit of space characters)" ) );
        hLayout->addWidget( spinbox );
        hLayout->addStretch( 1 );
        label->setBuddy( spinbox );
        addOptionWidget( spinbox );

    }

    // default text-edition configuration
    textEditConfiguration( page, static_cast<Flags>(Flag::AllTextEdition) & ~static_cast<Flags>(Flag::TabEmulation|Flag::TextEditionFlags) );

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
        checkbox->setToolTip( tr( "Use wrap mode read from document class in place of the one specified in the default configuration" ) );
        addOptionWidget( checkbox );

        OptionCheckBox* wrap_checkbox = new OptionCheckBox( tr( "Wrap text" ), box, "WRAP_TEXT" );
        wrap_checkbox->setToolTip( tr( "Turn on/off line wrapping at editor border" ) );
        layout->addWidget( wrap_checkbox );
        addOptionWidget( wrap_checkbox );

        connect( checkbox, SIGNAL(toggled(bool)), wrap_checkbox, SLOT(setDisabled(bool)) );

        layout->addWidget( checkbox = new OptionCheckBox( tr( "Show line numbers" ), box, "SHOW_LINE_NUMBERS" ) );
        checkbox->setToolTip( tr( "Turn on/off line numbers" ) );
        addOptionWidget( checkbox );

        // auto hide cursor
        QLabel* label;
        GridLayout* gridLayout = new GridLayout();
        gridLayout->setMargin(0);
        gridLayout->setMaxCount(2);
        gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
        layout->addLayout( gridLayout );

        OptionSpinBox* spinbox;
        gridLayout->addWidget( label = new QLabel( tr( "Automatically hide mouse cursor after: " ), box ) );
        gridLayout->addWidget( spinbox = new OptionSpinBox( box, "AUTOHIDE_CURSOR_DELAY" ) );
        spinbox->setSuffix( tr( "s" ) );
        addOptionWidget( spinbox );

        // text encoding
        OptionComboBox* combobox;
        gridLayout->addWidget( label = new QLabel( tr( "Default font encoding:" ), box ) );
        gridLayout->addWidget( combobox = new OptionComboBox( box, "TEXT_ENCODING" ) );
        QList<QByteArray> codecs( QTextCodec::availableCodecs() );
        for( const auto& value:codecs ) { combobox->addItem( value ); }
        label->setBuddy( combobox );
        addOptionWidget( combobox );

    }

    // display
    page = &addPage( IconEngine::get( IconNames::PreferencesColors ), tr( "Colors" ), tr( "Text edition color settings" ) );

    // additional colors
    page->layout()->addWidget( box = new QWidget( page ) );

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMaxCount(2);
    gridLayout->setMargin(0);
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( gridLayout );

    gridLayout->addWidget( label = new QLabel( tr( "Parenthesis matching:" ), box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "PARENTHESIS_COLOR" ) );
    label->setBuddy( colorDisplay );
    addOptionWidget( colorDisplay );
    checkbox->setToolTip( tr( "Color for matching parenthesis" ) );

    gridLayout->addWidget( label = new QLabel( tr( "Tagged paragraphs:" ), box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "TAGGED_BLOCK_COLOR" ) );
    label->setBuddy( colorDisplay );
    addOptionWidget( colorDisplay );
    checkbox->setToolTip( tr( "Color for tagged paragraphs" ) );

    gridLayout->addWidget( label = new QLabel( tr( "Conflicting paragraphs:" ), box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "DIFF_CONFLICT_COLOR" ) );
    label->setBuddy( colorDisplay );
    addOptionWidget( colorDisplay );
    colorDisplay->setToolTip( tr( "Highlight color for diff conflict paragraphs" ) );

    gridLayout->addWidget( label = new QLabel( tr( "Added paragraphs:" ), box ) );
    gridLayout->addWidget( colorDisplay = new OptionColorDisplay( box, "DIFF_ADDED_COLOR" ) );
    label->setBuddy( colorDisplay );
    addOptionWidget( colorDisplay );
    colorDisplay->setToolTip( tr( "Highlight color for diff added paragraphs" ) );

    // navigation
    QHBoxLayout *hLayout;

    // multiple views
    page = &addPage( IconEngine::get( IconNames::PreferencesMultipleViews ), tr( "Multiple Views" ), tr( "Multiple views configuration" ) );
    page->layout()->addWidget( box = new QWidget( page ) );

    gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( gridLayout );

    OptionComboBox *combobox;

    // opening
    gridLayout->addWidget( label = new QLabel( tr( "Default open mode:" ), box ) );
    gridLayout->addWidget( combobox = new OptionComboBox( box, tr( "OPEN_MODE" ) ) );
    combobox->setUseValue( false );
    combobox->addItem( tr( "Open in Current Windoow" ) );
    combobox->addItem( tr( "Open in New Window" ) );

    label->setBuddy( combobox );
    addOptionWidget( combobox );
    combobox->setToolTip( tr( "Configure how new files are oppened" ) );

    // splitting
    gridLayout->addWidget( label = new QLabel( tr( "Default view orientation:" ), box ) );
    gridLayout->addWidget( combobox = new OptionComboBox( box, "ORIENTATION" ) );
    combobox->setUseValue( false );
    combobox->addItem( tr( "Top to Bottom" ) );
    combobox->addItem( tr( "Left to Right" ) );

    label->setBuddy( combobox );
    addOptionWidget( combobox );
    combobox->setToolTip( tr( "Configure how new views are organized" ) );

    // splitting
    gridLayout->addWidget( label = new QLabel( tr( "Default view orientation (diff mode):" ), box ) );
    gridLayout->addWidget( combobox = new OptionComboBox( box, "DIFF_ORIENTATION" ) );
    combobox->setUseValue( false );
    combobox->addItem( tr( "Top to Bottom" ) );
    combobox->addItem( tr( "Left to Right" ) );

    label->setBuddy( combobox );
    addOptionWidget( combobox );
    combobox->setToolTip( tr( "Configure how new views are organized in <i>diff</i> mode" ) );
    gridLayout->setColumnStretch( 1, 1 );

    // recent files
    page = &addPage( IconEngine::get( IconNames::PreferencesRecentFiles ), tr( "Recent Files" ), tr( "Recent files list settings" ), true );
    RecentFilesConfiguration* recentFilesConfiguration = new RecentFilesConfiguration( page, Singleton::get().application<Application>()->recentFiles() );
    page->layout()->addWidget( recentFilesConfiguration );
    addOptionWidget( recentFilesConfiguration );

    recentFilesConfiguration->read();
    connect( this, SIGNAL(ok()), recentFilesConfiguration, SLOT(write()) );
    connect( this, SIGNAL(apply()), recentFilesConfiguration, SLOT(write()) );
    connect( this, SIGNAL(reset()), recentFilesConfiguration, SLOT(reload()) );

    // tooltips
    page = &addPage( IconEngine::get( IconNames::PreferencesAppearance), tr( "Tooltips" ), tr( "Tooltips appearance" ) );
    page->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Show tooltips" ), page, "SHOW_TOOLTIPS" ) );
    addOptionWidget( checkbox );

    hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(5);
    page->layout()->addItem( hLayout );

    BaseFileInfoConfigurationWidget* configurationWidget;
    hLayout->addSpacing( _checkBoxSpacing() );
    hLayout->addWidget( configurationWidget = new BaseFileInfoConfigurationWidget( page ) );
    configurationWidget->setEnabled( false );
    connect( checkbox, SIGNAL(toggled(bool)), configurationWidget, SLOT(setEnabled(bool)) );
    addOptionWidget( configurationWidget );

    // misc
    page = &addPage( IconEngine::get( IconNames::PreferencesUnsorted ), tr( "Unsorted" ), tr( "Additional unsorted settings" ) );
    page->layout()->addWidget( box = new QGroupBox( tr( "Backup and Autosave" ), page ) );

    box->setLayout( new QVBoxLayout() );

    box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Make backup of files when saving modifications" ), box, "BACKUP" ) );
    checkbox->setToolTip( tr( "Make backup of the file prior to saving modifications" ) );
    addOptionWidget( checkbox );

    box->layout()->addWidget( checkbox = new OptionCheckBox( tr( "Save files regularly in a temporary directory" ), box, "AUTOSAVE" ) );
    checkbox->setToolTip(
        tr( "Make automatic copies of edited files in\n"
        "specified directory to allow crash recovery" ) );
    addOptionWidget( checkbox );

    gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->layout()->addItem( gridLayout );

    gridLayout->addWidget( label = new QLabel( tr( "Autosave interval:" ), box ) );
    gridLayout->addWidget( spinbox = new OptionSpinBox( box, "AUTOSAVE_INTERVAL" ) );
    spinbox->setSuffix( "s" );
    spinbox->setMinimum( 1 );
    spinbox->setMaximum( 300 );
    spinbox->setToolTip( tr( "Interval between two autosave" ) );
    label->setBuddy( spinbox );
    addOptionWidget( spinbox );

    OptionBrowsedLineEditor *edit;
    gridLayout->addWidget( label = new QLabel( tr( "Autosave path:" ), box ) );
    gridLayout->addWidget( edit = new OptionBrowsedLineEditor( box, "AUTOSAVE_PATH" ) );
    edit->setToolTip( tr( "Directory where autosaved files are stored" ) );
    label->setBuddy( edit );
    addOptionWidget( edit );

    checkbox->setChecked( false );
    spinbox->setEnabled( false );
    edit->setEnabled( false );
    connect( checkbox, SIGNAL(toggled(bool)), spinbox, SLOT(setEnabled(bool)) );
    connect( checkbox, SIGNAL(toggled(bool)), edit, SLOT(setEnabled(bool)) );

    // misc
    page->layout()->addWidget( box = new QGroupBox( tr( "Third-Party Applications" ), page ) );

    box->setLayout( new QVBoxLayout() );

    hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    box->layout()->addItem( hLayout );

    hLayout->addWidget( label = new QLabel( tr( "Diff command:" ), box ) );
    hLayout->addWidget( edit = new OptionBrowsedLineEditor( box, "DIFF_COMMAND" ) );
    edit->setToolTip( tr( "Command used to diff files" ) );
    label->setBuddy( edit );
    addOptionWidget( edit );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    // load initial configuration
    read();

}
