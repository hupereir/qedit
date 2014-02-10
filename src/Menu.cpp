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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "Menu.h"
#include "Menu.moc"

#include "Application.h"
#include "BlockDelimiterDisplay.h"
#include "DebugMenu.h"
#include "DefaultHelpText.h"
#include "DocumentClassMenu.h"
#include "HelpManager.h"
#include "HelpText.h"
#include "MainWindow.h"
#include "NavigationFrame.h"
#include "RecentFilesMenu.h"
#include "SessionFilesFrame.h"
#include "InformationDialog.h"
#include "Singleton.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TextMacroMenu.h"
#include "Util.h"
#include "WindowServer.h"
#include "XmlOptions.h"

//_______________________________________________
Menu::Menu( QWidget* parent ):
    QMenuBar( parent ),
    Counter( "Menu" )
{
    Debug::Throw( "Menu::Menu.\n" );

    // file menu
    QMenu* menu = addMenu( tr( "File" ) );

    // retrieve mainwindow
    const Application& application( *Singleton::get().application<Application>() );
    const MainWindow& mainwindow( *static_cast<MainWindow*>( window() ) );

    menu->addAction( &mainwindow.newFileAction() );
    menu->addAction( &mainwindow.cloneAction() );
    menu->addAction( &mainwindow.detachAction() );
    menu->addAction( &mainwindow.openAction() );

    // open previous menu
    menu->addMenu( recentFilesMenu_ = new RecentFilesMenu( this, application.recentFiles() ) );

    // additional actions
    menu->addSeparator();
    menu->addAction( &mainwindow.closeDisplayAction() );
    menu->addAction( &mainwindow.closeWindowAction() );
    menu->addAction( &mainwindow.saveAction() );
    menu->addAction( &mainwindow.saveAsAction() );
    menu->addAction( &application.windowServer().saveAllAction() );
    menu->addAction( &mainwindow.revertToSaveAction() );
    menu->addSeparator();

    // print and close
    menu->addAction( &mainwindow.printAction() );
    menu->addAction( &mainwindow.printPreviewAction() );
    menu->addAction( &mainwindow.htmlAction() );
    menu->addSeparator();

    // document class
    menu->addAction( &mainwindow.filePropertiesAction() );
    menu->addMenu( documentClassMenu_ = new DocumentClassMenu( this ) );
    documentClassMenu_->setTitle( tr( "Document Type" ) );

    menu->addSeparator();
    menu->addAction( &application.closeAction() );

    // recent files menu current file needs to be updated prior to the menu to be shown
    // this is performed every time the "file menu" is shown.
    connect( menu, SIGNAL(aboutToShow()), SLOT(_updateRecentFilesMenu()) );

    // Edit menu
    editMenu_ = addMenu( tr( "Edit" ) );
    connect( editMenu_, SIGNAL(aboutToShow()), SLOT(_updateEditMenu()) );

    // Search menu
    searchMenu_ = addMenu( tr( "Search" ) );
    connect( searchMenu_, SIGNAL(aboutToShow()), SLOT(_updateSearchMenu()) );

    // windows
    windowsActionGroup_ = new ActionGroup( this );
    windowsMenu_ = addMenu( tr( "Session" ) );
    connect( windowsMenu_, SIGNAL(aboutToShow()), this, SLOT(_updateWindowsMenu()) );
    connect( windowsMenu_, SIGNAL(triggered(QAction*)), SLOT(_selectFile(QAction*)) );

    // tools
    toolsMenu_ = addMenu( tr( "Tools" ) );
    connect( toolsMenu_, SIGNAL(aboutToShow()), this, SLOT(_updateToolsMenu()) );

    // macros
    addMenu( macroMenu_ = new TextMacroMenu( this ) );
    macroMenu_->setTitle( tr( "Macros" ) );
    connect( macroMenu_, SIGNAL(aboutToShow()), this, SLOT(updateMacroMenu()) );
    connect( macroMenu_, SIGNAL(textMacroSelected(QString)), SLOT(_selectMacro(QString)) );

    // Settings
    preferenceMenu_ = addMenu( tr( "Settings" ) );
    connect( preferenceMenu_, SIGNAL(aboutToShow()), this, SLOT(_updatePreferenceMenu()) );

    // help manager
    Base::HelpManager* help( new Base::HelpManager( this ) );
    help->setWindowTitle( tr( "Qedit Handbook" ) );
    help->install( helpText );
    help->install( Base::helpText, false );

    // create help menu
    menu = addMenu( tr( "Help" ) );
    menu->addAction( &help->displayAction() );
    menu->addSeparator();
    menu->addAction( &application.aboutAction() );
    menu->addAction( &application.aboutQtAction() );

    // debug menu
    menu->addSeparator();
    DebugMenu *debugMenu( new DebugMenu( menu ) );
    menu->addAction( debugMenu->menuAction() );
    debugMenu->menuAction()->setVisible( false );

}

//_______________________________________________
Menu::~Menu( void )
{ Debug::Throw( "Menu::~Menu.\n" ); }

//_______________________________________________
void Menu::updateMacroMenu( void )
{

    // retrieve current display
    const TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
    bool hasSelection( display.textCursor().hasSelection() );
    const TextMacro::List& macros( display.macros() );

    macroMenu_->update( macros );
    macroMenu_->updateState( hasSelection );
    macroMenu_->addSeparator();
    macroMenu_->addAction( &display.noAutomaticMacrosAction() );

    return;
}

//_______________________________________________
void Menu::_updateRecentFilesMenu( void )
{
    Debug::Throw( "Menu::_updateRecentFilesMenu.\n" );
    const TextDisplay& display( static_cast<MainWindow*>( window() )->activeDisplay() );
    if( !display.isNewDocument() ) recentFilesMenu_->setCurrentFile( display.file() );
}

//_______________________________________________
void Menu::_updateEditMenu( void )
{
    Debug::Throw( "Menu::_updateEditMenu.\n" );

    editMenu_->clear();

    TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
    editMenu_->addAction( &display.undoAction() );
    editMenu_->addAction( &display.redoAction() );
    editMenu_->addSeparator();

    editMenu_->addAction( &display.cutAction() );
    editMenu_->addAction( &display.copyAction() );
    editMenu_->addAction( &display.pasteAction() );
    editMenu_->addSeparator();

    editMenu_->addAction( &display.upperCaseAction() );
    editMenu_->addAction( &display.lowerCaseAction() );
}

//_______________________________________________
void Menu::_updateSearchMenu( void )
{
    Debug::Throw( "Menu::_updateSearchMenu.\n" );

    searchMenu_->clear();

    const TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
    searchMenu_->addAction( &display.findAction() );
    searchMenu_->addAction( &display.findAgainAction() );
    searchMenu_->addAction( &display.findSelectionAction() );
    searchMenu_->addAction( &display.replaceAction() );
    searchMenu_->addAction( &display.replaceAgainAction() );
    searchMenu_->addSeparator();

    searchMenu_->addAction( &display.gotoLineAction() );

}

//_______________________________________________
void Menu::_updatePreferenceMenu( void )
{

    Debug::Throw( "Menu::_updatePreferenceMenu.\n" );

    // reference to needed objects
    const Application& application( *Singleton::get().application<Application>() );
    const MainWindow& mainwindow( *static_cast<MainWindow*>(window()) );
    const TextDisplay& display( mainwindow.activeDisplay() );

    // clear menu
    preferenceMenu_->clear();

    // textdisplay actions
    preferenceMenu_->addAction( &mainwindow.navigationFrame().visibilityAction() );
    preferenceMenu_->addAction( &display.showLineNumberAction() );
    preferenceMenu_->addAction( &display.showBlockDelimiterAction() );
    preferenceMenu_->addAction( &display.wrapModeAction() );
    preferenceMenu_->addAction( &display.tabEmulationAction() );
    preferenceMenu_->addAction( &display.textIndentAction() );
    preferenceMenu_->addAction( &display.textHighlightAction() );
    preferenceMenu_->addAction( &display.blockHighlightAction() );
    preferenceMenu_->addAction( &display.parenthesisHighlightAction() );

    preferenceMenu_->addSeparator();
    #if WITH_ASPELL
    preferenceMenu_->addAction( &display.autoSpellAction() );
    preferenceMenu_->addAction( &display.dictionaryMenuAction() );
    preferenceMenu_->addAction( &display.filterMenuAction() );

    // disable autospell action if dictionaries are empty
    const bool hasDictionaries( !display.textHighlight().spellParser().interface().dictionaries().empty() );
    display.autoSpellAction().setEnabled( hasDictionaries );
    display.dictionaryMenuAction().setEnabled( hasDictionaries );
    #endif

    preferenceMenu_->addAction( &display.textEncodingMenuAction() );

    // configurations (from application)
    preferenceMenu_->addSeparator();
    preferenceMenu_->addAction( &application.configurationAction() );
    preferenceMenu_->addAction( &application.documentClassesConfigurationAction() );
    #if WITH_ASPELL
    preferenceMenu_->addAction( &application.spellCheckConfigurationAction() );
    #endif

    return;
}

//_______________________________________________
void Menu::_updateToolsMenu( void )
{

    Debug::Throw( "Menu::_updateToolsMenu.\n" );

    // retrieve mainwindow and current display
    const MainWindow& mainwindow( *static_cast<MainWindow*>(window()) );
    const TextDisplay& display( mainwindow.activeDisplay() );

    // retrieve flags needed to set button state
    const bool editable( !display.isReadOnly() );
    const bool hasSelection( display.textCursor().hasSelection() );
    const bool hasIndent( display.textIndentAction().isEnabled() );

    // clear menu
    toolsMenu_->clear();

    // selection indentation
    toolsMenu_->addAction( &display.indentSelectionAction() );
    display.indentSelectionAction().setEnabled( editable && hasSelection && hasIndent );

    //if( display.baseIndentAction().isEnabled() )
    { toolsMenu_->addAction( &display.baseIndentAction() ); }

    // tab replacement
    toolsMenu_->addAction( &display.leadingTabsAction() );
    display.leadingTabsAction().setEnabled( display.hasLeadingTabs() );

    #if WITH_ASPELL
    // spell checker
    toolsMenu_->addAction( &display.spellcheckAction() );

    // disable autospell action if dictionaries are empty
    const bool hasDictionaries( !display.textHighlight().spellParser().interface().dictionaries().empty() );
    display.spellcheckAction().setEnabled( hasDictionaries );
    #endif

    // diff files
    toolsMenu_->addSeparator();
    toolsMenu_->addAction( &mainwindow.diffAction() );

    const bool hasTags( display.hasTaggedBlocks() );
    const bool currentBlockTagged( hasTags && display.isCurrentBlockTagged() );

    toolsMenu_->addAction( &display.tagBlockAction() );
    display.tagBlockAction().setText( hasSelection ? tr( "Tag Selected Blocks" ) : tr( "Tag current block" ) );

    toolsMenu_->addAction( &display.nextTagAction() );
    display.nextTagAction().setEnabled( hasTags );

    toolsMenu_->addAction( &display.previousTagAction() );
    display.previousTagAction().setEnabled( hasTags );

    toolsMenu_->addAction( &display.clearTagAction() );
    display.clearTagAction().setEnabled( currentBlockTagged );

    toolsMenu_->addAction( &display.clearAllTagsAction() );
    display.clearAllTagsAction().setEnabled( hasTags );

    // blocks delimiters
    if( display.showBlockDelimiterAction().isEnabled() && display.showBlockDelimiterAction().isChecked() )
    {
        toolsMenu_->addSeparator();
        display.blockDelimiterDisplay().updateCurrentBlockActionState();
        display.blockDelimiterDisplay().addActions( *toolsMenu_ );
    }

    // rehighlight
    toolsMenu_->addSeparator();
    QAction* action = toolsMenu_->addAction( tr( "Rehighlight" ), window(), SLOT(rehighlight()) );
    bool enabled( display.textHighlightAction().isEnabled() && display.textHighlightAction().isChecked() );

    #if WITH_ASPELL
    enabled |= ( display.autoSpellAction().isEnabled() || display.autoSpellAction().isChecked() );
    #endif

    action->setEnabled( enabled );


}

//_______________________________________________
void Menu::_updateWindowsMenu( void )
{

    Debug::Throw( "Menu::_updateWindowsMenu.\n" );
    windowsMenu_->clear();

    // add session handling
    const Application& application( *Singleton::get().application<Application>() );
    windowsMenu_->addAction( &application.saveSessionAction() );
    windowsMenu_->addAction( &application.restoreSessionAction() );
    windowsMenu_->addAction( &application.discardSessionAction() );
    windowsMenu_->addSeparator();

    // retrieve current display
    const TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
    const QString& currentFile( display.file() );

    // clear files map
    fileActions_.clear();

    // retrieve all files
    foreach( const FileRecord& record,  Singleton::get().application<Application>()->windowServer().records() )
    {

        // retrieve file and check
        const File& file( record.file() );

        // add menu item
        QAction* action = windowsMenu_->addAction( file );
        action->setCheckable( true );
        action->setChecked( currentFile == file );
        windowsActionGroup_->addAction( action );

        // insert in map for later callback.
        fileActions_.insert( action, file );

    }

    windowsMenu_->addSeparator();
    SessionFilesFrame& sessionFilesFrame( static_cast<MainWindow*>(window())->navigationFrame().sessionFilesFrame() );
    windowsMenu_->addAction( &sessionFilesFrame.previousFileAction() );
    windowsMenu_->addAction( &sessionFilesFrame.nextFileAction() );

}

//_______________________________________________
void Menu::_selectMacro( QString name )
{
    Debug::Throw( "Menu::_SelectMacro.\n" );
    static_cast<MainWindow*>(window())->activeDisplay().processMacro( name );
    return;
}

//_______________________________________________
void Menu::_selectFile( QAction* action )
{
    Debug::Throw( "Menu::_selectFile.\n" );

    // try retrieve id in map
    ActionMap::iterator iter = fileActions_.find( action );
    if( iter == fileActions_.end() ) return;

    // retrieve all mainwindows
    Base::KeySet<MainWindow> windows( &Singleton::get().application<Application>()->windowServer() );

    // retrieve window matching file name
    Base::KeySet<MainWindow>::iterator windowIter( std::find_if(
        windows.begin(),
        windows.end(),
        MainWindow::SameFileFTor( iter.value() ) ) );

    // check if window was found
    if( windowIter == windows.end() )
    {
        InformationDialog( this, QString( tr( "Unable to find a window containing file '%1'" ) ).arg( iter.value() ) ).exec();
        return;
    }

    // select display in found window
    (*windowIter)->selectDisplay( iter.value() );
    (*windowIter)->uniconify();

    return;
}
