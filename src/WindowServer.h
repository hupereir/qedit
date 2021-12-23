#ifndef WindowServer_h
#define WindowServer_h

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

#include "CommandLineParser.h"
#include "Counter.h"
#include "File.h"
#include "FileRecord.h"
#include "Key.h"
#include "TextSelection.h"

#include <QAction>
#include <QObject>

class MainWindow;
class ScratchFileMonitor;
class TextDisplay;
class TextView;

class WindowServer: public QObject, public Base::Key, private Base::Counter<WindowServer>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit WindowServer( QObject* = nullptr );

    //* create new empty main window
    MainWindow& newMainWindow();

    enum Flag
    {
        None = 0,
        ModifiedOnly = 1<<0,
        ExistingOnly = 1<<1
    };

    Q_DECLARE_FLAGS( Flags, Flag );

    //* returns list of opened files
    /** the active_window parameter is used to possibly tag files that belong to it */
    FileRecord::List records( Flags = None, QWidget* activeWindow = 0 ) const;

    //* close all windows gracefully
    /** returns false if the opperation was canceled. */
    bool closeAll();

    //* read file from arguments and open relevant windows
    void readFilesFromArguments( const CommandLineParser& );

    //* open all records in list
    void open( const FileRecord::List& );

    //*@name actions
    //@{

    //* save all
    QAction& saveAllAction() const
    { return *saveAllAction_; }

    //@}

    //* open mode
    enum class OpenMode
    {
        ActiveWindow,
        NewWindow
    };

    //* multiple files replace
    void multipleFileReplace( const QList<File>&, const TextSelection &);

    //* orientation
    enum class OrientationMode
    {
        Normal,
        Diff
    };

    //* default orientation
    const Qt::Orientation& defaultOrientation( WindowServer::OrientationMode mode = OrientationMode::Normal )
    { return mode == OrientationMode::Normal ? defaultOrientation_:defaultDiffOrientation_; }

    Q_SIGNALS:

    //* emitted whenever the session file list is modified
    void sessionFilesChanged();

    private:

    //* update configuration
    void _updateConfiguration();

    //* active window changed
    void _activeWindowChanged( MainWindow* );

    //* update actions
    void _updateActions();

    //*@ new file methods
    //@{

    //* new file
    void _newFile()
    { _newFile( openMode_ ); }

    //* new file
    void _newFile( Qt::Orientation );

    //@}

    //*@ open methods
    //@{

    //* from dialog
    bool _open()
    { return _open( _selectFileFromDialog() ); }

    //* open file
    bool _open( const FileRecord &record )
    { return _open( record, openMode_ ); }

    //* open in new window
    bool _openInNewWindow( const FileRecord &record )
    { return _open( record, OpenMode::NewWindow ); }

    //* open in active window
    bool _openInActiveWindow( const FileRecord &record )
    { return _open( record, OpenMode::ActiveWindow ); }

    //* open in current tab
    bool _openInActiveView( const FileRecord &record )
    { return _open( record, defaultOrientation( OrientationMode::Normal ) ); }

    //* open in active view
    bool _openHorizontal()
    { return _open( _selectFileFromDialog(), Qt::Vertical ); }

    //* open in active view
    bool _openVertical()
    { return _open( _selectFileFromDialog(), Qt::Horizontal ); }

    //@}

    //* detach
    /** this closes the active view and opens it in a separate window */
    void _detach();

    //* detach
    /** this closes the active view and opens it in a separate window */
    void _detach( const File& );

    //* reparent
    /**
    close display containing first file,
    create matching one in Window that contains second
    */
    void _reparent( const File&, const File& );

    //* reparent
    /**
    close display containing first file,
    create matching one in Window that contains second
    */
    void _reparentToMain( const File&, const File& );

    //* save all edited files
    void _saveAll();

    //* save selected file
    void _save( FileRecord::List );

    //* close selected files
    bool _close( const QStringList );

    //* close selected files
    bool _close( FileRecord::List );

    //* new file
    void _newFile( OpenMode );

    //* open file
    bool _open( const FileRecord&, OpenMode );

    //* open file
    bool _open( const FileRecord&, Qt::Orientation );

    //* find mainwindow matching given file
    MainWindow& _findWindow( const File& );

    //* find text view matching given file
    TextView& _findView( const File& );

    //* find display matching given file
    bool _hasDisplay( const File& ) const;

    //* find display matching given file
    TextDisplay& _findDisplay( const File& );

    //* select file record from dialog
    /** return empty record if no file is opened or file is directory */
    FileRecord _selectFileFromDialog();

    //* returns true if new file should be created
    bool _createNewFile( const FileRecord& );

    //* detach display
    void _detach( TextDisplay& );

    //* apply command-line arguments to currant display
    void _applyCommandLineArguments( TextDisplay&, const CommandLineParser& );

    //* active window
    void _setActiveWindow( MainWindow& );

    //* true when active window is valid
    bool _hasActiveWindow() const;

    //* active window
    MainWindow& _activeWindow()
    { return *activeWindow_; }

    //* active window
    const MainWindow& _activeWindow() const
    { return *activeWindow_; }

    //* default orientation for split tabs
    void _setDefaultOrientation( WindowServer::OrientationMode mode, Qt::Orientation value )
    {
        switch( mode )
        {
            case OrientationMode::Diff:
            defaultDiffOrientation_ = value;
            break;

            default:
            case OrientationMode::Normal:
            defaultOrientation_ = value;
            break;
        }

    }

    //* true at first call (via Application::realizeWidget)
    bool firstCall_ = true;

    //* default orientation
    Qt::Orientation defaultOrientation_ = Qt::Horizontal;

    //* default orientation (diff mode
    Qt::Orientation defaultDiffOrientation_ = Qt::Horizontal;

    //* open mode
    OpenMode openMode_ = OpenMode::ActiveWindow;

    //* active window
    MainWindow* activeWindow_ = nullptr;

    //*@name actions
    //@{

    //* save all modified files
    QAction* saveAllAction_ = nullptr;

    //@}

    //* scratch files
    ScratchFileMonitor* scratchFileMonitor_ = nullptr;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( WindowServer::Flags );

#endif
