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
\file DocumentClassManagerDialog.cpp
\brief list document classes
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QMenu>

#include "CustomToolBar.h"
#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassIcons.h"
#include "DocumentClassManagerDialog.h"
#include "DocumentClassModifiedDialog.h"
#include "DocumentClassDialog.h"
#include "FileDialog.h"
#include "IconEngine.h"
#include "InformationDialog.h"
#include "Options.h"
#include "PixmapEngine.h"
#include "QuestionDialog.h"
#include "StatusBar.h"
#include "TreeView.h"

using namespace std;

//______________________________________________________________
DocumentClassManagerDialog::DocumentClassManagerDialog( QWidget* parent, const DocumentClassManager& manager ):
    BaseMainWindow( parent ),
    Counter( "DocumentClassManagerDialog" ),
    backup_( manager ),
    modified_( false )
{
    Debug::Throw( "DocumentClassManagerDialog::DocumentClassManagerDialog.\n" );

    setOptionName( "DOCUMENT_CLASS_MANAGER_DIALOG" );

    // actions
    _installActions();

    // toolbar
    _installToolBars();

    // menu
    _installMenuBar();

    StatusBar *statusbar = new StatusBar( this );
    setStatusBar( statusbar );
    statusbar->addLabel(1);
    statusbar->addClock();

    // main widget
    QWidget* main = new QWidget( this );
    main->setLayout( new QVBoxLayout() );
    main->layout()->setMargin(2);
    main->layout()->setSpacing(5);
    setCentralWidget( main );

    // create list
    main->layout()->addWidget( list_ = new TreeView( main ) );
    _list().setModel( &_model() );
    _list().setOptionName( "DOCUMENT_CLASS_MANAGER_LIST" );

    // set connections
    connect( &_list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _edit() ) );
    connect( _list().selectionModel(), SIGNAL( selectionChanged( const QItemSelection& , const QItemSelection& ) ), SLOT( _updateActions() ) );

    _loadClasses( manager );
    _updateActions();

    _updateWindowTitle();
    adjustSize();

}

//___________________________________________________
void DocumentClassManagerDialog::closeEvent( QCloseEvent* event )
{

    Debug::Throw( "DocumentClassManager::closeEvent.\n" );
    if( _modified() )
    {
        switch( DocumentClassModifiedDialog( this ).centerOnParent().exec() )
        {
            case DocumentClassModifiedDialog::YES:
            {
                QStringList warnings( _saveDocumentClasses() );
                if( warnings.empty() ) break;

                QString buffer;
                QTextStream what( &buffer );
                if( warnings.size() > 1 ) what << "The following errors have been encountered while saving: " << endl;
                else what << "The following error has been encountered while saving: " << endl;

                for( QStringList::const_iterator iter = warnings.begin(); iter != warnings.end(); ++iter )
                { what << *iter << endl; }

                what << "Exit anyway ?";

                if( !QuestionDialog( this, buffer ).setWindowTitle( "Save Document Classes - Qedit" ).centerOnParent().exec() )
                { event->ignore(); }

                break;

            }

            case DocumentClassModifiedDialog::CANCEL:
            event->ignore();
            break;

            case DocumentClassModifiedDialog::NO:
            default:
            break;
        }
    }

    return;

}

//___________________________________________________
void DocumentClassManagerDialog::_updateActions( void )
{

    Debug::Throw( "DocumentClassManagerDialog::_updateActions.\n" );
    bool hasSelection( !_list().selectionModel()->selectedRows().empty() );
    _editAction().setEnabled( hasSelection );
    _removeAction().setEnabled( hasSelection );

}

//___________________________________________________
void DocumentClassManagerDialog::_add( void )
{

    Debug::Throw( "DocumentClassManagerDialog::_add.\n" );

    // create dialog
    DocumentClassDialog dialog( this );
    if( dialog.exec() == QDialog::Accepted )
    {

        DocumentClass new_document_class( dialog.documentClass() );
        new_document_class.setModified( true );
        _model().add( new_document_class );
        _list().resizeColumns();
        _checkModified();

    }

    return;
}

//___________________________________________________
void DocumentClassManagerDialog::_edit( void )
{
    Debug::Throw( "DocumentClassManagerDialog::_edit.\n" );
    QModelIndex current( _list().selectionModel()->currentIndex() );
    if( !current.isValid() )
    {
        InformationDialog( this, "No item selected. <Edit> canceled." ).exec();
        return;
    }

    // retrieve DocumentClass matching name
    const DocumentClass& document_class( _model().get( current ) );

    // create dialog
    DocumentClassDialog dialog( this );
    dialog.setDocumentClass( document_class );
    if( dialog.exec() == QDialog::Accepted )
    {
        DocumentClass new_document_class( dialog.documentClass() );
        if( !( new_document_class == document_class ) )
        {
            // replace class in model
            new_document_class.setModified( true );
            _model().replace( current, new_document_class );
            _list().resizeColumns();

            _checkModified();

        }

    }

    return;
}

//___________________________________________________
void DocumentClassManagerDialog::_remove( void )
{
    Debug::Throw( "DocumentClassManagerDialog::_remove.\n" );
    QModelIndex current( _list().selectionModel()->currentIndex() );
    if( !current.isValid() )
    {
        InformationDialog( this, "No item selected. <Remove> canceled." ).exec();
        return;
    }

    // remove from model
    _model().remove( _model().get( current ) );
    _checkModified();

}

//___________________________________________________
void DocumentClassManagerDialog::_loadFile( void )
{
    Debug::Throw( "DocumentClassManagerDialog::_loadFile.\n" );

    // get file from dialog
    File file( FileDialog( this ).getFile() );
    if( file.isNull() ) return;

    // try load from file manager and add to options
    DocumentClassManager manager;
    if( manager.read( file ) )
    {
        _loadClasses( manager );
        _checkModified();
    }

    return;

}

//___________________________________________________
void DocumentClassManagerDialog::_save( void )
{

    Debug::Throw( "DocumentClassManagerDialog::_save.\n" );

    QStringList warnings( _saveDocumentClasses() );

    // display warnings
    if( !warnings.empty() )
    {
        QString buffer;
        QTextStream what( &buffer );
        if( warnings.size() > 1 ) what << "The following errors have been encountered while saving: " << endl;
        else what << "The following error has been encountered while saving: " << endl;

        for( QStringList::const_iterator iter = warnings.begin(); iter != warnings.end(); ++iter )
        { what << *iter << endl; }

        InformationDialog( this, buffer ).setWindowTitle( "Save Document Classes - qedit" ).centerOnParent().exec();

    }

    return;

}

//___________________________________________________
void DocumentClassManagerDialog::_reload( void )
{

    Debug::Throw( "DocumentClassManagerDialog::_reload" );

    // check modifications
    if( _modified() && !QuestionDialog( this, "Discard changes to document classses ?" ).setWindowTitle( "Reload Document Classes - qedit" ).exec() ) return;

    _model().update( DocumentClassModel::List( backup_.classes().begin(), backup_.classes().end() ) );
    _setModified( false );

}

//________________________________________________________
void DocumentClassManagerDialog::_installActions( void )
{

    Debug::Throw( "DocumentClassManagerDialog::_installActions" );

    // clos action
    addAction( closeAction_ = new QAction( IconEngine::get( ICONS::EXIT ), "&Exit", this  ) );
    connect( closeAction_, SIGNAL( triggered() ), SLOT( close() ) );
    closeAction_->setToolTip( "Exit document class edition" );
    closeAction_->setShortcut( Qt::CTRL + Qt::Key_Q );

    // new document class
    addAction( newAction_ = new QAction( IconEngine::get( ICONS::NEW ), "&New", this  ) );
    connect( newAction_, SIGNAL( triggered() ), SLOT( _add() ) );
    newAction_->setToolTip( "Create new document class" );
    newAction_->setShortcut( Qt::CTRL + Qt::Key_N );

    // open
    addAction( openAction_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open", this  ) );
    connect( openAction_, SIGNAL( triggered() ), SLOT( _loadFile() ) );
    openAction_->setToolTip( "Load additional classes from file" );
    openAction_->setShortcut( Qt::CTRL + Qt::Key_O );

    // save
    addAction( saveAction_ = new QAction( IconEngine::get( ICONS::SAVE ), "Save", this  ) );
    connect( saveAction_, SIGNAL( triggered() ), SLOT( _save() ) );
    saveAction_->setToolTip( "Save document classes modifications" );
    saveAction_->setShortcut( Qt::CTRL + Qt::Key_S );
    saveAction_->setEnabled( false );

    // edit
    addAction( editAction_ = new QAction( IconEngine::get( ICONS::EDIT ), "&Edit", this  ) );
    connect( editAction_, SIGNAL( triggered() ), SLOT( _edit() ) );
    editAction_->setToolTip( "Edit file from which selected document class is read" );

    // remove
    addAction( removeAction_ = new QAction( IconEngine::get( ICONS::REMOVE ), "&Remove", this  ) );
    connect( removeAction_, SIGNAL( triggered() ), SLOT( _remove() ) );
    removeAction_->setToolTip( "Remove selected document class from list" );

    // reload
    addAction( reloadAction_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Rel&oad", this  ) );
    connect( reloadAction_, SIGNAL( triggered() ), SLOT( _reload() ) );
    reloadAction_->setToolTip( "Reload all classes" );
    reloadAction_->setShortcut( Qt::Key_F5 );
    reloadAction_->setEnabled( false );


}

//___________________________________________________
void DocumentClassManagerDialog::_installMenuBar( void )
{

    Debug::Throw( "DocumentClassManagerDialog::_installMenuBar.\n" );
    QMenuBar* menubar = new QMenuBar( this );

    QMenu* menu = menubar->addMenu( "&File" );
    menu->addAction( &_closeAction() );

    setMenuBar( menubar );

}

//___________________________________________________
void DocumentClassManagerDialog::_installToolBars( void )
{

    Debug::Throw( "DocumentClassManagerDialog::_installToolBars.\n" );
    CustomToolBar* toolbar = new CustomToolBar( "main", this, "DOCUMENT_CLASS_MANAGER_TOOLBAR" );
    toolbar->addAction( &_newAction() );
    toolbar->addAction( &_openAction() );
    toolbar->addAction( &_saveAction() );
    toolbar->addAction( &_editAction() );
    toolbar->addAction( &_removeAction() );
    toolbar->addAction( &_reloadAction() );

}

//___________________________________________________
void DocumentClassManagerDialog::_loadClasses( const DocumentClassManager& manager )
{

    Debug::Throw( "DocumentClassManagerDialog::_loadClasses.\n" );

    // retrieve classes from DocumentClass manager
    const DocumentClassManager::List& classes( manager.classes() );
    Debug::Throw(0) << "DocumentClassManagerDialog::_loadClasses - lists: " << classes.size() << endl;

    // add to list
    _model().add( DocumentClassModel::List( classes.begin(), classes.end() ) );
    _list().resizeColumns();

}

//___________________________________________________
void DocumentClassManagerDialog::_checkModified( void )
{

    const DocumentClassModel::List& classes( _model().get() );
    _setModified( !(
        find_if( classes.begin(), classes.end(), DocumentClass::ModifiedFTor() ) == classes.end() &&
        DocumentClassManager::List( classes.begin(), classes.end() ) == backup_.classes() ) );

}

//___________________________________________________
void DocumentClassManagerDialog::_setModified( bool value )
{

    if( modified_ == value ) return;

    modified_ = value;
    _saveAction().setEnabled( modified_ );
    _reloadAction().setEnabled( modified_ );
    _updateWindowTitle();
}

//___________________________________________________
void DocumentClassManagerDialog::_updateWindowTitle( void )
{
    QString title;
    QTextStream what( &title );
    what << "Document Classes - qedit";
    if( _modified() ) what << " (modified)";
    setWindowTitle( title );
}

//_____________________________________________________________
QStringList DocumentClassManagerDialog::_saveDocumentClasses( void )
{

    Debug::Throw( "DocumentClassManagerDialog::_saveDocumentClasses.\n" );
    assert( _modified() );

    // write all classes to disc
    QStringList warnings;
    DocumentClassModel::List classes( _model().get() );
    for( DocumentClassModel::List::iterator iter = classes.begin(); iter != classes.end(); ++iter )
    {

        DocumentClass& document_class( *iter );
        if( !document_class.modified() ) continue;
        if( !document_class.file().isWritable() )
        {

            QString what;
            QTextStream( &what ) << "Cannot write document class named " << document_class.name() << " to file " << document_class.file();
            warnings.push_back( what );

        } else if( !DocumentClassManager().write( document_class, document_class.file() ) ) {

            QString what;
            QTextStream( &what ) << "Failed to write document class named " << document_class.name() << " to file " << document_class.file();
            warnings.push_back( what );

        } else {

            document_class.setModified( false );
            _model().add( document_class );

        }

    }

    // update DocumentClassManager
    backup_.setClasses( DocumentClassManager::List( classes.begin(), classes.end() ) );

    // write fileNames to options
    XmlOptions::get().clearSpecialOptions( "PATTERN_FILENAME" );
    for( DocumentClassModel::List::iterator iter = classes.begin(); iter != classes.end(); ++iter )
    { XmlOptions::get().add( "PATTERN_FILENAME", iter->file() ); }

    // check modifications
    _checkModified();

    return warnings;
}
