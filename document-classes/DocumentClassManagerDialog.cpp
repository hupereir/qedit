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
* ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA   02111-1307 USA
*
*******************************************************************************/

#include "DocumentClassManagerDialog.h"

#include "BaseIcons.h"
#include "DocumentClassManager.h"
#include "DocumentClass.h"
#include "FileDialog.h"
#include "IconEngine.h"
#include "TreeView.h"
#include "XmlOptions.h"

#include <QtGui/QHeaderView>
#include <QtGui/QLayout>

//______________________________________________________________________
DocumentClassManagerDialog::DocumentClassManagerDialog( QWidget* parent ):
    CustomDialog( parent, OkButton|CancelButton )
{

    Debug::Throw( "DocumentClassManagerDialog::DocumentClassManagerDialog.\n" );
    setOptionName( "DOCUMENT_CLASS_MANAGER_DIALOG" );

    // customize layout
    layout()->setMargin(0);
    buttonLayout().setMargin(5);

    QHBoxLayout* hLayout( new QHBoxLayout() );
    hLayout->setSpacing(5);
    hLayout->setMargin(0);
    mainLayout().addLayout( hLayout );

    // setup list
    hLayout->addWidget( list_ = new TreeView( this ) );
    list_->setItemMargin( 2 );
    list_->setModel( &model_ );
    list_->setOptionName( "DOCUMENT_CLASS_MANAGER_LIST" );

    // buttons
    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setSpacing(5);
    vLayout->setMargin(0);
    hLayout->addLayout( vLayout );

    QPushButton* button;
    vLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::ADD ), "Add", this ) );
    connect( button, SIGNAL( clicked( void ) ), SLOT( _add( void ) ) );

    vLayout->addStretch( 1 );

    _read();

}

//______________________________________________________________________
void DocumentClassManagerDialog::_read( void )
{

    DocumentClassManager classManager;
    foreach( const Option& option, XmlOptions::get().specialOptions( "PATTERN_FILENAME" ) )
    { classManager.read( QString( option.raw() ) ); }

    // read build-in patterns
    foreach( const Option& option, XmlOptions::get().specialOptions( "DEFAULT_PATTERN_FILENAME" ) )
    { classManager.read( QString( option.raw() ) ); }

    // update model
    model_.set( classManager.classes() );

}

//______________________________________________________________________
void DocumentClassManagerDialog::_add( void )
{

    Debug::Throw( "DocumentClassManagerDialog::_add.\n" );
    FileDialog dialog( this );
    dialog.setAcceptMode( QFileDialog::AcceptOpen );
    dialog.setFileMode( QFileDialog::ExistingFile );

    QString file( dialog.getFile() );
    if( file.isNull() ) return;

    DocumentClassManager classManager;
    classManager.read( file );
    model_.add( classManager.classes() );

}
