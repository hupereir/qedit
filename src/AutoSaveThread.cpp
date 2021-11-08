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
#include "Operators.h"
#include "Util.h"
#include "XmlOptions.h"


#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QTextStream>
#include <QTextCodec>

//_______________________________________________________________
void AutoSaveThread::setFile( const File &file )
{
    QMutexLocker locker( &mutex_ );
    File tmp( autoSaveName( file ) );
    if( tmp != file_ )
    {
        flags_ |= FileChanged;
        file_ = tmp;
    }
}


//_______________________________________________________________
void AutoSaveThread::setContent( const QString& content )
{
    QMutexLocker locker( &mutex_ );
    if( content_ != content )
    {
        flags_ |= ContentChanged;
        content_ = content;
    }
}

//________________________________________________________________
void AutoSaveThread::setTextEncoding( const QByteArray &encoding )
{
    QMutexLocker locker( &mutex_ );
    if( textEncoding_ != encoding )
    {
        flags_ |= EncodingChanged;
        textEncoding_ = encoding;
    }
}

//________________________________________________________________
void AutoSaveThread::setUseCompression( bool useCompression )
{
    QMutexLocker locker( &mutex_ );
    if( useCompression_ != useCompression )
    {
        flags_ |= CompressionChanged;
        useCompression_ = useCompression;
    }
}

//________________________________________________________________
File AutoSaveThread::autoSaveName( const File& file )
{

    // get full path of current file, relative to root.
    // replace special characters by "_"
    File relativeName( QDir::root().relativeFilePath( file ).replace( QLatin1String("/"), QLatin1String("_") ).replace(QLatin1String(":"),QLatin1String("_")) );

    // get qedit default autosave path
    QString autoSavePath = QStringLiteral( "%1/qedit/%2" ).arg( XmlOptions::get().raw( QStringLiteral("AUTOSAVE_PATH") ), Util::user() );

    // generate autosave name
    return relativeName.addPath( File( QDir( autoSavePath ).absolutePath() ) );

}

//_______________________________________________________________
void AutoSaveThread::run()
{

    if( flags_ )
    {

        // make sure path exists
        QDir path( file().path() );
        if( !( path.exists() || path.mkpath( QStringLiteral(".") ) ) ) return;

        // write to file
        QFile out( file_ );
        if( !out.open( QIODevice::WriteOnly ) ) return;

        // make sure that last line ends with "end of line"
        auto text( content_ );
        if( !text.isEmpty() && text[text.size()-1] != '\n' ) text += '\n';

        QTextCodec* codec( QTextCodec::codecForName( textEncoding_ ) );
        auto content( codec->fromUnicode( text ) );
        if( useCompression_ ) content = qCompress( content );
        out.write( content );
        out.close();

        flags_ = None;

    }

}
