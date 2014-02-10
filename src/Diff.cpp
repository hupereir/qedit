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

/*!
\file Diff.cpp
\brief make diff between two files, stores conflict and added lines for both
\author  Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "Diff.h"
#include "Diff.moc"
#include "HighlightBlockFlags.h"
#include "XmlOptions.h"
#include "TextDisplay.h"
#include "TimeStamp.h"
#include "InformationDialog.h"
#include "Util.h"

#include <QFile>

//__________________________________________________________________
Diff::Diff( QObject* parent ):
    QObject( parent ),
    Counter( "Diff" ),
    files_( 2 ),
    process_( this )
{
    Debug::Throw( "Diff::Diff\n" );
    connect( &process_, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(_parseOutput(int,QProcess::ExitStatus)) );
}

//_________________________________________________________________
bool Diff::run( void )
{
    Debug::Throw( "Diff::run.\n" );

    // see if process is not already running
    if( process_.state() != QProcess::NotRunning )
    {
        error_ = tr( "diff command already running." );
        return false;
    }

    // reset error
    error_ = "";

    // check files
    foreach( const FileInformation& fileInformation, files_ )
    {

        // check if filename is empty
        if( fileInformation.file().isEmpty() )
        {
            error_ = tr( "invalid file name." );
            return false;
        }

        // check if file exists
        if( !fileInformation.file().exists() )
        {
            error_ = QString( tr( "file '%1' does not exist." ) ).arg( fileInformation.file() );
            return false;
        }

        // check if file is a directory
        if( fileInformation.file().isDirectory() )
        {
            error_ = QString( tr( "file '%1' is a directory." ) ).arg( fileInformation.file() );
            return false;
        }

    }

    // clear paragraph ranges
    _clear();

    // create command and pass to process
    QStringList command;
    command
        << XmlOptions::get().raw( "DIFF_COMMAND" )
        << files_[0].file()
        << files_[1].file();

    // run
    process_.start( command );
    return true;

}

//________________________________________________________________
void Diff::_clear( void )
{

    Debug::Throw( "Diff::_Clear.\n" );
    for( FileInformation::List::iterator iter = files_.begin(); iter != files_.end(); ++iter )
    { iter->clear(); }
    return;
}

//________________________________________________________________
void Diff::_parseOutput( int code, QProcess::ExitStatus status )
{
    Debug::Throw(  "Diff::_parseOutput.\n" );

    // check exit code
    if( status != QProcess::NormalExit )
    {
        InformationDialog( 0, QString( tr( "diff excited with code %1" ) ).arg( code ) ).exec();
        return;
    }

    // retrieve all stdout and put into stream
    QByteArray out( process_.readAllStandardOutput() );
    QStringList in( QString( out ).split( "\n" ) );
    int index(0);

    foreach( QString buffer, in )
    {

        index++;

        static const QString removedLines( "<>-" );
        if( removedLines.indexOf( buffer[0] ) >= 0 ) continue;

        // parse remaining lines
        _parseLine( buffer );
    }

    // highlight displays
    for( FileInformation::List::iterator iter = files_.begin(); iter != files_.end(); ++iter )
    { iter->highlightDisplay(); }

    // delete this object
    deleteLater();

    return;

}

//______________________________________________________
void Diff::_parseLine( const QString& line )
{
    Debug::Throw() << "Diff::_parseLine - " << line << endl;

    // see if line is a conflict
    int position( line.indexOf( "c" ) );
    if( position >= 0 )
    {
        QVector<Range> ranges(2);
        ranges[0] = _parseRange(line.left( position ));
        ranges[1] = _parseRange(line.mid( position+1 ));

        for( unsigned int i = 0; i<= 1; i++ )
        { files_[i].insertConflictRange( ranges[i] ); }
        return;
    }


    // see if line is added
    position = line.indexOf( "a" );
    if( position >= 0 )
    {
        QString range( line.mid( position+1, line.size() ) );
        files_[1].insertAddedRange( _parseRange( range ) );
        return;
    }

    // see if line is deleted
    position = line.indexOf( "d" );
    if( position >= 0 )
    {
        QString range( line.left( position ) );
        files_[0].insertAddedRange( _parseRange( range ) );
        return;
    }

    // dump error message if line format was not recognized
    Debug::Throw() << "Diff::_parseLine - unrecognized line format: " << line;
    return;

}

//_____________________________________________________
Diff::Range Diff::_parseRange( const QString& range )
{

    Debug::Throw() << "Diff::_parseRange - " << range << endl;

    // look for "," in string
    int position( range.indexOf( "," ) );
    Range out( ( position < 0 ) ?
        Range( range.toUInt(), range.toUInt() ):
        Range( range.left( position ).toUInt(), range.mid( position+1 ).toUInt() ) );

    Debug::Throw() << "Diff::_parseRange - (" << out.first << "," << out.second << ")" << endl;
    return out;

}

//___________________________________________________________________
Diff::FileInformation::FileInformation( void ):
    display_( 0 ),
    isTemporary_( false )
{ Debug::Throw( "Diff::FileInformation::FileInformation.\n" ); }

//___________________________________________________________________
void Diff::FileInformation::setDisplay( TextDisplay& display )
{

    Debug::Throw( "Diff::FileInformation::setDisplay.\n" );

    // keep pointer to display
    display_ = &display;

    // try use provided filename
    // if exists and if display is not modified
    if( !(
        _display().document()->isModified() ||
        _display().file().isEmpty() ||
        _display().isNewDocument() ||
        !_display().file().exists() ) )
    {

        // use provided file. Set as non-temporary
        file_ = _display().file().expand();
        isTemporary_ = false;

    } else {

        // create temporary file
        QString buffer;
        QTextStream( &buffer ) << "/tmp/_qedit_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid();

        // store
        file_ = buffer;

        // try dump text in file
        QFile out( buffer );
        if( !out.open( QIODevice::WriteOnly ) )
        {
            Debug::Throw() << "Diff::FileInformation::setDisplay - cannot write to file " << buffer << endl;
            return;
        }

        // dump text
        out.write( _display().toPlainText().toLatin1() );
        out.close();

        // keep file as temporary, so that
        // it gets deleted at destruction
        isTemporary_ = true;

    }

    Debug::Throw() << "Diff::FileInformation::setDisplay - file: " << file_ << endl;

    return;
}

//________________________________________________________________________
Diff::FileInformation::~FileInformation( void )
{
    Debug::Throw() << "Diff::FileInformation::~FileInformation" << endl;
    if( isTemporary_ ) file_.remove();
}

//________________________________________________________________________
void Diff::FileInformation::highlightDisplay( void )
{

    Debug::Throw() << "Diff::FileInformation::highlightDisplay." << endl;

    // loop over display blocks
    unsigned int id(1);
    _display().setUpdatesEnabled( false );
    for( QTextBlock block( _display().document()->begin() ); block.isValid(); block = block.next(), id++ )
    {

        // see if block is a conflict
        if( conflicts_.find( id ) != conflicts_.end() )
        {

            _display().tagBlock( block, TextBlock::DIFF_CONFLICT );

        } else if( added_.find( id ) != added_.end() ) {

            _display().tagBlock( block, TextBlock::DIFF_ADDED );

        } else _display().clearTag( block, TextBlock::DIFF_CONFLICT | TextBlock::DIFF_ADDED );

    }

    _display().setUpdatesEnabled( true );

    // get associated displays and update all
    // this is needed due to the setUpdatesEnabled above
    Base::KeySet<TextDisplay> displays( &_display() );
    displays.insert( &_display() );
    foreach( TextDisplay* display, displays )
    { display->viewport()->update(); }

    return;
};
