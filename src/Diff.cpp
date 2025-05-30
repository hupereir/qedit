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

#include "Diff.h"
#include "HighlightBlockFlags.h"
#include "InformationDialog.h"
#include "TextBlockRange.h"
#include "TextDisplay.h"
#include "TimeStamp.h"
#include "Util.h"
#include "XmlOptions.h"

#include <QFile>

namespace
{

    //! local temporary file counter, needed to avoid collisions
    static unsigned int tempFileIndex = 0;

}

//__________________________________________________________________
Diff::Diff( QObject* parent ):
    QObject( parent ),
    Counter( QStringLiteral("Diff") ),
    process_( this )
{ connect( &process_, QOverload<int,QProcess::ExitStatus>::of( &QProcess::finished ), this, &Diff::_parseOutput ); }

//_________________________________________________________________
bool Diff::run()
{
    Debug::Throw( QStringLiteral("Diff::run.\n") );

    // see if process is not already running
    if( process_.state() != QProcess::NotRunning )
    {
        error_ = tr( "diff command already running." );
        return false;
    }

    // reset error
    error_ = QLatin1String("");

    // check files
    for( const auto& fileInformation:files_ )
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
            error_ = tr( "file '%1' does not exist." ).arg( fileInformation.file() );
            return false;
        }

        // check if file is a directory
        if( fileInformation.file().isDirectory() )
        {
            error_ = tr( "file '%1' is a directory." ).arg( fileInformation.file() );
            return false;
        }

    }

    // clear paragraph ranges
    _clear();

    // create command and pass to process
    const QStringList command({
        XmlOptions::get().raw( QStringLiteral("DIFF_COMMAND") ),
        files_[0].file(),
        files_[1].file()
    });

    // run
    process_.start( command );
    return true;

}

//________________________________________________________________
void Diff::_clear()
{

    Debug::Throw( QStringLiteral("Diff::_Clear.\n") );
    for( auto& file:files_ ) file.clear();
    return;
}

//________________________________________________________________
void Diff::_parseOutput( int code, QProcess::ExitStatus status )
{
    Debug::Throw(  QStringLiteral("Diff::_parseOutput.\n") );

    // check exit code
    if( status != QProcess::NormalExit )
    {
        InformationDialog( nullptr, tr( "diff excited with code %1" ).arg( code ) ).exec();
        return;
    }

    // retrieve all stdout and put into stream
    const auto in( QString( process_.readAllStandardOutput()  ).split( QLatin1Char('\n') ) );
    for( const auto& buffer:in )
    {

        if( buffer.isEmpty() ) continue;

        static const QString removedLines( QStringLiteral("<>-") );
        if( removedLines.indexOf( buffer[0] ) >= 0 ) continue;

        // parse remaining lines
        _parseLine( buffer );
    }

    // highlight displays
    for( auto& file:files_ )
    { file.highlightDisplay(); }

    return;

}

//______________________________________________________
void Diff::_parseLine( const QString& line )
{
    Debug::Throw() << "Diff::_parseLine - " << line << Qt::endl;

    // see if line is a conflict
    int position( line.indexOf( QLatin1String("c") ) );
    if( position >= 0 )
    {

        const std::array<Range,2> ranges =
        {{
            _parseRange(line.left( position )),
            _parseRange(line.mid( position+1 ))
        }};

        for( int i = 0; i<= 1; i++ )
        { files_[i].insertConflictRange( ranges[i] ); }
        return;
    }


    // see if line is added
    position = line.indexOf( QLatin1String("a") );
    if( position >= 0 )
    {
        auto range( line.mid( position+1, line.size() ) );
        files_[1].insertAddedRange( _parseRange( range ) );
        return;
    }

    // see if line is deleted
    position = line.indexOf( QLatin1String("d") );
    if( position >= 0 )
    {
        auto range( line.left( position ) );
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

    Debug::Throw() << "Diff::_parseRange - " << range << Qt::endl;

    // look for "," in string
    int position( range.indexOf( QLatin1String(",") ) );
    const auto out( ( position < 0 ) ?
        Range( range.toUInt(), range.toUInt() ):
        Range( range.left( position ).toUInt(), range.mid( position+1 ).toUInt() ) );

    Debug::Throw() << "Diff::_parseRange - (" << out.first << "," << out.second << ")" << Qt::endl;
    return out;

}

//___________________________________________________________________
Diff::FileInformation::FileInformation()
{ Debug::Throw( QStringLiteral("Diff::FileInformation::FileInformation.\n") ); }

//___________________________________________________________________
void Diff::FileInformation::setDisplay( TextDisplay& display )
{

    Debug::Throw( QStringLiteral("Diff::FileInformation::setDisplay.\n") );

    // keep pointer to display
    display_ = &display;

    // try use provided filename
    // if exists and if display is not modified
    if( !(
        display_->document()->isModified() ||
        display_->file().isEmpty() ||
        display_->isNewDocument() ||
        !display_->file().exists() ) )
    {

        // use provided file. Set as non-temporary
        file_ = display_->file().expanded();
        isTemporary_ = false;

    } else {

        // create temporary file
        file_.set(
            QStringLiteral( "/tmp/_qedit_diff_%1_%2_%3_%4" )
            .arg( Util::user() )
            .arg( TimeStamp::now().unixTime() )
            .arg( Util::pid() )
            .arg( ++tempFileIndex ) );

        Debug::Throw() << "Diff::FileInformation::setDisplay - writting content to file: " << file_ << Qt::endl;


        // try dump text in file
        QFile out( file_ );
        if( !out.open( QIODevice::WriteOnly ) )
        {
            Debug::Throw() << "Diff::FileInformation::setDisplay - cannot write to file " << file_ << Qt::endl;
            return;
        }

        // dump text
        out.write( qPrintable( display_->toPlainText() ) );
        out.close();

        // keep file as temporary, so that
        // it gets deleted at destruction
        isTemporary_ = true;

    }

    Debug::Throw() << "Diff::FileInformation::setDisplay - file: " << file_ << Qt::endl;

    return;
}

//________________________________________________________________________
Diff::FileInformation::~FileInformation()
{ if( isTemporary_ ) file_.remove(); }

//________________________________________________________________________
void Diff::FileInformation::highlightDisplay()
{

    Debug::Throw() << "Diff::FileInformation::highlightDisplay." << Qt::endl;

    // loop over display blocks
    int id(1);
    display_->setUpdatesEnabled( false );
    for( const auto& block:TextBlockRange( display_->document() ) )
    {

        // see if block is a conflict
        if( conflicts_.find( id ) != conflicts_.end() )
        {

            display_->tagBlock( block, TextBlock::DiffConflict );

        } else if( added_.find( id ) != added_.end() ) {

            display_->tagBlock( block, TextBlock::DiffAdded );

        } else display_->clearTag( block, TextBlock::DiffConflict | TextBlock::DiffAdded );

        // increment block id
        ++id;

    }

    display_->setUpdatesEnabled( true );

    // get associated displays and update all
    // this is needed due to the setUpdatesEnabled above
    Base::KeySet<TextDisplay> displays( display_ );
    displays.insert( display_ );
    for( const auto& display:displays )
    { display->viewport()->update(); }

    return;
};
