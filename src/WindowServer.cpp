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

#include "WindowServer.h"

#include "Application.h"
#include "FileDialog.h"
#include "Debug.h"
#include "CloseFilesDialog.h"
#include "FileList.h"
#include "FileRecordProperties.h"
#include "FileSystemFrame.h"
#include "Icons.h"
#include "IconEngine.h"
#include "MainWindow.h"
#include "Menu.h"
#include "NavigationFrame.h"
#include "NewFileDialog.h"
#include "InformationDialog.h"
#include "ProgressDialog.h"
#include "QuestionDialog.h"
#include "QtUtil.h"
#include "RecentFilesFrame.h"
#include "RecentFilesMenu.h"
#include "SaveAllDialog.h"
#include "ScratchFileMonitor.h"
#include "SessionFilesFrame.h"
#include "Singleton.h"
#include "Util.h"

#include <QAction>
#include <QApplication>
#include <QTextStream>

//________________________________________________________________
WindowServer::WindowServer( QObject* parent ):
    QObject( parent ),
    Counter( "WindowServer" ),
    firstCall_( true ),
    defaultOrientation_( Qt::Horizontal ),
    defaultDiffOrientation_( Qt::Vertical ),
    openMode_( ACTIVE_WINDOW ),
    activeWindow_( 0 )
{

    Debug::Throw( "WindowServer::WindowServer.\n" );

    // create actions
    saveAllAction_ = new QAction( IconEngine::get( ICONS::SAVE_ALL ), tr( "Save All" ), this );
    connect( saveAllAction_, SIGNAL( triggered() ), SLOT( _saveAll() ) );

    // scratch files
    scratchFileMonitor_ = new ScratchFileMonitor( this );
    connect( qApp, SIGNAL( aboutToQuit( void ) ), scratchFileMonitor_, SLOT( deleteScratchFiles( void ) ) );

    // configuration
    connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
    _updateConfiguration();

}

//________________________________________________________________
WindowServer::~WindowServer( void )
{ Debug::Throw( "WindowServer::~WindowServer.\n" ); }

//_____________________________________
MainWindow& WindowServer::newMainWindow( void )
{
    Debug::Throw( "WindowServer::newMainWindow.\n" );
    MainWindow* window = new MainWindow();

    BASE::Key::associate( this, window );
    _setActiveWindow( *window );

    // these connections are needed to make sure all windows recieve modifications of session files
    connect( window, SIGNAL( destroyed() ), SIGNAL( sessionFilesChanged() ) );
    connect( window, SIGNAL( modificationChanged() ), SIGNAL( sessionFilesChanged() ) );
    connect( window, SIGNAL( modificationChanged() ), SLOT( _updateActions() ) );
    connect( window, SIGNAL( scratchFileCreated( const File& ) ), scratchFileMonitor_, SLOT( add( const File& ) ) );
    connect( this, SIGNAL( sessionFilesChanged() ), &window->navigationFrame().sessionFilesFrame(), SLOT( update() ) );

    connect( window, SIGNAL( activated( MainWindow* ) ), SLOT( _activeWindowChanged( MainWindow* ) ) );
    connect( &window->newFileAction(), SIGNAL( triggered() ), SLOT( _newFile() ) );

    // open actions
    connect( &window->openAction(), SIGNAL( triggered() ), SLOT( _open() ) );
    connect( &window->openHorizontalAction(), SIGNAL( triggered() ), SLOT( _openHorizontal() ) );
    connect( &window->openVerticalAction(), SIGNAL( triggered() ), SLOT( _openVertical() ) );
    connect( &window->detachAction(), SIGNAL( triggered() ), SLOT( _detach() ) );

    connect( &window->navigationFrame().sessionFilesFrame().model(), SIGNAL( reparentFiles( const File&, const File& ) ), SLOT( _reparent( const File&, const File& ) ) );
    connect( &window->navigationFrame().sessionFilesFrame().model(), SIGNAL( reparentFilesToMain( const File&, const File& ) ), SLOT( _reparentToMain( const File&, const File& ) ) );

    // open actions
    connect( &window->menu().recentFilesMenu(), SIGNAL( fileSelected( FileRecord ) ), SLOT( _open( FileRecord ) ) );
    connect( &window->navigationFrame().sessionFilesFrame(), SIGNAL( fileActivated( FileRecord ) ), SLOT( _open( FileRecord ) ) );
    connect( &window->navigationFrame().recentFilesFrame(), SIGNAL( fileActivated( FileRecord ) ), SLOT( _open( FileRecord ) ) );
    connect( &window->navigationFrame().fileSystemFrame(), SIGNAL( fileActivated( FileRecord ) ), SLOT( _open( FileRecord ) ) );

    // other actions
    connect( &window->navigationFrame().sessionFilesFrame(), SIGNAL( filesSaved( FileRecord::List ) ), SLOT( _save( FileRecord::List ) ) );
    connect( &window->navigationFrame().sessionFilesFrame(), SIGNAL( filesClosed( FileRecord::List ) ), SLOT( _close( FileRecord::List ) ) );

    return *window;
}

//______________________________________________________
FileRecord::List WindowServer::records( Flags recordFlags, QWidget* window ) const
{

    Debug::Throw( "WindowServer::records.\n" );

    // output
    FileRecord::List records;

    // get associated main windows
    Application& application( *Singleton::get().application<Application>() );
    foreach( MainWindow* windowIter, BASE::KeySet<MainWindow>( this ) )
    {
        // check if current window match the one passed in argument
        bool isActiveWindow( windowIter == window );

        // retrieve associated TextDisplays
        foreach( TextDisplay* display,  BASE::KeySet<TextDisplay>( windowIter->associatedDisplays() ) )
        {

            // check modification status
            if( (recordFlags&ModifiedOnly) && !display->document()->isModified() ) continue;
            if( (recordFlags&ExistingOnly ) && display->isNewDocument() ) continue;

            // retrieve file
            // store in map if not empty
            const File& file( display->file() );
            if( file.isEmpty() ) continue;

            // insert in map (together with modification status)
            FileRecord record = display->isNewDocument() ? FileRecord( file ):application.recentFiles().get(file);

            // set flags
            unsigned int flags( FileRecordProperties::NONE );
            if( display->document()->isModified() ) flags |= FileRecordProperties::MODIFIED;
            if( display->fileCheckData().flag() != FileCheck::Data::NONE ) flags |= FileRecordProperties::ALTERED;

            if( isActiveWindow )
            {
                flags |= FileRecordProperties::ACTIVE;
                if( windowIter->isActiveDisplay( *display ) ) flags |= FileRecordProperties::SELECTED;
            }

            // assign flags and store
            records << record.setFlags( flags );

        }

    }

    std::sort( records.begin(), records.end(), FileRecord::FileFTor() );
    records.erase( std::unique( records.begin(), records.end(), FileRecord::SameFileFTor() ), records.end() );
    return records;

}

//______________________________________________________
bool WindowServer::closeAll( void )
{

    Debug::Throw( "WindowServer::closeAll.\n" );

    // retrieve opened files
    FileRecord::List records( WindowServer::records() );

    // ask for confirmation if more than one file is opened.
    if( records.size() > 1 )
    {
        CloseFilesDialog dialog( &_activeWindow(), records );
        dialog.centerOnParent();
        if( !dialog.exec() ) return false;
    }

    QStringList files;
    foreach( const FileRecord& record, records )
    { files << record.file(); }

    return _close( files );

}

//______________________________________________________
void WindowServer::readFilesFromArguments( CommandLineArguments arguments )
{

    Debug::Throw() << "WindowServer::readFilesFromArguments." << endl;

    // retrieve files from arguments
    CommandLineParser parser( Application::commandLineParser( arguments ) );
    QStringList filenames( parser.orphans() );

    // close mode
    if( parser.hasFlag( "--close" ) )
    {
        _close( filenames );
        _setFirstCall( false );
        return;
    }

    // check number of files
    if( filenames.size() > 10 )
    {
        QString buffer =
            QString( tr( "Do you really want to open %1 files at the same time ?\n"
            "This might be very resource intensive and can overload your computer.\n"
            "If you choose No, only the first file will be opened." ) ).arg( filenames.size() );
        if( !QuestionDialog( &_activeWindow(), buffer ).exec() ) filenames = filenames.mid(0,1);

    }

    // check if at least one file is opened
    bool fileOpened( false );

    // tabbed | diff mode
    bool tabbed( parser.hasFlag( "--tabbed" ) );
    bool diff( parser.hasFlag( "--diff" ) );
    if( ( tabbed || diff ) && filenames.size() > 1 )
    {
        Qt::Orientation orientation( defaultOrientation( diff ? DIFF:NORMAL ) );
        if( parser.hasOption( "--orientation" ) )
        {
            QString value( parser.option( "--orientation" ) );
            if( value == "vertical" ) orientation = Qt::Vertical;
            else if( value == "horizontal" ) orientation = Qt::Horizontal;
        }

        bool first( true );
        foreach( const QString& filename, filenames )
        {

            if( first )
            {

                if( (fileOpened |= _open( File( filename ).expand()) ) )
                {
                    _applyArguments( _activeWindow().activeDisplay(), arguments );
                    first = false;
                }

            } else {

                if( (fileOpened |= _open( File( filename ).expand(), orientation )) )
                { _applyArguments( _activeWindow().activeDisplay(), arguments ); }

            }

        }

        if( diff )
        {
            if( !first && _activeWindow().activeView().independentDisplayCount() == 2 ) _activeWindow().diffAction().trigger();
            else InformationDialog( &_activeWindow(), tr( "invalid number of files selected. <Diff> canceled." ) ).exec();
        }

    } else {

        // default mode
        foreach( const QString& filename, filenames )
        {

            OpenMode mode( _openMode() );
            if( parser.hasFlag( "--same-window" ) ) mode = ACTIVE_WINDOW;
            else if( parser.hasFlag( "--new-window" ) ) mode = NEW_WINDOW;

            bool opened = _open( File( filename ).expand(), mode );
            if( opened ) { _applyArguments( _activeWindow().activeDisplay(), arguments ); }
            fileOpened |= opened;

        }

    }

    if( !fileOpened )
    {
        if( _firstCall() ) {

            // at first call and if no file was oppened,
            // set the current display as a new document.
            // also force update of the recent files frame
            // and center on desktop
            _activeWindow().activeView().setIsNewDocument();
            _activeWindow().navigationFrame().recentFilesFrame().update();

        }

        _activeWindow().uniconify();

    }
    _setFirstCall( false );

    Debug::Throw() << "WindowServer::readFilesFromArguments - done." << endl;
    return;

}

//______________________________________________________
void WindowServer::open( const FileRecord::List& records )
{
    foreach( const FileRecord& record, records )
    { _open( record ); }

}

//___________________________________________________________
void WindowServer::multipleFileReplace( QList<File> files, TextSelection selection )
{
    Debug::Throw( "WindowServer::multipleFileReplace.\n" );

    // keep track of number of replacements
    unsigned int counts(0);

    // create progressDialog
    ProgressDialog dialog;
    dialog.setAttribute( Qt::WA_DeleteOnClose );
    QtUtil::centerOnWidget( &dialog, qApp->activeWindow() );
    dialog.setWindowTitle( Util::windowTitle( "Replace in Text - Qedit" ) );
    dialog.show();

    // loop over files to get relevant displays
    int maximum(0);
    BASE::KeySet<TextDisplay> displays;
    BASE::KeySet<MainWindow> windows( this );
    foreach( const File& file, files )
    {

        // find matching window
        BASE::KeySet<MainWindow>::iterator iter = std::find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( file ) );
        Q_ASSERT( iter != windows.end() );

        // loop over views
        BASE::KeySet<TextView> views( *iter );
        foreach( TextView* view, views )
        {
            if( !view->selectDisplay( file ) ) continue;
            TextDisplay* display( &view->activeDisplay() );
            connect( display, SIGNAL( progressAvailable( int ) ), &dialog, SLOT( setValue( int ) ) );
            maximum += display->toPlainText().size();
            displays.insert( display );
        }

    }

    dialog.setMaximum( maximum );

    // loop over displays and perform replacement
    foreach( TextDisplay* display, displays )
    {
        counts += display->replaceInWindow( selection, false );
        dialog.setOffset( dialog.value() );
    }

    // close progress dialog
    dialog.close();

    // popup dialog
    QString buffer;
    QTextStream what( &buffer );
    if( !counts ) what << "string not found.";
    else if( counts == 1 ) what << "1 replacement performed";
    else what << counts << " replacements performed";
    InformationDialog( &_activeWindow(), buffer ).setWindowTitle( "Replace in Text - Qedit" ).centerOnWidget( qApp->activeWindow() ).exec();

    return;
}

//____________________________________________
void WindowServer::_updateConfiguration( void )
{

    Debug::Throw( "WindowServer::_updateConfiguration.\n" );
    _setOpenMode( (OpenMode) XmlOptions::get().get<int>( "OPEN_MODE" ) );
    _setDefaultOrientation( NORMAL, (Qt::Orientation) XmlOptions::get().get<int>( "ORIENTATION" ) );
    _setDefaultOrientation( DIFF, (Qt::Orientation) XmlOptions::get().get<int>( "DIFF_ORIENTATION" ) );

}

//____________________________________________
void WindowServer::_activeWindowChanged( MainWindow* window )
{
    Debug::Throw() << "WindowServer::_activeWindowChanged - " << window->key() << endl;
    _setActiveWindow( *window );
}

//_______________________________________________
void WindowServer::_updateActions( void )
{

    Debug::Throw( "WindowServer::_updateActions.\n" );
    saveAllAction().setEnabled( !records( ModifiedOnly ).empty() );

}

//_______________________________________________
void WindowServer::_newFile( WindowServer::OpenMode mode )
{

    Debug::Throw( "WindowServer::_newFile.\n" );

    // retrieve all MainWindows
    BASE::KeySet<MainWindow> windows( this );

    // try find empty editor
    TextView* view(0);
    BASE::KeySet<MainWindow>::iterator iter = std::find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
    if( iter != windows.end() )
    {

        // select the view that contains the empty display
        BASE::KeySet<TextView> views( *iter );
        BASE::KeySet<TextView>::iterator viewIter( std::find_if( views.begin(), views.end(), MainWindow::EmptyFileFTor() ) );
        Q_ASSERT( viewIter != views.end() );
        (*iter)->setActiveView( **viewIter );
        view = *viewIter;

        // uniconify
        (*iter)->uniconify();

    }

    // if no window found, create a new one
    if( !view ) {

        if( mode == NEW_WINDOW )
        {

            MainWindow &window( newMainWindow() );
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

    Debug::Throw( "WindowServer::_newFile.\n" );

    // retrieve active view
    TextView& activeView( _activeWindow().activeView() );

    // look for an empty display
    // create a new display if none is found
    BASE::KeySet<TextDisplay> displays( activeView );
    BASE::KeySet<TextDisplay>::iterator iter( std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
    if( iter == displays.end() ) activeView.splitDisplay( orientation, false );

    activeView.setIsNewDocument();
    return;

}

//_______________________________________________
bool WindowServer::_open( FileRecord record, WindowServer::OpenMode mode )
{

    Debug::Throw() << "WindowServer::_open - file: " << record.file() << "." << endl;

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
    BASE::KeySet<MainWindow> windows( this );

    // try find editor with matching name
    BASE::KeySet<MainWindow>::iterator iter = std::find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file() ) );
    if( iter != windows.end() )
    {

        (*iter)->uniconify();
        (*iter)->selectDisplay( record.file() );
        _setActiveWindow( **iter );
        Debug::Throw() << "WindowServer::_open - file: " << record.file() << " found matching." << endl;
        return true;

    }

    // create file if it does not exist
    if( !( record.file().exists() || _createNewFile( record ) ) ) return false;

    // try find empty editor
    TextView* view(0);
    iter = std::find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
    if( iter != windows.end() )
    {

        // select the view that contains the empty display
        BASE::KeySet<TextView> views( *iter );
        BASE::KeySet<TextView>::iterator viewIter( std::find_if( views.begin(), views.end(), MainWindow::EmptyFileFTor() ) );
        Q_ASSERT( viewIter != views.end() );
        (*viewIter)->setFile( record.file() );
        (*iter)->setActiveView( **viewIter );
        view = *viewIter;

        // uniconify
        (*iter)->uniconify();

    }

    // if no window found, create a new one
    if( !view ) {

        if( mode == NEW_WINDOW )
        {

            MainWindow &window( newMainWindow() );
            view = &window.activeView();
            view->setFile( record.file() );
            window.show();

        } else {

            // create new view
            view = &_activeWindow().newTextView( record );

            // uniconify
            _activeWindow().uniconify();

        }

    }

    // assign file
    //view->setFile( record.file() );
    qApp->processEvents();

    Debug::Throw() << "WindowServer::_open - file: " << record.file() << " done." << endl;
    return true;

}

//_______________________________________________
bool WindowServer::_open( FileRecord record, Qt::Orientation orientation )
{

    Debug::Throw() << "WindowServer::_open - file: " << record.file() << " orientation: " << orientation << endl;

    // do nothing if record is empty
    if( record.file().isEmpty() ) return false;

    // create file if it does not exist
    if( !( record.file().exists() || _createNewFile( record ) ) ) return false;

    // retrieve active view
    TextView& activeView( _activeWindow().activeView() );

    // retrieve all windows and find one matching
    BASE::KeySet<MainWindow> windows( this );
    BASE::KeySet<MainWindow>::iterator iter = std::find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file() ) );
    if( iter != windows.end() )
    {

        // find matching view
        BASE::KeySet<TextView> views( *iter );
        BASE::KeySet<TextView>::iterator viewIter = std::find_if( views.begin(), views.end(), MainWindow::SameFileFTor( record.file() ) );
        Q_ASSERT( viewIter != views.end() );

        // check if the found view is the current
        if( *viewIter == &activeView )
        {
            (*iter)->uniconify();
            _setActiveWindow( **iter );
            return true;
        }

        // select found display in TextView
        (*viewIter)->selectDisplay( record.file() );

        QString buffer;
        QTextStream( &buffer )
            << "The file " << record.file() << " is already opened in another window.\n"
            << "Do you want to close the other display and open the file here ?";
        if( !QuestionDialog( &_activeWindow(), buffer ).exec() )
        {
            (*iter)->uniconify();
            return false;
        }

        // look for an empty display
        // create a new display if none is found
        BASE::KeySet<TextDisplay> displays( &activeView );
        BASE::KeySet<TextDisplay>::iterator displayIter( std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
        TextDisplay& display( displayIter == displays.end() ? activeView.splitDisplay( orientation, false ):**displayIter );

        // retrieve active display from previous window
        TextDisplay& previous_display( (*viewIter)->activeDisplay() );

        // store modification state
        bool modified( previous_display.document()->isModified() );

        // clone
        display.synchronize( &previous_display );

        // set previous display as unmdified
        previous_display.document()->setModified( false );

        // close display
        displays = BASE::KeySet<TextDisplay>( &previous_display );
        displays.insert( &previous_display );
        foreach( TextDisplay* display, displays )
        { (*viewIter)->closeDisplay( *display ); }

        // restore modification state and make new display active
        display.setModified( modified );
        activeView.setActiveDisplay( display );
        display.setFocus();

    } else {

        // look for an empty display
        // create a new display if none is found
        BASE::KeySet<TextDisplay> displays( activeView );
        BASE::KeySet<TextDisplay>::iterator displayIter( std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
        if( displayIter == displays.end() ) activeView.splitDisplay( orientation, false );

        // open file in this window
        activeView.setFile( record.file() );

    }

    return true;

}

//_______________________________________________
void WindowServer::_detach( void )
{

    Debug::Throw( "WindowServer::_detach.\n" );

    MainWindow& activeWindowLocal( _activeWindow() );

    // check number of independent displays
    Q_ASSERT( activeWindowLocal.activeView().independentDisplayCount() > 1 || BASE::KeySet<TextView>( &_activeWindow() ).size() > 1 );

    // get current display
    TextDisplay& activeDisplayLocal( activeWindowLocal.activeView().activeDisplay() );

    // check number of displays associated to active
    if( !BASE::KeySet<TextDisplay>(activeDisplayLocal).empty() )
    {
        InformationDialog( &activeWindowLocal,
            "Software limitation:\n"
            "Active display has clones in the current window.\n"
            "It cannot be detached." ).exec();
        return;
    }

    // save modification state
    bool modified( activeDisplayLocal.document()->isModified() );

    // create MainWindow
    MainWindow& window( newMainWindow() );
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

    Debug::Throw( "WindowServer::_reparent.\n" );

    // retrieve windows
    TextDisplay& firstDisplay( _findDisplay( first ) );
    TextView& first_view( _findView( first ) );

    // check for first display clones
    if( !BASE::KeySet<TextDisplay>(firstDisplay).empty() )
    {
        InformationDialog( &firstDisplay,
            "Software limitation:\n"
            "Dropped display has clones in the current window.\n"
            "It cannot be reparented.\n" ).exec();
        return;
    }

    // save modification state
    bool modified( firstDisplay.document()->isModified() );

    // retrieve second display and corresponding view
    TextView& view = _findView( second );

    // do nothing if first and second display already belong to the same view
    if( &view == &first_view ) return;

    // create new display in text view
    view.selectDisplay( second );
    TextDisplay& newDisplay = view.splitDisplay( defaultOrientation( NORMAL ), false );
    newDisplay.synchronize( &firstDisplay );

    // close display
    firstDisplay.document()->setModified( false );
    first_view.closeDisplay( firstDisplay );

    // restore modification state
    newDisplay.setModified( modified );
    view.setActiveDisplay( newDisplay );

    // make view active
    BASE::KeySet<MainWindow> windows( view );
    Q_ASSERT( windows.size() == 1 );
    MainWindow& window( **windows.begin() );
    window.setActiveView( view );
    window.raise();

    return;

}

//_______________________________________________
void WindowServer::_reparentToMain( const File& first, const File& second )
{

    Debug::Throw( "WindowServer::_reparentToMain.\n" );

    // retrieve windows
    TextDisplay& firstDisplay( _findDisplay( first ) );
    TextView& first_view( _findView( first ) );
    MainWindow& first_window( _findWindow( first ) );

    // retrieve main window associated to second file
    MainWindow& window( _findWindow( second ) );

    // make sure first view has multiple files
    if( (&first_window == &window ) && first_view.independentDisplayCount() < 2 ) return;

    // check for first display clones
    if( !BASE::KeySet<TextDisplay>(firstDisplay).empty() )
    {
        InformationDialog( &firstDisplay,
            "Software limitation:\n"
            "Dropped display has clones in the current window.\n"
            "It cannot be reparented.\n" ).exec();
        return;
    }

    // save modification state
    bool modified( firstDisplay.document()->isModified() );

    // create new text view in window
    TextView& view( window.newTextView() );
    view.activeDisplay().synchronize( &firstDisplay );

    // close display
    firstDisplay.document()->setModified( false );
    first_view.closeDisplay( firstDisplay );

    // restore modification state
    view.activeDisplay().setModified( modified );

    // raise window
    window.raise();

    return;

}

//_______________________________________________
void WindowServer::_saveAll( void )
{
    Debug::Throw( "WindowServer::_saveAll.\n" );
    _save( records( ModifiedOnly ) );
    return;

}

//_______________________________________________
void WindowServer::_save( FileRecord::List records )
{
    Debug::Throw( "WindowServer::_save.\n" );

    // check how many records are modified
    if( records.empty() ) return;

    // ask for confirmation
    if( records.size() > 1 && !SaveAllDialog( &_activeWindow(), records ).exec() ) return;

    // retrieve windows
    foreach( MainWindow* window, BASE::KeySet<MainWindow>( this ) )
    {

        // retrieve displays
        foreach( TextDisplay* display,  window->associatedDisplays() )
        {
            if( !display->document()->isModified() ) continue;
            if( std::find_if( records.begin(), records.end(), FileRecord::SameFileFTor( display->file() ) ) == records.end() ) continue;
            display->save();
        }

    }

}

//_______________________________________________
bool WindowServer::_close( FileRecord::List records )
{
    Debug::Throw( "WindowServer::_close.\n" );

    // check how many records are modified
    Q_ASSERT( !records.empty() );

    // ask for confirmation
    if( records.size() > 1 && !CloseFilesDialog( &_activeWindow(), records ).exec() ) return true;

    QStringList files;
    foreach( const FileRecord& record, records )
    { files << record.file(); }

    return _close( files );

}

//________________________________________________________________
bool WindowServer::_close( QStringList files )
{

    int state( AskForSaveDialog::Unknown );

    // need a first loop over associated windows to store modified files
    QSet<QString> modifiedFiles;
    BASE::KeySet<MainWindow> windows( this );
    foreach( MainWindow* window, windows )
    {
        foreach( TextDisplay* display, window->associatedDisplays() )
        {

            // see if file is in list
            if( std::find( files.begin(), files.end(), display->file() ) == files.end() ) continue;
            if( display->document()->isModified() ) modifiedFiles.insert( display->file() );

        }

    }

    // retrieve windows
    foreach( MainWindow* window, windows )
    {

        // retrieve views
        BASE::KeySet<TextView> views( window );
        foreach( TextView* view, views )
        {

            BASE::KeySet<TextDisplay> displays( view );
            foreach( TextDisplay* display, displays )
            {

                // see if file is in list
                if( std::find( files.begin(), files.end(), display->file() ) == files.end() ) continue;

                Debug::Throw() << "WindowServer::_close - file: " << display->file() << endl;

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
    MainWindow* out( 0 );

    // retrieve windows
    foreach( MainWindow* window, BASE::KeySet<MainWindow>( this ) )
    {

        // retrieve displays
        foreach( TextDisplay* display, window->associatedDisplays() )
        {
            if( display->file() == file )
            {
                out = window;
                break;
            }
        }

    }

    Q_CHECK_PTR( out );
    return *out;

}

//_______________________________________________
TextView& WindowServer::_findView( const File& file )
{

    TextDisplay& display = _findDisplay( file );
    BASE::KeySet<TextView> views( display );
    Q_ASSERT( views.size() == 1 );
    return **views.begin();
}

//_______________________________________________
TextDisplay& WindowServer::_findDisplay( const File& file )
{
    TextDisplay* out( 0 );

    // retrieve windows
    foreach( MainWindow* window, BASE::KeySet<MainWindow>( this ) )
    {

        // retrieve displays
        foreach( TextDisplay* display, window->associatedDisplays() )
        {
            if( display->file() == file )
            {
                out = display;
                break;
            }
        }

    }

    Q_CHECK_PTR( out );
    return *out;

}
//_______________________________________________
FileRecord WindowServer::_selectFileFromDialog( void )
{

    Debug::Throw( "WindowServer::_selectFileFromDialog.\n" );

    FileRecord record;

    // create file dialog
    File file( FileDialog( &_activeWindow() ).selectFile(_activeWindow().activeDisplay().workingDirectory() ).getFile() );
    if( file.isNull() ) return record;
    else file = file.expand();

    // assign file to record
    record.setFile( file );
    return record;

}

//_______________________________________________
bool WindowServer::_createNewFile( const FileRecord& record )
{
    Debug::Throw( "WindowServer::_createNewFile.\n" );

    // check file is valid
    if( record.file().isEmpty() ) return false;

    // create NewFileDialog
    NewFileDialog::ReturnCodes buttons( NewFileDialog::Create | NewFileDialog::Cancel );
    if( records().empty() ) buttons |= NewFileDialog::Exit;

    NewFileDialog dialog( &_activeWindow(), record.file(), buttons );
    dialog.centerOnParent();
    int state = dialog.exec();

    Debug::Throw() << "WindowServer::Open - New file dialog state: " << state << endl;
    switch( state )
    {

        case NewFileDialog::Create:
        {
            File fullname( record.file().expand() );
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
        case NewFileDialog::Exit: qApp->quit();
        case NewFileDialog::Cancel: return false;
        break;

    }

    return false;

}

//________________________________________________________________
void WindowServer::_applyArguments( TextDisplay& display, CommandLineArguments arguments )
{
    Debug::Throw( "WindowServer::_applyArguments.\n" );

    CommandLineParser parser( Application::commandLineParser( arguments ) );

    //! see if autospell action is required
    bool autospell( parser.hasFlag( "--autospell" ) );

    //! see if autospell filter and dictionary are required
    QString filter = parser.hasOption( "--filter" ) ? parser.option( "--filter" ) : "";
    QString dictionary = parser.hasOption( "--dictionary" ) ? parser.option( "--dictionary" ) : "";
    Debug::Throw() << "WindowServer::_applyArguments -"
        << " filter:" << filter
        << " dictionary: " << dictionary
        << endl;

    if( autospell ) display.autoSpellAction().setChecked( true );
    if( !filter.isEmpty() ) display.selectFilter( filter );
    if( !dictionary.isEmpty() ) display.selectDictionary( dictionary );
    Debug::Throw( "WindowServer::_applyArguments - done.\n" );

}

//________________________________________________________________
void WindowServer::_setActiveWindow( MainWindow& window )
{
    Debug::Throw() << "WindowServer::setActiveWindow - key: " << window.key() << endl;
    Q_ASSERT( window.isAssociated( this ) );
    activeWindow_ = &window;
}


//________________________________________________________________
bool WindowServer::_hasActiveWindow( void ) const
{

    // check if active window exists
    if( !activeWindow_ ) return false;

    // check if found in list of associated windows
    BASE::KeySet<MainWindow> windows( this );

    // not sure it works when the window pointed to by activeWindow_ has been removed
    return windows.find( activeWindow_ ) != windows.end();

}
