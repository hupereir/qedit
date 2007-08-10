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

#include "CustomProcess.h"
#include "Counter.h"
#include "Debug.h"
#include "File.h"

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>

class TextDisplay;

//! make diff between two files, stores conflict and added lines for both
class Diff: public QObject, public Counter
{
  
  //! Qt meta object declaration
  Q_OBJECT
  
  public:

  //! first/second file 
  enum { 
    
    //! tag for first file
    FIRST,
    
    //! tag for second file
    SECOND
  };

  //! constructor
  Diff( QObject* parent );

  //! store files from text Displays
  void setTextDisplays( TextDisplay& first, TextDisplay& second )
  { 
    files_[FIRST].setDisplay( first );
    files_[SECOND].setDisplay( second );
  }
    
  //! run process
  /*! returns true if command could run succesfully */
  bool run( void );
  
  //! error message
  const std::string& error( void ) const
  { return error_; }
  
  //! paragraph range
  class Range: public std::pair< unsigned int, unsigned int > 
  {
    
    public: 
    
    //! constructor
    Range( int unsigned first, unsigned int second ):
      std::pair<unsigned int, unsigned int>( std::make_pair( first, second ) )
    {}
    
    //! true if the range contains index
    bool contains( const unsigned int& i ) const
    { return i>= first && i<= second; }
    
    //! used to retrieve ranges that contains a given block id
    class ContainsFTor
    {
      
      public:
  
      //! constructor
      ContainsFTor( const unsigned int& id ):
        id_( id )
        {}
        
      //! predicate
      bool operator() ( const Range& range )
      { return range.contains( id_ ); }
      
      private:
      
      //! prediction
      unsigned int id_;
  
    };
    
  };

  //! range list
  typedef std::set< Range > RangeSet;
  
  private slots:
  
  //! parse the diff output
  void _parseOutput( int, QProcess::ExitStatus );
  
  //! autodelete object
  void _autoDelete( void )
  { delete this; }
  
  private:

  //! clear ranges
  void _clear( void );
    
  //! parse a diff line
  void _parseLine( const std::string& line );

  //! parse a diff line
  static Range _parseRange( const std::string& range );

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
    { added_.insert( range ); }
    
    //! add conflict range
    void insertConflictRange( const Range& range )
    { conflicts_.insert( range ); }

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
      Exception::checkPointer( display_, DESCRIPTION( "invalid display" ) );
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
    RangeSet added_;
    
    //! conflict paragraphs
    RangeSet conflicts_;

    //!@name colors
    //@{
    
    QColor conflict_color_;
    
    QColor added_color_; 
        
    //@}
    
  };
  
  //! file specific diff information
  std::vector< FileInformation > files_;
  
  //! process
  CustomProcess process_;
  
  //! possible error string
  std::string error_;
  
};

#endif
