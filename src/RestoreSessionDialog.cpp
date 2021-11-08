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

#include "RestoreSessionDialog.h"
#include "Debug.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "TreeView.h"
#include "XmlOptions.h"


#include <QTextStream>

//__________________________________________________
RestoreSessionDialog::RestoreSessionDialog( QWidget* parent, FileRecord::List files ):
    QuestionDialog( parent )
{

    Debug::Throw( QStringLiteral("RestoreSessionDialog::RestoreSessionDialog.\n") );
    setWindowTitle( tr( "Restore Saved Session" ) );

    if( files.size() == 1 )
    {

        setText( tr( "Restore file '%1' from saved session ?" ).arg( files.front().file() ) );

    } else {

        setText( tr( "Restore %1 files from saved session ?" ).arg( files.size() ) );

        auto treeView = new TreeView( this );
        setDetails( treeView );
        treeView->setModel( &model_ );
        model_.set( files );

        // mask
        int mask(
            (1<<FileRecordModel::FileName)|
            (1<<FileRecordModel::Path ));
        int classColumn( model_.findColumn( QStringLiteral("class_name") ) );
        if( classColumn >= 0 ) mask |= (1<<classColumn);
        treeView->setMask( mask );
        treeView->resizeColumns();

        treeView->setOptionName( QStringLiteral("SAVE_FILES_LIST") );

        // sort list and select all items
        if( XmlOptions::get().contains( QStringLiteral("SESSION_FILES_SORT_COLUMN") ) && XmlOptions::get().contains( QStringLiteral("SESSION_FILES_SORT_ORDER") ) )
        {
            treeView->sortByColumn(
                XmlOptions::get().get<int>( QStringLiteral("SESSION_FILES_SORT_COLUMN") ),
                (Qt::SortOrder)(XmlOptions::get().get<int>( QStringLiteral("SESSION_FILES_SORT_ORDER") ) ) );
        }

    }

    // rename buttons
    okButton().setText( tr( "Restore" ) );
    okButton().setIcon( IconEngine::get( IconNames::Open ) );
    adjustSize();

}
