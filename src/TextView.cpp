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
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file TextView.cpp
  \brief handle multiple text views
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>

#include "Application.h"
#include "AutoSave.h"
#include "CustomFileDialog.h"
#include "Debug.h"
#include "Diff.h"
#include "MainWindow.h"
#include "NewFileDialog.h"
#include "TextView.h"

using namespace std;

//___________________________________________________________________
TextView::TextView( QWidget* parent ):
  QWidget( parent ),
  Counter( "TextView" ),
  active_display_( 0 ),
  position_timer_(this)  
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
  static_cast<Application*>(qApp)->autoSave().newThread( &display );

  // position update timer
  position_timer_.setSingleShot( true );
  position_timer_.setInterval( 100 );
  
}

//___________________________________________________________________
TextView::~TextView()
{ Debug::Throw( "TextView::~TextView.\n" ); }

//____________________________________________
void TextView::setIsNewDocument( void )
{
  Debug::Throw( "TextView::setIsNewDocument.\n" );

  // look for first empty display
  BASE::KeySet<TextDisplay> displays( this );  
  BASE::KeySet<TextDisplay>::iterator iter = find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() );
  assert( iter != displays.end() );
  TextDisplay &display( **iter );
   
  // set display as new document
  display.setIsNewDocument();
  
  // set focus
  setActiveDisplay( display );
  display.setFocus();
  emit displayCountChanged();
  
  Debug::Throw( "TextView::setIsNewDocument - done.\n" );
  
  return;
}

//____________________________________________
void TextView::setFile( File file )
{
  Debug::Throw() << "TextView::setFile - " << file << endl;

  // look for first empty display
  BASE::KeySet<TextDisplay> displays( this );  
  BASE::KeySet<TextDisplay>::iterator iter = find_if( displays.begin(), displays.end(), TextDisplay::EmptyFileFTor() );
  assert( iter != displays.end() );
  TextDisplay &display( **iter );
   
  // open file in active display
  if( !file.empty() ) display.setFile( file );
  else display.updateDocumentClass();
  
  // set focus
  setActiveDisplay( display );
  display.setFocus();
  emit displayCountChanged();
    
  return;
}

//________________________________________________________________
unsigned int TextView::independentDisplayCount( void )
{
  unsigned int out( 0 );
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {     
    // increment if no associated display is found in the already processed displays
    if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) == iter ) out++;
  }
  
  return out;
}

//________________________________________________________________
unsigned int TextView::modifiedDisplayCount( void )
{
  
  unsigned int out( 0 );
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { 
    // increment if no associated display is found in the already processed displays
    // and if current is modified
    if( 
      std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) == iter && 
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
  
  BASE::KeySet<TextDisplay> displays( this );
  BASE::KeySet<TextDisplay>::iterator iter( std::find_if(
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
  Debug::Throw() << "TextView::setActiveDisplay - key: " << display.key() << std::endl;
  assert( display.isAssociated( this ) );
  
  active_display_ = &display;
  if( !activeDisplay().isActive() )
  {

    BASE::KeySet<TextDisplay> displays( this );
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { (*iter)->setActive( false ); }
    
    activeDisplay().setActive( true );
    emit needUpdate( TextDisplay::ALL );

  }
  
  Debug::Throw( "TextView::setActiveDisplay - done.\n" );
  
}

//___________________________________________________________
void TextView::closeDisplay( TextDisplay& display )
{
  
  Debug::Throw( "TextView::closeDisplay.\n" );
  
  // check if display is modified and has no associates in window
  if( 
    display.document()->isModified() && 
    BASE::KeySet<TextDisplay>( &display ).empty() &&
    display.askForSave() ==  AskForSaveDialog::CANCEL ) return;

  // retrieve parent and grandparent of current display
  QWidget* parent( display.parentWidget() );    
  QSplitter* parent_splitter( dynamic_cast<QSplitter*>( parent ) );
  
  // retrieve displays associated to current
  BASE::KeySet<TextDisplay> displays( &display );
  
  // check if display is a new document
  // remove its filename from server if needed
  if( display.isNewDocument() && displays.empty() )
  { TextDisplay::newDocumentNameServer().remove( display.file() ); }

  // check how many children remain in parent_splitter if any
  // take action if it is less than 2 (the current one to be deleted, and another one)
  if( parent_splitter && parent_splitter->count() == 2 ) 
  {
    
    // retrieve child that is not the current editor
    // need to loop over existing widgets because the editor above has not been deleted yet
    QWidget* child(0);
    for( int index = 0; index < parent_splitter->count(); index++ )
    { 
      if( parent_splitter->widget( index ) != &display ) 
      {
        child = parent_splitter->widget( index );
        break;
      }
    }    
    assert( child );
    Debug::Throw( "TextView::closeDisplay - found child.\n" );
    
    // retrieve splitter parent
    QWidget* grand_parent( parent_splitter->parentWidget() );
    
    // try cast to a splitter
    QSplitter* grand_parent_splitter( dynamic_cast<QSplitter*>( grand_parent ) );
    
    // move child to grand_parent_splitter if any
    if( grand_parent_splitter )
    {  grand_parent_splitter->insertWidget( grand_parent_splitter->indexOf( parent_splitter ), child ); }
    else
    {
      child->setParent( grand_parent );
      grand_parent->layout()->addWidget( child );
    }
    
    // delete parent_splitter, now that it is empty
    // delete parent_splitter;
    parent_splitter->deleteLater();
    
  } else {
    
    // the editor is deleted only if its parent splitter is not
    // otherwise this will trigger double deletion of the editor 
    // which will then crash
    display.deleteLater();
    
  }
  
  // if no associated displays, retrieve all, set the first as active
  if( displays.empty() ) displays = BASE::KeySet<TextDisplay>( this );
  for( BASE::KeySet<TextDisplay>::reverse_iterator iter = displays.rbegin(); iter != displays.rend(); iter++ )
  { 
    //Debug::Throw(0, "TextView::closeDisplay - changing focus.\n" );
    if( (*iter) != &display ) {
      //Debug::Throw(0, "TextView::closeDisplay - found active display.\n" );
      setActiveDisplay( **iter );
      activeDisplay().setFocus();
      break;
    }
  }  
    
  Debug::Throw( "TextView::closeDisplay - done.\n" );
  
}

//___________________________________________________________
TextDisplay& TextView::splitDisplay( const Qt::Orientation& orientation, const bool& clone )
{
  
  Debug::Throw( "TextView::splitDisplay.\n" );

  // keep local pointer to current active display
  TextDisplay& active_display_local( activeDisplay() );  
  
  // compute desired dimension of the new splitter
  // along its splitting direction
  int dimension(0);
  if( clone ) dimension = (orientation == Qt::Horizontal) ? active_display_local.width():active_display_local.height();
  else dimension = (orientation == Qt::Horizontal) ? width():height();

  // create new splitter
  QSplitter& splitter( _newSplitter( orientation, clone ) );
  
  // create new display
  TextDisplay& display( _newTextDisplay(0) );
  
  // insert in splitter, at correct position
  if( clone ) splitter.insertWidget( splitter.indexOf( &active_display_local )+1, &display );
  else splitter.addWidget( &display );

  // recompute dimension
  // take the max of active display and splitter,
  // in case no new splitter was created.
  dimension = max( dimension, (orientation == Qt::Horizontal) ? splitter.width():splitter.height() );
  
  // assign equal size to all splitter children
  QList<int> sizes;
  for( int i=0; i<splitter.count(); i++ )
  { sizes.push_back( dimension/splitter.count() ); }
  splitter.setSizes( sizes );

  // synchronize both displays, if cloned
  if( clone )
  {

    /*
     if there exists no clone of active display,
     backup text and register a new Sync object
    */
    BASE::KeySet<TextDisplay> displays( &active_display_local );

    // clone new display
    display.synchronize( &active_display_local );
    
    // perform associations
    // check if active displays has associates and propagate to new
    for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
    { BASE::Key::associate( &display, *iter ); }

    // associate this display to AutoSave threads
    BASE::KeySet<AutoSaveThread> threads( &active_display_local );
    for( BASE::KeySet<AutoSaveThread>::iterator iter = threads.begin(); iter != threads.end(); iter++ )
    { BASE::Key::associate( &display, *iter ); }
 
    // associate new display to active
    BASE::Key::associate( &display, &active_display_local );
    emit displayCountChanged();

  } else {

    // register new AutoSave thread
    static_cast<Application*>(qApp)->autoSave().newThread( &display );

  }

  return display;

}

//____________________________________________
void TextView::saveAll( void )
{
  Debug::Throw( "TextView::saveAll.\n" );

  // retrieve all displays
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { if( (*iter)->document()->isModified() ) (*iter)->save(); }

  return;

}


//____________________________________________
void TextView::ignoreAll( void )
{
  Debug::Throw( "TextView::ignoreAll.\n" );

  // retrieve all displays
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  { (*iter)->setModified( false ); }

  return;

}

//________________________________________________________________
void TextView::selectClassName( QString name )
{
  Debug::Throw( "TextView::SelectClassName.\n" );

  // retrieve all displays matching active
  // and update class name
  BASE::KeySet<TextDisplay> displays( &activeDisplay() );
  displays.insert( &activeDisplay() );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    (*iter)->setClassName( name );
    (*iter)->updateDocumentClass();
  }
  
  // rehighlight
  activeDisplay().rehighlight();

}

//________________________________________________________________
void TextView::rehighlight( void )
{
  Debug::Throw( "TextView::rehighlight.\n" );

  // retrieve associated TextDisplay
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {
    // this trick allow to run the rehighlight only once per set of associated displays
    if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) == iter ) (*iter)->rehighlight();
  }

  return;
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
    QtUtil::infoDialog( this, "Too many files opened. Diff canceled." );
    return;
  }

  if( n_displays < 2 )
  {
    QtUtil::infoDialog( this, "Too few files opened. Diff canceled." );
    return;
  }

  // create diff object
  Diff* diff = new Diff( this );

  // store active display as first to compare
  TextDisplay& first = activeDisplay();
  
  // retrieve displays associated to window
  // look for the first one that is not associated to the active display
  BASE::KeySet<TextDisplay> displays( this );
  BASE::KeySet<TextDisplay>::iterator iter = displays.begin();
  for(; iter != displays.end(); iter++ )
  {
    if( !( *iter == &first || (*iter)->isAssociated( &first ) ) )
    {
      diff->setTextDisplays( first, *(*iter) );
      break;
    }
  }
  
  // check that one display was found
  assert( iter != displays.end() );
  
  // try run
  if( !diff->run() )
  {
    QtUtil::infoDialog( this, diff->error() );
    delete diff;
    return;
  }
  
  return;
  
}

//____________________________________________
void TextView::enterEvent( QEvent* e )
{

  Debug::Throw( "TextView::enterEvent.\n" );
  QWidget::enterEvent( e );

  // keep track of displays to be deleted, if any
  BASE::KeySet<TextDisplay> dead_displays;

  // retrieve displays
  BASE::KeySet<TextDisplay> displays( this );
  for( BASE::KeySet<TextDisplay>::iterator iter = displays.begin(); iter != displays.end(); iter++ )
  {

    // this trick allow to run only once per set of displays associated to the same file
    if( std::find_if( displays.begin(), iter, BASE::Key::IsAssociatedFTor( *iter ) ) != iter ) continue;
    
    // keep local reference of current display
    TextDisplay &display( **iter );
    
    // check file
    if( display.checkFileRemoved() == FileRemovedDialog::CLOSE ) 
    { 
        
      // register displays as dead
      BASE::KeySet<TextDisplay> associated_displays( &display );
      for( BASE::KeySet<TextDisplay>::iterator display_iter = associated_displays.begin(); display_iter != associated_displays.end(); display_iter++ )
      { dead_displays.insert( *display_iter ); }
      dead_displays.insert( &display );
            
    } else {
 
      (*iter)->checkFileReadOnly();
      (*iter)->checkFileModified();

    }
    
  }
  
  // delete dead_displays
  if( !dead_displays.empty() )
  {

    Debug::Throw() << "TextView::enterEvent - dead displays: " << dead_displays.size() << endl;
    for( BASE::KeySet<TextDisplay>::iterator iter = dead_displays.begin(); iter != dead_displays.end(); iter++ )
    { closeDisplay( **iter ); }

  }

  // update window title
  emit needUpdate( TextDisplay::WINDOW_TITLE );

  Debug::Throw( "TextView::enterEvent - done.\n" );

}

//____________________________________________
void TextView::_checkDisplays( void )
{

  Debug::Throw() << "TextView::_checkDisplays" << endl;
  BASE::KeySet<TextDisplay> displays( this );
  if( displays.empty() ) close();
  
}

//____________________________________________
void TextView::_activeDisplayChanged( TextEditor* editor )
{
  Debug::Throw() << "TextView::_activeDisplayChanged - " << editor->key() << endl;
  setActiveDisplay( *static_cast<TextDisplay*>(editor) );  
}

//____________________________________________________________
QSplitter& TextView::_newSplitter( const Qt::Orientation& orientation, const bool& clone )
{

  Debug::Throw( "TextView::_newSplitter.\n" );
  QSplitter *splitter = 0;
  
  if( clone )
  {
    
    /* in clone mode, a new splitter is added as a parent of the activeDisplay.
    the new (cloned) TextDisplay will appear side by side with it */

    // retrieve parent of current display
    QWidget* parent( activeDisplay().parentWidget() );  

    // try catch to splitter
    // do not create a new splitter if the parent has same orientation
    QSplitter *parent_splitter( dynamic_cast<QSplitter*>( parent ) );
    if( parent_splitter && parent_splitter->orientation() == orientation ) splitter = parent_splitter;
    else {
      
 
      // move splitter to the first place if needed
      if( parent_splitter ) 
      {
        
        Debug::Throw( "TextView::_newSplitter - found parent splitter.\n" );
        // create a splitter with correct orientation
        // give him no parent, because the parent is set in QSplitter::insertWidget()
        splitter = new LocalSplitter(0);
        splitter->setOrientation( orientation );
        parent_splitter->insertWidget( parent_splitter->indexOf( &activeDisplay() ), splitter );
        
      } else {
        
        // create a splitter with correct orientation
        splitter = new LocalSplitter(parent);
        splitter->setOrientation( orientation );
        parent->layout()->addWidget( splitter );
        
      }
      
      // reparent current display
      splitter->addWidget( &activeDisplay() );
      
      // resize parent splitter if any
      if( parent_splitter )
      {
        int dimension = ( parent_splitter->orientation() == Qt::Horizontal) ? 
          parent_splitter->width():
          parent_splitter->height();
    
        QList<int> sizes;
        for( int i=0; i<parent_splitter->count(); i++ )
        { sizes.push_back( dimension/parent_splitter->count() ); }
        parent_splitter->setSizes( sizes );
      
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
    const QObjectList& children( TextView::children() );
    for( QObjectList::const_iterator iter = children.begin(); iter != children.end() && !child; iter++ )
    { child = dynamic_cast<QWidget*>( *iter ); }

    // check child could be retrieved
    assert( child );

    // try cast child to splitter
    // if exists and have same orientation, do not create a new one
    QSplitter* child_splitter( dynamic_cast<QSplitter*>( child ) );
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
  connect( display, SIGNAL( needUpdate( unsigned int ) ), SIGNAL( needUpdate( unsigned int ) ) );
  connect( display, SIGNAL( hasFocus( TextEditor* ) ), SLOT( _activeDisplayChanged( TextEditor* ) ) );
  connect( display, SIGNAL( cursorPositionChanged() ), &position_timer_, SLOT( start() ) );
  connect( display, SIGNAL( overwriteModeChanged() ), SIGNAL( overwriteModeChanged() ) );
  
  connect( display, SIGNAL( undoAvailable( bool ) ), SIGNAL( undoAvailable( bool ) ) );
  connect( display, SIGNAL( redoAvailable( bool ) ), SIGNAL( redoAvailable( bool ) ) );

  connect( display, SIGNAL( destroyed( void ) ), SLOT( _checkDisplays( void ) ) );  
  connect( display, SIGNAL( destroyed( void ) ), SIGNAL( displayCountChanged( void ) ) );  
  
  // retrieve parent main window
  MainWindow &window = *static_cast<MainWindow*>( TextView::window() );
  
  // customize display actions
  /* this is needed to be able to handle a single dialog for stacked windows */
  display->gotoLineAction().disconnect();
  connect( &display->gotoLineAction(), SIGNAL( triggered() ), &window, SLOT( selectLineFromDialog() ) );
  
  display->findAction().disconnect();
  connect( &display->findAction(), SIGNAL( triggered() ), &window, SLOT( findFromDialog() ) );
  connect( display, SIGNAL( noMatchFound() ), &window, SIGNAL( noMatchFound() ) );
  connect( display, SIGNAL( matchFound() ), &window, SIGNAL( matchFound() ) );
  
  display->replaceAction().disconnect();
  connect( &display->replaceAction(), SIGNAL( triggered() ), &window, SLOT( replaceFromDialog() ) );
  
  // associate display to this editFrame
  BASE::Key::associate( this, display );
  
  // update current display and focus
  setActiveDisplay( *display );
  display->setFocus();
  Debug::Throw() << "TextView::_newTextDisplay - key: " << display->key() << endl;
  Debug::Throw( "TextView::_newTextDisplay - done.\n" );
    
  return *display;
  
}
