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
* Place, Suite 330, Boston, MA 02111-1307 USA                           
*                         
*                         
*******************************************************************************/

/*!
  \file Diff.cc
  \brief make diff between two files, stores conflict and added lines for both
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QFile>

#include "Diff.h"
#include "HighlightBlockFlags.h"
#include "XmlOptions.h"
#include "TextDisplay.h"
#include "TimeStamp.h"
#include "QtUtil.h"
#include "Util.h"


using namespace std;

//__________________________________________________________________
Diff::Diff( QObject* parent ):
  QObject( parent ),
  Counter( "Diff" ),
  files_(2),
  process_( this )
{ 
  Debug::Throw( "Diff::Diff\n" );   
  connect( &process_, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( _parseOutput( int, QProcess::ExitStatus ) ) );
}
  
//_________________________________________________________________
bool Diff::run( void )
{
  Debug::Throw( "Diff::run.\n" );
  
  // see if process is not already running
  if( process_.state() != QProcess::NotRunning )
  {
    error_ = "diff command already running.";
    return false;
  }
  
  // reset error
  error_ = "";
  
  // check files
  for( vector<FileInformation>::const_iterator iter = files_.begin(); iter != files_.end(); iter++ )
  { 
    
    // check if filename is empty
    if( iter->file().empty() ) 
    {
      error_ = "invalid file name.";
      return false;
    }
    
    // check if file exists
    if( !iter->file().exist() ) {
      error_ = string( "file " ) + iter->file() + string( " does not exist." );
      return false;
    }
    
    // check if file is a directory
    if( iter->file().isDirectory() ) 
    {
      error_ = string( "file " ) + iter->file() + string( " is a directory." );
      return false;
    }
    
  }

  // clear paragraph ranges
  _clear();
  
  // create command and pass to process
  ostringstream what; 
  what << XmlOptions::get().raw( "DIFF_COMMAND" ) << " " << files_[FIRST].file() << " " << files_[SECOND].file();
  Debug::Throw() << "Diff::run - command: " << what.str() << endl;

  // run
  process_.start( what.str() ); 
  return true;
  
}

//________________________________________________________________
void Diff::_clear( void )
{ 
  
  Debug::Throw( "Diff::_Clear.\n" );
  for( vector<FileInformation>::iterator iter = files_.begin(); iter != files_.end(); iter++ )
  { iter->clear(); }  
  return;
}
  
//________________________________________________________________
void Diff::_parseOutput( int code, QProcess::ExitStatus status )
{
  Debug::Throw( "Diff::_parseOutput.\n" );

  // check exit code
  if( status != QProcess::NormalExit )
  {
    ostringstream what;
    what << "diff excited with code " << code;
    QtUtil::infoDialog( 0, what.str() );
    return;
  }
    
  // retrieve all stdout and put into stream
  QByteArray out( process_.readAllStandardOutput() );
  istringstream in( out.constData() );

  const int linesize( 512 );
  char buffer[linesize];
  while ((in.rdstate() & ios::failbit) == 0 ) 
  {
   
    in.getline( buffer, linesize, '\n');  
    if( (!buffer ) || buffer[0] == '\0' || !(string( buffer ).size()) ) continue;
   
    static const std::string removed_lines( "<>-" );
    if( removed_lines.find( buffer[0] ) != string::npos ) continue;
    
    // parse remaining lines
    _parseLine( buffer );
    
  }
  
  // highlight displays 
  for( vector<FileInformation>::iterator iter = files_.begin(); iter != files_.end(); iter++ )
  { iter->highlightDisplay(); }
  
  // delete object
  _autoDelete();
  
  return;
  
}

//______________________________________________________
void Diff::_parseLine( const std::string& line )
{
  Debug::Throw() << "Diff::_parseLine - " << line << endl;
  
  // see if line is a conflict
  size_t position( line.find( "c" ) );
  if( position != string::npos )
  {
    vector<Range> ranges(2);
    ranges[FIRST] = _parseRange(line.substr( 0, position ));
    ranges[SECOND] = _parseRange(line.substr( position+1, line.size() - position - 1 ));

    for( unsigned int i = 0; i<= SECOND; i++ )
    { files_[i].insertConflictRange( ranges[i] ); }
    return;
  }
  
  
  // see if line is added
  position = line.find( "a" );
  if( position != string::npos )
  {
    string range( line.substr( position+1, line.size() - position - 1 ) );
    files_[SECOND].insertAddedRange( _parseRange( range ) );
    return;
  }
  
  // see if line is deleted
  position = line.find( "d" );
  if( position != string::npos )
  {
    string range( line.substr( 0, position ) );
    files_[FIRST].insertAddedRange( _parseRange( range ) );
    return;
  }

  // dump error message if line format was not recognized
  Debug::Throw() << "Diff::_parseLine - unrecognized line format: " << line;
  return;
  
}

//_____________________________________________________
Diff::Range Diff::_parseRange( const std::string& range )
{
  
  Debug::Throw() << "Diff::_parseRange - " << range << endl;
  
  // look for "," in string
  size_t position( range.find( "," ) ); 
  Range out( ( position == string::npos ) ?
    Range( Str( range ).get<unsigned int>(), Str( range ).get<unsigned int>() ):
    Range( Str( range.substr( 0, position ) ).get<unsigned int>(), Str( range.substr( position+1, range.size() - position - 1 ) ).get<unsigned int>() ) );
    
  Debug::Throw() << "Diff::_parseRange - (" << out.first << "," << out.second << ")" << endl;
  return out;
  
}
 
//___________________________________________________________________
Diff::FileInformation::FileInformation( void ):
  display_( 0 ),
  is_temporary_( false )
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
    _display().file().empty() || 
    !_display().file().exist() ) ) 
  {
    
    // use provided file. Set as non-temporary
    file_ = _display().file().expand();
    is_temporary_ = false;

  } else {
   
    // create temporary file
    ostringstream what;
    what << "/tmp/_qedit_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid();
    
    // store
    file_ = what.str();
    
    // try dump text in file
    QFile out( what.str().c_str() );
    if( !out.open( QIODevice::WriteOnly ) ) 
    {
      Debug::Throw() << "Diff::FileInformation::setDisplay - cannot write to file " << what.str() << endl;
      return;
    }

    // dump text
    out.write( _display().toPlainText().toAscii() );
    out.close();
    
    // keep file as temporary, so that
    // it gets deleted at destruction
    is_temporary_ = true;
    
  }
  
  Debug::Throw() << "Diff::FileInformation::setDisplay - file: " << file_ << endl;
  
  return;
}

//________________________________________________________________________
Diff::FileInformation::~FileInformation( void )
{
  Debug::Throw() << "Diff::FileInformation::~FileInformation" << endl;
  if( is_temporary_ ) file_.remove();
}

//________________________________________________________________________
void Diff::FileInformation::highlightDisplay( void )
{ 
    
  // loop over display blocks
  unsigned int id(0);
  for( QTextBlock block( _display().document()->begin() ); block.isValid(); block = block.next(), id++ )
  {
    
    // see if block is a conflict
    if( find_if( conflicts_.begin(), conflicts_.end(), Range::ContainsFTor( id ) ) != conflicts_.end() )
    { _display().tagBlock( block, TextBlock::DIFF_CONFLICT ); }
    
    // see if block is a added
    else if( find_if( added_.begin(), added_.end(), Range::ContainsFTor( id ) ) != added_.end() )
    { _display().tagBlock( block, TextBlock::DIFF_ADDED ); }
  
    else _display().clearBlockTag( block, TextBlock::DIFF_CONFLICT | TextBlock::DIFF_ADDED );
  }
  
  return; 
};
