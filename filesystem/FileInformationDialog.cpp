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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
\file FileInformationDialog.cpp
\brief  file informations
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "AnimatedTabWidget.h"
#include "BaseIcons.h"
#include "CustomPixmap.h"
#include "GridLayout.h"
#include "Debug.h"
#include "ElidedLabel.h"
#include "FileList.h"
#include "FileRecord.h"
#include "IconEngine.h"
#include "FileInformationDialog.h"
#include "FileRecordProperties.h"
#include "FileSystemModel.h"
#include "QtUtil.h"
#include "TimeStamp.h"

#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>

//_________________________________________________________
FileInformationDialog::FileInformationDialog( QWidget* parent, const FileRecord& record ):
    CustomDialog( parent, CloseButton )
{
    Debug::Throw( "FileInformationDialog::FileInformationDialog.\n" );

    // file name
    const File& file( record.file() );
    setWindowTitle( (file.isEmpty() ? File("File Information"):file.localName() )+ " - Qedit" );
    setOptionName( "FILE_INFORMATION_DIALOG" );

    // customize layout
    layout()->setMargin(0);
    layout()->setSpacing(0);
    buttonLayout().setMargin(5);

    tabWidget_ = new AnimatedTabWidget( this );
    mainLayout().addWidget( &tabWidget() );

    // general information
    QWidget *box;
    tabWidget().addTab( box = new QWidget(), "General" );

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setMargin(5);
    hLayout->setSpacing(5);
    box->setLayout( hLayout );

    // try load Question icon
    CustomPixmap pixmap = CustomPixmap().find( ICONS::INFORMATION );

    if( !pixmap.isNull() )
    {
        QLabel* label = new QLabel(box);
        label->setPixmap( pixmap );
        hLayout->addWidget( label, 0, Qt::AlignTop );
    }

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing( 5 );
    hLayout->addLayout( layout, 1 );

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setSpacing( 5 );
    gridLayout->setMaxCount( 2 );
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    layout->addLayout( gridLayout );

    QLabel* label;
    gridLayout->addWidget( label = new QLabel( "File name: ", box ) );
    gridLayout->addWidget( label = new ElidedLabel( file.isEmpty() ? File("Untitled"):file.localName(), box ) );
    QFont font( label->font() );
    font.setWeight( QFont::Bold );
    label->setFont( font );
    label->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );

    // path
    if( !file.isEmpty() )
    {
        gridLayout->addWidget( label = new QLabel( "Path: ", box ) );
        gridLayout->addWidget( label = new ElidedLabel( file.path(), box ) );
        label->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );
    }

    // type
    if( record.hasFlag( FileSystemModel::FOLDER | FileSystemModel::DOCUMENT ) )
    {
        gridLayout->addWidget( label = new QLabel( "Type: ", box ) );
        QString buffer;
        QTextStream what( &buffer );
        if( record.hasFlag( FileSystemModel::LINK ) ) what << "Link";
        if( record.hasFlag( FileSystemModel::FOLDER ) ) what << "Folder";
        if( record.hasFlag( FileSystemModel::DOCUMENT ) ) what << "Document";
        gridLayout->addWidget( label = new QLabel( buffer, box ) );

    }

    // size
    gridLayout->addWidget( label = new QLabel( "Size: ", box ) );
    gridLayout->addWidget( label = new QLabel( file.exists() ? file.sizeString(): "0", box ) );

    //  created
    gridLayout->addWidget( label = new QLabel( "Created: ", box ) );
    gridLayout->addWidget( label = new QLabel( file.exists() ? TimeStamp( file.created() ).toString():"unknown", box ) );

    // last accessed
    gridLayout->addWidget( label = new QLabel( "Accessed: ", box ) );
    gridLayout->addWidget( label = new QLabel( file.exists() ? TimeStamp( file.lastAccessed() ).toString():"never", box ) );

    // last modification
    gridLayout->addWidget( label = new QLabel( "Modified: ", box ) );
    gridLayout->addWidget( label = new QLabel( file.exists() ? TimeStamp( file.lastModified() ).toString():"never", box ) );

    // document class
    if( record.hasProperty( FileRecordProperties::CLASS_NAME ) )
    {
        gridLayout->addWidget( label = new QLabel( "Class: ", box ) );
        gridLayout->addWidget( label = new QLabel( record.property( FileRecordProperties::CLASS_NAME ), box ) );
    }

    if( record.hasProperty( FileRecordProperties::DICTIONARY ) )
    {
        gridLayout->addWidget( label = new QLabel( "Spell-check dictionary: ", box ) );
        gridLayout->addWidget( label = new QLabel( record.property( FileRecordProperties::DICTIONARY ), box ) );
    }

    if( record.hasProperty( FileRecordProperties::FILTER ) )
    {
        gridLayout->addWidget( label = new QLabel( "Spell-check filter: ", box ) );
        gridLayout->addWidget( label = new QLabel( record.property( FileRecordProperties::FILTER ), box ) );
    }

    gridLayout->setColumnStretch( 1, 1 );
    layout->addStretch( 1 );

    // permissions tab
    tabWidget().addTab( box = new QWidget(), "Permissions" );
    layout = new QVBoxLayout();
    layout->setMargin(5);
    layout->setSpacing( 5 );
    box->setLayout( layout );

    gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing( 5 );
    gridLayout->setMaxCount( 4 );
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    layout->addItem( gridLayout );

    gridLayout->addWidget( new QLabel( "<b>Permissions: </b>", box ) );
    gridLayout->addWidget( new QLabel( "Read", box ), Qt::AlignHCenter );
    gridLayout->addWidget( new QLabel( "Write", box ), Qt::AlignHCenter );
    gridLayout->addWidget( new QLabel( "Execute", box ), Qt::AlignHCenter );

    typedef std::map< QFile::Permission, QCheckBox* > CheckBoxMap;
    CheckBoxMap checkboxes;

    gridLayout->addWidget( new QLabel( "Owner: ", box ) );
    gridLayout->addWidget( checkboxes[QFile::ReadOwner ]  = new QCheckBox( box ), Qt::AlignHCenter );
    gridLayout->addWidget( checkboxes[QFile::WriteOwner ]  = new QCheckBox( box ), Qt::AlignHCenter );
    gridLayout->addWidget( checkboxes[QFile::ExeOwner  ]  = new QCheckBox( box ), Qt::AlignHCenter );

    // on unix, right now, Qt does not return the current user permissions. Disable them from the dialog
    #if !defined(Q_WS_X11)
    gridLayout->addWidget( new QLabel( "User: ", box ) );
    gridLayout->addWidget( checkboxes[QFile::ReadUser ]  = new QCheckBox( box ), Qt::AlignHCenter );
    gridLayout->addWidget( checkboxes[QFile::WriteUser]  = new QCheckBox( box ), Qt::AlignHCenter );
    gridLayout->addWidget( checkboxes[QFile::ExeUser  ]  = new QCheckBox( box ), Qt::AlignHCenter );
    #endif

    gridLayout->addWidget( new QLabel( "Group: ", box ) );
    gridLayout->addWidget( checkboxes[QFile::ReadGroup  ] = new QCheckBox( box ), Qt::AlignHCenter );
    gridLayout->addWidget( checkboxes[QFile::WriteGroup ] = new QCheckBox( box ), Qt::AlignHCenter );
    gridLayout->addWidget( checkboxes[QFile::ExeGroup   ] = new QCheckBox( box ), Qt::AlignHCenter );

    gridLayout->addWidget( new QLabel( "Others: ", box ) );
    gridLayout->addWidget( checkboxes[QFile::ReadOther  ] = new QCheckBox( box ), Qt::AlignHCenter );
    gridLayout->addWidget( checkboxes[QFile::WriteOther ] = new QCheckBox( box ), Qt::AlignHCenter );
    gridLayout->addWidget( checkboxes[QFile::ExeOther   ] = new QCheckBox( box ), Qt::AlignHCenter );

    QFile::Permissions permissions( file.permissions() );
    for( CheckBoxMap::iterator iter = checkboxes.begin(); iter != checkboxes.end(); ++iter )
    {
        iter->second->setChecked( permissions & iter->first );
        iter->second->setEnabled( false );
    }

    // group and user id
    if( file.exists() )
    {

        layout->addWidget( new QLabel( "<b>Ownership: </b>", box ) );

        gridLayout = new GridLayout();
        gridLayout->setMargin(0);
        gridLayout->setSpacing( 5 );
        gridLayout->setMaxCount( 2 );
        gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
        layout->addItem( gridLayout );

        // user id
        gridLayout->addWidget( label = new QLabel( "User: ", box ) );
        gridLayout->addWidget( label = new QLabel( file.userName(), box ) );

        // group id
        gridLayout->addWidget( label = new QLabel( "Group: ", box ) );
        gridLayout->addWidget( label = new QLabel( file.groupName(), box ) );
        gridLayout->setColumnStretch( 1, 1 );

    }

    layout->addStretch(1);

    Debug::Throw( "FileInformationDialog::FileInformationDialog - Permissions tab filled.\n" );
    adjustSize();

}
