#ifndef Diff_h 
#define Diff_h 

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
  \file Diff.h
  \brief make diff between two files, stores conflict and added lines for both
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/


#include <assert.h>

#include <map>
#include <set>
#include <vector>

#include "CustomProcess.h"
#include "Counter.h"
#include "Debug.h"
#include "File.h"

class TextDisplay;

//! make diff between two files, stores conflict and added lines for both
class Diff: public QObject, public Counter
{
  
  //! Qt meta object declaration
  Q_OBJECT
  
  public:

  //! constructor
  Diff( QObject* parent );

  //! store files from text Displays
  void setTextDisplays( TextDisplay& first, TextDisplay& second )
  { 
    files_[0].setDisplay( first );
    files_[1].setDisplay( second );
  }
    
  //! run process
  /*! returns true if command could run succesfully */
  bool run( void );
  
  //! error message
  const QString& error( void ) const
  { return error_; }
  
  //! paragraph range
  typedef std::pair< unsigned int, unsigned int > Range;

  //! range list
  typedef std::set< unsigned int > BlockSet;

  private slots:
  
  //! parse the diff output
  void _parseOutput( int, QProcess::ExitStatus );
    
  private:
  
  //! clear ranges
  void _clear( void );
    
  //! parse a diff line
  void _parseLine( const QString& line );

  //! parse a diff line
  static Range _parseRange( const QString& range );

  //! file specific diff informations
  class FileInformation
  {
    
    public:
    
    //! default constructor
    FileInformation( void );
  
    //! destructor
    ~FileInformation( void );
    
    //! set display
    void setDisplay( TextDisplay& display );
    
    //! file
    const File& file( void ) const
    { return file_; }
    
    //! add added range
    void insertAddedRange( const Range& range )
    { 
      for( unsigned int index = range.first; index <= range.second; index++ )
      { added_.insert( index ); }
    }
    
    //! add conflict range
    void insertConflictRange( const Range& range )
    { 
      for( unsigned int index = range.first; index <= range.second; index++ )
      { conflicts_.insert( index ); }
    }


    //! clear
    void clear( void )
    { 
      added_.clear();
      conflicts_.clear();
    }

    //! highlight text display based on conflicts and added paragraphs
    void highlightDisplay( void );
    
    private:

    //! text display
    TextDisplay& _display( void )
    { 
      assert( display_  );
      return *display_; 
    }


    //! text display 
    TextDisplay* display_;
    
    //! file
    File file_;
  
    /*! 
      \brief true when file is a temporary and should be deleted 
      at destruction
    */
    bool is_temporary_;
    
    //! added paragraphs
    BlockSet added_;
    
    //! conflict paragraphs
    BlockSet conflicts_;
    
  };
  
  //! file specific diff information
  std::vector< FileInformation > files_;
  
  //! process
  CustomProcess process_;
  
  //! possible error string
  QString error_;
  
};

#endif
