
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

#include "AutoSaveThread.h"
#include "XmlOptions.h"
#include "Str.h"
#include "Util.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QTextStream>
#include <QTextCodec>

//_______________________________________________________________
void AutoSaveThread::setFile( const File& file )
{
    QMutexLocker locker( &mutex_ );
    File tmp( autoSaveName( file ) );
    if( tmp != file_ )
    {
        flags_ &= FileChanged;
        file_ = tmp;
    } else flags_ |= Flags( ~FileChanged );
}


//_______________________________________________________________
void AutoSaveThread::setContents( const QString& contents )
{
    QMutexLocker locker( &mutex_ );
    if( contents_ != contents )
    {
        flags_ &= ContentChanged;
        contents_ = contents;
    } else flags_ |= Flags( ~ContentChanged );

}

//________________________________________________________________
void AutoSaveThread::setTextEncoding( const QString& encoding )
{
    QMutexLocker locker( &mutex_ );
    if( encoding != textEncoding_ )
    {
        flags_ &= TextEncodingChanged;
        textEncoding_ = encoding;
    } else flags_ |= Flags( ~TextEncodingChanged );
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
        << "/qedit/" << Util::user();

    // generate autosave name
    File tmpFile = File( relativeName ).addPath( QDir( autoSavePath ).absolutePath() );
    return tmpFile;

}

//_______________________________________________________________
void AutoSaveThread::run( void )
{

    if( flags_ )
    {

        // make sure path exists
        QDir path( file().path() );
        if( !( path.exists() || path.mkpath( "." ) ) ) return;

        // get encoding
        QTextCodec* codec( QTextCodec::codecForName( qPrintable( textEncoding_ ) ) );
        Q_ASSERT( codec );

        // write to file
        QFile out( file() );
        if( !out.open( QIODevice::WriteOnly ) ) return;
        out.write( codec->fromUnicode( contents_ ) );
        out.close();

        flags_ = None;

    }

}
