
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

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QTextStream>

#include "AutoSaveThread.h"
#include "XmlOptions.h"
#include "Str.h"
#include "Util.h"

//_______________________________________________________________
void AutoSaveThread::setFile( const File& file )
{
    QMutexLocker locker( &mutex_ );
    File tmp( autoSaveName( file ) );
    if( tmp != file_ )
    {
        fileChanged_ = true;
        file_ = tmp;
    } else fileChanged_ = false;
}


//_______________________________________________________________
void AutoSaveThread::setContents( const QString& contents )
{
    QMutexLocker locker( &mutex_ );
    if( contents_ != contents )
    {
        contentsChanged_ = true;
        contents_ = contents;
    } else contentsChanged_ = false;

}

//________________________________________________________________
File AutoSaveThread::autoSaveName( const File& file )
{

    // get full path of current file, relative to root.
    // replace special characters by "_"
    QString relativeName = QDir::root().relativeFilePath( file ).replace( "/", "_" ).replace(":","_");

    // get qedit default autosave path
    QString autoSavePath;
    QTextStream( &autoSavePath )
        << XmlOptions::get().raw( "AUTOSAVE_PATH" )
        << "/qedit";

    // generate autosave name
    File tmpFile = File( relativeName ).addPath( QDir( autoSavePath ).absolutePath() );
    return tmpFile;

}

//_______________________________________________________________
void AutoSaveThread::run( void )
{

    if( contentsChanged_ || fileChanged_ )
    {

        // make sure path exists
        QDir path( file().path() );
        if( !( path.exists() || path.mkpath( "." ) ) ) return;

        // write to file
        QFile out( file() );
        if( !out.open( QIODevice::WriteOnly ) ) return;
        out.write( contents_.toAscii() );
        out.close();

    }

}
