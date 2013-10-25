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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "BlockDelimiter.h"
#include "BlockDelimiterSegment.h"
#include "CollapsedBlockData.h"
#include "Counter.h"

#include <QObject>
#include <QPair>
#include <QMap>

#include <QAction>
#include <QColor>
#include <QFont>
#include <QTextBlock>
#include <QTextCursor>

class HighlightBlockData;
class TextEditor;

//! display block delimiters
class BlockDelimiterDisplay: public QObject, public Counter
{

    //! Qt meta object
    Q_OBJECT

    public:

    //! constructor
    BlockDelimiterDisplay(TextEditor*);

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
    int collapsedBlockCount( const int& block ) const
    {
        int out( 0 );
        for( CollapsedBlockMap::const_iterator iter = collapsedBlocks_.begin(); iter != collapsedBlocks_.end() && iter.key() <= block ; ++iter, ++out )
        {}

        return out;
    }


    //! set width
    void setWidth( const int& );

    //! width
    const int& width( void ) const
    { return width_; }

    //! paint
    virtual void paint( QPainter& );

    //! mouse press event
    virtual void mousePressEvent( QMouseEvent* );

    //! offset
    void setOffset( int offset )
    { offset_ = offset; }

    //! add block delimiters actions to specified menu
    void addActions( QMenu& );

    //! expand all
    QAction& expandAllAction( void ) const
    { return *expandAllAction_; }

    //! update expand/collapse current block action state
    void updateCurrentBlockActionState( void );

    public Q_SLOTS:

    //! need update
    void needUpdate( void );

    //! expand all blocks
    void expandAllBlocks( void );

    private Q_SLOTS:

    //! collapse current block
    void _collapseCurrentBlock( void );

    //! expand current block
    void _expandCurrentBlock( void );

    //! collapse top level block
    void _collapseTopLevelBlocks( void );

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

    //! update segments
    void _updateSegments( bool );

    //! offest
    const int& _offset( void ) const
    { return offset_; }

    //!@name actions
    //@{

    //! expand current block
    QAction& _collapseCurrentAction( void ) const
    { return *collapseCurrentAction_; }

    //! expand current block
    QAction& _expandCurrentAction( void ) const
    { return *expandCurrentAction_; }

    //! collapse top level block
    QAction& _collapseAction( void ) const
    { return *collapseAction_; }

    //@}

    //! block marker type
    enum BlockMarkerType {
        BEGIN,
        END
    };

    //! update segment markers
    void _updateSegmentMarkers( void );

    //! update segment markers
    void _updateMarker( QTextBlock&, unsigned int&, BlockMarker&, const BlockMarkerType& flag ) const;

    //! block pair
    typedef QPair<QTextBlock, QTextBlock> TextBlockPair;

    //! find blocks that match a given segment
    TextBlockPair _findBlocks( const BlockDelimiterSegment&, HighlightBlockData*& ) const;

    //! find blocks that match a given segment
    /*!
    \param block running block used to parse the document
    \param segment the segment to be found
    \param data the user data associated to the output segment
    */
    TextBlockPair _findBlocks( QTextBlock&, unsigned int&, const BlockDelimiterSegment&, HighlightBlockData*& ) const;

    //! select segment from cursor
    void _selectSegmentFromCursor( const int& );

    //! select segment from mouse position
    void _selectSegmentFromPosition( const QPoint& );

    //! selected segment
    void _setSelectedSegment( const BlockDelimiterSegment& segment )
    { selectedSegment_ = segment; }

    //! selected segment
    const BlockDelimiterSegment& _selectedSegment( void ) const
    { return selectedSegment_; }

    //! expand current block
    void _expand( const QTextBlock&, HighlightBlockData*, const bool& recursive = false ) const;

    //! collapse blocks
    void _collapse( const QTextBlock&, const QTextBlock&, HighlightBlockData* ) const;

    //! get collapsed data for all blocs between first and second argument
    CollapsedBlockData _collapsedData( const QTextBlock&, const QTextBlock& ) const;

    //! editor
    TextEditor& _editor( void ) const
    { return *editor_; }

    //! draw delimiter
    void _drawDelimiter( QPainter& painter, const QRect& rect, const bool& collapsed ) const;

    //! associated editor
    TextEditor* editor_;

    //! block delimiters
    BlockDelimiter::List delimiters_;

    //! block segments
    BlockDelimiterSegment::List segments_;

    //! selected block segment
    BlockDelimiterSegment selectedSegment_;

    //! map block id and number of collapsed blocks
    typedef QMap<int, int> CollapsedBlockMap;

    //! map block id and number of collapsed blocks
    CollapsedBlockMap collapsedBlocks_;

    //! true when _updateSegments needs to be called in paintEvent
    bool needUpdate_;

    //! foreground color
    QColor foreground_;

    //! background color
    QColor background_;

    //!@name marker dimension
    //@{

    int offset_;
    int width_;
    int halfWidth_;
    int top_;
    int rectTopLeft_;
    int rectWidth_;
    int markerBottomRight_;
    //@}

    //!@name actions
    //@{

    //! collapse current block
    QAction* collapseCurrentAction_;

    //! expand current block
    QAction* expandCurrentAction_;

    //! collapse top level blocks
    QAction* collapseAction_;

    //! expand all
    QAction* expandAllAction_;

    //@}

};

#endif
