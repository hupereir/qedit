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

#include "Application.h"
#include "AutoSave.h"
#include "Debug.h"
#include "Diff.h"
#include "InformationDialog.h"
#include "MainWindow.h"
#include "NewFileDialog.h"
#include "Singleton.h"
#include "TextView.h"

#include <QLayout>

//___________________________________________________________________
TextView::TextView( QWidget* parent ):
    QWidget( parent ),
    Counter( "TextView" ),
    positionTimer_(this)
{

    Debug::Throw( "TextView::TextView.\n" );

    setAttribute( Qt::WA_DeleteOnClose );

    // main layout
    setLayout( new QVBoxLayout() );
    layout()->setMargin(0);
    layout()->setSpacing(0);

    // create new Text display and register autosave thread
    TextDisplay& display = _newTextDisplay( this );
    layout()->addWidget( &display );
    display.setActive( true );
    Singleton::get().application<Application>()->autoSave().newThread( &display );
    Singleton::get().application<Application>()->fileCheck().registerDisplay( &display );

    // position update timer
    positionTimer_.setSingleShot( true );
    positionTimer_.setInterval( 100 );

}

//____________________________________________
void TextView::setIsNewDocument( void )
{
    Debug::Throw( "TextView::setIsNewDocument.\n" );

    // look for first empty display
    Base::KeySet<TextDisplay> displays( this );
    auto iter = std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() );
    if( iter == displays.end() )
    {
        Debug::Throw(0) << "TextView::setIsNewDocument - invalid display" << endl;
        return;
    }

    TextDisplay &display( **iter );

    // set display as new document
    display.setIsNewDocument();

    // set focus
    setActiveDisplay( display );
    display.setFocus();
    emit displayCountChanged();

    return;
}

//____________________________________________
void TextView::setFile( File file )
{

    Debug::Throw() << "TextView::setFile - " << file << endl;
    if( file.isEmpty() )
    {
        Debug::Throw(0) << "TextView::setFile - invalid file name" << file << endl;
        return;
    }

    // look for first empty display
    Base::KeySet<TextDisplay> displays( this );
    auto iter = std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() );
    if( iter == displays.end() )
    {
        Debug::Throw(0) << "TextView::setFile - invalid display" << endl;
        return;
    }

    TextDisplay &display( **iter );

    // open file in active display
    display.setFile( file );

    // set focus
    setActiveDisplay( display );
    display.setFocus();
    emit displayCountChanged();

    return;
}

//________________________________________________________________
int TextView::independentDisplayCount( void ) const
{
    int out( 0 );
    Base::KeySet<TextDisplay> displays( this );
    for( auto&& iter = displays.begin(); iter != displays.end(); ++iter )
    { if( std::find_if( displays.begin(), iter, Base::Key::IsAssociatedFTor( *iter ) ) == iter ) out++; }

    return out;
}

//________________________________________________________________
int TextView::modifiedDisplayCount( void ) const
{

    int out( 0 );
    Base::KeySet<TextDisplay> displays( this );
    for( auto&& iter = displays.begin(); iter != displays.end(); ++iter )
    {
        // increment if no associated display is found in the already processed displays
        // and if current is modified
        if(
            std::find_if( displays.begin(), iter, Base::Key::IsAssociatedFTor( *iter ) ) == iter &&
            (*iter)->document()->isModified() )
        { out++; }
    }

    return out;

}

//________________________________________________________________
bool TextView::selectDisplay( const File& file )
{

    Debug::Throw( "TextView::selectDisplay.\n" );

    // check if active display match.
    if( TextDisplay::SameFileFTor( file )( &activeDisplay() ) ) return true;

    Base::KeySet<TextDisplay> displays( this );
    auto iter( std::find_if(
        displays.begin(),
        displays.end(),
        TextDisplay::SameFileFTor( file ) ) );
    if( iter == displays.end() ) return false;

    // change active display
    setActiveDisplay( **iter );
    (*iter)->setFocus();

    return true;

}

//________________________________________________________________
void TextView::closeActiveDisplay( void )
{ closeDisplay( activeDisplay() ); }

//________________________________________________________________
void TextView::setActiveDisplay( TextDisplay& display )
{

    Debug::Throw() << "TextView::setActiveDisplay - key: " << display.key() << endl;
    if( !display.isAssociated( this ) )
    {
        Debug::Throw(0) << "TextView::setActiveDisplay - invalid display" << endl;
        return;
    }

    if( activeDisplay_ != &display )
    {

        activeDisplay_ = &display;
        if( display.file().size() || display.isNewDocument() )
        { emit needUpdate( TextDisplay::ActiveDisplayChanged ); }

    }

    if( !activeDisplay().isActive() )
    {

        Base::KeySet<TextDisplay> displays( this );
        displays.remove( &activeDisplay() );
        for( const auto& display:displays )
        { display->setActive( false ); }

        activeDisplay().setActive( true );

    }

}

//___________________________________________________________
void TextView::closeDisplay( TextDisplay& display )
{

    Debug::Throw() << "TextView::closeDisplay - display: " << display.key() << endl;

    // check if display is modified and has no associates in window
    if(
        display.document()->isModified() &&
        Base::KeySet<TextDisplay>( &display ).empty() &&
        display.askForSave() ==  AskForSaveDialog::Cancel ) return;

    // retrieve parent and grandparent of current display
    QWidget* parent( display.parentWidget() );
    QSplitter* parentSplitter( qobject_cast<QSplitter*>( parent ) );

    // retrieve displays associated to current
    Base::KeySet<TextDisplay> displays( &display );

    // check if display is a new document
    // remove its filename from server if needed
    if( display.isNewDocument() && displays.empty() )
    { TextDisplay::newDocumentNameServer().remove( display.file() ); }

    // check how many children remain in parentSplitter if any
    // take action if it is less than 2 (the current one to be deleted, and another one)
    if( parentSplitter && parentSplitter->count() == 2 )
    {

        // retrieve child that is not the current editor
        // need to loop over existing widgets because the editor above has not been deleted yet
        QWidget* child(0);
        for( int index = 0; index < parentSplitter->count(); index++ )
        {
            if( parentSplitter->widget( index ) != &display )
            {
                child = parentSplitter->widget( index );
                break;
            }
        }
        Q_CHECK_PTR( child );

        // retrieve splitter parent
        QWidget* grand_parent( parentSplitter->parentWidget() );

        // try cast to a splitter
        QSplitter* grandParentSplitter( qobject_cast<QSplitter*>( grand_parent ) );

        // move child to grandParentSplitter if any
        if( grandParentSplitter )
        {

            grandParentSplitter->insertWidget( grandParentSplitter->indexOf( parentSplitter ), child );

        } else {

            child->setParent( grand_parent );
            grand_parent->layout()->addWidget( child );

        }

        // delete parentSplitter, now that it is empty
        display.setIsClosed( true );
        parentSplitter->deleteLater();


    } else {

        // the editor is deleted only if its parent splitter is not
        // otherwise this will trigger double deletion of the editor
        // which will then crash
        display.setIsClosed( true );
        display.deleteLater();

    }

    // if no associated displays, retrieve all, set the first as active
    if( displays.empty() ) displays = Base::KeySet<TextDisplay>( this );
    Base::KeySetIterator<TextDisplay> iterator( displays );
    iterator.toBack();
    while( iterator.hasPrevious() )
    {
        TextDisplay* current( iterator.previous() );
        if( current != &display && !current->isClosed() )
        {
            setActiveDisplay( *current );
            activeDisplay().setFocus();
            break;
        }
    }


}

//___________________________________________________________
TextDisplay& TextView::splitDisplay( const Qt::Orientation& orientation, bool clone )
{

    Debug::Throw( "TextView::splitDisplay.\n" );

    // keep local pointer to current active display
    TextDisplay& activeDisplayLocal( activeDisplay() );

    // compute desired dimension of the new splitter
    // along its splitting direction
    int dimension(0);
    if( clone ) dimension = (orientation == Qt::Horizontal) ? activeDisplayLocal.width():activeDisplayLocal.height();
    else dimension = (orientation == Qt::Horizontal) ? width():height();

    // create new splitter
    QSplitter& splitter( _newSplitter( orientation, clone ) );

    // create new display
    TextDisplay& display( _newTextDisplay(0) );

    // insert in splitter, at correct position
    if( clone ) splitter.insertWidget( splitter.indexOf( &activeDisplayLocal )+1, &display );
    else splitter.addWidget( &display );

    // recompute dimension
    // take the max of active display and splitter,
    // in case no new splitter was created.
    dimension = qMax( dimension, (orientation == Qt::Horizontal) ? splitter.width():splitter.height() );

    // assign equal size to all splitter children
    QList<int> sizes;
    for( int i=0; i<splitter.count(); i++ )
    { sizes << dimension/splitter.count(); }
    splitter.setSizes( sizes );

    // synchronize both displays, if cloned
    if( clone )
    {

        /*
        if there exists no clone of active display,
        backup text and register a new Sync object
        */
        Base::KeySet<TextDisplay> displays( &activeDisplayLocal );

        // clone new display
        display.synchronize( &activeDisplayLocal );

        // perform associations
        // check if active displays has associates and propagate to new
        for( const auto& iter:displays )
        { Base::Key::associate( &display, iter ); }

        // associate this display to AutoSave threads
        Base::KeySet<AutoSaveThread> threads( &activeDisplayLocal );
        for( const auto& thread:threads )
        { Base::Key::associate( &display, thread ); }

        // associate new display to active
        Base::Key::associate( &display, &activeDisplayLocal );

    } else {

        // register new AutoSave thread
        Singleton::get().application<Application>()->autoSave().newThread( &display );
        Singleton::get().application<Application>()->fileCheck().registerDisplay( &display );

    }

    return display;

}

//____________________________________________
void TextView::saveAll( void )
{
    Debug::Throw( "TextView::saveAll.\n" );

    // retrieve all displays
    for( const auto& display:Base::KeySet<TextDisplay>( this ) )
    { if( display->document()->isModified() ) display->save(); }

    return;

}


//____________________________________________
void TextView::ignoreAll( void )
{
    Debug::Throw( "TextView::ignoreAll.\n" );

    // retrieve all displays
    for( const auto& display:Base::KeySet<TextDisplay>( this ) )
    { display->setModified( false ); }

    return;

}

//________________________________________________________________
void TextView::rehighlight( void )
{
    Debug::Throw( "TextView::rehighlight.\n" );

    // retrieve associated TextDisplay
    Base::KeySet<TextDisplay> displays( this );
    for( auto&& iter = displays.begin(); iter != displays.end(); ++iter )
    {
        // this trick allow to run the rehighlight only once per set of associated displays
        if( std::find_if( displays.begin(), iter, Base::Key::IsAssociatedFTor( *iter ) ) == iter ) (*iter)->rehighlight();
    }

    return;
}

//____________________________________________
void TextView::checkDisplayModifications( TextEditor* editor )
{

    Debug::Throw( "TextView::checkDisplayModifications.\n" );

    // convert to TextDisplay
    TextDisplay& display( *static_cast<TextDisplay*>( editor ) );

    Base::KeySet<TextDisplay>  deadDisplays;

    // check file
    if( display.checkFileRemoved() == FileRemovedDialog::Close )
    {

        // register displays as dead
        Base::KeySet<TextDisplay> associatedDisplays( &display );
        for( const auto& displayIter:associatedDisplays )
        { deadDisplays.insert( displayIter ); }

        display.document()->setModified( false );
        deadDisplays.insert( &display );

    } else {

        display.checkFileReadOnly();
        display.checkFileModified();

    }

    // clear flags
    display.clearFileCheckData();

    // delete dead displays
    if( !deadDisplays.empty() )
    {

        Debug::Throw() << "TextView::checkDisplayModifications - dead displays: " << deadDisplays.size() << endl;
        for( const auto& display:deadDisplays )
        { closeDisplay( *display ); }

    }

}

//_______________________________________________________
void TextView::diff( void )
{
    Debug::Throw( "TextView::diff.\n" );

    // retrieve displays
    int n_displays( independentDisplayCount() );

    // check number of files
    if( n_displays > 2 )
    {
        InformationDialog( this, tr( "Too many files opened. Diff canceled." ) ).exec();
        return;
    }

    if( n_displays < 2 )
    {
        InformationDialog( this, tr( "Too few files opened. Diff canceled." ) ).exec();
        return;
    }

    // create diff object
    // Diff* diff = new Diff( this );
    Diff *diff( new Diff( this ) );

    // store active display as first to compare
    TextDisplay& first = activeDisplay();

    // retrieve displays associated to window
    // look for the first one that is not associated to the active display
    Base::KeySet<TextDisplay> displays( this );
    auto iter = displays.begin();
    for(; iter != displays.end(); ++iter )
    {
        if( !( *iter == &first || (*iter)->isAssociated( &first ) ) )
        {
            diff->setTextDisplays( first, *(*iter) );
            break;
        }
    }

    // check that one display was found
    if( iter == displays.end() )
    {
        Debug::Throw(0) << "TextView::diff - display not found" << endl;
        return;
    }

    // try run
    if( !diff->run() )
    {
        InformationDialog( this, diff->error() ).exec();
        diff->deleteLater();
        return;
    }

    return;

}

//____________________________________________
void TextView::_checkDisplays( void )
{

    Debug::Throw() << "TextView::_checkDisplays - key: " << key() << endl;
    Base::KeySet<TextDisplay> displays( this );
    if( displays.empty() )
    {
        Debug::Throw() << "TextView::_checkDisplays - closing" << endl;
        setIsClosed( true );
        close();
        return;
    }

    emit displayCountChanged();

    // this should not happen and will likely generate a crash later on.
    // it is meant for debugging
    if( displays.find( &activeDisplay() ) == displays.end() )
    { activeDisplay_ = 0; }

}

//____________________________________________
void TextView::_activeDisplayChanged( TextEditor* editor )
{
    Debug::Throw() << "TextView::_activeDisplayChanged - " << editor->key() << endl;
    setActiveDisplay( *static_cast<TextDisplay*>(editor) );
}

//____________________________________________________________
QSplitter& TextView::_newSplitter( const Qt::Orientation& orientation, bool clone )
{

    Debug::Throw( "TextView::_newSplitter.\n" );
    QSplitter *splitter = 0;

    if( clone )
    {

        /* in clone mode, a new splitter is added as a parent of the activeDisplay.
        the new (cloned) TextDisplay will appear side by side with it */

        // retrieve parent of current display
        QWidget* parent( activeDisplay().parentWidget() );

        // try cast to splitter
        // do not create a new splitter if the parent has same orientation
        QSplitter *parentSplitter( qobject_cast<QSplitter*>( parent ) );
        if( parentSplitter && parentSplitter->orientation() == orientation ) splitter = parentSplitter;
        else {


            // move splitter to the first place if needed
            if( parentSplitter )
            {

                Debug::Throw( "TextView::_newSplitter - found parent splitter.\n" );
                // create a splitter with correct orientation
                // give him no parent, because the parent is set in QSplitter::insertWidget()
                splitter = new LocalSplitter(0);
                splitter->setOrientation( orientation );
                parentSplitter->insertWidget( parentSplitter->indexOf( &activeDisplay() ), splitter );

            } else {

                // create a splitter with correct orientation
                splitter = new LocalSplitter(parent);
                splitter->setOrientation( orientation );
                parent->layout()->addWidget( splitter );

            }

            // reparent current display
            splitter->addWidget( &activeDisplay() );

            // resize parent splitter if any
            if( parentSplitter )
            {
                int dimension = ( parentSplitter->orientation() == Qt::Horizontal) ?
                    parentSplitter->width():
                    parentSplitter->height();

                QList<int> sizes;
                for( int i=0; i<parentSplitter->count(); i++ )
                { sizes << dimension/parentSplitter->count(); }
                parentSplitter->setSizes( sizes );

            }

        }

    } else {

        /*
        in no clone mode, a new splitter is created at the top level
        the new (cloned) TextDisplay will appear side by side with all other displays
        */

        // keep track of first (either TextDisplay or QSplitter) from this
        QWidget *child(0);

        // retrieve children and loop
        for( const auto& object:TextView::children() )
        {
            if( ( child = qobject_cast<QWidget*>( object ) ) )
            { break; }

        }

        // check child could be retrieved
        Q_CHECK_PTR( child );

        // try cast child to splitter
        // if exists and have same orientation, do not create a new one
        QSplitter* child_splitter( qobject_cast<QSplitter*>( child ) );
        if( child_splitter && child_splitter->orientation() == orientation ) splitter = child_splitter;
        else {
            // create new splitter
            splitter = new LocalSplitter( this );
            splitter->setOrientation( orientation );
            layout()->addWidget( splitter );

            // reparent first child
            splitter->addWidget( child );
        }
    }

    // return created splitter
    return *splitter;

}

//_____________________________________________________________
TextDisplay& TextView::_newTextDisplay( QWidget* parent )
{
    Debug::Throw( "\nTextView::_newTextDisplay.\n" );

    // create textDisplay
    TextDisplay* display = new TextDisplay( parent );

    // connections
    connect( display, SIGNAL(needUpdate(TextDisplay::UpdateFlags)), SIGNAL(needUpdate(TextDisplay::UpdateFlags)) );
    connect( display, SIGNAL(hasFocus(TextEditor*)), SLOT(_activeDisplayChanged(TextEditor*)) );
    connect( display, SIGNAL(hasFocus(TextEditor*)), SLOT(checkDisplayModifications(TextEditor*)) );
    connect( display, SIGNAL(cursorPositionChanged()), &positionTimer_, SLOT(start()) );
    connect( display, SIGNAL(modifiersChanged(TextEditor::Modifiers)), SIGNAL(modifiersChanged(TextEditor::Modifiers)) );

    connect( display, SIGNAL(undoAvailable(bool)), SIGNAL(undoAvailable(bool)) );
    connect( display, SIGNAL(redoAvailable(bool)), SIGNAL(redoAvailable(bool)) );

    connect( display, SIGNAL(destroyed()), SLOT(_checkDisplays()) );
    connect( display, SIGNAL(destroyed()), SIGNAL(displayCountChanged()) );

    // retrieve parent main window
    MainWindow &window = *static_cast<MainWindow*>( TextView::window() );

    // customize display actions
    /* this is needed to be able to handle a single dialog for stacked windows */
    display->gotoLineAction().disconnect();
    connect( &display->gotoLineAction(), SIGNAL(triggered()), &window, SLOT(selectLineFromDialog()) );

    display->findAction().disconnect();
    connect( &display->findAction(), SIGNAL(triggered()), &window, SLOT(findFromDialog()) );
    connect( display, SIGNAL(noMatchFound()), &window, SIGNAL(noMatchFound()) );
    connect( display, SIGNAL(matchFound()), &window, SIGNAL(matchFound()) );
    connect( display, SIGNAL(lineNotFound()), &window, SIGNAL(lineNotFound()) );
    connect( display, SIGNAL(lineFound()), &window, SIGNAL(lineFound()) );

    display->replaceAction().disconnect();
    connect( &display->replaceAction(), SIGNAL(triggered()), &window, SLOT(replaceFromDialog()) );

    // associate display to this editFrame
    Base::Key::associate( this, display );

    // update current display and focus
    setActiveDisplay( *display );
    display->setFocus();

    return *display;

}

//__________________________________________________
LocalSplitter::LocalSplitter( QWidget* parent ):
    QSplitter( parent ),
    Counter( "LocalSplitter" )
{ Debug::Throw( "LocalSplitter::LocalSplitter.\n" ); }

//__________________________________________________
LocalSplitter::~LocalSplitter( void )
{ Debug::Throw( "LocalSplitter::~LocalSplitter.\n" ); }
