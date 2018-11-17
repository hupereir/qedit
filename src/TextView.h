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
class TextView: public QWidget, public Base::Key, private Base::Counter<TextView>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit TextView( QWidget* );

    //* true if widget is being deleted
    bool isClosed() const
    { return closed_; }

    //* set to true if widget is to be deleted
    void setIsClosed( bool value )
    { closed_ = value; }

    //*@name display management
    //@{

    //* true if has associated displays
    bool hasDisplays() const
    { return !Base::KeySet<TextDisplay>( this ).empty(); }

    //* return number of displays
    int displayCount() const
    { return Base::KeySet<TextDisplay>( this ).size(); }

    //* return number of independant displays
    int independentDisplayCount() const;

    //* return number of independent modified displays
    int modifiedDisplayCount() const;

    //@}

    //*@name display management
    //@{

    //* retrieve active display
    TextDisplay& activeDisplay()
    {
        Q_CHECK_PTR( activeDisplay_ );
        return *activeDisplay_;
    }

    //* retrieve active display
    const TextDisplay& activeDisplay() const
    { return *activeDisplay_; }

    //* select display from file
    bool selectDisplay( const File& file );

    //* true if display passed in argument is active
    bool isActiveDisplay( const TextDisplay& display ) const
    { return &display == activeDisplay_; }

    //* change active display manualy
    void setActiveDisplay( TextDisplay& );

    //* close display
    void closeActiveDisplay();

    //* close display
    /** Ask for save if display is modified */
    void closeDisplay( TextDisplay& );

    //@}

    //* set new document
    void setIsNewDocument();

    //* set file and read
    void setFile( File file );

    //* split display
    TextDisplay& splitDisplay( const Qt::Orientation&, bool clone );

    //* save all displays
    void saveAll();

    //* ignore all display modifications
    void ignoreAll();

    //* select class name
    void selectClassName( QString value )
    { activeDisplay().selectClassName( value ); }

    //* rehighlight all displays
    void rehighlight();

    //* diff files
    void diff();

    //* position timer
    QTimer& positionTimer()
    { return positionTimer_; }

    Q_SIGNALS:

    //* emitted when parent window must be update
    void needUpdate( TextDisplay::UpdateFlags );

    //* current display overwrite mode changed
    void modifiersChanged( TextEditor::Modifiers );

    //* independent display count changed
    void displayCountChanged();

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
    void _checkDisplays();

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

#endif
