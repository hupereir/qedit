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
\file Menu.cpp
\brief main menu
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "Application.h"
#include "BlockDelimiterDisplay.h"
#include "Config.h"
#include "DebugMenu.h"
#include "DefaultHelpText.h"
#include "DocumentClassMenu.h"
#include "HelpManager.h"
#include "HelpText.h"
#include "MainWindow.h"
#include "Menu.h"
#include "NavigationFrame.h"
#include "RecentFilesMenu.h"
#include "SessionFilesFrame.h"
#include "InformationDialog.h"
#include "Singleton.h"
#include "TextDisplay.h"
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
    QMenu* menu = addMenu( "&File" );

    // retrieve mainwindow
    Application& application( *Singleton::get().application<Application>() );
    MainWindow& mainwindow( *static_cast<MainWindow*>( window() ) );

    menu->addAction( &mainwindow.newFileAction() );
    menu->addAction( &mainwindow.cloneAction() );
    menu->addAction( &mainwindow.detachAction() );
    menu->addAction( &mainwindow.openAction() );

    // open previous menu
    recentFilesMenu_ = new RecentFilesMenu( this, application.recentFiles() );
    menu->addMenu( recentFilesMenu_ );

    // additional actions
    menu->addSeparator();
    menu->addAction( &mainwindow.closeDisplayAction() );
    menu->addAction( &mainwindow.closeWindowAction() );
    menu->addAction( &mainwindow.saveAction() );
    menu->addAction( &mainwindow.saveAsAction() );
    menu->addAction( &application.windowServer().saveAllAction() );
    menu->addAction( &mainwindow.revertToSaveAction() );
    menu->addSeparator();

    // document class
    menu->addMenu( documentClassMenu_ = new DocumentClassMenu( this ) );
    documentClassMenu_->setTitle( "Set &document class" );

    // print and close
    menu->addAction( &mainwindow.printAction() );

    menu->addSeparator();
    menu->addAction( &application.closeAction() );

    // recent files menu current-file needs to be updated prior to the menu to be shown
    // this is performed every time the "file menu" is shown.
    connect( menu, SIGNAL( aboutToShow() ), SLOT( _updateRecentFilesMenu() ) );

    // Edit menu
    editMenu_ = addMenu( "&Edit" );
    connect( editMenu_, SIGNAL( aboutToShow() ), SLOT( _updateEditMenu() ) );

    // Search menu
    searchMenu_ = addMenu( "&Search" );
    connect( searchMenu_, SIGNAL( aboutToShow() ), SLOT( _updateSearchMenu() ) );

    // tools
    toolsMenu_ = addMenu( "&Tools" );
    connect( toolsMenu_, SIGNAL( aboutToShow() ), this, SLOT( _updateToolsMenu() ) );

    // macros
    addMenu( macroMenu_ = new TextMacroMenu( this ) );
    macroMenu_->setTitle( "&Macro" );
    connect( macroMenu_, SIGNAL( aboutToShow() ), this, SLOT( _updateMacroMenu() ) );
    connect( macroMenu_, SIGNAL( textMacroSelected( QString ) ), SLOT( _selectMacro( QString ) ) );

    // Settings
    preferenceMenu_ = addMenu( "&Settings" );
    connect( preferenceMenu_, SIGNAL( aboutToShow() ), this, SLOT( _updatePreferenceMenu() ) );

    // windows
    windowsActionGroup_ = new ActionGroup( this );
    windowsMenu_ = addMenu( "&Window" );
    connect( windowsMenu_, SIGNAL( aboutToShow() ), this, SLOT( _updateWindowsMenu() ) );
    connect( windowsMenu_, SIGNAL( triggered( QAction* ) ), SLOT( _selectFile( QAction* ) ) );

    // help manager
    BASE::HelpManager* help( new BASE::HelpManager( this ) );
    help->setWindowTitle( "Qedit Handbook" );
    File help_file( XmlOptions::get().raw( "HELP_FILE" ) );
    if( help_file.exists() ) help->install( help_file );
    else
    {
        help->setFile( help_file );
        help->install( helpText );
        help->install( BASE::helpText, false );
    }

    // create help menu
    menu = addMenu( "&Help" );
    menu->addAction( &help->displayAction() );
    menu->addSeparator();
    menu->addAction( &application.aboutQtAction() );
    menu->addAction( &application.aboutAction() );

    // debug menu
    //#ifdef DEBUG
    menu->addSeparator();
    DebugMenu *debug_menu( new DebugMenu( this ) );
    debug_menu->setTitle( "&Debug" );
    debug_menu->addAction( &help->dumpAction() );
    debug_menu->addAction( &application.monitoredFilesAction() );
    menu->addMenu( debug_menu );
    //#endif

}

//_______________________________________________
Menu::~Menu( void )
{ Debug::Throw( "Menu::~Menu.\n" ); }

//_______________________________________________
void Menu::_updateRecentFilesMenu( void )
{
    Debug::Throw( "Menu::_updateRecentFilesMenu.\n" );
    recentFilesMenu_->setCurrentFile( static_cast<MainWindow*>( Menu::window() )->activeDisplay().file() );
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

    TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
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
    Application& application( *Singleton::get().application<Application>() );
    MainWindow& mainwindow( *static_cast<MainWindow*>(window()) );
    TextDisplay& display( mainwindow.activeDisplay() );

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
    preferenceMenu_->addAction( &display.noAutomaticMacrosAction() );

    #if WITH_ASPELL
    preferenceMenu_->addSeparator();
    preferenceMenu_->addAction( &display.autoSpellAction() );
    preferenceMenu_->addAction( &display.dictionaryMenuAction() );
    preferenceMenu_->addAction( &display.filterMenuAction() );
    #endif

    // configurations (from application)
    preferenceMenu_->addSeparator();
    #if WITH_ASPELL
    preferenceMenu_->addAction( &application.spellCheckConfigurationAction() );
    #endif
    preferenceMenu_->addAction( &application.documentClassConfigurationAction() );
    preferenceMenu_->addAction( &application.configurationAction() );

    return;
}

//_______________________________________________
void Menu::_updateToolsMenu( void )
{

    Debug::Throw( "Menu::_updateToolsMenu.\n" );

    // retrieve mainwindow and current display
    MainWindow& mainwindow( *static_cast<MainWindow*>(window()) );
    TextDisplay& display( mainwindow.activeDisplay() );

    // retrieve flags needed to set button state
    bool editable( !display.isReadOnly() );
    bool has_selection( display.textCursor().hasSelection() );
    bool has_indent( display.textIndentAction().isEnabled() );

    // clear menu
    toolsMenu_->clear();

    // selection indentation
    toolsMenu_->addAction( &display.indentSelectionAction() );
    display.indentSelectionAction().setEnabled( editable && has_selection && has_indent );

    //if( display.baseIndentAction().isEnabled() )
    { toolsMenu_->addAction( &display.baseIndentAction() ); }

    // tab replacement
    toolsMenu_->addAction( &display.leadingTabsAction() );
    display.leadingTabsAction().setEnabled( display.hasLeadingTabs() );

    // spell checker
    toolsMenu_->addAction( &display.spellcheckAction() );

    // diff files
    toolsMenu_->addSeparator();
    toolsMenu_->addAction( &mainwindow.diffAction() );

    bool has_tags( display.hasTaggedBlocks() );
    bool current_block_tagged( has_tags && display.isCurrentBlockTagged() );

    toolsMenu_->addAction( &display.tagBlockAction() );
    display.tagBlockAction().setText( has_selection ? "&Tag selected blocks":"&Tag current block" );

    toolsMenu_->addAction( &display.nextTagAction() );
    display.nextTagAction().setEnabled( has_tags );

    toolsMenu_->addAction( &display.previousTagAction() );
    display.previousTagAction().setEnabled( has_tags );

    toolsMenu_->addAction( &display.clearTagAction() );
    display.clearTagAction().setEnabled( current_block_tagged );

    toolsMenu_->addAction( &display.clearAllTagsAction() );
    display.clearAllTagsAction().setEnabled( has_tags );

    // blocks delimiters
    if( display.showBlockDelimiterAction().isEnabled() && display.showBlockDelimiterAction().isChecked() )
    {
        toolsMenu_->addSeparator();
        display.blockDelimiterDisplay().updateCurrentBlockActionState();
        display.blockDelimiterDisplay().addActions( *toolsMenu_ );
    }

    // rehighlight
    toolsMenu_->addSeparator();
    QAction* action = toolsMenu_->addAction( "&Rehighlight", window(), SLOT( rehighlight() ) );
    bool enabled( display.textHighlightAction().isEnabled() && display.textHighlightAction().isChecked() );

    #if WITH_ASPELL
    enabled |= ( display.autoSpellAction().isEnabled() || display.autoSpellAction().isChecked() );
    #endif

    action->setEnabled( enabled );


}

//_______________________________________________
void Menu::_updateMacroMenu( void )
{

    Debug::Throw( "Menu::_updateMacroMenu.\n" );

    // retrieve current display
    TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
    bool has_selection( display.textCursor().hasSelection() );

    macroMenu().setTextMacros( display.macros(), has_selection );
    return;
}

//_______________________________________________
void Menu::_updateWindowsMenu( void )
{

    Debug::Throw( "Menu::_updateWindowsMenu.\n" );
    windowsMenu_->clear();

    // retrieve current display
    TextDisplay& display( static_cast<MainWindow*>(window())->activeDisplay() );
    windowsMenu_->addAction( &display.filePropertiesAction() );

    const QString& current_file( display.file() );

    // clear files map
    fileActions_.clear();

    // retrieve all files
    bool first = true;
    FileRecord::List records( Singleton::get().application<Application>()->windowServer().records() );
    for( FileRecord::List::const_iterator iter = records.begin(); iter != records.end(); ++iter )
    {

        // retrieve file and check
        const File& file( iter->file() );

        // if first valid file, add separator
        if( first )
        {
            windowsMenu_->addSeparator();
            first = false;
        }

        // add menu item
        QAction* action = windowsMenu_->addAction( file );
        action->setCheckable( true );
        action->setChecked( current_file == file );
        windowsActionGroup_->addAction( action );

        // insert in map for later callback.
        fileActions_.insert( std::make_pair( action, file ) );

    }


    windowsMenu_->addSeparator();
    SessionFilesFrame& session_files_frame( static_cast<MainWindow*>(window())->navigationFrame().sessionFilesFrame() );
    windowsMenu_->addAction( &session_files_frame.previousFileAction() );
    windowsMenu_->addAction( &session_files_frame.nextFileAction() );

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
    std::map<QAction*, File>::iterator iter = fileActions_.find( action );
    if( iter == fileActions_.end() ) return;

    // retrieve all mainwindows
    BASE::KeySet<MainWindow> windows( &Singleton::get().application<Application>()->windowServer() );

    // retrieve window matching file name
    BASE::KeySet<MainWindow>::iterator window_iter( find_if(
        windows.begin(),
        windows.end(),
        MainWindow::SameFileFTor( iter->second ) ) );

    // check if window was found
    if( window_iter == windows.end() )
    {
        QString buffer;
        QTextStream( &buffer ) << "Unable to find a window containing file " << iter->second;
        InformationDialog( this, buffer ).exec();
        return;
    }

    // select display in found window
    (*window_iter)->selectDisplay( iter->second );
    (*window_iter)->uniconify();

    return;
}
