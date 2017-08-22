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

#include "MainWindow.h"

#include "Application.h"
#include "AutoSave.h"
#include "BaseStatusBar.h"
#include "BlockDelimiterDisplay.h"
#include "ClockLabel.h"
#include "Command.h"
#include "CppUtil.h"
#include "CustomToolButton.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "Diff.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "DocumentClassToolBar.h"
#include "ElidedLabel.h"
#include "FileCheckDialog.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "FileSelectionDialog.h"
#include "FileSystemFrame.h"
#include "HighlightBlockFlags.h"
#include "HtmlDialog.h"
#include "HtmlHelper.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "InformationDialog.h"
#include "LineEditor.h"
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
#include "ReplaceWidget.h"
#include "SelectLineWidget.h"
#include "SessionFilesFrame.h"
#include "Singleton.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "TextMacroMenu.h"
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
MainWindow::MainWindow(  QWidget* parent ):
    BaseMainWindow( parent ),
    Counter( "MainWindow" )
{

    Debug::Throw( "MainWindow::MainWindow.\n" );
    setOptionName( "WINDOW" );

    // tell window to delete on exit
    setAttribute( Qt::WA_DeleteOnClose );

    // install actions
    _installActions();

    // additional actions from Application
    // they need to be added so that short cuts still work even when menu bar is hidden)
    auto&& application( Base::Singleton::get().application<Application>() );
    addAction( &application->closeAction() );

    // menu
    setMenuBar( menu_ = new Menu( this ) );
    connect( &menu().documentClassMenu(), SIGNAL(documentClassSelected(QString)), SLOT(selectClassName(QString)) );

    // main widget is a splitter to store navigation window and active view
    QSplitter* splitter = new QSplitter( this );
    splitter->setOrientation( Qt::Horizontal );
    setCentralWidget( splitter );

    // insert navigationFrame
    navigationFrame_ = new NavigationFrame(0, application->recentFiles() );
    navigationFrame_->setDefaultWidth( XmlOptions::get().get<int>( "NAVIGATION_FRAME_WIDTH" ) );
    splitter->addWidget( &navigationFrame() );

    connect( &navigationFrame_->visibilityAction(), SIGNAL(toggled(bool)), SLOT(_toggleNavigationFrame(bool)) );

    // need to add navigationFrame visibility action to this list
    // to enable shortcut event if the frame is hidden
    addAction( &navigationFrame_->visibilityAction() );
    connect( &navigationFrame_->sessionFilesFrame(), SIGNAL(fileSelected(FileRecord)), SLOT(_selectDisplay(FileRecord)) );
    connect( &navigationFrame_->recentFilesFrame(), SIGNAL(fileSelected(FileRecord)), SLOT(_selectDisplay(FileRecord)) );

    addAction( &navigationFrame_->sessionFilesFrame().nextFileAction() );
    addAction( &navigationFrame_->sessionFilesFrame().previousFileAction() );

    // insert stack widget
    // right container
    rightContainer_ = new QWidget( nullptr );
    rightContainer_->setLayout( new QVBoxLayout );
    rightContainer_->layout()->setMargin(0);
    rightContainer_->layout()->setSpacing(0);
    splitter->addWidget( rightContainer_ );

    // stack
    stack_ = new QStackedWidget( rightContainer_ );
    rightContainer_->layout()->addWidget( stack_ );
    stack_->layout()->setMargin(2);
    connect( stack_, SIGNAL(widgetRemoved(int)), SLOT(_activeViewChanged()) );

    // embedded widgets
    _createFindWidget();
    _createReplaceWidget();
    _createSelectLineWidget();

    // create first text view
    newTextView();

    // assign stretch factors
    splitter->setStretchFactor( 0, 0 );
    splitter->setStretchFactor( 1, 1 );
    connect( splitter, SIGNAL(splitterMoved(int,int)), SLOT(_splitterMoved()) );

    // state frame
    setStatusBar( statusbar_ = new BaseStatusBar( this ) );

    // file editor
    statusbar_->addPermanentWidget( fileEditor_ = new ElidedLabel( statusbar_ ), 1 );

    // other labels
    statusbar_->addLabels( 3, 0 );
    statusbar_->label(0).setAlignment( Qt::AlignCenter );
    statusbar_->label(1).setAlignment( Qt::AlignCenter );
    statusbar_->label(2).setAlignment( Qt::AlignCenter );
    statusbar_->addClock();

    fileEditor_->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );

    // toolbars
    _installToolbars();

    //* configuration
    connect( application, SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    connect( application, SIGNAL(saveConfiguration()), SLOT(_saveConfiguration()) );
    connect( qApp, SIGNAL(aboutToQuit()), SLOT(_saveConfiguration()) );
    _updateConfiguration();

}

//___________________________________________________________
TextView& MainWindow::newTextView( FileRecord record )
{
    Debug::Throw( "MainWindow::newTextView.\n" );

    // create new view and add to this file
    auto view = new TextView( this );
    Base::Key::associate( this, view );

    // connections
    _connectView( *view );

    // open file if valid
    if( record.file().exists() ) view->setFile( record.file() );

    // add to stack and set active
    stack_->addWidget( view );
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
    if( view.isClosed() ) return;

    // store active view
    activeView_ = &view;
    activeView_->activeDisplay().setFocusDelayed();

    // update stack if needed
    if( stack_->currentWidget() !=  &activeView() ) stack_->setCurrentWidget( &activeView() );

    // update displays, actions, etc.
    if( !activeView_->activeDisplay().file().isEmpty() || activeView_->activeDisplay().isNewDocument() )
    { _update( TextDisplay::ActiveViewChanged ); }

}


//_____________________________________________________________________
Base::KeySet<TextDisplay> MainWindow::associatedDisplays() const
{
    Base::KeySet<TextDisplay> displays;
    for( const auto& view:Base::KeySet<TextView>( this ) )
    { displays.unite( Base::KeySet<TextDisplay>(view) ); }

    return displays;
}

//_____________________________________________________________________
bool MainWindow::selectDisplay( const File& file )
{

    Debug::Throw() << "MainWindow::selectDisplay - file: " << file << endl;
    Debug::Throw() << "MainWindow::selectDisplay - active view: " << activeView_->key() << endl;
    Debug::Throw() << "MainWindow::selectDisplay - active view displays: " << Base::KeySet<TextDisplay>( &activeView() ).size() << endl;

    // do nothing if already selected
    if( !activeView_->isClosed() && activeView_->activeDisplay().file() == file ) return true;

    for( const auto& view:Base::KeySet<TextView>( this ) )
    {

        if( !view->isClosed() && view->selectDisplay( file ) )
        {
            // make sure selected view is visible
            if( stack_->currentWidget() != view )
            { setActiveView( *view ); }
            return true;
        }

    }

    return false;
}

//_____________________________________________________________________
void MainWindow::saveAll()
{
    Debug::Throw( "MainWindow::saveAll.\n" );
    for( const auto& view:Base::KeySet<TextView>( this ) )
    { view->saveAll(); }
}

//_____________________________________________________________________
void MainWindow::ignoreAll()
{
    Debug::Throw( "MainWindow::ignoreAll.\n" );
    for( const auto& view:Base::KeySet<TextView>( this ) )
    { view->ignoreAll(); }
}

//_____________________________________________________________________
void MainWindow::findFromDialog()
{
    Debug::Throw( "MainWindow::findFromDialog.\n" );

    // create find widget
    if( !findWidget_ ) _createFindWidget();
    findWidget_->show();
    findWidget_->editor().setFocus();
    activeDisplay().ensureCursorVisible();

    /*
    setting the default text values
    must be done after the dialog is shown
    otherwise it may be automatically resized
    to very large sizes due to the input text
    */
    QString text( activeDisplay().selection().text() );
    if( !text.isEmpty() )
    {
        const int maxLength( 1024 );
        text = text.left( maxLength );
    }

    findWidget_->enableRegExp( true );
    findWidget_->synchronize();
    findWidget_->matchFound();
    findWidget_->setText( text );

    return;
}

//_____________________________________________________________________
void MainWindow::replaceFromDialog()
{
    Debug::Throw( "MainWindow::replaceFromDialog.\n" );

    // create replace widget
    if( !replaceWidget_ ) _createReplaceWidget();

    // show replace widget and set focus
    replaceWidget_->show();
    replaceWidget_->editor().setFocus();
    activeDisplay().ensureCursorVisible();

    /*
    setting the default text values
    must be done after the dialog is shown
    otherwise it may be automatically resized
    to very large sizes due to the input text
    */

    // synchronize combo-boxes
    replaceWidget_->synchronize();
    replaceWidget_->matchFound();

    // update find text
    QString text;
    if( !( text = qApp->clipboard()->text( QClipboard::Selection) ).isEmpty() ) replaceWidget_->setText( text );
    else if( activeDisplay().textCursor().hasSelection() ) replaceWidget_->setText( activeDisplay().textCursor().selectedText() );
    else if( !( text = TextDisplay::lastSelection().text() ).isEmpty() ) replaceWidget_->setText( text );

    // update replace text
    if( !TextDisplay::lastSelection().replaceText().isEmpty() ) replaceWidget_->setReplaceText( TextDisplay::lastSelection().replaceText() );

    return;
}

//________________________________________________
void MainWindow::selectLineFromDialog()
{

    Debug::Throw( "TextEditor::selectLineFromDialog.\n" );

    // create select line widget
    if( !selectLineWidget_ ) _createSelectLineWidget();

    selectLineWidget_->show();
    selectLineWidget_->matchFound();
    selectLineWidget_->editor().clear();
    selectLineWidget_->editor().setFocus();

}

//___________________________________________________________
void MainWindow::_revertToSave()
{

    Debug::Throw( "MainWindow::_revertToSave.\n" );

    // check filename
    if( activeDisplay().file().isEmpty() || activeDisplay().isNewDocument() )
    {
        InformationDialog dialog( this, tr( "No filename given. <Reload> canceled." ) );
        dialog.setWindowTitle( tr( "Reload Document - Qedit" ) );
        dialog.exec();
        return;
    }

    // ask for confirmation
    QString buffer;
    if( activeDisplay().document()->isModified() ) buffer = QString( tr( "Discard changes to file '%1' ?" ) ).arg( activeDisplay().file().localName() );
    else buffer = QString( tr( "Reload file '%1' ?" ) ).arg( activeDisplay().file().localName() );

    QuestionDialog dialog( this, buffer );
    dialog.setWindowTitle( tr( "Reload Document - Qedit" ) );
    if( !dialog.exec() ) return;

    activeDisplay().revertToSave();

}

//___________________________________________________________
void MainWindow::_print()
{
    Debug::Throw( "MainWindow::_print.\n" );
    PrintHelper helper( this, &activeDisplay() );
    _print( helper );
}

//___________________________________________________________
void MainWindow::_print( PrintHelper& helper )
{

    // create printer
    QPrinter printer( QPrinter::HighResolution );
    printer.setDocName( activeDisplay().file().localName() );

    // create options widget
    PrinterOptionWidget* optionWidget( new PrinterOptionWidget );
    optionWidget->setHelper( &helper );
    connect( optionWidget, SIGNAL(orientationChanged(QPrinter::Orientation)), &helper, SLOT(setOrientation(QPrinter::Orientation)) );
    connect( optionWidget, SIGNAL(pageModeChanged(BasePrintHelper::PageMode)), &helper, SLOT(setPageMode(BasePrintHelper::PageMode)) );

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setWindowTitle( tr( "Print Document - qedit" ) );

    using WidgetList = QList<QWidget*>;
    dialog.setOptionTabs( Base::makeT<WidgetList>({ optionWidget }) );
    if( dialog.exec() == QDialog::Rejected ) return;

    // add output file to scratch files, if any
    if( !printer.outputFileName().isEmpty() )
    { emit scratchFileCreated( File( printer.outputFileName() ) ); }

    helper.print( &printer );

    return;

}

//___________________________________________________________
void MainWindow::_printPreview()
{
    Debug::Throw( "MainWindow::_printPreview.\n" );

    // create helper
    PrintHelper helper( this, &activeDisplay() );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this, CustomDialog::OkButton|CustomDialog::CancelButton );
    dialog.setWindowTitle( tr( "Print Preview - qedit" ) );
    dialog.setHelper( &helper );
    if( !dialog.exec() ) return;
    _print( helper );

}

//___________________________________________________________
void MainWindow::_toHtml()
{
    Debug::Throw( "MainWindow::_toHtml.\n" );

    // create dialog, connect and execute
    HtmlDialog dialog( this );
    dialog.setWindowTitle( tr( "Export to HTML - qedit" ) );
    dialog.setFile( File( activeDisplay().file().truncatedName() + ".html" ) );
    if( !dialog.exec() ) return;

    // retrieve/check file
    File file( dialog.file() );
    if( file.isEmpty() ) {
        InformationDialog(this, tr( "No output file specified. <Export to HTML> canceled." ) ).exec();
        return;
    }

    QFile out( file );
    if( !out.open( QIODevice::WriteOnly ) )
    {
        InformationDialog( this, QString( tr( "Cannot write to file '%1'. <Export to HTML> canceled." ) ).arg( file ) ).exec();
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
    int modifiedDisplays(0);
    Base::KeySet<TextDisplay> displays;
    for( const auto& view:Base::KeySet<TextView>( this ) )
    {

        // update the number of modified displays
        modifiedDisplays += view->modifiedDisplayCount();

        // store associated textDisplays in main set
        displays.unite( Base::KeySet<TextDisplay>( view ) );

    }

    for( auto&& iter = displays.begin(); iter != displays.end(); ++iter )
    {

        // get local reference to display
        TextDisplay& display( **iter );

        // check if this display is modified
        if( !display.document()->isModified() ) continue;

        // this trick allow to run  only once per set of associated displays
        if( std::any_of( displays.begin(), iter, Base::Key::IsAssociatedFTor( &display ) ) ) continue;

        // ask for save
        int state( display.askForSave( modifiedDisplays > 1 ) );
        if( state == AskForSaveDialog::YesToAll ) saveAll();
        else if( state == AskForSaveDialog::NoToAll ) ignoreAll();
        else if( state == AskForSaveDialog::Cancel ) {

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
        if( navigationFrame_->visibilityAction().isChecked() )
        { XmlOptions::get().set<int>( "NAVIGATION_FRAME_WIDTH", navigationFrame_->width() ); }

    } else return BaseMainWindow::timerEvent( event );

}

//________________________________________________________
void MainWindow::_updateConfiguration()
{

    Debug::Throw( "MainWindow::_updateConfiguration.\n" );

    resize( sizeHint() );

    // navigation frame visibility
    navigationFrame_->visibilityAction().setChecked( XmlOptions::get().get<bool>("SHOW_NAVIGATION_FRAME") );

    // assign icons to file in open previous menu based on class manager
    auto&& recentFiles( Base::Singleton::get().application<Application>()->recentFiles() );
    auto&& classManager(Base::Singleton::get().application<Application>()->classManager());
    for( const auto& record:recentFiles.records() )
    {

        // FileRecord& record( *iter );
        if( !record.hasProperty( FileRecordProperties::ClassName ) ) continue;
        DocumentClass documentClass( classManager.get( record.property( FileRecordProperties::ClassName ) ) );
        if( documentClass.icon().isEmpty() ) continue;

        // set icon property and store in recentFiles list
        recentFiles.get( record.file() ).addProperty( FileRecordProperties::Icon, documentClass.icon() );

    }

}

//________________________________________________________
void MainWindow::_saveConfiguration()
{ Debug::Throw( "MainWindow::_saveConfiguration.\n" ); }

//________________________________________________________
void MainWindow::_toggleNavigationFrame( bool state )
{
    Debug::Throw( "MainWindow::_toggleNavigationFrame.\n" );
    XmlOptions::get().set<bool>( "SHOW_NAVIGATION_FRAME", state );
}

//________________________________________________________
void MainWindow::_splitterMoved()
{
    Debug::Throw( "MainWindow::_splitterMoved.\n" );
    resizeTimer_.start( 200, this );
}

//________________________________________________________
void MainWindow::_activeViewChanged()
{

    Debug::Throw() << "MainWindow::_activeViewChanged" << endl;

    auto widget( stack_->currentWidget() );
    if( !widget ) close();
    else setActiveView( *static_cast<TextView*>( widget ) );

}

//_______________________________________________________
void MainWindow::_splitDisplay()
{ activeView_->splitDisplay( Base::Singleton::get().application<Application>()->windowServer().defaultOrientation(), true ); }

void MainWindow::_multipleFileReplace()
{
    Debug::Throw( "MainWindow::_multipleFileReplace.\n" );
    auto selection( replaceWidget_->selection( false ) );

    // show dialog and check answer
    FileSelectionDialog dialog( this, selection );
    dialog.centerOnParent();
    if( !dialog.exec() ) return;

    // replace all in selected files
    Base::Singleton::get().application<Application>()->windowServer().multipleFileReplace( dialog.selectedFiles(), selection );

    return;
}

//_______________________________________________________
void MainWindow::_update( TextDisplay::UpdateFlags flags )
{

    Debug::Throw() << "MainWindow::_update - flags: " << flags << endl;

    if( flags & ( TextDisplay::FileName | TextDisplay::ReadOnly | TextDisplay::Modified ) )
    {

        _updateWindowTitle();
        saveAction_->setEnabled( !activeDisplay().isReadOnly() && activeDisplay().document()->isModified() );

    }

    if( flags & TextDisplay::Modified )
    { emit modificationChanged(); }

    if( flags & TextDisplay::FileName )
    {

        // update file editor
        if( fileEditor_ )
        {
            fileEditor_->setText( activeDisplay().file() );
            filePropertiesAction_->setEnabled( !( activeDisplay().file().isEmpty() || activeDisplay().isNewDocument() ) );
        }

        // update session file frame
        if( navigationFrame_ )
        {
            navigationFrame_->sessionFilesFrame().select( activeDisplay().file() );
            navigationFrame_->recentFilesFrame().select( activeDisplay().file() );
            navigationFrame_->fileSystemFrame().setWorkingPath( activeDisplay().workingDirectory() );
        }

        // cursor position
        _updateCursorPosition();

    }

    if( flags & TextDisplay::DocumentClassFlag && documentClassToolBar_ )
    { documentClassToolBar_->update( activeDisplay().className() ); }

    if( flags & (TextDisplay::Cut|TextDisplay::ReadOnly) )
    { cutAction_->setEnabled( activeDisplay().cutAction().isEnabled() ); }

    if( flags & TextDisplay::Copy )
    { copyAction_->setEnabled( activeDisplay().copyAction().isEnabled() ); }

    if( flags & (TextDisplay::Cut|TextDisplay::ReadOnly) )
    { pasteAction_->setEnabled( activeDisplay().pasteAction().isEnabled() ); }

    if( flags & (TextDisplay::UndoRedo|TextDisplay::ReadOnly) )
    {
        undoAction_->setEnabled( activeDisplay().undoAction().isEnabled() );
        redoAction_->setEnabled( activeDisplay().redoAction().isEnabled() );
    }

    if( _hasStatusBar() && (flags & TextDisplay::Modifiers) )
    {
        QStringList modifiers;
        if( activeDisplay().modifier( TextEditor::Modifier::Wrap ) ) modifiers.append( "WRAP" );
        if( activeDisplay().modifier( TextEditor::Modifier::Insert ) ) modifiers.append( "INS" );
        if( activeDisplay().modifier( TextEditor::Modifier::CapsLock ) ) modifiers.append( "CAPS" );
        if( activeDisplay().modifier( TextEditor::Modifier::NumLock ) ) modifiers.append( "NUM" );
        if( !modifiers.isEmpty() ) statusbar_->label(0).setText( modifiers.join( " " ) );
        else  statusbar_->label(0).clear();
    }

    if( flags & TextDisplay::DisplayCount )
    {
        int displayCount = activeView_->independentDisplayCount();
        int viewCount = Base::KeySet<TextView>( this ).size();

        // update detach action
        detachAction_->setEnabled( displayCount > 1 || viewCount > 1 );

        // update diff action
        diffAction_->setEnabled( displayCount == 2 );

    }

    // macros
    if( flags & TextDisplay::DocumentClassFlag )
    { menu().updateMacroMenu(); }

    if( flags & (TextDisplay::Cut|TextDisplay::Copy) )
    { menu().macroMenu().updateState( activeDisplay().textCursor().hasSelection() ); }

}

//_____________________________________________
void MainWindow::_updateReplaceInSelection()
{ if( replaceWidget_ ) replaceWidget_->enableReplaceInSelection( activeDisplay().hasSelection() ); }

//_____________________________________________
void MainWindow::_updateModifiers()
{ _update( TextDisplay::Modifiers ); }

//_____________________________________________
void MainWindow::_updateCursorPosition()
{

    if( !statusbar_ ) return;

    // retrieve position in text
    auto position( activeDisplay().textPosition() );

    /*
    if block delimiters are shown,
    need to count how many blocks are collapsed prior to current
    and increment paragraph consequently
    */
    if( activeDisplay().hasBlockDelimiterDisplay() ) position.paragraph() += activeDisplay().blockDelimiterDisplay().collapsedBlockCount( position.paragraph() );

    // update labels
    statusbar_->label(1).setText( QString( tr( "Line: %1" ) ).arg( position.paragraph()+1 ) , false );
    statusbar_->label(2).setText( QString( tr( "Column: %1" ) ).arg( position.index()+1 ) , false );

    return;
}

//___________________________________________________________
void MainWindow::_installActions()
{

    Debug::Throw( "MainWindow::_installActions.\n" );

    addAction( newFileAction_ = new QAction( IconEngine::get( IconNames::New ), tr( "New" ), this ) );
    newFileAction_->setShortcut( QKeySequence::New );
    newFileAction_->setToolTip( tr( "Create a new empty file" ) );

    addAction( cloneAction_ = new QAction( IconEngine::get( IconNames::ViewLeftRight ), tr( "Clone" ), this ) );
    cloneAction_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_N );
    cloneAction_->setToolTip( tr( "Clone current display" ) );
    connect( cloneAction_, SIGNAL(triggered()), SLOT(_splitDisplay()) );

    addAction( detachAction_ = new QAction( IconEngine::get( IconNames::ViewDetach ), tr( "Detach" ), this ) );
    detachAction_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_O );
    detachAction_->setToolTip( tr( "Detach current display" ) );
    detachAction_->setEnabled( false );

    addAction( openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open..." ), this ) );
    openAction_->setShortcut( QKeySequence::Open );
    openAction_->setToolTip( tr( "Open an existing file" ) );

    addAction( openHorizontalAction_ =new QAction( IconEngine::get( IconNames::ViewBottom ), tr( "Open Display Top/Bottom..." ), this ) );
    openHorizontalAction_->setToolTip( tr( "Open a new display vertically" ) );

    addAction( openVerticalAction_ =new QAction( IconEngine::get( IconNames::ViewRight ), tr( "Open Display Left/Right..." ), this ) );
    openVerticalAction_->setToolTip( tr( "Open a new display horizontally" ) );

    addAction( closeDisplayAction_ = new QAction( IconEngine::get( IconNames::ViewRemove ), tr( "Close Display" ), this ) );
    closeDisplayAction_->setShortcut( QKeySequence::Close );
    closeDisplayAction_->setToolTip( tr( "Close current display" ) );
    connect( closeDisplayAction_, SIGNAL(triggered()), SLOT(_closeDisplay()) );

    addAction( closeWindowAction_ = new QAction( IconEngine::get( IconNames::Close ), tr( "Close Window" ), this ) );
    closeWindowAction_->setShortcut( Qt::SHIFT+Qt::CTRL+Qt::Key_W );
    closeWindowAction_->setToolTip( tr( "Close current display" ) );
    connect( closeWindowAction_, SIGNAL(triggered()), SLOT(_closeWindow()) );

    addAction( saveAction_ = new QAction( IconEngine::get( IconNames::Save ), tr( "Save" ), this ) );
    saveAction_->setShortcut( QKeySequence::Save );
    saveAction_->setToolTip( tr( "Save current file" ) );
    connect( saveAction_, SIGNAL(triggered()), SLOT(_save()) );

    addAction( saveAsAction_ = new QAction( IconEngine::get( IconNames::SaveAs ), tr( "Save As..." ), this ) );
    saveAsAction_->setShortcut( QKeySequence::SaveAs );
    saveAsAction_->setToolTip( tr( "Save current file with a different name" ) );
    connect( saveAsAction_, SIGNAL(triggered()), SLOT(_saveAs()) );

    addAction( revertToSaveAction_ = new QAction( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this ) );
    revertToSaveAction_->setShortcut( QKeySequence::Refresh );
    revertToSaveAction_->setToolTip( tr( "Reload saved version of current file" ) );
    connect( revertToSaveAction_, SIGNAL(triggered()), SLOT(_revertToSave()) );

    addAction( printAction_ = new QAction( IconEngine::get( IconNames::Print ), tr( "Print..." ), this ) );
    printAction_->setToolTip( tr( "Print current file" ) );
    printAction_->setShortcut( QKeySequence::Print );
    connect( printAction_, SIGNAL(triggered()), SLOT(_print()) );

    addAction( printPreviewAction_ = new QAction( IconEngine::get( IconNames::PrintPreview ), tr( "Print Preview..." ), this ) );
    connect( printPreviewAction_, SIGNAL(triggered()), SLOT(_printPreview()) );

    addAction( htmlAction_ = new QAction( IconEngine::get( IconNames::Html ), tr( "Export to HTML..." ), this ) );
    connect( htmlAction_, SIGNAL(triggered()), SLOT(_toHtml()) );

    addAction( undoAction_ = new QAction( IconEngine::get( IconNames::Undo ), tr( "Undo" ), this ) );
    undoAction_->setToolTip( tr( "Undo last action" ) );
    undoAction_->setEnabled( false );
    connect( undoAction_, SIGNAL(triggered()), SLOT(_undo()) );

    addAction( redoAction_ = new QAction( IconEngine::get( IconNames::Redo ), tr( "Redo" ), this ) );
    redoAction_->setToolTip( tr( "Redo last undone action" ) );
    redoAction_->setEnabled( false );
    connect( redoAction_, SIGNAL(triggered()), SLOT(_redo()) );

    addAction( cutAction_ = new QAction( IconEngine::get( IconNames::Cut ), tr( "Cut" ), this ) );
    cutAction_->setToolTip( tr( "Cut current selection and copy to clipboard" ) );
    cutAction_->setEnabled( false );
    connect( cutAction_, SIGNAL(triggered()), SLOT(_cut()) );

    addAction( copyAction_ = new QAction( IconEngine::get( IconNames::Copy ), tr( "Copy" ), this ) );
    copyAction_->setToolTip( tr( "Copy current selection to clipboard" ) );
    copyAction_->setEnabled( false );
    connect( copyAction_, SIGNAL(triggered()), SLOT(_copy()) );

    addAction( pasteAction_ = new QAction( IconEngine::get( IconNames::Paste ), tr( "Paste" ), this ) );
    pasteAction_->setToolTip( tr( "Paste clipboard to text" ) );
    pasteAction_->setEnabled( !qApp->clipboard()->text().isEmpty() );
    connect( pasteAction_, SIGNAL(triggered()), SLOT(_paste()) );

    addAction( filePropertiesAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Properties..." ), this ) );
    filePropertiesAction_->setShortcut( Qt::ALT + Qt::Key_Return );
    filePropertiesAction_->setToolTip( tr( "Display file informations" ) );
    filePropertiesAction_->setEnabled( false );
    connect( filePropertiesAction_, SIGNAL(triggered()), SLOT(_fileInfo()) );

    addAction( spellcheckAction_ = new QAction( IconEngine::get( IconNames::SpellCheck ), tr( "Spell Check..." ), this ) );
    #if USE_ASPELL
    connect( spellcheckAction_, SIGNAL(triggered()), SLOT(_spellcheck()) );
    spellcheckAction_->setEnabled( !SpellCheck::SpellInterface().dictionaries().empty() );
    #endif

    addAction( diffAction_ = new QAction( "Diff Files", this ) );
    connect( diffAction_, SIGNAL(triggered()), SLOT(_diff()) );
    diffAction_->setEnabled( false );

    addAction( splitDisplayHorizontalAction_ =new QAction( IconEngine::get( IconNames::ViewTopBottom ), tr( "Clone Display Top/Bottom" ), this ) );
    splitDisplayHorizontalAction_->setToolTip( tr( "Clone current display vertically" ) );
    connect( splitDisplayHorizontalAction_, SIGNAL(triggered()), SLOT(_splitDisplayVertical()) );

    addAction( splitDisplayVerticalAction_ =new QAction( IconEngine::get( IconNames::ViewLeftRight ), tr( "Clone Display Left/Right" ), this ) );
    splitDisplayVerticalAction_->setToolTip( tr( "Clone current display horizontally" ) );
    connect( splitDisplayVerticalAction_, SIGNAL(triggered()), SLOT(_splitDisplayHorizontal()) );

}

//______________________________________________________________________
void MainWindow::_installToolbars()
{

    // file toolbar
    auto toolbar = new CustomToolBar( tr( "Main Toolbar" ), this, "FILE_TOOLBAR" );
    toolbar->addAction( newFileAction_ );
    toolbar->addAction( openAction_ );
    toolbar->addAction( saveAction_ );

    // edition toolbar
    toolbar = new CustomToolBar( tr( "Edition" ), this, "EDITION_TOOLBAR" );
    toolbar->addAction( undoAction_ );
    toolbar->addAction( redoAction_ );
    toolbar->addAction( cutAction_ );
    toolbar->addAction( copyAction_ );
    toolbar->addAction( pasteAction_ );

    // extra toolbar
    toolbar = new CustomToolBar( tr( "Tools" ), this, "EXTRA_TOOLBAR" );
    toolbar->addAction( filePropertiesAction_ );

    #if USE_ASPELL
    toolbar->addAction( spellcheckAction_ );
    #endif

    // splitting toolbar
    toolbar = new CustomToolBar( tr( "Multiple Displays" ), this, "SPLIT_TOOLBAR" );
    toolbar->addAction( splitDisplayHorizontalAction_ );
    toolbar->addAction( splitDisplayVerticalAction_ );
    toolbar->addAction( openHorizontalAction_ );
    toolbar->addAction( openVerticalAction_ );
    toolbar->addAction( closeDisplayAction_ );
    toolbar->addAction( detachAction_ );

    // document class toolbar
    documentClassToolBar_ = new DocumentClassToolBar( this );
    connect( documentClassToolBar_, SIGNAL(documentClassSelected(QString)), SLOT(selectClassName(QString)) );

    // navigation toolbar
    auto navigationToolbar = new NavigationToolBar( this );
    navigationToolbar->connect( navigationFrame() );


}

//______________________________________________________________________
void MainWindow::_createFindWidget()
{

    Debug::Throw( "MainWindow::_createFindWidget.\n" );
    if( !findWidget_ )
    {

        findWidget_ = new BaseFindWidget( rightContainer_ );
        rightContainer_->layout()->addWidget( findWidget_ );
        connect( findWidget_, SIGNAL(find(TextSelection)), SLOT(_find(TextSelection)) );
        connect( this, SIGNAL(matchFound()), findWidget_, SLOT(matchFound()) );
        connect( this, SIGNAL(noMatchFound()), findWidget_, SLOT(noMatchFound()) );
        connect( &findWidget_->closeButton(), SIGNAL(clicked()), SLOT(_restoreFocus()) );
        findWidget_->hide();

    }

    return;

}

//_____________________________________________________________________
void MainWindow::_createReplaceWidget()
{
    Debug::Throw( "MainWindow::_CreateReplaceDialog.\n" );
    if( !( replaceWidget_ ) )
    {

        replaceWidget_ = new ReplaceWidget( rightContainer_ );
        rightContainer_->layout()->addWidget( replaceWidget_ );
        connect( replaceWidget_, SIGNAL(find(TextSelection)), SLOT(_find(TextSelection)) );
        connect( replaceWidget_, SIGNAL(replace(TextSelection)), SLOT(_replace(TextSelection)) );
        connect( replaceWidget_, SIGNAL(replaceInWindow(TextSelection)), SLOT(_replaceInWindow(TextSelection)) );
        connect( replaceWidget_, SIGNAL(replaceInSelection(TextSelection)), SLOT(_replaceInSelection(TextSelection)) );
        connect( replaceWidget_, SIGNAL(replaceInFiles()), SLOT(_multipleFileReplace()) );
        connect( replaceWidget_, SIGNAL(menuAboutToShow()), SLOT(_updateReplaceInSelection()) );
        connect( &replaceWidget_->closeButton(), SIGNAL(clicked()), SLOT(_restoreFocus()) );
        replaceWidget_->hide();

        connect( this, SIGNAL(matchFound()), replaceWidget_, SLOT(matchFound()) );
        connect( this, SIGNAL(noMatchFound()), replaceWidget_, SLOT(noMatchFound()) );

    }

}

//_________________________________________________________________
void MainWindow::_createSelectLineWidget()
{
    if( !selectLineWidget_ )
    {
        selectLineWidget_ = new SelectLineWidget( this, true );
        rightContainer_->layout()->addWidget( selectLineWidget_ );
        connect( selectLineWidget_, SIGNAL(lineSelected(int)), SLOT(_selectLine(int)) );
        connect( this, SIGNAL(lineFound()), selectLineWidget_, SLOT(matchFound()) );
        connect( this, SIGNAL(lineNotFound()), selectLineWidget_, SLOT(noMatchFound()) );
        connect( &selectLineWidget_->closeButton(), SIGNAL(clicked()), SLOT(_restoreFocus()) );
        selectLineWidget_->hide();
    }
}

//_________________________________________________________________
void MainWindow::_connectView( TextView& view )
{
    Debug::Throw( "MainWindow::_connectView.\n" );
    connect( &view, SIGNAL(modifiersChanged(TextEditor::Modifiers)), SLOT(_updateModifiers()) );
    connect( &view, SIGNAL(needUpdate(TextDisplay::UpdateFlags)), SLOT(_update(TextDisplay::UpdateFlags)) );
    connect( &view, SIGNAL(displayCountChanged()), SLOT(_updateDisplayCount()) );
    connect( &view, SIGNAL(displayCountChanged()), &Base::Singleton::get().application<Application>()->windowServer(), SIGNAL(sessionFilesChanged()) );
    connect( &view, SIGNAL(undoAvailable(bool)), &undoAction(), SLOT(setEnabled(bool)) );
    connect( &view, SIGNAL(redoAvailable(bool)), &redoAction(), SLOT(setEnabled(bool)) );
    connect( &view.positionTimer(), SIGNAL(timeout()), SLOT(_updateCursorPosition()) );

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
        .setModified( activeDisplay().document()->isModified() ).get()
        );

}
