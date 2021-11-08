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

#include "CloseFilesDialog.h"
#include "Debug.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "TreeView.h"
#include "XmlOptions.h"


#include <QTextStream>

//__________________________________________________
CloseFilesDialog::CloseFilesDialog( QWidget* parent, FileRecord::List files ):
    QuestionDialog( parent )
{

    Debug::Throw( QStringLiteral("CloseFilesDialog::CloseFilesDialog.\n") );
    setWindowTitle( tr( "Close Files" ) );
    setOptionName( QStringLiteral("CLOSE_FILES_DIALOG") );

    Q_ASSERT( !files.empty() );

    if( files.size() == 1 )
    {

        setText( tr( "Editing '%1'. Close ?" ).arg( files.front().file() ) );

    } else {

        setText( tr( "Editing %1 files. Close ?" ).arg( files.size() ) );

        TreeView* treeView = new TreeView( this );
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

        treeView->setOptionName( QStringLiteral("CLOSE_FILES_LIST") );

        // sort list and select all items
        if( XmlOptions::get().contains( QStringLiteral("SESSION_FILES_SORT_COLUMN") ) && XmlOptions::get().contains( QStringLiteral("SESSION_FILES_SORT_ORDER") ) )
        {
            treeView->sortByColumn(
                XmlOptions::get().get<int>( QStringLiteral("SESSION_FILES_SORT_COLUMN") ),
                (Qt::SortOrder)(XmlOptions::get().get<int>( QStringLiteral("SESSION_FILES_SORT_ORDER") ) ) );
        }

    }

    // rename buttons
    okButton().setText( tr( "Close" ) );
    okButton().setIcon( IconEngine::get( IconNames::DialogClose ) );
    adjustSize();

}
