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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "SaveAllDialog.h"
#include "SaveAllDialog.moc"

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
    setWindowTitle( tr( "Save Files - Qedit" ) );

    Q_ASSERT( !files.empty() );


    if( files.size() == 1 )
    {

        setText( QString( tr( "File '%1' is modified. Save ?" ) ).arg( files.front().file() ) );

    } else {

        setText( QString( tr( "%1 files are modified. Save ?" ) ).arg( files.size() ) );

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
    okButton().setText( tr( "Save" ) );
    okButton().setIcon( IconEngine::get( ICONS::SAVE ) );
    adjustSize();

}
