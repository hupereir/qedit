#ifndef WindowServer_h
#define WindowServer_h

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

#include "CommandLineArguments.h"
#include "Counter.h"
#include "File.h"
#include "FileRecord.h"
#include "Key.h"
#include "TextSelection.h"

#include <QtGui/QAction>
#include <QtCore/QObject>

class MainWindow;
class ScratchFileMonitor;
class TextDisplay;
class TextView;

class WindowServer: public QObject, public Counter, public BASE::Key
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //!@name Open mode string
    //@{

    static const QString SINGLE_WINDOW;

    static const QString MULTIPLE_WINDOWS;

    //@}

    //! constructor
    WindowServer( QObject* parent = 0 );

    //! destructor
    virtual ~WindowServer( void );

    //! create new empty main window
    MainWindow& newMainWindow( void );

    //! returns list of opened files
    /*! the active_window parameter is used to possibly tag files that belong to it */
    FileRecord::List records( bool modified_only = false, QWidget* active_window = 0 ) const;

    //! close all windows gracefully
    /*! returns false if the opperation was cancelled. */
    bool closeAll( void );

    //! read file from arguments and open relevant windows
    void readFilesFromArguments( CommandLineArguments );

    //!@name actions
    //@{

    //! save all
    QAction& saveAllAction( void ) const
    { return *saveAllAction_; }

    //@}

    //! open mode
    enum OpenMode
    {

        ACTIVE_WINDOW,
        NEW_WINDOW

    };

    //! multiple files replace
    void multipleFileReplace( QList<File>, TextSelection );

    //! orientation
    enum OrientationMode
    {
        NORMAL,
        DIFF
    };

    //! default orientation
    const Qt::Orientation& defaultOrientation( const OrientationMode& mode = NORMAL )
    { return mode == NORMAL ? defaultOrientation_:defaultDiffOrientation_; }

    signals:

    //! emmited whenever the session file list is modified
    void sessionFilesChanged( void );

    private slots:

    //! update configuration
    void _updateConfiguration( void );

    //! active window changed
    void _activeWindowChanged( MainWindow* );

    //! update actions
    void _updateActions( void );

    //!@ new file methods
    //@{

    //! new file
    void _newFile( void )
    { _newFile( _openMode() ); }

    //! new file
    void _newFile( Qt::Orientation );

    //@}

    //!@ open methods
    //@{

    //! from dialog
    bool _open( void )
    { return _open( _selectFileFromDialog() ); }

    //! open file
    bool _open( FileRecord record )
    { return _open( record, _openMode() ); }

    //! open in new window
    bool _openInNewWindow( FileRecord record )
    { return _open( record, NEW_WINDOW ); }

    //! open in active window
    bool _openInActiveWindow( FileRecord record )
    { return _open( record, ACTIVE_WINDOW ); }

    //! open in current tab
    bool _openInActiveView( FileRecord record )
    { return _open( record, defaultOrientation( NORMAL ) ); }

    //! open in active view
    bool _openHorizontal( void )
    { return _open( _selectFileFromDialog(), Qt::Vertical ); }

    //! open in active view
    bool _openVertical( void )
    { return _open( _selectFileFromDialog(), Qt::Horizontal ); }

    //@}

    //! detach
    /*! this closes the active view and opens it in a separate window */
    void _detach( void );

    //! reparent
    /*!
    close display containing first file,
    create matching one in Window that contains second
    */
    void _reparent( const File&, const File& );

    //! reparent
    /*!
    close display containing first file,
    create matching one in Window that contains second
    */
    void _reparentToMain( const File&, const File& );

    //! save all edited files
    void _saveAll( void );

    //! save selected file
    void _save( FileRecord::List );

    //! close selected files
    bool _close( const QStringList );

    //! close selected files
    bool _close( FileRecord::List );

    private:

    //! new file
    void _newFile( OpenMode );

    //! open file
    bool _open( FileRecord, OpenMode );

    //! open file
    bool _open( FileRecord, Qt::Orientation );

    //! find mainwindow matching given file
    MainWindow& _findWindow( const File& );

    //! find text view matching given file
    TextView& _findView( const File& );

    //! find display matching given file
    TextDisplay& _findDisplay( const File& );

    //! select file record from dialog
    /*! return empty record if no file is opened or file is directory */
    FileRecord _selectFileFromDialog( void );

    //! returns true if new file should be created
    bool _createNewFile( const FileRecord& );

    //! apply command-line arguments to currant display
    void _applyArguments( TextDisplay&, CommandLineArguments );

    //! active window
    void _setActiveWindow( MainWindow& );

    //! true when active window is valid
    bool _hasActiveWindow( void ) const;

    //! active window
    MainWindow& _activeWindow( void )
    { return *activeWindow_; }

    //! active window
    const MainWindow& _activeWindow( void ) const
    { return *activeWindow_; }

    //! default orientation for split tabs
    void _setDefaultOrientation( const OrientationMode& mode, const Qt::Orientation& value )
    {
        switch( mode )
        {
            case DIFF:
            defaultDiffOrientation_ = value;
            break;

            default:
            case NORMAL:
            defaultOrientation_ = value;
            break;
        }

    }

    //! default orientation for split tabs
    const Qt::Orientation _defaultdiffOrientation( void ) const
    { return defaultDiffOrientation_; }

    //! default orientation for split tabs
    void _setDiffOrientation( const Qt::Orientation& value )
    { defaultDiffOrientation_ = value; }

    //! open mode
    const OpenMode& _openMode( void ) const
    { return openMode_; }

    //! open mode
    void _setOpenMode( const OpenMode& mode )
    { openMode_ = mode; }

    //! first call
    const bool& _firstCall( void ) const
    { return firstCall_; }

    //! first call
    void _setFirstCall( bool value )
    { firstCall_ = value; }

    //! true at first call (via Application::realizeWidget)
    bool firstCall_;

    //! default orientation
    Qt::Orientation defaultOrientation_;

    //! default orientation (diff mode
    Qt::Orientation defaultDiffOrientation_;

    //! open mode
    OpenMode openMode_;

    //! active window
    MainWindow* activeWindow_;

    //!@name actions
    //@{

    //! save all modified files
    QAction* saveAllAction_;

    //@}

    //! scratch files
    ScratchFileMonitor* scratchFileMonitor_;

};

#endif
