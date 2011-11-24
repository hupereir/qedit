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
* Place, Suite 330, Boston, MA 02111-1307 USA
*
*
*******************************************************************************/

/*!
\file WindowServer.cpp
\brief handles opened edition windows
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

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
#include "SessionFilesFrame.h"
#include "Singleton.h"
#include "Util.h"

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtCore/QTextStream>

//________________________________________________________________
const QString WindowServer::SINGLE_WINDOW = "open in current window";
const QString WindowServer::MULTIPLE_WINDOWS = "open in new window";

//________________________________________________________________
WindowServer::WindowServer( QObject* parent ):
    QObject( parent ),
    Counter( "WindowServer" ),
    firstCall_( true ),
    defaultOrientation_( Qt::Horizontal ),
    defaultDiffOrientation_( Qt::Vertical ),
    open_mode_( ACTIVE_WINDOW ),
    activeWindow_( 0 )
{

    Debug::Throw( "WindowServer::WindowServer.\n" );

    // create actions
    saveAllAction_ = new QAction( IconEngine::get( ICONS::SAVE_ALL ), "Save A&ll", this );
    connect( saveAllAction_, SIGNAL( triggered() ), SLOT( _saveAll() ) );

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
FileRecord::List WindowServer::records( bool modified_only, QWidget* window ) const
{

    Debug::Throw( "WindowServer::records.\n" );

    // output
    FileRecord::List records;

    // get associated main windows
    Application& application( *Singleton::get().application<Application>() );
    BASE::KeySet<MainWindow> windows( this );
    for( BASE::KeySet<MainWindow>::iterator windowIter = windows.begin(); windowIter != windows.end(); ++windowIter )
    {

        // check if current window match the one passed in argument
        bool is_active_window( *windowIter == window );

        // retrieve associated TextDisplays
        BASE::KeySet<TextDisplay> displays( (*windowIter)->associatedDisplays() );
        for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); ++iter )
        {

            // check modification status
            if( modified_only && !(*iter)->document()->isModified() ) continue;

            // retrieve file
            // store in map if not empty
            const File& file( (*iter)->file() );
            if( file.isEmpty() ) continue;

            // insert in map (together with modification status)
            FileRecord record = (*iter)->isNewDocument() ? FileRecord( file ):application.recentFiles().get(file);

            // set flags
            unsigned int flags( FileRecordProperties::NONE );
            if( (*iter)->document()->isModified() ) flags |= FileRecordProperties::MODIFIED;
            if( (*iter)->fileCheckData().flag() != FileCheck::Data::NONE ) flags |= FileRecordProperties::ALTERED;

            if( is_active_window )
            {
                flags |= FileRecordProperties::ACTIVE;
                if( (*windowIter)->isActiveDisplay( **iter ) ) flags |= FileRecordProperties::SELECTED;
            }

            // assign flags and store
            records.push_back( record.setFlags( flags ) );

        }

    }

    sort( records.begin(), records.end(), FileRecord::FileFTor() );
    records.erase( unique( records.begin(), records.end(), FileRecord::SameFileFTor() ), records.end() );
    Debug::Throw( "WindowServer::records - done.\n" );
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

    std::list<QString> files;
    for( FileRecord::List::const_iterator iter = records.begin(); iter != records.end(); ++iter )
    { files.push_back( iter->file() ); }

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
        QString buffer;
        QTextStream what( &buffer );
        what << "Do you really want to open " << filenames.size() << " files at the same time ?" << endl;
        what << "This might be very resource intensive and can overload your computer." << endl;
        what << "If you choose No, only the first file will be opened.";
        if( !QuestionDialog( &_activeWindow(), buffer ).exec() )
        { filenames = filenames.mid(0,1); }

    }

    // check if at least one file is opened
    bool file_opened( false );

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
        for( QStringList::const_iterator iter = filenames.begin(); iter != filenames.end(); ++iter )
        {

            if( first )
            {

                if( file_opened |= _open( File( *iter ).expand() ) )
                {
                    _applyArguments( _activeWindow().activeDisplay(), arguments );
                    first = false;
                }

            } else {

                if( file_opened |= _open( File( *iter ).expand(), orientation ) )
                { _applyArguments( _activeWindow().activeDisplay(), arguments ); }

            }

        }

        if( diff )
        {
            if( !first && _activeWindow().activeView().independentDisplayCount() == 2 ) _activeWindow().diffAction().trigger();
            else InformationDialog( &_activeWindow(), "invalid number of files selected. <Diff> canceled." ).exec();
        }

    } else {

        // default mode
        for( QStringList::const_iterator iter = filenames.begin(); iter != filenames.end(); ++iter )
        {

            OpenMode mode( _openMode() );
            if( parser.hasFlag( "--same-window" ) ) mode = ACTIVE_WINDOW;
            else if( parser.hasFlag( "--new-window" ) ) mode = NEW_WINDOW;

            bool opened = _open( File( *iter ).expand(), mode );
            if( opened ) { _applyArguments( _activeWindow().activeDisplay(), arguments ); }
            file_opened |= opened;

        }

    }

    if( !file_opened )
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

//___________________________________________________________
void WindowServer::multipleFileReplace( std::list<File> files, TextSelection selection )
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
    for( std::list<File>::iterator iter = files.begin(); iter != files.end(); ++iter )
    {

        File& file( *iter );

        // find matching window
        BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( file ) );
        assert( iter != windows.end() );

        // loop over views
        BASE::KeySet<TextView> views( *iter );
        for( BASE::KeySet<TextView>::iterator viewIter = views.begin(); viewIter != views.end(); ++viewIter )
        {
            if( !(*viewIter)->selectDisplay( file ) ) continue;
            TextDisplay* display( &(*viewIter)->activeDisplay() );
            connect( display, SIGNAL( progressAvailable( int ) ), &dialog, SLOT( setValue( int ) ) );
            maximum += display->toPlainText().size();
            displays.insert( display );
        }

    }

    dialog.setMaximum( maximum );

    // loop over displays and perform replacement
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); ++iter )
    {
        counts += (*iter)->replaceInWindow( selection, false );
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
    _setOpenMode( XmlOptions::get().raw( "OPEN_MODE" ) == MULTIPLE_WINDOWS ? NEW_WINDOW:ACTIVE_WINDOW );
    _setDefaultOrientation( NORMAL, XmlOptions::get().raw( "ORIENTATION" ) == MainWindow::LEFT_RIGHT ? Qt::Horizontal : Qt::Vertical );
    _setDefaultOrientation( DIFF, XmlOptions::get().raw( "DIFF_ORIENTATION" ) == MainWindow::LEFT_RIGHT ? Qt::Horizontal : Qt::Vertical );

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
    saveAllAction().setEnabled( !records( true ).empty() );

}

//_______________________________________________
void WindowServer::_newFile( WindowServer::OpenMode mode )
{

    Debug::Throw( "WindowServer::_newFile.\n" );

    // retrieve all MainWindows
    BASE::KeySet<MainWindow> windows( this );

    // try find empty editor
    TextView* view(0);
    BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
    if( iter != windows.end() )
    {

        // select the view that contains the empty display
        BASE::KeySet<TextView> views( *iter );
        BASE::KeySet<TextView>::iterator viewIter( find_if( views.begin(), views.end(), MainWindow::EmptyFileFTor() ) );
        assert( viewIter != views.end() );
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
    TextView& active_view( _activeWindow().activeView() );

    // look for an empty display
    // create a new display if none is found
    BASE::KeySet<TextDisplay> displays( active_view );
    BASE::KeySet<TextDisplay>::iterator iter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
    if( iter == displays.end() ) active_view.splitDisplay( orientation, false );

    active_view.setIsNewDocument();
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
    BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file() ) );
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
    iter = find_if( windows.begin(), windows.end(), MainWindow::EmptyFileFTor() );
    if( iter != windows.end() )
    {

        // select the view that contains the empty display
        BASE::KeySet<TextView> views( *iter );
        BASE::KeySet<TextView>::iterator viewIter( find_if( views.begin(), views.end(), MainWindow::EmptyFileFTor() ) );
        assert( viewIter != views.end() );
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
    TextView& active_view( _activeWindow().activeView() );

    // retrieve all windows and find one matching
    BASE::KeySet<MainWindow> windows( this );
    BASE::KeySet<MainWindow>::iterator iter = find_if( windows.begin(), windows.end(), MainWindow::SameFileFTor( record.file() ) );
    if( iter != windows.end() )
    {

        // find matching view
        BASE::KeySet<TextView> views( *iter );
        BASE::KeySet<TextView>::iterator viewIter = find_if( views.begin(), views.end(), MainWindow::SameFileFTor( record.file() ) );
        assert( viewIter != views.end() );

        // check if the found view is the current
        if( *viewIter == &active_view )
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
        BASE::KeySet<TextDisplay> displays( &active_view );
        BASE::KeySet<TextDisplay>::iterator displayIter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
        TextDisplay& display( displayIter == displays.end() ? active_view.splitDisplay( orientation, false ):**displayIter );

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
        for( BASE::KeySet<TextDisplay>::iterator displayIter = displays.begin(); displayIter != displays.end(); ++displayIter )
        { (*viewIter)->closeDisplay( **displayIter ); }

        // restore modification state and make new display active
        display.setModified( modified );
        active_view.setActiveDisplay( display );
        display.setFocus();

    } else {

        // look for an empty display
        // create a new display if none is found
        BASE::KeySet<TextDisplay> displays( active_view );
        BASE::KeySet<TextDisplay>::iterator displayIter( find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() ) );
        if( displayIter == displays.end() ) active_view.splitDisplay( orientation, false );

        // open file in this window
        active_view.setFile( record.file() );

    }

    return true;

}

//_______________________________________________
void WindowServer::_detach( void )
{

    Debug::Throw( "WindowServer::_detach.\n" );

    MainWindow& activeWindow_local( _activeWindow() );

    // check number of independent displays
    assert( activeWindow_local.activeView().independentDisplayCount() > 1 || BASE::KeySet<TextView>( &_activeWindow() ).size() > 1 );

    // get current display
    TextDisplay& active_display_local( activeWindow_local.activeView().activeDisplay() );

    // check number of displays associated to active
    if( !BASE::KeySet<TextDisplay>(active_display_local).empty() )
    {
        InformationDialog( &activeWindow_local,
            "Software limitation:\n"
            "Active display has clones in the current window.\n"
            "It cannot be detached." ).exec();
        return;
    }

    // save modification state
    bool modified( active_display_local.document()->isModified() );

    // create MainWindow
    MainWindow& window( newMainWindow() );
    window.show();

    // clone its display from the current
    window.activeView().activeDisplay().synchronize( &active_display_local );

    // close display
    active_display_local.document()->setModified( false );
    activeWindow_local.activeView().closeDisplay( active_display_local );

    // update modification state
    window.activeView().activeDisplay().document()->setModified( modified );

    return;

}

//_______________________________________________
void WindowServer::_reparent( const File& first, const File& second )
{

    Debug::Throw( "WindowServer::_reparent.\n" );

    // retrieve windows
    TextDisplay& first_display( _findDisplay( first ) );
    TextView& first_view( _findView( first ) );

    // check for first display clones
    if( !BASE::KeySet<TextDisplay>(first_display).empty() )
    {
        InformationDialog( &first_display,
            "Software limitation:\n"
            "Dropped display has clones in the current window.\n"
            "It cannot be reparented.\n" ).exec();
        return;
    }

    // save modification state
    bool modified( first_display.document()->isModified() );

    // retrieve second display and corresponding view
    TextView& view = _findView( second );

    // do nothing if first and second display already belong to the same view
    if( &view == &first_view ) return;

    // create new display in text view
    view.selectDisplay( second );
    TextDisplay& newDisplay = view.splitDisplay( defaultOrientation( NORMAL ), false );
    newDisplay.synchronize( &first_display );

    // close display
    first_display.document()->setModified( false );
    first_view.closeDisplay( first_display );

    // restore modification state
    newDisplay.setModified( modified );
    view.setActiveDisplay( newDisplay );

    // make view active
    BASE::KeySet<MainWindow> windows( view );
    assert( windows.size() == 1 );
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
    TextDisplay& first_display( _findDisplay( first ) );
    TextView& first_view( _findView( first ) );
    MainWindow& first_window( _findWindow( first ) );

    // retrieve main window associated to second file
    MainWindow& window( _findWindow( second ) );

    // make sure first view has multiple files
    if( (&first_window == &window ) && first_view.independentDisplayCount() < 2 ) return;

    // check for first display clones
    if( !BASE::KeySet<TextDisplay>(first_display).empty() )
    {
        InformationDialog( &first_display,
            "Software limitation:\n"
            "Dropped display has clones in the current window.\n"
            "It cannot be reparented.\n" ).exec();
        return;
    }

    // save modification state
    bool modified( first_display.document()->isModified() );

    // create new text view in window
    TextView& view( window.newTextView() );
    view.activeDisplay().synchronize( &first_display );

    // close display
    first_display.document()->setModified( false );
    first_view.closeDisplay( first_display );

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
    _save( records( true ) );
    return;

}

//_______________________________________________
void WindowServer::_save( FileRecord::List records )
{
    Debug::Throw( "WindowServer::_save.\n" );

    // check how many records are modified
    assert( !records.empty() );

    // ask for confirmation
    if( records.size() > 1 && !SaveAllDialog( &_activeWindow(), records ).exec() ) return;

    // retrieve windows
    BASE::KeySet<MainWindow> windows( this );
    for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); ++iter )
    {

        // retrieve displays
        BASE::KeySet<TextDisplay> displays( (*iter)->associatedDisplays() );
        for( BASE::KeySet<TextDisplay>::iterator displayIter = displays.begin(); displayIter != displays.end(); ++displayIter )
        {
            TextDisplay& display( **displayIter );
            if( !display.document()->isModified() ) continue;
            if( find_if( records.begin(), records.end(), FileRecord::SameFileFTor( display.file() ) ) == records.end() ) continue;
            display.save();
        }

    }

}


//_______________________________________________
void WindowServer::_close( QStringList filenames )
{

    FileRecord::List records;
    for( QStringList::const_iterator iter = filenames.begin(); iter != filenames.end(); ++iter )
    { records.push_back( FileRecord( *iter ) ); }

    _close( records );

}

//_______________________________________________
void WindowServer::_close( FileRecord::List records )
{
    Debug::Throw( "WindowServer::_close.\n" );

    // check how many records are modified
    assert( !records.empty() );

    // ask for confirmation
    if( records.size() > 1 && !CloseFilesDialog( &_activeWindow(), records ).exec() ) return;

    std::list<QString> files;
    for( FileRecord::List::const_iterator iter = records.begin(); iter != records.end(); ++iter )
    { files.push_back( iter->file() ); }

    _close( files );

}

//________________________________________________________________
bool WindowServer::_close( const std::list<QString>& files )
{

    int state( AskForSaveDialog::UNKNOWN );

    // need a first loop over associated windows to store modified files
    std::set<QString> modifiedFiles;
    BASE::KeySet<MainWindow> windows( this );
    for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); ++iter )
    {
        BASE::KeySet<TextDisplay> displays( (*iter)->associatedDisplays() );
        for( BASE::KeySet<TextDisplay>::iterator displayIter = displays.begin(); displayIter != displays.end(); ++displayIter )
        {

            // see if file is in list
            TextDisplay& display( **displayIter );
            if( find( files.begin(), files.end(), display.file() ) == files.end() ) continue;
            if( display.document()->isModified() ) modifiedFiles.insert( display.file() );

        }

    }

    // retrieve windows
    for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); ++iter )
    {

        // retrieve views
        BASE::KeySet<TextView> views( *iter );
        for( BASE::KeySet<TextView>::iterator viewIter = views.begin(); viewIter != views.end(); ++viewIter )
        {

            TextView& view( **viewIter );
            BASE::KeySet<TextDisplay> displays( &view );
            for( BASE::KeySet<TextDisplay>::iterator displayIter = displays.begin(); displayIter != displays.end(); ++displayIter )
            {
                TextDisplay& display( **displayIter );

                // see if file is in list
                if( find( files.begin(), files.end(), display.file() ) == files.end() ) continue;

                Debug::Throw() << "WindowServer::_close - file: " << display.file() << endl;

                if( display.document()->isModified() )
                {

                    if( state == AskForSaveDialog::YES_TO_ALL ) display.save();
                    else if( state == AskForSaveDialog::NO_TO_ALL ) display.document()->setModified( false );
                    else
                    {
                        state = display.askForSave( modifiedFiles.size() > 1 );
                        if( state == AskForSaveDialog::YES_TO_ALL || state == AskForSaveDialog::YES ) display.save();
                        else if( state == AskForSaveDialog::NO_TO_ALL  || state == AskForSaveDialog::YES ) display.document()->setModified( false );
                        else if( state == AskForSaveDialog::CANCEL ) return false;
                        modifiedFiles.erase( display.file() );
                        view.closeDisplay( display );
                    }

                }

                view.closeDisplay( display );

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
    BASE::KeySet<MainWindow> windows( this );
    for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); ++iter )
    {

        // retrieve displays
        BASE::KeySet<TextDisplay> displays( (*iter)->associatedDisplays() );
        for( BASE::KeySet<TextDisplay>::iterator displayIter = displays.begin(); displayIter != displays.end(); ++displayIter )
        {
            if( (*displayIter)->file() == file )
            {
                out = *iter;
                break;
            }
        }

    }

    assert( out );
    return *out;

}

//_______________________________________________
TextView& WindowServer::_findView( const File& file )
{

    TextDisplay& display = _findDisplay( file );
    BASE::KeySet<TextView> views( display );
    assert( views.size() == 1 );
    return **views.begin();
}

//_______________________________________________
TextDisplay& WindowServer::_findDisplay( const File& file )
{
    TextDisplay* out( 0 );

    // retrieve windows
    BASE::KeySet<MainWindow> windows( this );
    for( BASE::KeySet<MainWindow>::iterator iter = windows.begin(); iter != windows.end(); ++iter )
    {

        // retrieve displays
        BASE::KeySet<TextDisplay> displays( (*iter)->associatedDisplays() );
        for( BASE::KeySet<TextDisplay>::iterator displayIter = displays.begin(); displayIter != displays.end(); ++displayIter )
        {
            if( (*displayIter)->file() == file )
            {
                out = *displayIter;
                break;
            }
        }

    }

    assert( out );
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
    int buttons( NewFileDialog::CREATE | NewFileDialog::CANCEL );
    if( records().empty() ) buttons |= NewFileDialog::EXIT;

    NewFileDialog dialog( &_activeWindow(), record.file(), buttons );
    dialog.centerOnParent();
    int state = dialog.exec();

    Debug::Throw() << "WindowServer::Open - New file dialog state: " << state << endl;
    switch( state )
    {

        case NewFileDialog::CREATE:
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

        case NewFileDialog::EXIT: qApp->quit();

        case NewFileDialog::CANCEL: return false;

        default:
        assert(0);
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
    assert( window.isAssociated( this ) );
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
