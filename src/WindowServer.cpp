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

#include "WindowServer.h"
#include "Application.h"
#include "BaseFileSystemWidget.h"
#include "CloseFilesDialog.h"
#include "Debug.h"
#include "FileDialog.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "InformationDialog.h"
#include "MainWindow.h"
#include "MenuBar.h"
#include "NewFileDialog.h"
#include "ProgressDialog.h"
#include "QtUtil.h"
#include "QuestionDialog.h"
#include "RecentFilesMenu.h"
#include "RecentFilesWidget.h"
#include "SaveAllDialog.h"
#include "ScratchFileMonitor.h"
#include "SessionFilesView.h"
#include "SessionFilesWidget.h"
#include "SidePanelWidget.h"
#include "Singleton.h"
#include "Util.h"


#include <QAction>
#include <QApplication>
#include <QTextStream>

#include <numeric>

//________________________________________________________________
WindowServer::WindowServer( QObject* parent ):
    QObject( parent ),
    Counter( QStringLiteral("WindowServer") )
{

    Debug::Throw( QStringLiteral("WindowServer::WindowServer.\n") );

    // create actions
    saveAllAction_ = new QAction( IconEngine::get( IconNames::SaveAll ), tr( "Save All" ), this );
    connect( saveAllAction_, &QAction::triggered, this, &WindowServer::_saveAll );

    // scratch files
    scratchFileMonitor_ = new ScratchFileMonitor( this );
    connect( qApp, &QCoreApplication::aboutToQuit, scratchFileMonitor_, &ScratchFileMonitor::deleteScratchFiles );

    // configuration
    connect( Base::Singleton::get().application<Application>(), &Application::configurationChanged, this, &WindowServer::_updateConfiguration );
    _updateConfiguration();

}

//_____________________________________
MainWindow& WindowServer::newMainWindow()
{
    Debug::Throw( QStringLiteral("WindowServer::newMainWindow.\n") );
    auto window = new MainWindow;

    Base::Key::associate( this, window );
    _setActiveWindow( *window );

    // these connections are needed to make sure all windows recieve modifications of session files
    connect( window, &QObject::destroyed, this, &WindowServer::sessionFilesChanged );
    connect( window, &MainWindow::modificationChanged, this, &WindowServer::sessionFilesChanged );
    connect( window, &MainWindow::modificationChanged, this, &WindowServer::_updateActions );
    connect( window, &MainWindow::scratchFileCreated, scratchFileMonitor_, &ScratchFileMonitor::add );
    connect( this, &WindowServer::sessionFilesChanged, &window->sidePanelWidget().sessionFilesWidget(), &SessionFilesWidget::updateFiles );

    connect( window, &MainWindow::activated, this, &WindowServer::_activeWindowChanged );
    connect( &window->newFileAction(), &QAction::triggered, this, QOverload<>::of(&WindowServer::_newFile) );

    // open actions
    connect( &window->openAction(), &QAction::triggered, this, QOverload<>::of( &WindowServer::_open ) );
    connect( &window->openHorizontalAction(), &QAction::triggered, this, &WindowServer::_openHorizontal );
    connect( &window->openVerticalAction(), &QAction::triggered, this, &WindowServer::_openVertical );
    connect( &window->detachAction(), &QAction::triggered, this, QOverload<>::of(&WindowServer::_detach) );

    connect( &window->sidePanelWidget().sessionFilesWidget().model(), &SessionFilesModel::reparentFiles, this, &WindowServer::_reparent );
    connect( &window->sidePanelWidget().sessionFilesWidget().model(), &SessionFilesModel::reparentFilesToMain, this, &WindowServer::_reparentToMain );

    connect( static_cast<SessionFilesView*>(&window->sidePanelWidget().sessionFilesWidget().list()), &SessionFilesView::reparentFilesToMain, this, &WindowServer::_reparentToMain );
    connect( static_cast<SessionFilesView*>(&window->sidePanelWidget().sessionFilesWidget().list()), &SessionFilesView::detach, this, QOverload<>::of(&WindowServer::_detach) );

    // open actions
    connect( &window->menuBar().recentFilesMenu(), &RecentFilesMenu::fileSelected, this, QOverload<const FileRecord&>::of(&WindowServer::_open) );
    connect( &window->sidePanelWidget().sessionFilesWidget(), &SessionFilesWidget::fileActivated, this, QOverload<const FileRecord&>::of(&WindowServer::_open) );
    connect( &window->sidePanelWidget().recentFilesWidget(), &RecentFilesWidget::fileActivated, this, QOverload<const FileRecord&>::of(&WindowServer::_open) );
    connect( &window->sidePanelWidget().fileSystemWidget(), &BaseFileSystemWidget::fileActivated, this, QOverload<const FileRecord&>::of(&WindowServer::_open) );

    // other actions
    connect( &window->sidePanelWidget().sessionFilesWidget(), &SessionFilesWidget::filesSaved, this, &WindowServer::_save );
    connect( &window->sidePanelWidget().sessionFilesWidget(), &SessionFilesWidget::filesClosed, this, QOverload<FileRecord::List>::of(&WindowServer::_close) );

    return *window;
}

//______________________________________________________
FileRecord::List WindowServer::records( Flags recordFlags, QWidget* window ) const
{

    Debug::Throw( QStringLiteral("WindowServer::records.\n") );

    // output
    FileRecord::List records;

    // get associated main windows
    auto application( Base::Singleton::get().application<Application>() );
    for( const auto& windowIter:Base::KeySet<MainWindow>( this ) )
    {
        // check if current window match the one passed in argument
        bool isActiveWindow( windowIter == window );

        // retrieve associated TextDisplays
        for( const auto& display:Base::KeySet<TextDisplay>( windowIter->associatedDisplays() ) )
        {

            // check modification status
            if( (recordFlags&ModifiedOnly) && !display->document()->isModified() ) continue;
            if( (recordFlags&ExistingOnly ) && display->isNewDocument() ) continue;

            // retrieve file
            // store in map if not empty
            const File& file( display->file() );
            if( file.isEmpty() ) continue;

            // insert in map (together with modification status)
            FileRecord record = display->isNewDocument() ? FileRecord( file ):application->recentFiles().get(file);

            // set flags
            int flags( FileRecordProperties::None );
            if( display->document()->isModified() ) flags |= FileRecordProperties::Modified;
            if( display->fileCheckData().flag() != FileCheckData::Flag::None ) flags |= FileRecordProperties::Altered;

            if( isActiveWindow )
            {
                flags |= FileRecordProperties::Active;
                if( windowIter->isActiveDisplay( *display ) ) flags |= FileRecordProperties::Selected;
            }

            // assign flags and store
            records.append( record.setFlags( flags ) );

        }

    }

    std::sort( records.begin(), records.end(), FileRecord::FileFTor() );
    records.erase( std::unique( records.begin(), records.end(), FileRecord::SameFileFTorBinary() ), records.end() );
    return records;

}

//______________________________________________________
bool WindowServer::closeAll()
{

    Debug::Throw( QStringLiteral("WindowServer::closeAll.\n") );

    // retrieve opened files
    auto records( WindowServer::records() );

    // ask for confirmation if more than one file is opened.
    if( records.size() > 1 )
    {
        CloseFilesDialog dialog( &_activeWindow(), records );
        dialog.centerOnParent();
        if( !dialog.exec() ) return false;
    }

    QStringList files;
    std::transform( records.begin(), records.end(), std::back_inserter(files), []( const FileRecord& record ) { return record.file(); } );

    return _close( files );

}

//______________________________________________________
void WindowServer::readFilesFromArguments( const CommandLineParser& parser )
{

    Debug::Throw( QStringLiteral("WindowServer::readFilesFromArguments.\n") );

    // retrieve files from arguments
    // const CommandLineParser parser( Application::commandLineParser( arguments ) );
    auto filenames( parser.orphans() );

    // close mode
    if( parser.hasFlag( QStringLiteral("--close") ) )
    {
        _close( filenames );
        firstCall_ = false;
        return;
    }

    // check number of files
    if( filenames.size() > 10 )
    {
        const auto buffer =
            tr( "Do you really want to open %1 files at the same time ?\n"
            "This might be resource intensive and can overload your computer.\n"
            "If you choose No, only the first file will be opened." ).arg( filenames.size() );
        if( !QuestionDialog( &_activeWindow(), buffer ).exec() ) filenames = filenames.mid(0,1);

    }

    // check if at least one file is opened
    bool fileOpened( false );

    // tabbed | diff mode
    const bool tabbed( parser.hasFlag( QStringLiteral("--tabbed") ) );
    bool diff( parser.hasFlag( QStringLiteral("--diff") ) );
    if( ( tabbed || diff ) && filenames.size() > 1 )
    {
        Qt::Orientation orientation( defaultOrientation( diff ? OrientationMode::Diff:OrientationMode::Normal ) );
        if( parser.hasOption( QStringLiteral("--orientation") ) )
        {
            QString value( parser.option( QStringLiteral("--orientation") ) );
            if( value == QLatin1String("vertical") ) orientation = Qt::Vertical;
            else if( value == QLatin1String("horizontal") ) orientation = Qt::Horizontal;
        }

        bool first( true );
        for( const auto& filename:filenames )
        {

            if( first )
            {

                if( (fileOpened |= _open( FileRecord( File( filename ).expand() ) ) ) )
                {
                    _applyCommandLineArguments( _activeWindow().activeDisplay(), parser );
                    first = false;
                    _activeWindow().uniconify();
                }

            } else {

                if( (fileOpened |= _open( FileRecord( File( filename ).expand() ), orientation )) )
                { _applyCommandLineArguments( _activeWindow().activeDisplay(), parser ); }

            }

        }

        if( diff )
        {
            if( !first && _activeWindow().activeView().independentDisplayCount() == 2 ) _activeWindow().diffAction().trigger();
            else InformationDialog( &_activeWindow(), tr( "invalid number of files selected. <Diff> canceled." ) ).exec();
        }

    } else {

        // default mode
        bool first( true );
        for( const auto& filename:filenames )
        {

            OpenMode mode( openMode_ );
            if( parser.hasFlag( QStringLiteral("--same-window") ) ) mode = OpenMode::ActiveWindow;
            else if( parser.hasFlag( QStringLiteral("--new-window") ) ) mode = OpenMode::NewWindow;

            bool opened = _open( FileRecord( File( filename ).expand() ), mode );
            if( opened ) { _applyCommandLineArguments( _activeWindow().activeDisplay(), parser ); }
            fileOpened |= opened;

            if( first && opened )
            {
                _activeWindow().uniconify();
                first = false;
            }

        }

    }

    if( !fileOpened && firstCall_ )
    {
        // at first call and if no file was oppened,
        // set the current display as a new document.
        // also force update of the recent files frame
        // and center on desktop
        _activeWindow().activeView().setIsNewDocument();
        _activeWindow().sidePanelWidget().recentFilesWidget().updateFiles();
    }
    firstCall_ = false;

    _activeWindow().uniconify();

    return;

}

//______________________________________________________
void WindowServer::open( const FileRecord::List& records )
{
    for( const auto& record:records )
    { _open( record ); }

}

//___________________________________________________________
void WindowServer::multipleFileReplace( const QList<File> &files, const TextSelection &selection )
{
    Debug::Throw( QStringLiteral("WindowServer::multipleFileReplace.\n") );

    // create progressDialog
    ProgressDialog dialog;
    dialog.setAttribute( Qt::WA_DeleteOnClose );
    QtUtil::centerOnWidget( &dialog, qApp->activeWindow() );
    dialog.setWindowTitle( Util::windowTitle( tr( "Replace in Text" ) ) );
    dialog.show();

    // loop over files to get relevant displays
    int maximum(0);
    Base::KeySet<TextDisplay> displays;
    Base::KeySet<MainWindow> windows( this );
    for( const auto& file:files )
    {

        // find matching window
        auto iter = std::find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( file.expanded() ) );
        if( iter == windows.end() ) continue;

        // loop over views
        for( const auto& view:Base::KeySet<TextView>( *iter ) )
        {

            if( !view->selectDisplay( file ) ) continue;

            auto display( &view->activeDisplay() );
            connect( display, &TextDisplay::progressAvailable, &dialog, &ProgressDialog::setValue );
            maximum += display->toPlainText().size();
            displays.insert( display );
        }

    }

    dialog.setMaximum( maximum );

    // loop over displays and perform replacement
    const auto counts = std::accumulate( displays.begin(), displays.end(), 0,
        [&dialog, &selection](int counts, TextDisplay* display)
        {
            counts += display->replaceInWindow( selection, false );
            dialog.setOffset( dialog.value() );
            return std::move(counts);
        } );

    // close progress dialog
    dialog.close();

    // popup dialog
    QString buffer;
    QTextStream what( &buffer );
    if( !counts ) what << "string not found.";
    else if( counts == 1 ) what << "1 replacement performed";
    else what << counts << " replacements performed";

    {
        InformationDialog dialog( &_activeWindow(), buffer );
        dialog.setWindowTitle( tr( "Replace in Text" ) );
        dialog.centerOnWidget( qApp->activeWindow() );
        dialog.exec();
    }

    return;
}

//____________________________________________
void WindowServer::_updateConfiguration()
{

    Debug::Throw( QStringLiteral("WindowServer::_updateConfiguration.\n") );
    openMode_ = (OpenMode) XmlOptions::get().get<int>( QStringLiteral("OPEN_MODE") );
    _setDefaultOrientation( OrientationMode::Normal, (Qt::Orientation) XmlOptions::get().get<int>( QStringLiteral("ORIENTATION") ) );
    _setDefaultOrientation( OrientationMode::Diff, (Qt::Orientation) XmlOptions::get().get<int>( QStringLiteral("DIFF_ORIENTATION") ) );

}

//____________________________________________
void WindowServer::_activeWindowChanged( MainWindow* window )
{
    Debug::Throw() << "WindowServer::_activeWindowChanged - " << window->key() << Qt::endl;
    _setActiveWindow( *window );
}

//_______________________________________________
void WindowServer::_updateActions()
{

    Debug::Throw( QStringLiteral("WindowServer::_updateActions.\n") );
    saveAllAction_->setEnabled( !records( ModifiedOnly ).empty() );

}

//_______________________________________________
void WindowServer::_newFile( WindowServer::OpenMode mode )
{

    Debug::Throw( QStringLiteral("WindowServer::_newFile.\n") );

    // retrieve all MainWindows
    Base::KeySet<MainWindow> windows( this );

    // try find empty editor
    TextView* view( nullptr );
    auto iter = std::find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
    if( iter != windows.end() )
    {

        // select the view that contains the empty display
        Base::KeySet<TextView> views( *iter );
        auto viewIter( std::find_if( views.begin(), views.end(), MainWindow::EmptyFileFTor() ) );
        if( viewIter == views.end() )
        {

            Debug::Throw(0, QStringLiteral("WindowServer::_newFile - invalid view.\n") );
            return;
        }

        (*iter)->setActiveView( **viewIter );
        view = *viewIter;

        // uniconify
        (*iter)->uniconify();

    }

    // if no window found, create a new one
    if( !view )
    {

        if( mode == OpenMode::NewWindow )
        {

            auto &window( newMainWindow() );
            view = &window.activeView();
            window.show();

        } else {

            view = &_activeWindow().newTextView();
            _activeWindow().uniconify();

        }

    }

    // mark view as new document
    view->setIsNewDocument();

    return;

}


//_______________________________________________
void WindowServer::_newFile( Qt::Orientation orientation )
{

    Debug::Throw( QStringLiteral("WindowServer::_newFile.\n") );

    // retrieve active view
    auto& activeView( _activeWindow().activeView() );

    // look for an empty display
    // create a new display if none is found
    Base::KeySet<TextDisplay> displays( activeView );
    if( std::none_of( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) ) activeView.splitDisplay( orientation, false );

    activeView.setIsNewDocument();
    return;

}

//_______________________________________________
bool WindowServer::_open( const FileRecord &record, WindowServer::OpenMode mode )
{

    Debug::Throw() << "WindowServer::_open - file: " << record.file() << "." << Qt::endl;

    // do nothing if record is empty
    if( record.file().isEmpty() ) return false;

    // see if file is directory
    if( record.file().isDirectory() )
    {

        QString buffer;
        QTextStream( &buffer ) << "File \"" << record.file() << "\" is a directory. <Open> canceled.";
        InformationDialog( &_activeWindow(), buffer ).exec();
        return false;

    }

    // retrieve all MainWindows
    Base::KeySet<MainWindow> windows( this );

    // try find editor with matching name
    auto iter = std::find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file().expanded() ) );
    if( iter != windows.end() )
    {

        (*iter)->selectDisplay( record.file() );
        _setActiveWindow( **iter );
        Debug::Throw() << "WindowServer::_open - file: " << record.file() << " found matching." << Qt::endl;
        return true;

    }

    // create file if it does not exist
    if( !( record.file().exists() || _createNewFile( record ) ) ) return false;

    // try find empty editor
    TextView* view = nullptr;
    iter = std::find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
    if( iter != windows.end() )
    {

        // select the view that contains the empty display
        Base::KeySet<TextView> views( *iter );
        auto viewIter( std::find_if( views.begin(), views.end(), MainWindow::EmptyFileFTor() ) );
        if( viewIter == views.end() ) return false;

        (*viewIter)->setFile( record.file() );
        (*iter)->setActiveView( **viewIter );
        view = *viewIter;

    }

    // if no window found, create a new one
    if( !view )
    {

        if( mode == OpenMode::NewWindow )
        {

            MainWindow &window( newMainWindow() );
            view = &window.activeView();
            view->setFile( record.file() );
            window.show();

        } else {

            // create new view
            _activeWindow().newTextView( record );

        }

    }

    qApp->processEvents();

    Debug::Throw() << "WindowServer::_open - file: " << record.file() << " done." << Qt::endl;
    return true;

}

//_______________________________________________
bool WindowServer::_open( const FileRecord &record, Qt::Orientation orientation )
{

    Debug::Throw() << "WindowServer::_open - file: " << record.file() << " orientation: " << orientation << Qt::endl;

    // do nothing if record is empty
    if( record.file().isEmpty() ) return false;

    // create file if it does not exist
    if( !( record.file().exists() || _createNewFile( record ) ) ) return false;

    // retrieve active view
    auto& activeView( _activeWindow().activeView() );

    // retrieve all windows and find one matching
    Base::KeySet<MainWindow> windows( this );
    auto iter = std::find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file().expanded() ) );
    if( iter != windows.end() )
    {

        // find matching view
        Base::KeySet<TextView> views( *iter );
        auto&& viewIter = std::find_if( views.begin(), views.end(), MainWindow::SameFileFTor( record.file().expanded() ) );
        if( viewIter == views.end() )
        {

            Debug::Throw(0, QStringLiteral("WindowServer::_open - invalid view.\n") );
            return false;
        }

        // check if the found view is the current
        if( *viewIter == &activeView )
        {
            (*iter)->uniconify();
            _setActiveWindow( **iter );
            return true;
        }

        // select found display in TextView
        (*viewIter)->selectDisplay( record.file() );

        QString buffer( tr(
            "The file %1 is already opened in another window.\n"
            "Do you want to close the other display and open the file here ?" ).arg( record.file() ) );

        if( !QuestionDialog( &_activeWindow(), buffer ).exec() )
        {
            (*iter)->uniconify();
            return false;
        }

        // look for an empty display
        // create a new display if none is found
        Base::KeySet<TextDisplay> displays( &activeView );
        auto displayIter( std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
        TextDisplay& display( displayIter == displays.end() ? activeView.splitDisplay( orientation, false ):**displayIter );

        // retrieve active display from previous window
        auto&& previousDisplay( (*viewIter)->activeDisplay() );

        // store modification state
        bool modified( previousDisplay.document()->isModified() );

        // clone
        display.synchronize( &previousDisplay );

        // set previous display as unmdified
        previousDisplay.document()->setModified( false );

        // close display
        displays = Base::KeySet<TextDisplay>( &previousDisplay );
        displays.insert( &previousDisplay );
        for( const auto& display:displays )
        { (*viewIter)->closeDisplay( *display ); }

        // restore modification state and make new display active
        display.setModified( modified );
        activeView.setActiveDisplay( display );
        display.setFocus();

    } else {

        // look for an empty display
        // create a new display if none is found
        Base::KeySet<TextDisplay> displays( activeView );
        if( std::none_of( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) ) activeView.splitDisplay( orientation, false );

        // open file in this window
        activeView.setFile( record.file() );

    }

    return true;

}

//_______________________________________________
void WindowServer::_detach()
{

    Debug::Throw( QStringLiteral("WindowServer::_detach.\n") );

    // check number of independent displays
    auto& activeWindowLocal( _activeWindow() );
    if( activeWindowLocal.activeView().independentDisplayCount() <= 1 && Base::KeySet<TextView>( &_activeWindow() ).size() <= 1 )
    { return; }

    // detach active display
    _detach( activeWindowLocal.activeView().activeDisplay() );

}

//_______________________________________________
void WindowServer::_detach( const File& file )
{

    Debug::Throw() << "WindowServer::_detach - file: " << file << Qt::endl;

    // check number of independent displays
    auto&& activeWindowLocal( _activeWindow() );
    if( activeWindowLocal.activeView().independentDisplayCount() <= 1 && Base::KeySet<TextView>( &_activeWindow() ).size() <= 1 )
    {
        Debug::Throw( QStringLiteral("WindowServer::_detach - invalid display count.\n") );
        return;
    }

    _detach( _findDisplay( file ) );
}

//_______________________________________________
void WindowServer::_detach( TextDisplay& activeDisplayLocal )
{

    // check number of displays associated to active
    auto&& activeWindowLocal( _activeWindow() );
    if( !Base::KeySet<TextDisplay>(activeDisplayLocal).empty() )
    {
        InformationDialog( &activeWindowLocal,
            tr( "Software limitation:\n"
            "Active display has clones in the current window.\n"
            "It cannot be detached.") ).exec();
        return;
    }

    // save modification state
    bool modified( activeDisplayLocal.document()->isModified() );

    // create MainWindow
    auto& window( newMainWindow() );
    window.move( QCursor::pos() - QPoint( window.width()/2, 0 ) );
    window.show();

    // clone its display from the current
    window.activeView().activeDisplay().synchronize( &activeDisplayLocal );

    // close display
    activeDisplayLocal.document()->setModified( false );
    activeWindowLocal.activeView().closeDisplay( activeDisplayLocal );

    // update modification state
    window.activeView().activeDisplay().document()->setModified( modified );

    return;

}

//_______________________________________________
void WindowServer::_reparent( const File& first, const File& second )
{

    Debug::Throw() << "WindowServer::_reparent - first: " << first << " second: " << second << Qt::endl;
    // retrieve windows
    auto&& firstDisplay( _findDisplay( first ) );
    auto&& firstView( _findView( first ) );

    // check for first display clones
    if( !Base::KeySet<TextDisplay>(firstDisplay).empty() )
    {
        InformationDialog( &firstDisplay,
            tr( "Software limitation:\n"
            "Dropped display has clones in the current window.\n"
            "It cannot be reparented.") ).exec();
        return;
    }

    // save modification state
    bool modified( firstDisplay.document()->isModified() );

    // retrieve second display and corresponding view
    auto& view = _findView( second );

    // do nothing if first and second display already belong to the same view
    if( &view == &firstView ) return;

    // create new display in text view
    view.selectDisplay( second );
    TextDisplay& newDisplay = view.splitDisplay( defaultOrientation( OrientationMode::Normal ), false );
    newDisplay.synchronize( &firstDisplay );

    // close display
    firstDisplay.document()->setModified( false );
    firstView.closeDisplay( firstDisplay );

    // restore modification state
    newDisplay.setModified( modified );
    view.setActiveDisplay( newDisplay );

    // make view active
    Base::KeySet<MainWindow> windows( view );
    if( windows.size() != 1 )
    {
        Debug::Throw(0, QStringLiteral("WindowServer::_reparent - invalid number of windows.\n") );
        return;
    }

    auto&& window( **windows.begin() );
    window.setActiveView( view );
    window.raise();

    return;

}

//_______________________________________________
void WindowServer::_reparentToMain( const File& first, const File& second )
{

    Debug::Throw() << "WindowServer::_reparentToMain - first: " << first << " second: " << second << Qt::endl;

    // retrieve windows
    auto& firstDisplay( _findDisplay( first ) );
    auto& firstView( _findView( first ) );
    auto& firstWindow( _findWindow( first ) );

    // retrieve main window associated to second file
    auto& window( _findWindow( second ) );

    // make sure first view has multiple files
    if( (&firstWindow == &window ) && firstView.independentDisplayCount() < 2 ) return;

    // check for first display clones
    if( !Base::KeySet<TextDisplay>(firstDisplay).empty() )
    {
        InformationDialog( &firstDisplay,
            tr( "Software limitation:\n"
            "Dropped display has clones in the current window.\n"
            "It cannot be reparented.") ).exec();
        return;
    }

    // save modification state
    bool modified( firstDisplay.document()->isModified() );

    // create new text view in window
    auto& view( window.newTextView() );
    view.activeDisplay().synchronize( &firstDisplay );

    // close display
    firstDisplay.document()->setModified( false );
    firstView.closeDisplay( firstDisplay );

    // restore modification state
    view.activeDisplay().setModified( modified );

    // raise window
    window.raise();

    return;

}

//_______________________________________________
void WindowServer::_saveAll()
{
    Debug::Throw( QStringLiteral("WindowServer::_saveAll.\n") );
    _save( records( ModifiedOnly ) );
    return;

}

//_______________________________________________
void WindowServer::_save( FileRecord::List records )
{
    Debug::Throw( QStringLiteral("WindowServer::_save.\n") );

    // check how many records are modified
    if( records.empty() ) return;

    // ask for confirmation
    if( records.size() > 1 && !SaveAllDialog( &_activeWindow(), records ).exec() ) return;

    // retrieve windows
    for( const auto& window:Base::KeySet<MainWindow>( this ) )
    {

        // retrieve displays
        for( const auto& display:window->associatedDisplays() )
        {
            if( !display->document()->isModified() ) continue;
            if( std::none_of( records.begin(), records.end(), FileRecord::SameFileFTorUnary( display->file() ) ) ) continue;
            display->save();
        }

    }

}

//_______________________________________________
bool WindowServer::_close( FileRecord::List records )
{
    Debug::Throw( QStringLiteral("WindowServer::_close.\n") );

    // check how many records are modified
    if( records.isEmpty() ) return false;

    // ask for confirmation
    if( records.size() > 1 && !CloseFilesDialog( &_activeWindow(), records ).exec() ) return true;

    QStringList files;
    std::transform( records.begin(), records.end(), std::back_inserter( files ), []( const FileRecord& record ) { return record.file(); } );

    return _close( files );

}

//________________________________________________________________
bool WindowServer::_close( QStringList files )
{

    int state( AskForSaveDialog::Unknown );

    // need a first loop over associated windows to store modified files
    QSet<QString> modifiedFiles;
    Base::KeySet<MainWindow> windows( this );
    for( const auto& window:windows )
    {
        for( const auto& display:window->associatedDisplays() )
        {

            // see if file is in list
            if( std::none_of( files.begin(), files.end(), Base::Functor::SameFTor<QString>(display->file()) ) ) continue;
            if( display->document()->isModified() ) modifiedFiles.insert( display->file() );

        }

    }

    // retrieve windows
    for( const auto& window:windows )
    {

        // retrieve views
        for( const auto& view:Base::KeySet<TextView>( window ) )
        {

            for( const auto& display:Base::KeySet<TextDisplay>( view ) )
            {

                // see if file is in list
                if( std::none_of( files.begin(), files.end(), Base::Functor::SameFTor<QString>(display->file()) ) ) continue;

                Debug::Throw() << "WindowServer::_close - file: " << display->file() << Qt::endl;

                if( display->document()->isModified() )
                {

                    if( state == AskForSaveDialog::YesToAll ) display->save();
                    else if( state == AskForSaveDialog::NoToAll ) display->document()->setModified( false );
                    else
                    {
                        state = display->askForSave( modifiedFiles.size() > 1 );
                        if( state == AskForSaveDialog::YesToAll || state == AskForSaveDialog::Yes ) display->save();
                        else if( state == AskForSaveDialog::NoToAll  || state == AskForSaveDialog::No ) display->document()->setModified( false );
                        else if( state == AskForSaveDialog::Cancel ) return false;
                        modifiedFiles.remove( display->file() );
                    }

                }

                view->closeDisplay( *display );

            }
        }
    }

    return true;
}

//_______________________________________________
MainWindow& WindowServer::_findWindow( const File& file )
{

    MainWindow* out = nullptr;

    // retrieve windows
    for( const auto& window:Base::KeySet<MainWindow>( this ) )
    {

        // retrieve displays
        auto displays( window->associatedDisplays() );
        if( std::any_of( displays.begin(), displays.end(), TextDisplay::SameFileFTor( file.expanded() ) ) )
        {
            out = window;
            break;
        }

    }

    return *out;

}

//_______________________________________________
TextView& WindowServer::_findView( const File& file )
{

    auto& display = _findDisplay( file );
    Base::KeySet<TextView> views( display );
    if( views.size() != 1 )
    {
        Debug::Throw(0, QStringLiteral("WindowServer::_findView - no view found.\n") );
        abort();
    }

    return **views.begin();
}

//_______________________________________________
TextDisplay& WindowServer::_findDisplay( const File& file )
{

    Debug::Throw() << "WindowServer::_findDisplay - file: " << file << Qt::endl;
    TextDisplay* out = nullptr;

    // retrieve windows
    for( const auto& window:Base::KeySet<MainWindow>( this ) )
    {

        // retrieve displays
        auto displays( window->associatedDisplays() );
        auto iter = std::find_if( displays.begin(), displays.end(), TextDisplay::SameFileFTor( file ) );
        // auto iter = std::find_if( displays.begin(), displays.end(), TextDisplay::SameFileFTor( file.expanded() ) );
        if( iter != displays.end() )
        {
            out = *iter;
            break;
        }
    }

    return *out;

}

//_______________________________________________
FileRecord WindowServer::_selectFileFromDialog()
{

    Debug::Throw( QStringLiteral("WindowServer::_selectFileFromDialog.\n") );

    FileRecord record;

    // create file dialog
    File file( FileDialog( &_activeWindow() ).selectFile(_activeWindow().activeDisplay().workingDirectory() ).getFile() );
    if( file.isEmpty() ) return record;
    else file.expand();

    // assign file to record
    record.setFile( file );
    return record;

}

//_______________________________________________
bool WindowServer::_createNewFile( const FileRecord& record )
{
    Debug::Throw( QStringLiteral("WindowServer::_createNewFile.\n") );

    // check file is valid
    if( record.file().isEmpty() ) return false;

    // create NewFileDialog
    NewFileDialog::ReturnCodes buttons( NewFileDialog::Create | NewFileDialog::Cancel );
    if( records().empty() ) buttons |= NewFileDialog::Exit;

    NewFileDialog dialog( &_activeWindow(), record.file(), buttons );
    dialog.centerOnParent();
    int state = dialog.exec();

    Debug::Throw() << "WindowServer::Open - New file dialog state: " << state << Qt::endl;
    switch( state )
    {

        case NewFileDialog::Create:
        {
            File fullname( record.file().expanded() );
            if( !fullname.create() )
            {

                QString buffer;
                QTextStream( &buffer ) << "Unable to create file " << record.file() << ".";
                InformationDialog( &_activeWindow(), buffer ).exec();
                return false;

            } else return true;

        }

        break;

        default:
        case NewFileDialog::Exit: qApp->quit(); return false;
        case NewFileDialog::Cancel: return false;
        break;

    }

    return false;

}

//________________________________________________________________
void WindowServer::_applyCommandLineArguments( TextDisplay& display, const CommandLineParser& parser )
{
    Debug::Throw( QStringLiteral("WindowServer::_applyCommandLineArguments.\n") );

    //! see if autospell action is required
    bool autospell( parser.hasFlag( QStringLiteral("--autospell") ) );

    #if WITH_ASPELL
    //! see if autospell filter and dictionary are required
    QString filter = parser.hasOption( QStringLiteral("--filter") ) ? parser.option( QStringLiteral("--filter") ) : QLatin1String("");
    QString dictionary = parser.hasOption( QStringLiteral("--dictionary") ) ? parser.option( QStringLiteral("--dictionary") ) : QLatin1String("");
    Debug::Throw() << "WindowServer::_applyCommandLineArguments -"
        << " filter:" << filter
        << " dictionary: " << dictionary
        << Qt::endl;

    if( !filter.isEmpty() ) display.selectFilter( filter );
    if( !dictionary.isEmpty() ) display.selectDictionary( dictionary );
    #endif

    if( autospell ) display.autoSpellAction().setChecked( true );
    Debug::Throw( QStringLiteral("WindowServer::_applyCommandLineArguments - done.\n") );

}

//________________________________________________________________
void WindowServer::_setActiveWindow( MainWindow& window )
{
    Debug::Throw() << "WindowServer::setActiveWindow - key: " << window.key() << Qt::endl;
    if( !window.isAssociated( this ) )
    {
        Debug::Throw(0, QStringLiteral("WindowServer::_setActiveWindow - invalid window.\n") );
        return;
    }

    activeWindow_ = &window;
}


//________________________________________________________________
bool WindowServer::_hasActiveWindow() const
{

    // check if active window exists
    if( !activeWindow_ ) return false;

    // check if found in list of associated windows
    Base::KeySet<MainWindow> windows( this );

    // not sure it works when the window pointed to by activeWindow_ has been removed
    return windows.find( activeWindow_ ) != windows.end();

}
