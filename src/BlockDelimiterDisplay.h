#ifndef BlockDelimiterDisplay_h
#define BlockDelimiterDisplay_h

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
  \file BlockDelimiterDisplay.h
  \brief display block delimiters
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QAction>
#include <QColor>
#include <QFont>
#include <QTextBlock>
#include <QTextCursor>
#include <QObject>

#include "BlockDelimiter.h"
#include "BlockDelimiterSegment.h"
#include "CollapsedBlockData.h"
#include "Counter.h"

class HighlightBlockData;
class TextDisplay;

//! display block delimiters
class BlockDelimiterDisplay: public QObject, public Counter 
{

  //! Qt meta object
  Q_OBJECT

  public:
  
  //! constructor
  BlockDelimiterDisplay(TextDisplay*);

  //! destructor
  virtual ~BlockDelimiterDisplay();
  
  //! block delimiters
  bool setBlockDelimiters( const BlockDelimiter::List& delimiters )
  {
    if( delimiters == delimiters_ ) return false;
    delimiters_ = delimiters; 
    return true;
  }
  
  //! block delimiters
  const BlockDelimiter::List& blockDelimiters( void ) const
  { return delimiters_; }
  
  //! synchronization
  void synchronize( const BlockDelimiterDisplay* );
      
  //! number of collapsed block until given block ID
  unsigned int collapsedBlockCount( const int& block ) const
  { return ( collapsed_blocks_.empty() ) ? 0 : collapsed_blocks_.lower_bound( block )->second; }

  //! set width
  void setWidth( const int& );
  
  //! width
  const int& width( void ) const
  { return width_; }
  
  //! custom delimiter symbols
  void setCustomSymbols( bool value )
  { custom_symbols_ = value; }
  
  //! paint
  virtual void paint( QPainter& );

  //! mouse press event
  virtual void mousePressEvent( QMouseEvent* );
  
  //!@name actions
  //@{
  
  //! set action visibility
  void setActionVisibility( const bool& state );
  
  //! expand current block
  QAction& collapseCurrentAction( void ) const
  { return *collapse_current_action_; }
  
  //! expand current block
  QAction& expandCurrentAction( void ) const
  { return *expand_current_action_; }

  //! collapse top level block
  QAction& collapseAction( void ) const
  { return *collapse_action_; }
  
  //! expand all
  QAction& expandAllAction( void ) const
  { return *expand_all_action_; }

  //@}
    
  //! offset
  void setOffset( int offset ) 
  { offset_ = offset; }
  
  public slots:
  
  //! update expand/collapse current block action state
  void updateCurrentBlockActionState( void );
      
  private slots:
  
  //! collapse current block
  void _collapseCurrentBlock( void );
  
  //! expand current block
  void _expandCurrentBlock( void );

  //! collapse top level block
  void _collapseTopLevelBlocks( void );

  //! expand all blocks
  void _expandAllBlocks( void );  
  
  //! need update
  void _needUpdate( void );
    
  //! contents changed
  void _contentsChanged( void );
  
  //! block count changed
  void _blockCountChanged( void );
  
  private:
  
  //! install actions
  void _installActions( void );
  
  //! synchronize BlockFormats and BlockData
  void _synchronizeBlockData( void ) const;

  //! update segments
  void _updateSegments( void );
    
  //! offest
  const int& _offset( void ) const
  { return offset_; }
  
  //! block marker type
  enum BlockMarkerType
  {
    BEGIN,
    END
  };
 
  //! update segment markers
  void _updateSegmentMarkers( void );
  
  //! update segment markers
  void _updateMarker( QTextBlock&, unsigned int&, BlockMarker&, const BlockMarkerType& flag ) const;

  //! block pair
  typedef std::pair<QTextBlock, QTextBlock> TextBlockPair;

  //! find blocks that match a given segment
  TextBlockPair _findBlocks( const BlockDelimiterSegment&, HighlightBlockData*& ) const;  
  
  //! find blocks that match a given segment
  /*! 
  \param block running block used to parse the document
  \param segment the segment to be found
  \param data the user data associated to the output segment
  */
  TextBlockPair _findBlocks( QTextBlock&, unsigned int&, const BlockDelimiterSegment&, HighlightBlockData*& ) const;  
  
  //! expand current block
  void _expand( const QTextBlock&, HighlightBlockData*, const bool& recursive = false ) const;

  //! collapse blocks 
  void _collapse( const QTextBlock&, const QTextBlock&, HighlightBlockData* ) const;

  //! get collapsed data for all blocs between first and second argument
  CollapsedBlockData _collapsedData( const QTextBlock&, const QTextBlock& ) const;
  
  //! editor
  TextDisplay& _editor( void ) const
  { return *editor_; }
  
  //! draw delimiter
  void _drawDelimiter( QPainter& painter, const QRect& rect, const bool& collapsed ) const;

  //! associated editor
  TextDisplay* editor_;
  
  //! block delimiters
  BlockDelimiter::List delimiters_;
  
  //! block segments
  BlockDelimiterSegment::List segments_;
  
  //! map block id and number of collapsed blocks
  typedef std::map<int, int> CollapsedBlockMap;
  
  //! map block id and number of collapsed blocks
  CollapsedBlockMap collapsed_blocks_;
   
  //! true when _updateSegments needs to be called in paintEvent
  bool need_update_; 
    
  //! use custom block delimiters
  bool custom_symbols_;
  
  //! foreground color
  QColor foreground_;
  
  //! background color
  QColor background_;
  
  //!@name marker dimension
  //@{

  int offset_;
  int width_;
  int half_width_;
  int top_;
  int rect_top_left_;
  int rect_width_;
  int marker_bottom_right_;
  //@}
  
  //!@name actions
  //@{
  
  //! collapse current block
  QAction* collapse_current_action_;
  
  //! expand current block
  QAction* expand_current_action_;
  
  //! collapse top level blocks
  QAction* collapse_action_;

  //! expand all
  QAction* expand_all_action_;  
  
  //@}
      
};

#endif
