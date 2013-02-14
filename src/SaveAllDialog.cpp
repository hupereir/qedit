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

#include "SaveAllDialog.h"

#include "AnimatedTreeView.h"
#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "XmlOptions.h"

#include <QTextStream>

//__________________________________________________
SaveAllDialog::SaveAllDialog( QWidget* parent, FileRecord::List files ):
    QuestionDialog( parent )
{

    Debug::Throw( "SaveAllDialog::SaveAllDialog.\n" );
    setWindowTitle( "Save Files - Qedit" );

    Q_ASSERT( !files.empty() );


    if( files.size() == 1 )
    {

        QString buffer;
        QTextStream( &buffer ) << "File " << files.front().file() << " is modified. Save ?";
        setText( buffer );

    } else {

        QString buffer;
        QTextStream( &buffer ) << files.size() << " files are modified. Save ?";
        setText( buffer );

        AnimatedTreeView* treeView = new AnimatedTreeView( this );
        setDetails( treeView );
        treeView->setModel( &model_ );
        model_.set( files );

        // mask
        unsigned int mask(
            (1<<FileRecordModel::FILE)|
            (1<<FileRecordModel::PATH ));
        int classColumn( model_.findColumn( "class_name" ) );
        if( classColumn >= 0 ) mask |= (1<<classColumn);
        treeView->setMask( mask );
        treeView->resizeColumns();

        treeView->setOptionName( "SAVE_FILES_LIST" );

        // sort list and select all items
        if( XmlOptions::get().contains( "SESSION_FILES_SORT_COLUMN" ) && XmlOptions::get().contains( "SESSION_FILES_SORT_ORDER" ) )
        {
            treeView->sortByColumn(
                XmlOptions::get().get<int>( "SESSION_FILES_SORT_COLUMN" ),
                (Qt::SortOrder)(XmlOptions::get().get<int>( "SESSION_FILES_SORT_ORDER" ) ) );
        }

    }

    // rename buttons
    okButton().setText( "Save" );
    okButton().setIcon( IconEngine::get( ICONS::SAVE ) );
    adjustSize();

}
