#ifndef LineNumberDisplay_h
#define LineNumberDisplay_h

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
* software; if not, write to the Free Software , Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file LineNumberDisplay.h
  \brief display line number of a text editor
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QColor>
#include <QFont>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTextBlock>
#include <QObject>
#include <QWheelEvent>

#include "Counter.h"
#include "Debug.h"

class TextEditor;

//! display line number of a text editor
class LineNumberDisplay: public QObject, public Counter
{

  //! Qt meta object
  Q_OBJECT  

  public:
    
  //! constructor
  LineNumberDisplay(TextEditor*);

  //! destructor
  virtual ~LineNumberDisplay();
  
  //! synchronization
  void synchronize( LineNumberDisplay* );
  
  //! width
  virtual bool updateWidth( const int& );
  
  //! width
  virtual const int& width( void ) const
  { return width_; }
  
  //! paint
  virtual void paint( QPainter& );
  
  private slots:
  
  //! configuration
  void _updateConfiguration( void );
  
  //! need update
  void _needUpdate( void )
  { need_update_ = true; }
  
  //! contents changed
  void _contentsChanged( void );
  
  //! block count changed
  void _blockCountChanged( void );
    
  //! current block has changed
  void _currentBlockChanged( void );
  
  private:
  
  //! editor
  TextEditor& _editor( void ) const
  { return *editor_; }
    
  //! map block number and position
  class LineNumberData
  {
    
    public:
    
    //! list of data
    typedef std::vector<LineNumberData> List;
    
    //! constructor
    LineNumberData( const unsigned int& id = 0, const unsigned int& line_number = 0, const int& cursor = 0):
      id_( id ),
      line_number_( line_number ),
      cursor_( cursor ),
      position_( -1 ),
      valid_( false )
    {}
    
    //! id
    const unsigned int& id( void ) const
    { return id_; }

    //! line number
    const unsigned int& lineNumber( void ) const
    { return line_number_; }
    
    //! y
    const int& cursor( void ) const
    { return cursor_; }
    
    //! position
    void setPosition( const int& position )
    {
      valid_ = (position >= 0);
      position_ = position;
    }
    
    //! position
    const int& position( void ) const
    { return position_; }
    
    //! validity
    const bool& isValid( void ) const
    { return valid_; }
    
    private:
    
    //! block id
    unsigned int id_;
    
    //! line number
    unsigned int line_number_;
    
    //! position
    int cursor_;

    //! position
    int position_;
    
    //! validity
    bool valid_;
    
  };

  //! update line number data
  void _updateLineNumberData( void );
  
  //! update invalid data 
  void _updateLineNumberData( QTextBlock&, unsigned int&, LineNumberData& ) const;
  
  //! update current block data
  bool _updateCurrentBlockData( void );
  
  //! associated editor
  TextEditor* editor_;
  
  //! true when line number data update is needed
  bool need_update_;

  //! true when current block data needs update
  bool need_current_block_update_;
  
  //! true if current block
  bool has_current_block_;
 
  //! highlight color
  QColor highlight_color_;
    
  //! width
  int width_;
  
  //! line number data
  LineNumberData::List line_number_data_;
  
  //! current block data
  LineNumberData current_block_data_;
  
};

#endif
