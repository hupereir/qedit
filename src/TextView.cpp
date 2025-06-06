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

#include "TextView.h"
#include "Application.h"
#include "AutoSave.h"
#include "AutoSaveThread.h"
#include "Debug.h"
#include "Diff.h"
#include "InformationDialog.h"
#include "MainWindow.h"
#include "MessageWidget.h"
#include "NewFileDialog.h"
#include "QtUtil.h"
#include "Singleton.h"


#include <QLayout>

//* local QSplitter object, derived from Counter
/** helps keeping track of how many splitters are created/deleted */
class LocalSplitter: public QSplitter, private Base::Counter<LocalSplitter>
{

    Q_OBJECT

    public:

    //* constructor
    explicit LocalSplitter( QWidget* parent = nullptr ):
        QSplitter( parent ),
        Counter( QStringLiteral("LocalSplitter") )
    { Debug::Throw( QStringLiteral("LocalSplitter::LocalSplitter.\n") ); }

};

#include "TextView.moc"

//___________________________________________________________________
TextView::TextView( QWidget* parent ):
    QWidget( parent ),
    Counter( QStringLiteral("TextView") ),
    positionTimer_(this)
{

    Debug::Throw( QStringLiteral("TextView::TextView.\n") );

    setAttribute( Qt::WA_DeleteOnClose );

    // main layout
    auto layout = new QVBoxLayout;
    setLayout( layout );
    QtUtil::setMargin(layout, 0);
    layout->setSpacing(0);

    // information layout
    informationLayout_ = new QVBoxLayout;
    QtUtil::setMargin(informationLayout_, 0);
    informationLayout_->setSpacing(0);
    layout->addLayout( informationLayout_, 0 );

    // editor layout
    editorLayout_ = new QVBoxLayout;
    QtUtil::setMargin(editorLayout_, 0);
    editorLayout_->setSpacing(0);
    layout->addLayout( editorLayout_, 1 );

    // create new Text display and register autosave thread
    TextDisplay& display = _newTextDisplay( this );
    editorLayout_->addWidget( &display, 1 );
    display.setActive( true );
    Base::Singleton::get().application<Application>()->autoSave().newThread( &display );
    Base::Singleton::get().application<Application>()->fileCheck().registerDisplay( &display );

    // create diff module
    diffModule_ = new Diff( this );

    // position update timer
    positionTimer_.setSingleShot( true );
    positionTimer_.setInterval( 100 );

}

//____________________________________________
void TextView::setIsNewDocument()
{
    Debug::Throw( QStringLiteral("TextView::setIsNewDocument.\n") );

    // look for first empty display
    Base::KeySet<TextDisplay> displays( this );
    auto iter = std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() );
    if( iter == displays.end() ) return;

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
void TextView::setFile( const File &file )
{

    Debug::Throw() << "TextView::setFile - " << file << Qt::endl;
    if( file.isEmpty() ) return;

    // look for first empty display
    Base::KeySet<TextDisplay> displays( this );
    auto iter = std::find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() );
    if( iter == displays.end() ) return;

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
int TextView::independentDisplayCount() const
{
    int out( 0 );

    Base::KeySet<TextDisplay> displays( this );
    for( auto&& iter = displays.begin(); iter != displays.end(); ++iter )
    { if( std::none_of( displays.begin(), iter, Base::Key::IsAssociatedFTor( *iter ) ) ) out++; }

    return out;
}

//________________________________________________________________
int TextView::modifiedDisplayCount() const
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

    Debug::Throw() << "TextView::selectDisplay - file: " << file << Qt::endl;

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
void TextView::closeActiveDisplay()
{ closeDisplay( activeDisplay() ); }

//________________________________________________________________
void TextView::setActiveDisplay( TextDisplay& display )
{

    Debug::Throw() << "TextView::setActiveDisplay - key: " << display.key() << Qt::endl;
    if( !display.isAssociated( this ) )
    {
        Debug::Throw(0, QStringLiteral("TextView::setActiveDisplay - invalid display.\n") );
        return;
    }

    if( activeDisplay_ != &display )
    {

        activeDisplay_ = &display;
        if( !display.file().isEmpty() || display.isNewDocument() )
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

    Debug::Throw() << "TextView::closeDisplay - display: " << display.key() << Qt::endl;

    // check if display is modified and has no associates in window
    if(
        display.document()->isModified() &&
        Base::KeySet<TextDisplay>( &display ).empty() &&
        display.askForSave() ==  AskForSaveDialog::Cancel ) return;

    // cleanup associated dialogs
    display.hideFileRemovedWidgets();
    display.hideFileModifiedWidgets();

    // retrieve displays associated to current
    Base::KeySet<TextDisplay> displays( &display );

    // check if display is a new document
    // remove its filename from server if needed
    if( display.isNewDocument() && displays.empty() )
    { TextDisplay::newDocumentNameServer().remove( display.file() ); }

    // retrieve parent and grandparent of current display
    auto parent( display.parentWidget() );
    auto parentSplitter( qobject_cast<QSplitter*>( parent ) );

    // check how many children remain in parentSplitter if any
    // take action if it is less than 2 (the current one to be deleted, and another one)
    if( parentSplitter && parentSplitter->count() == 2 )
    {

        // retrieve child that is not the current editor
        // need to loop over existing widgets because the editor above has not been deleted yet
        QWidget* child( nullptr );
        for( int index = 0; index < parentSplitter->count(); index++ )
        {
            if( parentSplitter->widget( index ) != &display )
            {
                child = parentSplitter->widget( index );
                break;
            }
        }

        // retrieve splitter parent
        auto grandParent( parentSplitter->parentWidget() );

        // try cast to a splitter
        auto grandParentSplitter( qobject_cast<QSplitter*>( grandParent ) );

        // move child to grandParentSplitter if any
        if( grandParentSplitter )
        {

            grandParentSplitter->insertWidget( grandParentSplitter->indexOf( parentSplitter ), child );

        } else if( grandParent == this ) {

            child->setParent( this );
            editorLayout_->addWidget( child, 1 );

        } else {

            Debug::Throw(0, QStringLiteral("TextView::closeDisplay - something is wrong with layouts.\n") );

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
    TextDisplay* active = nullptr;

    for( const auto& current:displays )
    {
        if( current != &display && !current->isClosed() )
        { active = current; }
    }

    if( active )
    {
        setActiveDisplay( *active );
        activeDisplay().setFocus();
    }


}

//___________________________________________________________
TextDisplay& TextView::splitDisplay( Qt::Orientation orientation, bool clone )
{

    Debug::Throw( QStringLiteral("TextView::splitDisplay.\n") );

    // keep local pointer to current active display
    auto& activeDisplayLocal( activeDisplay() );

    // compute desired dimension of the new splitter
    // along its splitting direction
    int dimension(0);
    if( clone ) dimension = (orientation == Qt::Horizontal) ? activeDisplayLocal.width():activeDisplayLocal.height();
    else dimension = (orientation == Qt::Horizontal) ? width():height();

    // create new splitter
    auto& splitter( _newSplitter( orientation, clone ) );

    // create new display
    auto& display( _newTextDisplay(nullptr) );

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
    { sizes.append( dimension/splitter.count() ); }
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
        Base::Singleton::get().application<Application>()->autoSave().newThread( &display );
        Base::Singleton::get().application<Application>()->fileCheck().registerDisplay( &display );

    }

    return display;

}

//____________________s________________________
void TextView::saveAll()
{
    Debug::Throw( QStringLiteral("TextView::saveAll.\n") );

    // retrieve all displays
    for( const auto& display:Base::KeySet<TextDisplay>( this ) )
    { if( display->document()->isModified() ) display->save(); }

    return;

}


//____________________________________________
void TextView::ignoreAll()
{
    Debug::Throw( QStringLiteral("TextView::ignoreAll.\n") );

    // retrieve all displays
    for( const auto& display:Base::KeySet<TextDisplay>( this ) )
    { display->setModified( false ); }

    return;

}

//________________________________________________________________
void TextView::rehighlight()
{
    Debug::Throw( QStringLiteral("TextView::rehighlight.\n") );

    // retrieve associated TextDisplay
    Base::KeySet<TextDisplay> displays( this );
    for( auto&& iter = displays.begin(); iter != displays.end(); ++iter )
    {
        // this trick allow to run the rehighlight only once per set of associated displays
        if( std::none_of( displays.begin(), iter, Base::Key::IsAssociatedFTor( *iter ) ) )
        { (*iter)->rehighlight(); }
    }

    return;
}

//____________________________________________
void TextView::checkDisplayModifications( TextEditor* editor )
{

    Debug::Throw( QStringLiteral("TextView::checkDisplayModifications.\n") );

    // convert to TextDisplay
    TextDisplay& display( *static_cast<TextDisplay*>( editor ) );
    display.checkFileRemoved();
    display.checkFileReadOnly();
    display.checkFileModified();

}

//_______________________________________________________
void TextView::diff()
{
    Debug::Throw( QStringLiteral("TextView::diff.\n") );

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
    auto diff( new Diff( this ) );

    // store active display as first to compare
    auto& first = activeDisplay();

    // retrieve displays associated to window
    // look for the first one that is not associated to the active display
    Base::KeySet<TextDisplay> displays( this );
    auto iter = std::find_if( displays.begin(), displays.end(),
        [&first]( TextDisplay* display )
        { return !( display == &first || display->isAssociated( &first ) ); } );

    // check that one display was found
    if( iter == displays.end() )
    {

        Debug::Throw(0, QStringLiteral("TextView::diff - display not found.\n") );
        return;

    } else {

        diff->setTextDisplays( first, *(*iter) );

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
void TextView::addMessageWidget( MessageWidget* widget )
{
    Debug::Throw( QStringLiteral("TextView::addMessageWidget.\n") );
    widget->setParent( this );
    informationLayout_->addWidget( widget );
    return;
}

//____________________________________________
void TextView::_checkDisplays()
{

    Debug::Throw() << "TextView::_checkDisplays - key: " << key() << Qt::endl;
    Base::KeySet<TextDisplay> displays( this );
    if( displays.empty() )
    {
        Debug::Throw( QStringLiteral("TextView::_checkDisplays - closing.\n") );
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
    Debug::Throw() << "TextView::_activeDisplayChanged - " << editor->key() << Qt::endl;
    setActiveDisplay( *static_cast<TextDisplay*>(editor) );
}

//____________________________________________
void TextView::_closeDisplay( const File& file )
{

    Debug::Throw( QStringLiteral("TextView::_closeDisplay.\n") );

    // do nothing if file not set
    if( file.isEmpty() ) return;

    // get associated displays
    Base::KeySet<TextDisplay> associatedDisplays( this );
    for( const auto& display:associatedDisplays )
    { if( display->file() == file ) closeDisplay( *display ); }

}

//____________________________________________________________
QSplitter& TextView::_newSplitter( Qt::Orientation orientation, bool clone )
{

    Debug::Throw( QStringLiteral("TextView::_newSplitter.\n") );
    QSplitter *splitter( nullptr );

    if( clone )
    {

        /* in clone mode, a new splitter is added as a parent of the activeDisplay.
        the new (cloned) TextDisplay will appear side by side with it */

        // retrieve parent of current display
        auto parent( activeDisplay().parentWidget() );

        // try cast to splitter
        // do not create a new splitter if the parent has same orientation
        auto parentSplitter( qobject_cast<QSplitter*>( parent ) );
        if( parentSplitter && parentSplitter->orientation() == orientation ) splitter = parentSplitter;
        else {


            // move splitter to the first place if needed
            if( parentSplitter )
            {

                Debug::Throw( QStringLiteral("TextView::_newSplitter - found parent splitter.\n") );
                // create a splitter with correct orientation
                // give him no parent, because the parent is set in QSplitter::insertWidget()
                splitter = new LocalSplitter;
                splitter->setOrientation( orientation );
                parentSplitter->insertWidget( parentSplitter->indexOf( &activeDisplay() ), splitter );

            } else if( parent == this ) {

                // create a splitter with correct orientation
                splitter = new LocalSplitter(this);
                splitter->setOrientation( orientation );
                editorLayout_->addWidget( splitter, 1 );

            } else {

                Debug::Throw(0, QStringLiteral("TextView::_newSplitter - something is wrong with layouts.\n") );

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
                { sizes.append( dimension/parentSplitter->count() ); }
                parentSplitter->setSizes( sizes );

            }

        }

    } else {

        /*
        in no clone mode, a new splitter is created at the top level
        the new (cloned) TextDisplay will appear side by side with all other displays
        */

        // keep track of first (either TextDisplay or QSplitter) from this
        QWidget *child( nullptr );

        // retrieve children and loop
        auto children( this->children() );
        for( const auto& object:children )
        {
            if( (child =  qobject_cast<QWidget*>( object ) ) )
            { break;}
        }

        // try cast child to splitter
        // if exists and have same orientation, do not create a new one
        auto childSplitter( qobject_cast<QSplitter*>( child ) );
        if( childSplitter && childSplitter->orientation() == orientation ) splitter = childSplitter;
        else {

            // create new splitter
            splitter = new LocalSplitter( this );
            splitter->setOrientation( orientation );
            static_cast<QBoxLayout*>(layout())->addWidget( splitter, 1 );

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
    Debug::Throw( QStringLiteral("\nTextView::_newTextDisplay.\n") );

    // create textDisplay
    auto display = new TextDisplay( parent );
    QtUtil::setWidgetSides(display, {});

    // connections
    connect( display, &TextDisplay::needUpdate, this, &TextView::needUpdate );
    connect( display, &TextDisplay::hasFocus, this, &TextView::_activeDisplayChanged );
    connect( display, &TextDisplay::hasFocus, this, &TextView::checkDisplayModifications );
    connect( display, &TextDisplay::cursorPositionChanged, &positionTimer_, QOverload<>::of(&QTimer::start) );
    connect( display, &TextEditor::modifiersChanged, this, &TextView::modifiersChanged );
    connect( display, &TextDisplay::requestClose, this, &TextView::_closeDisplay );

    connect( display, &QTextEdit::undoAvailable, this, &TextView::undoAvailable );
    connect( display, &QTextEdit::redoAvailable, this, &TextView::redoAvailable );

    connect( display, &QObject::destroyed, this, &TextView::_checkDisplays );
    connect( display, &QObject::destroyed, this, &TextView::displayCountChanged );

    // retrieve parent main window
    auto &window = *static_cast<MainWindow*>( TextView::window() );

    // customize display actions
    /* this is needed to be able to handle a single dialog for stacked windows */
    display->gotoLineAction().disconnect();
    connect( &display->gotoLineAction(), &QAction::triggered, &window, &MainWindow::selectLineFromDialog );

    display->findAction().disconnect();
    connect( &display->findAction(), &QAction::triggered, &window, &MainWindow::findFromDialog );
    connect( display, &TextEditor::noMatchFound, &window, &MainWindow::noMatchFound );
    connect( display, &TextEditor::matchFound, &window, &MainWindow::matchFound );
    connect( display, &TextEditor::lineNotFound, &window, &MainWindow::lineNotFound );
    connect( display, &TextEditor::lineFound, &window, &MainWindow::lineFound );

    display->replaceAction().disconnect();
    connect( &display->replaceAction(), &QAction::triggered, &window, &MainWindow::replaceFromDialog );

    // associate display to this editFrame
    Base::Key::associate( this, display );

    // update current display and focus
    setActiveDisplay( *display );
    display->setFocus();

    return *display;

}
