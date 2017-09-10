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

#include "FileInformationDialog.h"

#include "BaseFileInfo.h"
#include "FileRecordProperties.h"
#include "FileSystemModel.h"


//_________________________________________________________
FileInformationDialog::FileInformationDialog( QWidget* parent, const FileRecord& record ):
    BaseFileInformationDialog( parent )
{
    Debug::Throw( "FileInformationDialog::FileInformationDialog.\n" );

    // file name
    const File& file( record.file() );
    setWindowTitle( file.isEmpty() ? tr( "File Information" ) : file.localName() );

    // file name
    if( !file.isEmpty() )
    {
        setFile( file.localName() );
        setPath( file.path() );
    }

    // type
    if( record.hasFlag( BaseFileInfo::Folder | BaseFileInfo::Document ) )
    {
        if( record.hasFlag( BaseFileInfo::Link ) ) setType( tr( "Symbolic link" ) );
        else if( record.hasFlag( BaseFileInfo::Folder ) ) setType( tr( "Folder" ) );
        else if( record.hasFlag( BaseFileInfo::Document ) ) setType( tr( "Document" ) );
    }

    // size
    if( file.exists() )
    {
        setSize( file.fileSize() );
        setCreated( file.created() );
        setAccessed( file.lastAccessed() );
        setModified( file.lastModified() );
        setPermissions( file.permissions() );
        setUser( file.userName() );
        setGroup( file.groupName() );
    }

    // document class
    if( record.hasProperty( FileRecordProperties::ClassName ) )
    { addRow( tr( "Class:" ), record.property( FileRecordProperties::ClassName ) ); }

    if( record.hasProperty( FileRecordProperties::Dictionary ) )
    { addRow( tr( "Spell-check dictionary:" ), record.property( FileRecordProperties::Dictionary ) ); }

    if( record.hasProperty( FileRecordProperties::Filter ) )
    { addRow( tr( "Spell-check filter:" ), record.property( FileRecordProperties::Filter ) ); }

    adjustSize();

}
