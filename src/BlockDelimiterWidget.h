#ifndef BlockDelimiterWidget_h
#define BlockDelimiterWidget_h

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
  \file BlockDelimiterWidget.h
  \brief display block delimiters
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QAction>
#include <QColor>
#include <QFont>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextFormat>
#include <QWheelEvent>
#include <QWidget>

#include "BlockDelimiter.h"
#include "BlockDelimiterSegment.h"
#include "Counter.h"

class TextBlockData;
class TextDisplay;

//! display block delimiters
class BlockDelimiterWidget: public QWidget, public Counter 
{

  //! Qt meta object
  Q_OBJECT

  public:
  
  //! constructor
  BlockDelimiterWidget(TextDisplay*, QWidget* parent);

  //! destructor
  virtual ~BlockDelimiterWidget();
  
  //! block delimiters
  void setBlockDelimiters( const BlockDelimiter::List& delimiters )
  { delimiters_ = delimiters; }
  
  //! block delimiters
  const BlockDelimiter::List& blockDelimiters( void ) const
  { return delimiters_; }
  
  //! synchronization
  void synchronize( const BlockDelimiterWidget* );
      
  //! number of collapsed block until given block ID
  unsigned int collapsedBlockCount( const int& block ) const
  { return ( collapsed_blocks_.empty() ) ? 0 : collapsed_blocks_.lower_bound( block )->second; }
  
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
    
  public slots:
  
  //! update expand/collapse current block action state
  void updateCurrentBlockActionState( void );
  
  protected:
  
  //! paint
  virtual void paintEvent( QPaintEvent* );

  //! mouse press event
  virtual void mousePressEvent( QMouseEvent* );
    
  //! wheel event
  /*! wheel events are forwarded to the editor */
  virtual void wheelEvent( QWheelEvent* );
  
  private slots:
  
  //! configuration
  void _updateConfiguration( void );
        
  //! collapse current block
  void _collapseCurrentBlock( void );
  
  //! expand current block
  void _expandCurrentBlock( void );

  //! collapse top level block
  void _collapseTopLevelBlocks( void );

//! expand all blocks
  void _expandAllBlocks( void );  
  
  //! need update
  void _needUpdate( void )
  { need_update_ = true; }
    
  //! contents changed
  void _contentsChanged( void );
  
  //! block count changed
  void _blockCountChanged( void );
  
  private:
  
  //! install actions
  void _installActions( void );
  
  //! update segments
  /*! this might go into a slot, linked to document being modified */
  void _updateSegments( void );
  
  //! block pair
  typedef std::pair<QTextBlock, QTextBlock> TextBlockPair;
  
  //! find blocks that match a given segment
  /*! 
  \param block running block used to parse the document
  \param segment the segment to be found
  \param data the user data associated to the output segment
  */
  TextBlockPair _findBlocks( QTextBlock block, const BlockDelimiterSegment& segment, TextBlockData*& data ) const;  
  
  //! expand current block
  void _expand( const QTextBlock&, TextBlockData*, const bool& recursive = false ) const;

  //! collapse blocks 
  void _collapse( const QTextBlock&, const QTextBlock&, TextBlockData* ) const;

  //! editor
  TextDisplay& _editor( void ) const
  { return *editor_; }
  
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
  
  //!@name marker dimension
  //@{

  int width_;
  int half_width_;
  int top_;
  int rect_top_left_;
  int rect_width_;
  int marker_top_left_;
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
