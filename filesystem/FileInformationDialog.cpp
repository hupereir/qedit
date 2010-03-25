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

#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

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

using namespace std;

//_________________________________________________________
FileInformationDialog::FileInformationDialog( QWidget* parent, const FileRecord& record ):
    CustomDialog( parent, OkButton )
{
    Debug::Throw( "FileInformationDialog::FileInformationDialog.\n" );

    // file name
    const File& file( record.file() );
    setWindowTitle( (file.isEmpty() ? File("File Information"):file.localName() )+ " - Qedit" );

    tab_widget_ = new AnimatedTabWidget( this );
    mainLayout().addWidget( &tabWidget() );
    Debug::Throw( "FileInformationDialog::FileInformationDialog - tabWidget booked.\n" );

    // box to display additinal information
    QWidget *box;
    tabWidget().addTab( box = new QWidget(), "&General" );
    Debug::Throw( "FileInformationDialog::FileInformationDialog - general tab created.\n" );

    QHBoxLayout* h_layout = new QHBoxLayout();
    h_layout->setMargin(5);
    h_layout->setSpacing(5);
    box->setLayout( h_layout );

    //! try load Question icon
    CustomPixmap pixmap = CustomPixmap().find( ICONS::INFORMATION );

    QLabel* label = new QLabel(box);
    label->setPixmap( pixmap );
    h_layout->addWidget( label, 0, Qt::AlignTop );

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing( 5 );
    h_layout->addLayout( layout, 1 );

    GridLayout* grid_layout = new GridLayout();
    grid_layout->setSpacing( 5 );
    grid_layout->setMaxCount( 2 );
    grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    layout->addLayout( grid_layout );

    grid_layout->addWidget( label = new QLabel( "File name: ", box ) );
    grid_layout->addWidget( label = new ElidedLabel( file.isEmpty() ? File("Untitled"):file.localName(), box ) );
    QFont font( label->font() );
    font.setWeight( QFont::Bold );
    label->setFont( font );
    label->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );


    // path
    if( !file.isEmpty() )
    {
        grid_layout->addWidget( label = new QLabel( "Path: ", box ) );
        grid_layout->addWidget( label = new ElidedLabel( file.path(), box ) );
        label->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );
    }

    // type
    if( record.hasFlag( FileSystemModel::FOLDER | FileSystemModel::DOCUMENT ) )
    {
        grid_layout->addWidget( label = new QLabel( "Type: ", box ) );
        QString buffer;
        QTextStream what( &buffer );
        if( record.hasFlag( FileSystemModel::LINK ) ) what << "Link";
        if( record.hasFlag( FileSystemModel::FOLDER ) ) what << "Folder";
        if( record.hasFlag( FileSystemModel::DOCUMENT ) ) what << "Document";
        grid_layout->addWidget( label = new QLabel( buffer, box ) );

    }

    // size
    grid_layout->addWidget( label = new QLabel( "Size: ", box ) );
    grid_layout->addWidget( label = new QLabel( file.exists() ? file.sizeString(): "0", box ) );

    //  created
    grid_layout->addWidget( label = new QLabel( "Created: ", box ) );
    grid_layout->addWidget( label = new QLabel( file.exists() ? TimeStamp( file.created() ).toString():"unknown", box ) );

    // last accessed
    grid_layout->addWidget( label = new QLabel( "Accessed: ", box ) );
    grid_layout->addWidget( label = new QLabel( file.exists() ? TimeStamp( file.lastAccessed() ).toString():"never", box ) );

    // last modification
    grid_layout->addWidget( label = new QLabel( "Modified: ", box ) );
    grid_layout->addWidget( label = new QLabel( file.exists() ? TimeStamp( file.lastModified() ).toString():"never", box ) );

    // document class
    if( record.hasProperty( FileRecordProperties::CLASS_NAME ) )
    {
        grid_layout->addWidget( label = new QLabel( "Class: ", box ) );
        grid_layout->addWidget( label = new QLabel( record.property( FileRecordProperties::CLASS_NAME ), box ) );
    }

    if( record.hasProperty( FileRecordProperties::DICTIONARY ) )
    {
        grid_layout->addWidget( label = new QLabel( "Spell-check dictionary: ", box ) );
        grid_layout->addWidget( label = new QLabel( record.property( FileRecordProperties::DICTIONARY ), box ) );
    }

    if( record.hasProperty( FileRecordProperties::FILTER ) )
    {
        grid_layout->addWidget( label = new QLabel( "Spell-check filter: ", box ) );
        grid_layout->addWidget( label = new QLabel( record.property( FileRecordProperties::FILTER ), box ) );
    }

    grid_layout->setColumnStretch( 1, 1 );

    layout->addStretch( 1 );
    Debug::Throw( "FileInformationDialog::FileInformationDialog - General tab filled.\n" );

    // permissions tab
    tabWidget().addTab( box = new QWidget(), "&Permissions" );
    layout = new QVBoxLayout();
    layout->setMargin(5);
    layout->setSpacing( 5 );
    box->setLayout( layout );
    Debug::Throw( "FileInformationDialog::FileInformationDialog - Permissions tab created.\n" );

    grid_layout = new GridLayout();
    grid_layout->setMargin(0);
    grid_layout->setSpacing( 5 );
    grid_layout->setMaxCount( 4 );
    grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    layout->addItem( grid_layout );

    grid_layout->addWidget( new QLabel( "<b>Permissions: </b>", box ) );
    grid_layout->addWidget( new QLabel( "Read", box ), Qt::AlignHCenter );
    grid_layout->addWidget( new QLabel( "Write", box ), Qt::AlignHCenter );
    grid_layout->addWidget( new QLabel( "Execute", box ), Qt::AlignHCenter );

    typedef std::map< QFile::Permission, QCheckBox* > CheckBoxMap;
    CheckBoxMap checkboxes;

    grid_layout->addWidget( new QLabel( "Owner: ", box ) );
    grid_layout->addWidget( checkboxes[QFile::ReadOwner ]  = new QCheckBox( box ), Qt::AlignHCenter );
    grid_layout->addWidget( checkboxes[QFile::WriteOwner ]  = new QCheckBox( box ), Qt::AlignHCenter );
    grid_layout->addWidget( checkboxes[QFile::ExeOwner  ]  = new QCheckBox( box ), Qt::AlignHCenter );

    // on unix, right now, Qt does not return the current user permissions. Disable them from the dialog
    #ifndef Q_WS_X11
    grid_layout->addWidget( new QLabel( "User: ", box ) );
    grid_layout->addWidget( checkboxes[QFile::ReadUser ]  = new QCheckBox( box ), Qt::AlignHCenter );
    grid_layout->addWidget( checkboxes[QFile::WriteUser]  = new QCheckBox( box ), Qt::AlignHCenter );
    grid_layout->addWidget( checkboxes[QFile::ExeUser  ]  = new QCheckBox( box ), Qt::AlignHCenter );
    #endif

    grid_layout->addWidget( new QLabel( "Group: ", box ) );
    grid_layout->addWidget( checkboxes[QFile::ReadGroup  ] = new QCheckBox( box ), Qt::AlignHCenter );
    grid_layout->addWidget( checkboxes[QFile::WriteGroup ] = new QCheckBox( box ), Qt::AlignHCenter );
    grid_layout->addWidget( checkboxes[QFile::ExeGroup   ] = new QCheckBox( box ), Qt::AlignHCenter );

    grid_layout->addWidget( new QLabel( "Others: ", box ) );
    grid_layout->addWidget( checkboxes[QFile::ReadOther  ] = new QCheckBox( box ), Qt::AlignHCenter );
    grid_layout->addWidget( checkboxes[QFile::WriteOther ] = new QCheckBox( box ), Qt::AlignHCenter );
    grid_layout->addWidget( checkboxes[QFile::ExeOther   ] = new QCheckBox( box ), Qt::AlignHCenter );

    QFile::Permissions permissions( file.permissions() );
    for( CheckBoxMap::iterator iter = checkboxes.begin(); iter != checkboxes.end(); iter++ )
    {
        iter->second->setChecked( permissions & iter->first );
        iter->second->setEnabled( false );
    }

    // group and user id
    if( file.exists() )
    {

        layout->addWidget( new QLabel( "<b>Ownership: </b>", box ) );

        grid_layout = new GridLayout();
        grid_layout->setMargin(0);
        grid_layout->setSpacing( 5 );
        grid_layout->setMaxCount( 2 );
        grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
        layout->addItem( grid_layout );

        // user id
        grid_layout->addWidget( label = new QLabel( "User: ", box ) );
        grid_layout->addWidget( label = new QLabel( file.userName(), box ) );

        // group id
        grid_layout->addWidget( label = new QLabel( "Group: ", box ) );
        grid_layout->addWidget( label = new QLabel( file.groupName(), box ) );
        grid_layout->setColumnStretch( 1, 1 );

    }

    layout->addStretch(1);

    Debug::Throw( "FileInformationDialog::FileInformationDialog - Permissions tab filled.\n" );
    adjustSize();

}
