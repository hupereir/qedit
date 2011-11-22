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
  \file RemoveFilesDialog.cpp
  \brief QDialog used to add selected files to the repository
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QLayout>

#include "BaseIcons.h"
#include "IconEngine.h"
#include "RemoveFilesDialog.h"
#include "TreeView.h"
#include "XmlOptions.h"



//____________________________________________________________________________
RemoveFilesDialog::RemoveFilesDialog( QWidget* parent, const FileSystemModel::List& files ):
    CustomDialog( parent )
{

    Debug::Throw( "RemoveFilesDialog::RemoveFilesDialog.\n" );

    // label
    QLabel* text_label( new QLabel( "Remove files selected from following list ?", this ) );

    //! try load Question icon
    QPixmap question_pixmap( PixmapEngine::get( ICONS::WARNING ) );
    QHBoxLayout *h_layout( new QHBoxLayout() );
    h_layout->setSpacing(10);
    h_layout->setMargin(0);
    mainLayout().addLayout( h_layout );
    QLabel* label = new QLabel( this );
    label->setPixmap( question_pixmap );
    h_layout->addWidget( label, 0, Qt::AlignHCenter );
    h_layout->addWidget( text_label, 1, Qt::AlignLeft );

    // horizontal layout for list and comments
    h_layout = new QHBoxLayout();
    h_layout->setSpacing( 5 );
    h_layout->setMargin( 0 );
    mainLayout().addLayout( h_layout, 1 );

    // file list
    QVBoxLayout* v_layout = new QVBoxLayout();
    v_layout->setSpacing( 5 );
    v_layout->setMargin( 0 );
    h_layout->addLayout( v_layout );

    v_layout->addWidget( list_ = new TreeView( this ), 1 );
    _list().setSelectionMode( QAbstractItemView::MultiSelection );

    model_.add( files );
    model_.sort( FileSystemModel::FILE, Qt::AscendingOrder );

    _list().setModel( &model_ );
    _list().setMask( XmlOptions::get().get<int>( "FILE_SYSTEM_LIST_MASK" ) );
    _list().resizeColumnToContents( FileSystemModel::FILE );
    _list().selectAll();

    // vertical layout for selection buttons and comments
    v_layout = new QVBoxLayout();
    v_layout->setSpacing( 5 );
    v_layout->setMargin( 0 );
    h_layout->addLayout( v_layout );

    // select all
    QPushButton* button;
    v_layout->addWidget( button = new QPushButton( "&Select All", this ) );
    button->setToolTip( "Select all files in list" );
    connect( button, SIGNAL( clicked() ), &_list(), SLOT( selectAll() ) );

    // deselect all
    v_layout->addWidget( clearSelectionButton_ = new QPushButton( "&Clear Selection", this ) );
    clearSelectionButton_->setToolTip( "Deselect all files in list" );
    connect( clearSelectionButton_, SIGNAL( clicked() ), _list().selectionModel(), SLOT( clear() ) );

    v_layout->addWidget( recursiveCheckBox_ = new QCheckBox( "Remove recusively", this ) );
    recursiveCheckBox_->setToolTip( "Remove directories recursively" );

    v_layout->addStretch(1);

    // connection
    connect( _list().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateButtons() ) );
    _updateButtons();

}

//____________________________________________________________________________
FileSystemModel::List RemoveFilesDialog::selectedFiles( void ) const
{ return _model().get( _list().selectionModel()->selectedRows() ); }

//____________________________________________________________________
void RemoveFilesDialog::_updateButtons( void )
{
    Debug::Throw( "RemoveFilesDialog::_updateButtons.\n" );
    bool has_selection( !_list().selectionModel()->selectedRows().empty() );
    clearSelectionButton_->setEnabled( has_selection );
    okButton().setEnabled( has_selection );
}
