// $Id$
#ifndef TextView_h
#define TextView_h

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
  \file TextView.h
  \brief compound widget to associate TextDisplay and LineNumberWidget
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QFrame>

#include "BlockDelimiter.h"
#include "Counter.h"
#include "Key.h"
#include "TextDisplay.h"

class LineNumberWidget;
class BlockDelimiterWidget;

class TextView: public QFrame, public BASE::Key, public Counter
{
  
  //! Qt meta object declaration
  Q_OBJECT
    
  public:
  
  //! constructor
  TextView( QWidget* parent );
  
  //! synchronization
  void synchronize( const TextView* view );

  //! editor
  TextDisplay &editor( void ) const
  {
    assert( editor_ );
    return *editor_;
  }
    
  //! used to select editor with matching filename
  class SameFileFTor
  {
    public:

    //! constructor
    SameFileFTor( const File& file ):
      file_( file.expand() )
    {}

    //! predicate
    bool operator() ( const TextView* view ) const
    { return view->editor().file() == file_; }

    private:

    //! predicted file
    const File file_;

  };

  //! used to select editor with empty, unmodified file
  class EmptyFileFTor
  {
    public:

    //! predicate
    bool operator() ( const TextView* view ) const
    { return view->editor().file().empty() && !view->editor().document()->isModified(); }

  };

    
  //! used to select editor modified files
  class ModifiedFTor
  {
    public:

    //! predicate
    bool operator() ( const TextView* view ) const
    { return view->editor().document()->isModified(); }

  };
  
  private slots:
  
  //! toggle line numbers
  void _toggleShowLineNumbers( bool );
  
  //! enable block delimiters
  void _loadBlockDelimiters( BlockDelimiter::List );
  
  //! toggle block delimiters
  void _toggleShowBlockDelimiters( bool );
  
  private:
   
  //! line editor
  LineNumberWidget& _lineNumberWidget( void ) const
  { 
    assert( line_number_widget_ );
    return *line_number_widget_;
  }
  
  //! block delimitor widget
  BlockDelimiterWidget& _blockDelimiterWidget( void ) const
  {
    assert( block_delimiter_widget_ );
    return *block_delimiter_widget_;
  }

  //! text display
  TextDisplay* editor_;
   
  //! line number
  LineNumberWidget* line_number_widget_;

  //! block delimiter
  BlockDelimiterWidget* block_delimiter_widget_;
  
};

#endif
