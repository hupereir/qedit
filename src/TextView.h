#ifndef TextView_h
#define TextView_h

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

#include "Counter.h"
#include "FileRecord.h"
#include "Key.h"
#include "TextDisplay.h"

#include <QSplitter>
#include <QWidget>
#include <QTimer>

//* handles multiple views
class TextView: public QWidget, public Counter, public Base::Key
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    TextView( QWidget* );

    //* true if widget is being deleted
    bool isClosed( void ) const
    { return closed_; }

    //* set to true if widget is to be deleted
    void setIsClosed( bool value )
    { closed_ = value; }

    //*@name display management
    //@{

    //* true if has associated displays
    bool hasDisplays( void ) const
    { return !Base::KeySet<TextDisplay>( this ).isEmpty(); }

    //* return number of displays
    int displayCount( void ) const
    { return Base::KeySet<TextDisplay>( this ).count(); }

    //* return number of independant displays
    int independentDisplayCount( void ) const;

    //* return number of independent modified displays
    int modifiedDisplayCount( void ) const;

    //@}

    //*@name display management
    //@{

    //* retrieve active display
    TextDisplay& activeDisplay( void )
    {
        Q_CHECK_PTR( activeDisplay_ );
        return *activeDisplay_;
    }

    //* retrieve active display
    const TextDisplay& activeDisplay( void ) const
    { return *activeDisplay_; }

    //* select display from file
    bool selectDisplay( const File& file );

    //* true if display passed in argument is active
    bool isActiveDisplay( const TextDisplay& display ) const
    { return &display == activeDisplay_; }

    //* change active display manualy
    void setActiveDisplay( TextDisplay& );

    //* close display
    void closeActiveDisplay( void );

    //* close display
    /** Ask for save if display is modified */
    void closeDisplay( TextDisplay& );

    //@}

    //* set new document
    void setIsNewDocument( void );

    //* set file and read
    void setFile( File file );

    //* split display
    TextDisplay& splitDisplay( const Qt::Orientation&, bool clone );

    //* save all displays
    void saveAll( void );

    //* ignore all display modifications
    void ignoreAll( void );

    //* select class name
    void selectClassName( QString value )
    { activeDisplay().selectClassName( value ); }

    //* rehighlight all displays
    void rehighlight( void );

    //* diff files
    void diff( void );

    //* position timer
    QTimer& positionTimer( void )
    { return positionTimer_; }

    Q_SIGNALS:

    //* emitted when parent window must be update
    void needUpdate( TextDisplay::UpdateFlags );

    //* current display overwrite mode changed
    void modifiersChanged( TextEditor::Modifiers );

    //* independent display count changed
    void displayCountChanged( void );

    //* current display undo is available
    void undoAvailable( bool );

    //* current display redo is available
    void redoAvailable( bool );

    public Q_SLOTS:

    //* check modified displays
    void checkDisplayModifications( TextEditor* );

    private Q_SLOTS:

    //* check number of displays
    /**
    this is triggered by TextDisplay::destroyed()
    when no display is found the entire window is closed
    the active display is updated otherwise
    */
    void _checkDisplays( void );

    //* display focus changed
    void _activeDisplayChanged( TextEditor* );

    private:

    //* create new splitter
    QSplitter& _newSplitter( const Qt::Orientation&, bool  );

    //* create new TextDisplay
    TextDisplay& _newTextDisplay( QWidget* );

    //* closed
    bool closed_ = false;

    //* text display with focus
    TextDisplay* activeDisplay_ = nullptr;

    //* position update timer
    QTimer positionTimer_;

};

//* local QSplitter object, derived from Counter
/** helps keeping track of how many splitters are created/deleted */
class LocalSplitter: public QSplitter, public Counter
{

    public:

    //* constructor
    LocalSplitter( QWidget* );

    //* destructor
    virtual ~LocalSplitter( void );

};

#endif
