
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

/*!
  \file AutoSaveThread.cpp
  \brief independent thread used to make regular backups of files
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/


#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QTextStream>

#include "AutoSaveThread.h"
#include "XmlOptions.h"
#include "Str.h"
#include "Util.h"



//________________________________________________________________
File AutoSaveThread::autoSaveName( const File& file )
{

  // get full path of current file, relative to root.
  // replace special characters by "_"
  QString relative_name = QDir::root().relativeFilePath( file ).replace( "/", "_" ).replace(":","_");

  // get qedit default autosave path
  QString autosave_path;
  QTextStream( &autosave_path )
    << XmlOptions::get().raw( "AUTOSAVE_PATH" )
    << "/qedit";

  // generate autosave name
  File tmp_file = File( relative_name ).addPath( QDir( autosave_path ).absolutePath() );
  return tmp_file;

}

//_______________________________________________________________
void AutoSaveThread::setFile( const File& file )
{
  File tmp( autoSaveName( file ) );
  if( tmp != file_ )
  {
    file_changed_ = true;
    file_ = tmp;
  } else file_changed_ = false;
}


//_______________________________________________________________
void AutoSaveThread::setContents( const QString& contents )
{

  if( contents_ != contents )
  {
    contents_changed_ = true;
    contents_ = contents;
  } else contents_changed_ = false;

}


//_______________________________________________________________
void AutoSaveThread::run( void )
{

  if( contents_changed_ || file_changed_ )
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
