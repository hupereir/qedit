// $Id$

/***************************************************************************
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
****************************************************************************/

#include "MainWindow.h"

#include "AnimatedLineEditor.h"
#include "Application.h"
#include "AutoSave.h"
#include "BaseStatusBar.h"
#include "BlockDelimiterDisplay.h"
#include "ClockLabel.h"
#include "Config.h"
#include "CustomToolButton.h"
#include "Command.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "Diff.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "DocumentClassToolBar.h"
#include "FileCheckDialog.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "FileSelectionDialog.h"
#include "FileSystemFrame.h"
#include "BaseFindDialog.h"
#include "HighlightBlockFlags.h"
#include "HtmlDialog.h"
#include "HtmlHelper.h"
#include "IconEngine.h"
#include "Icons.h"
#include "InformationDialog.h"
#include "Menu.h"
#include "NavigationFrame.h"
#include "NavigationToolBar.h"
#include "NewFileDialog.h"
#include "PixmapEngine.h"
#include "PrinterOptionWidget.h"
#include "PrintHelper.h"
#include "PrintPreviewDialog.h"
#include "QtUtil.h"
#include "QuestionDialog.h"
#include "RecentFilesFrame.h"
#include "ReplaceDialog.h"
#include "SelectLineDialog.h"
#include "SessionFilesFrame.h"
#include "Singleton.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "TextMacroMenu.h"
#include "TransitionWidget.h"
#include "WindowServer.h"
#include "WindowTitle.h"
#include "XmlOptions.h"

#include <QObjectList>
#include <QApplication>
#include <QPrintDialog>
#include <QPrinter>
#include <QDomElement>
#include <QDomDocument>

//_____________________________________________________
const QString MainWindow::LEFT_RIGHT = "Left/right";
const QString MainWindow::TOP_BOTTOM = "Top/bottom";

//_____________________________________________________
MainWindow::MainWindow(  QWidget* parent ):
    BaseMainWindow( parent ),
    Counter( "MainWindow" ),
    menu_( 0 ),
    statusbar_( 0 ),
    fileEditor_( 0 ),
    documentClassToolBar_( 0 ),
    findDialog_( 0 ),
    replaceDialog_( 0 ),
    selectLineDialog_( 0 )
{

    Debug::Throw( "MainWindow::MainWindow.\n" );
    setOptionName( "WINDOW" );

    // tell window to delete on exit
    setAttribute( Qt::WA_DeleteOnClose );

    // install actions
    _installActions();

    // additional actions from Application
    // they need to be added so that short cuts still work even when menu bar is hidden)
    Application& application( *Singleton::get().application<Application>() );
    addAction( &application.closeAction() );

    // menu
    setMenuBar( menu_ = new Menu( this ) );
    connect( &menu().documentClassMenu(), SIGNAL( documentClassSelected( QString ) ), this, SLOT( selectClassName( QString ) ) );

    // main widget is a splitter to store navigation window and active view
    QSplitter* splitter = new QSplitter( this );
    splitter->setOrientation( Qt::Horizontal );
    setCentralWidget( splitter );

    // insert navigationFrame
    navigationFrame_ = new NavigationFrame(0, application.recentFiles() );
    navigationFrame_->setDefaultWidth( XmlOptions::get().get<int>( "NAVIGATION_FRAME_WIDTH" ) );
    splitter->addWidget( &navigationFrame() );

    connect( &navigationFrame().visibilityAction(), SIGNAL( toggled( bool ) ), SLOT( _toggleNavigationFrame( bool ) ) );

    // need to add navigationFrame visibility action to this list
    // to enable shortcut event if the frame is hidden
    addAction( &navigationFrame().visibilityAction() );
    connect( &navigationFrame().sessionFilesFrame(), SIGNAL( fileSelected( FileRecord ) ), SLOT( _selectDisplay( FileRecord ) ) );
    connect( &navigationFrame().recentFilesFrame(), SIGNAL( fileSelected( FileRecord ) ), SLOT( _selectDisplay( FileRecord ) ) );

    addAction( &navigationFrame().sessionFilesFrame().nextFileAction() );
    addAction( &navigationFrame().sessionFilesFrame().previousFileAction() );

    // insert stack widget
    splitter->addWidget( stack_ = new QStackedWidget(0) );
    _stack().layout()->setMargin(2);

    connect( &_stack(), SIGNAL( widgetRemoved( int ) ), SLOT( _activeViewChanged() ) );

    // transition widget
    _replaceTransitionWidget();

    // create first text view
    newTextView();

    // assign stretch factors
    splitter->setStretchFactor( 0, 0 );
    splitter->setStretchFactor( 1, 1 );
    connect( splitter, SIGNAL( splitterMoved( int, int ) ), SLOT( _splitterMoved( void ) ) );

    // state frame
    setStatusBar( statusbar_ = new BaseStatusBar( this ) );

    // create "Hidden" line editor to display filename
    statusbar_->addPermanentWidget( fileEditor_ = new QLabel( statusbar_ ), 1 );
    statusbar_->addPermanentWidget( new QWidget(), 1 );
    statusbar_->addLabels( 3, 0 );
    statusbar_->label(0).setAlignment( Qt::AlignCenter );
    statusbar_->label(1).setAlignment( Qt::AlignCenter );
    statusbar_->label(2).setAlignment( Qt::AlignCenter );
    statusbar_->addClock();

    fileEditor_->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );

    // toolbars
    _installToolbars();

    //! configuration
    connect( &application, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
    connect( &application, SIGNAL( saveConfiguration() ), SLOT( _saveConfiguration() ) );
    connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
    _updateConfiguration();

    // update buttons
    Debug::Throw( "MainWindow::MainWindow - done.\n" );

}

//___________________________________________________________
MainWindow::~MainWindow( void )
{
    Debug::Throw( "MainWindow::~MainWindow.\n" );
    disconnect( &_transitionWidget(), SIGNAL( destroyed() ) );
}

//___________________________________________________________
TextView& MainWindow::newTextView( FileRecord record )
{
    Debug::Throw( "MainWindow::newTextView.\n" );

    // create new view and add to this file
    TextView* view = new TextView( this );
    BASE::Key::associate( this, view );

    // connections
    _connectView( *view );

    // open file if valid
    if( record.file().exists() ) view->setFile( record.file() );

    // add to stack and set active
    _stack().addWidget( view );
    setActiveView( *view );

    return *view;
}

//_____________________________________________________________________
void MainWindow::setActiveView( TextView& view )
{

    Debug::Throw() << "MainWindow::setActiveView - key: " << view.key() << endl;

    // do nothing if active view did not change
    if( activeView_ == &view ) return;

    // this check is needed because the active view passed as argument
    // might be closing and have no associated display
    if( BASE::KeySet<TextDisplay>( &view ).empty() ) return;

    // store active view
    activeView_ = &view;
    activeView().activeDisplay().setFocusDelayed();

    // update stack if needed
    if( _stack().currentWidget() !=  &activeView() )
    {

        if( _transitionWidget().isEnabled() && isVisible() )
        {
            _transitionWidget().setParent( &activeView() );
            _transitionWidget().initialize( &_stack() );
        }

        _stack().setCurrentWidget( &activeView() );

        if( _transitionWidget().isEnabled() && isVisible() ) _transitionWidget().start();
        else _animationFinished();

    } else _animationFinished();

}


//_____________________________________________________________________
BASE::KeySet<TextDisplay> MainWindow::associatedDisplays( void ) const
{
    BASE::KeySet<TextDisplay> displays;
    foreach( TextView* view, BASE::KeySet<TextView>( this ) )
    { displays.unite( BASE::KeySet<TextDisplay>(view) ); }

    return displays;
}

//_____________________________________________________________________
bool MainWindow::selectDisplay( const File& file )
{

    Debug::Throw() << "MainWindow::selectDisplay - file: " << file << endl;

    // do nothing if already selected
    if( activeView().activeDisplay().file() == file ) return true;

    foreach( TextView* view, BASE::KeySet<TextView>( this ) )
    {

        if( view->selectDisplay( file ) )
        {
            // make sure selected view is visible
            if( _stack().currentWidget() != view )
            { setActiveView( *view ); }
            return true;
        }

    }

    return false;
}

//_____________________________________________________________________
void MainWindow::saveAll( void )
{
    Debug::Throw( "MainWindow::saveAll.\n" );
    foreach( TextView* view, BASE::KeySet<TextView>( this ) )
    { view->saveAll(); }
}

//_____________________________________________________________________
void MainWindow::ignoreAll( void )
{
    Debug::Throw( "MainWindow::ignoreAll.\n" );
    foreach( TextView* view, BASE::KeySet<TextView>( this ) )
    { view->ignoreAll(); }
}

//_____________________________________________________________________
void MainWindow::findFromDialog( void )
{
    Debug::Throw( "MainWindow::findFromDialog.\n" );

    // set default text
    // update find text
    QString text( activeDisplay().selection().text() );
    if( !text.isEmpty() )
    {
        const int max_length( 1024 );
        text = text.left( max_length );
    }

    /*
    setting the default text values
    must be done after the dialog is shown
    otherwise it may be automatically resized
    to very large sizes due to the input text
    */

    // set default string to find
    if( !findDialog_ ) _createBaseFindDialog();
    _findDialog().enableRegExp( true );
    _findDialog().centerOnParent().show();
    _findDialog().synchronize();
    _findDialog().clearLabel();
    _findDialog().setText( text );

    // changes focus
    _findDialog().activateWindow();
    _findDialog().editor().setFocus();

    return;
}

//_____________________________________________________________________
void MainWindow::replaceFromDialog( void )
{
    Debug::Throw( "MainWindow::replaceFromDialog.\n" );

    // create
    if( !replaceDialog_ ) _createReplaceDialog();

    // raise dialog
    _replaceDialog().centerOnParent().show();

    /*
    setting the default text values
    must be done after the dialog is shown
    otherwise it may be automatically resized
    to very large sizes due to the input text
    */

    // synchronize combo-boxes
    _replaceDialog().synchronize();
    _replaceDialog().clearLabel();

    // update find text
    QString text;
    if( !( text = qApp->clipboard()->text( QClipboard::Selection) ).isEmpty() ) _replaceDialog().setText( text );
    else if( activeDisplay().textCursor().hasSelection() ) _replaceDialog().setText( activeDisplay().textCursor().selectedText() );
    else if( !( text = TextDisplay::lastSelection().text() ).isEmpty() ) _replaceDialog().setText( text );

    // update replace text
    if( !TextDisplay::lastSelection().replaceText().isEmpty() ) _replaceDialog().setReplaceText( TextDisplay::lastSelection().replaceText() );

    // changes focus
    _replaceDialog().activateWindow();
    _replaceDialog().editor().setFocus();

    return;
}

//________________________________________________
void MainWindow::selectLineFromDialog( void )
{

    Debug::Throw( "TextEditor::selectLineFromDialog.\n" );
    if( !selectLineDialog_ )
    {
        selectLineDialog_ = new SelectLineDialog( this );
        connect( selectLineDialog_, SIGNAL( lineSelected( int ) ), SLOT( _selectLine( int ) ) );
    }

    selectLineDialog_->editor().clear();
    selectLineDialog_->centerOnParent().show();
    selectLineDialog_->activateWindow();
    selectLineDialog_->editor().setFocus();

}

//___________________________________________________________
void MainWindow::_revertToSave( void )
{

    Debug::Throw( "MainWindow::_revertToSave.\n" );

    // check filename
    if( activeDisplay().file().isEmpty() || activeDisplay().isNewDocument() )
    {
        InformationDialog( this, "No filename given. <Reload> canceled." ).setWindowTitle( "Reload Document - Qedit" ).exec();
        return;
    }

    // ask for confirmation
    QString buffer;
    QTextStream what( &buffer );
    if( activeDisplay().document()->isModified() ) what << "Discard changes to " << activeDisplay().file().localName() << "?";
    else what << "Reload file " << activeDisplay().file().localName() << "?";
    if( !QuestionDialog( this, buffer ).setWindowTitle( "Reload Document - Qedit" ).exec() ) return;

    activeDisplay().revertToSave();

}

//___________________________________________________________
void MainWindow::_print( void )
{
    Debug::Throw( "MainWindow::_print.\n" );

    // create printer
    QPrinter printer( QPrinter::HighResolution );
    printer.setDocName( activeDisplay().file().localName() );

    // create helper
    PrintHelper helper( this, &activeDisplay() );

    // create options widget
    PrinterOptionWidget* optionWidget( new PrinterOptionWidget() );
    optionWidget->setHelper( &helper );
    connect( optionWidget, SIGNAL( orientationChanged( QPrinter::Orientation ) ), &helper, SLOT( setOrientation( QPrinter::Orientation ) ) );
    connect( optionWidget, SIGNAL( pageModeChanged( BasePrintHelper::PageMode ) ), &helper, SLOT( setPageMode( BasePrintHelper::PageMode ) ) );

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setWindowTitle( "Print Document - qedit" );
    dialog.setOptionTabs( QList<QWidget *>() << optionWidget );
    if( dialog.exec() == QDialog::Rejected ) return;

    // add output file to scratch files, if any
    if( !printer.outputFileName().isEmpty() )
    { emit scratchFileCreated( printer.outputFileName() ); }

    helper.print( &printer );

    return;

}

//___________________________________________________________
void MainWindow::_printPreview( void )
{
    Debug::Throw( "MainWindow::_printPreview.\n" );

    // create helper
    PrintHelper helper( this, &activeDisplay() );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this );
    dialog.setWindowTitle( "Print Preview - qedit" );
    dialog.setHelper( &helper );
    dialog.exec();
}

//___________________________________________________________
void MainWindow::_toHtml( void )
{
    Debug::Throw( "MainWindow::_toHtml.\n" );

    // create dialog, connect and execute
    HtmlDialog dialog( this );
    dialog.setWindowTitle( "Export to HTML - qedit" );
    dialog.setFile( activeDisplay().file().truncatedName() + ".html" );
    if( !dialog.exec() ) return;

    // retrieve/check file
    File file( dialog.file() );
    if( file.isEmpty() ) {
        InformationDialog(this, "No output file specified. <Export to HTML> canceled." ).exec();
        return;
    }

    QFile out( file );
    if( !out.open( QIODevice::WriteOnly ) )
    {
        QString buffer;
        QTextStream( &buffer ) << "Cannot write to file \"" << file << "\". <Export to HTML> canceled.";
        InformationDialog( this, buffer ).exec();
        return;
    }

    // add as scratch file
    emit scratchFileCreated( file );

    // create helper and print
    HtmlHelper( this, &activeDisplay() ).print( &out );

    // close
    out.close();

    // retrieve command and execute
    const QString command( dialog.command() );
    if( !command.isEmpty() )
    { ( Command( command ) << file ).run(); }

    return;

}

//_______________________________________________________
bool MainWindow::event( QEvent* event )
{

    // check that all needed widgets/actions are valid and checked.
    switch (event->type())
    {

        case QEvent::WindowActivate:
        emit activated( this );
        break;

        default: break;
    }

    return BaseMainWindow::event( event );

}

//____________________________________________
void MainWindow::closeEvent( QCloseEvent* event )
{
    Debug::Throw( "MainWindow::closeEvent.\n" );

    // accept event
    event->accept();

    // look over TextDisplays
    if( !isModified() ) return;

    // loop over TextViews
    unsigned int modifiedDisplays(0);
    BASE::KeySet<TextDisplay> displays;
    foreach( TextView* view, BASE::KeySet<TextView>( this ) )
    {

        // update the number of modified displays
        modifiedDisplays += view->modifiedDisplayCount();

        // store associated textDisplays in main set
        displays.unite( BASE::KeySet<TextDisplay>( view ) );

    }

    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); ++iter )
    {

        // get local reference to display
        TextDisplay& display( **iter );

        // check if this display is modified
        if( !display.document()->isModified() ) continue;

        // this trick allow to run  only once per set of associated displays
        if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( &display ) ) != iter ) continue;

        // ask for save
        int state( display.askForSave( modifiedDisplays > 1 ) );
        if( state == AskForSaveDialog::YES_TO_ALL ) saveAll();
        else if( state == AskForSaveDialog::NO_TO_ALL ) ignoreAll();
        else if( state == AskForSaveDialog::CANCEL ) {

            event->ignore();
            return;

        }

    }

    return;
}

//_______________________________________________________
void MainWindow::timerEvent( QTimerEvent* event )
{

    if( event->timerId() == resizeTimer_.timerId() )
    {

        // stop timer
        resizeTimer_.stop();

        // save size
        if( navigationFrame().visibilityAction().isChecked() )
        { XmlOptions::get().set<int>( "NAVIGATION_FRAME_WIDTH", navigationFrame().width() ); }

    } else return BaseMainWindow::timerEvent( event );

}

//________________________________________________________
void MainWindow::_updateConfiguration( void )
{

    Debug::Throw( "MainWindow::_updateConfiguration.\n" );

    resize( sizeHint() );

    // navigation frame visibility
    navigationFrame().visibilityAction().setChecked( XmlOptions::get().get<bool>("SHOW_NAVIGATION_FRAME") );

    // assign icons to file in open previous menu based on class manager
    FileList& recentFiles( Singleton::get().application<Application>()->recentFiles() );
    DocumentClassManager& classManager(Singleton::get().application<Application>()->classManager());
    foreach( const FileRecord& record, recentFiles.records() )
    {

        // FileRecord& record( *iter );
        if( !record.hasProperty( FileRecordProperties::CLASS_NAME ) ) continue;
        DocumentClass documentClass( classManager.get( record.property( FileRecordProperties::CLASS_NAME ) ) );
        if( documentClass.icon().isEmpty() ) continue;

        // set icon property and store in recentFiles list
        recentFiles.get( record.file() ).addProperty( FileRecordProperties::ICON, documentClass.icon() );

    }

}

//________________________________________________________
void MainWindow::_saveConfiguration( void )
{ Debug::Throw( "MainWindow::_saveConfiguration.\n" ); }

//________________________________________________________
void MainWindow::_toggleNavigationFrame( bool state )
{
    Debug::Throw( "MainWindow::_toggleNavigationFrame.\n" );
    XmlOptions::get().set<bool>( "SHOW_NAVIGATION_FRAME", state );
}

//________________________________________________________
void MainWindow::_splitterMoved( void )
{
    Debug::Throw( "MainWindow::_splitterMoved.\n" );
    resizeTimer_.start( 200, this );
}

//________________________________________________________
void MainWindow::_activeViewChanged( void )
{

    Debug::Throw() << "MainWindow::_activeViewChanged" << endl;

    QWidget *widget( _stack().currentWidget() );
    if( !widget ) close();
    else setActiveView( *static_cast<TextView*>( widget ) );

}

//_______________________________________________________
void MainWindow::_splitDisplay( void )
{ activeView().splitDisplay( Singleton::get().application<Application>()->windowServer().defaultOrientation(), true ); }

//_______________________________________________________
void MainWindow::_multipleFileReplace( void )
{
    Debug::Throw( "MainWindow::_multipleFileReplace.\n" );
    TextSelection selection( _replaceDialog().selection( false ) );

    // show dialog and check answer
    FileSelectionDialog dialog( this, selection );
    QtUtil::centerOnParent( &dialog );
    if( !dialog.centerOnWidget( qApp->activeWindow() ).exec() ) return;

    // replace all in selected files
    Singleton::get().application<Application>()->windowServer().multipleFileReplace( dialog.selectedFiles(), selection );

    return;
}

//_______________________________________________________
void MainWindow::_update( TextDisplay::UpdateFlags flags )
{

    Debug::Throw() << "MainWindow::_update - flags: " << flags << endl;

    if( flags & ( TextDisplay::FILE_NAME | TextDisplay::READ_ONLY | TextDisplay::MODIFIED ) )
    {

        _updateWindowTitle();
        saveAction_->setEnabled( !activeDisplay().isReadOnly() && activeDisplay().document()->isModified() );

    }
    Debug::Throw() << "MainWindow::_update - window title done. "<< endl;

    if( flags & TextDisplay::MODIFIED )
    { emit modificationChanged(); }

    if( flags & TextDisplay::FILE_NAME )
    {

        // update file editor
        if( _hasFileEditor() )
        {
            _fileEditor().setText( activeDisplay().file() );
            filePropertiesAction_->setEnabled( !( activeDisplay().file().isEmpty() || activeDisplay().isNewDocument() ) );
        }

        Debug::Throw() << "MainWindow::_update - file editor done. "<< endl;

        // update session file frame
        if( _hasNavigationFrame() )
        {
            navigationFrame().sessionFilesFrame().select( activeDisplay().file() );
            navigationFrame().recentFilesFrame().select( activeDisplay().file() );
            navigationFrame().fileSystemFrame().setHome( activeDisplay().workingDirectory() );
        }

        Debug::Throw() << "MainWindow::_update - navigation frame done. "<< endl;

        // cursor position
        if( _hasStatusBar() ) _updateCursorPosition();
        Debug::Throw() << "MainWindow::_update - statusbar done. "<< endl;

    }

    if( flags & TextDisplay::DOCUMENT_CLASS && _hasDocumentClassToolBar() )
    { documentClassToolBar_->update( activeDisplay().className() ); }

    if( flags & (TextDisplay::CUT|TextDisplay::READ_ONLY) )
    { cutAction_->setEnabled( activeDisplay().cutAction().isEnabled() ); }

    if( flags & TextDisplay::COPY )
    { copyAction_->setEnabled( activeDisplay().copyAction().isEnabled() ); }

    if( flags & (TextDisplay::CUT|TextDisplay::READ_ONLY) )
    { pasteAction_->setEnabled( activeDisplay().pasteAction().isEnabled() ); }

    if( flags & (TextDisplay::UNDO_REDO|TextDisplay::READ_ONLY) )
    {
        undoAction_->setEnabled( activeDisplay().undoAction().isEnabled() );
        redoAction_->setEnabled( activeDisplay().redoAction().isEnabled() );
    }

    if( _hasStatusBar() && (flags & TextDisplay::MODIFIERS) )
    {
        Debug::Throw() << "MainWindow::_update - modifiers." << endl;
        QStringList modifiers;
        if( activeDisplay().modifier( TextEditor::ModifierWrap ) ) modifiers << "WRAP";
        if( activeDisplay().modifier( TextEditor::ModifierInsert ) ) modifiers << "INS";
        if( activeDisplay().modifier( TextEditor::ModifierCapsLock ) ) modifiers << "CAPS";
        if( activeDisplay().modifier( TextEditor::ModifierNumLock ) ) modifiers << "NUM";
        if( !modifiers.isEmpty() ) _statusBar().label(0).setText( modifiers.join( " " ) );
        else  _statusBar().label(0).clear();
    }

    if( flags & TextDisplay::DISPLAY_COUNT )
    {
        Debug::Throw() << "MainWindow::_update - display count." << endl;
        int displayCount = activeView().independentDisplayCount();
        int viewCount = BASE::KeySet<TextView>( this ).size();

        // update detach action
        detachAction_->setEnabled( displayCount > 1 || viewCount > 1 );

        // update diff action
        diffAction_->setEnabled( displayCount == 2 );
        Debug::Throw() << "MainWindow::_update - display count - done." << endl;

    }

    // macros
    if( flags & TextDisplay::DOCUMENT_CLASS )
    { menu().updateMacroMenu(); }

    if( flags & (TextDisplay::CUT|TextDisplay::COPY) )
    { menu().macroMenu().updateState( activeDisplay().textCursor().hasSelection() ); }

    Debug::Throw() << "MainWindow::_update - done." << endl;

}

//_____________________________________________
void MainWindow::_updateModifiers( void )
{ _update( TextDisplay::MODIFIERS ); }

//_____________________________________________
void MainWindow::_updateCursorPosition( void )
{

    // retrieve position in text
    TextPosition position( activeDisplay().textPosition() );

    /*
    if block delimiters are shown,
    need to count how many blocks are collapsed prior to current
    and increment paragraph consequently
    */
    if( activeDisplay().hasBlockDelimiterDisplay() ) position.paragraph() += activeDisplay().blockDelimiterDisplay().collapsedBlockCount( position.paragraph() );

    // update labels
    statusbar_->label(1).setText( QString( "Line: " ) + QString().setNum( position.paragraph()+1 ) , false );
    statusbar_->label(2).setText( QString( "Column: " ) + QString().setNum( position.index()+1 ) , false );

    return;
}

//_____________________________________________
void MainWindow::_replaceTransitionWidget( void )
{

    Debug::Throw( "MainWindow::_replaceTransitionWidget.\n" );

    transitionWidget_ = new TransitionWidget( this );
    _transitionWidget().setFlag( TransitionWidget::FROM_PARENT, false );
    _transitionWidget().hide();
    connect( &_transitionWidget(), SIGNAL( destroyed() ), SLOT( _replaceTransitionWidget() ) );
    connect( &_transitionWidget().timeLine(), SIGNAL( finished() ), SLOT( _animationFinished() ) );

}

//_____________________________________________
void MainWindow::_animationFinished( void )
{
    Debug::Throw( "MainWindow::_animationFinished.\n" );
    _transitionWidget().setParent( this );
    _transitionWidget().hide();

    // update displays, actions, etc.
    if( activeView().activeDisplay().file().size() || activeView().activeDisplay().isNewDocument() )
    { _update( TextDisplay::ACTIVE_VIEW_CHANGED ); }

    Debug::Throw( "MainWindow::_animationFinished - done.\n" );

}

//___________________________________________________________
void MainWindow::_installActions( void )
{

    Debug::Throw( "MainWindow::_installActions.\n" );

    addAction( newFileAction_ = new QAction( IconEngine::get( ICONS::NEW ), "New", this ) );
    newFileAction_->setShortcut( QKeySequence::New );
    newFileAction_->setToolTip( "Create a new empty file" );

    addAction( cloneAction_ = new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), "Clone", this ) );
    cloneAction_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_N );
    cloneAction_->setToolTip( "Clone current display" );
    connect( cloneAction_, SIGNAL( triggered() ), SLOT( _splitDisplay() ) );

    addAction( detachAction_ = new QAction( IconEngine::get( ICONS::VIEW_DETACH ), "Detach", this ) );
    detachAction_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_O );
    detachAction_->setToolTip( "Detach current display" );
    detachAction_->setEnabled( false );

    addAction( openAction_ = new QAction( IconEngine::get( ICONS::OPEN ), "Open...", this ) );
    openAction_->setShortcut( QKeySequence::Open );
    openAction_->setToolTip( "Open an existing file" );

    addAction( openHorizontalAction_ =new QAction( IconEngine::get( ICONS::VIEW_BOTTOM ), "Open Display Top/Bottom...", this ) );
    openHorizontalAction_->setToolTip( "Open a new display vertically" );

    addAction( openVerticalAction_ =new QAction( IconEngine::get( ICONS::VIEW_RIGHT ), "Open Display Left/Right...", this ) );
    openVerticalAction_->setToolTip( "Open a new display horizontally" );

    addAction( closeDisplayAction_ = new QAction( IconEngine::get( ICONS::VIEW_REMOVE ), "Close Display", this ) );
    closeDisplayAction_->setShortcut( QKeySequence::Close );
    closeDisplayAction_->setToolTip( "Close current display" );
    connect( closeDisplayAction_, SIGNAL( triggered() ), SLOT( _closeDisplay() ) );

    addAction( closeWindowAction_ = new QAction( IconEngine::get( ICONS::CLOSE ), "Close Window", this ) );
    closeWindowAction_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_W );
    closeWindowAction_->setToolTip( "Close current display" );
    connect( closeWindowAction_, SIGNAL( triggered() ), SLOT( _closeWindow() ) );

    addAction( saveAction_ = new QAction( IconEngine::get( ICONS::SAVE ), "Save", this ) );
    saveAction_->setShortcut( QKeySequence::Save );
    saveAction_->setToolTip( "Save current file" );
    connect( saveAction_, SIGNAL( triggered() ), SLOT( _save() ) );

    addAction( saveAsAction_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save As...", this ) );
    saveAsAction_->setShortcut( QKeySequence::SaveAs );
    saveAsAction_->setToolTip( "Save current file with a different name" );
    connect( saveAsAction_, SIGNAL( triggered() ), SLOT( _saveAs() ) );

    addAction( revertToSaveAction_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Reload", this ) );
    revertToSaveAction_->setShortcut( QKeySequence::Refresh );
    revertToSaveAction_->setToolTip( "Reload saved version of current file" );
    connect( revertToSaveAction_, SIGNAL( triggered() ), SLOT( _revertToSave() ) );

    addAction( printAction_ = new QAction( IconEngine::get( ICONS::PRINT ), "Print...", this ) );
    printAction_->setToolTip( "Print current file" );
    printAction_->setShortcut( QKeySequence::Print );
    connect( printAction_, SIGNAL( triggered() ), SLOT( _print() ) );

    addAction( printPreviewAction_ = new QAction( IconEngine::get( ICONS::PRINT_PREVIEW ), "Print Preview...", this ) );
    connect( printPreviewAction_, SIGNAL( triggered() ), SLOT( _printPreview() ) );

    addAction( htmlAction_ = new QAction( IconEngine::get( ICONS::HTML ), "Export to HTML...", this ) );
    connect( htmlAction_, SIGNAL( triggered() ), SLOT( _toHtml() ) );

    addAction( undoAction_ = new QAction( IconEngine::get( ICONS::UNDO ), "Undo", this ) );
    undoAction_->setToolTip( "Undo last action" );
    undoAction_->setEnabled( false );
    connect( undoAction_, SIGNAL( triggered() ), SLOT( _undo() ) );

    addAction( redoAction_ = new QAction( IconEngine::get( ICONS::REDO ), "Redo", this ) );
    redoAction_->setToolTip( "Redo last un-done action" );
    redoAction_->setEnabled( false );
    connect( redoAction_, SIGNAL( triggered() ), SLOT( _redo() ) );

    addAction( cutAction_ = new QAction( IconEngine::get( ICONS::CUT ), "Cut", this ) );
    cutAction_->setToolTip( "Cut current selection and copy to clipboard" );
    cutAction_->setEnabled( false );
    connect( cutAction_, SIGNAL( triggered() ), SLOT( _cut() ) );

    addAction( copyAction_ = new QAction( IconEngine::get( ICONS::COPY ), "Copy", this ) );
    copyAction_->setToolTip( "Copy current selection to clipboard" );
    copyAction_->setEnabled( false );
    connect( copyAction_, SIGNAL( triggered() ), SLOT( _copy() ) );

    addAction( pasteAction_ = new QAction( IconEngine::get( ICONS::PASTE ), "Paste", this ) );
    pasteAction_->setToolTip( "Paste clipboard to text" );
    pasteAction_->setEnabled( !qApp->clipboard()->text().isEmpty() );
    connect( pasteAction_, SIGNAL( triggered() ), SLOT( _paste() ) );

    addAction( filePropertiesAction_ = new QAction( IconEngine::get( ICONS::INFORMATION ), "Properties...", this ) );
    filePropertiesAction_->setShortcut( Qt::ALT + Qt::Key_Return );
    filePropertiesAction_->setToolTip( "Display file informations" );
    filePropertiesAction_->setEnabled( false );
    connect( filePropertiesAction_, SIGNAL( triggered() ), SLOT( _fileInfo() ) );

    addAction( spellcheckAction_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), "Spell Check...", this ) );
    #if WITH_ASPELL
    connect( spellcheckAction_, SIGNAL( triggered() ), SLOT( _spellcheck( void ) ) );

    // disable action if there is no dictionary
    spellcheckAction_->setEnabled( !SPELLCHECK::SpellInterface().dictionaries().empty() );
    #else
    spellcheckAction_->setVisible( false );
    #endif

    addAction( diffAction_ = new QAction( "Diff Files", this ) );
    connect( diffAction_, SIGNAL( triggered() ), SLOT( _diff() ) );
    diffAction_->setEnabled( false );

    addAction( splitDisplayHorizontalAction_ =new QAction( IconEngine::get( ICONS::VIEW_TOPBOTTOM ), "Clone Display Top/Bottom", this ) );
    splitDisplayHorizontalAction_->setToolTip( "Clone current display vertically" );
    connect( splitDisplayHorizontalAction_, SIGNAL( triggered() ), SLOT( _splitDisplayVertical() ) );

    addAction( splitDisplayVerticalAction_ =new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), "Clone Display Left/Right", this ) );
    splitDisplayVerticalAction_->setToolTip( "Clone current display horizontally" );
    connect( splitDisplayVerticalAction_, SIGNAL( triggered() ), SLOT( _splitDisplayHorizontal() ) );

}

//______________________________________________________________________
void MainWindow::_installToolbars( void )
{

    // file toolbar
    CustomToolBar* toolbar = new CustomToolBar( "Main", this, "FILE_TOOLBAR" );
    toolbar->addAction( newFileAction_ );
    toolbar->addAction( openAction_ );
    toolbar->addAction( saveAction_ );

    // edition toolbar
    toolbar = new CustomToolBar( "Edition", this, "EDITION_TOOLBAR" );
    toolbar->addAction( undoAction_ );
    toolbar->addAction( redoAction_ );
    toolbar->addAction( cutAction_ );
    toolbar->addAction( copyAction_ );
    toolbar->addAction( pasteAction_ );

    // extra toolbar
    toolbar = new CustomToolBar( "Tools", this, "EXTRA_TOOLBAR" );
    toolbar->addAction( filePropertiesAction_ );
    toolbar->addAction( spellcheckAction_ );

    // splitting toolbar
    toolbar = new CustomToolBar( "Multiple Displays", this, "SPLIT_TOOLBAR" );
    toolbar->addAction( splitDisplayHorizontalAction_ );
    toolbar->addAction( splitDisplayVerticalAction_ );
    toolbar->addAction( openHorizontalAction_ );
    toolbar->addAction( openVerticalAction_ );
    toolbar->addAction( closeDisplayAction_ );
    toolbar->addAction( detachAction_ );

    // document class toolbar
    documentClassToolBar_ = new DocumentClassToolBar( this );
    connect( documentClassToolBar_, SIGNAL( documentClassSelected( QString ) ), this, SLOT( selectClassName( QString ) ) );

    // navigation toolbar
    NavigationToolBar* navigation_toolbar = new NavigationToolBar( this );
    navigation_toolbar->connect( navigationFrame() );


}

//______________________________________________________________________
void MainWindow::_createBaseFindDialog( void )
{

    Debug::Throw( "MainWindow::_createBaseFindDialog.\n" );
    if( !findDialog_ )
    {

        findDialog_ = new BaseFindDialog( this );
        findDialog_->setWindowTitle( "Find in Text - Qedit" );
        connect( findDialog_, SIGNAL( find( TextSelection ) ), SLOT( _find( TextSelection ) ) );
        connect( this, SIGNAL( noMatchFound() ), findDialog_, SLOT( noMatchFound() ) );
        connect( this, SIGNAL( matchFound() ), findDialog_, SLOT( clearLabel() ) );

    }

    return;

}

//_____________________________________________________________________
void MainWindow::_createReplaceDialog( void )
{
    Debug::Throw( "MainWindow::_CreateReplaceDialog.\n" );
    if( !replaceDialog_ )
    {

        replaceDialog_ = new ReplaceDialog( this );
        replaceDialog_->setWindowTitle( "Replace in Text - Qedit" );
        connect( replaceDialog_, SIGNAL( find( TextSelection ) ), SLOT( _find( TextSelection ) ) );
        connect( replaceDialog_, SIGNAL( replace( TextSelection ) ), SLOT( _replace( TextSelection ) ) );
        connect( replaceDialog_, SIGNAL( replaceInWindow( TextSelection ) ), SLOT( _replaceInWindow( TextSelection ) ) );
        connect( replaceDialog_, SIGNAL( replaceInSelection( TextSelection ) ), SLOT( _replaceInSelection( TextSelection ) ) );
        connect( replaceDialog_, SIGNAL( replaceInFiles( void ) ), SLOT( _multipleFileReplace( void ) ) );

        connect( this, SIGNAL( noMatchFound( void ) ), replaceDialog_, SLOT( noMatchFound( void ) ) );
        connect( this, SIGNAL( matchFound( void ) ), replaceDialog_, SLOT( clearLabel( void ) ) );

    }

}

//_________________________________________________________________
void MainWindow::_connectView( TextView& view )
{
    Debug::Throw( "MainWindow::_connectView.\n" );
    connect( &view, SIGNAL( modifiersChanged( TextEditor::Modifiers ) ), SLOT( _updateModifiers( void ) ) );
    connect( &view, SIGNAL( needUpdate( TextDisplay::UpdateFlags ) ), SLOT( _update( TextDisplay::UpdateFlags ) ) );
    connect( &view, SIGNAL( displayCountChanged( void ) ), SLOT( _updateDisplayCount( void ) ) );
    connect( &view, SIGNAL( displayCountChanged( void ) ), &Singleton::get().application<Application>()->windowServer(), SIGNAL( sessionFilesChanged( void ) ) );
    connect( &view, SIGNAL( undoAvailable( bool ) ), &undoAction(), SLOT( setEnabled( bool ) ) );
    connect( &view, SIGNAL( redoAvailable( bool ) ), &redoAction(), SLOT( setEnabled( bool ) ) );
    connect( &view.positionTimer(), SIGNAL( timeout() ), SLOT( _updateCursorPosition() ) );

}

//___________________________________________________________
void MainWindow::_updateWindowTitle()
{
    Debug::Throw( "MainWindow::_updateWindowTitle.\n" );

    {
        bool readonly( activeDisplay().isReadOnly() );
        bool modified( activeDisplay().document()->isModified() );
        Debug::Throw() << "MainWindow::_updateWindowTitle -"
            << " file: " << activeDisplay().file()
            << " readonly: " << readonly
            << " modified: " << modified
            << endl;
    }

    setWindowTitle( WindowTitle( activeDisplay().file() )
        .setReadOnly( activeDisplay().isReadOnly() )
        .setModified( activeDisplay().document()->isModified() )
        );

}
