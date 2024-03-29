#ifndef BlockDelimiterDisplay_h
#define BlockDelimiterDisplay_h

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
#include <QHash>

#include <QAction>
#include <QColor>
#include <QFont>
#include <QTextBlock>
#include <QTextCursor>

class HighlightBlockData;
class TextEditor;

//* display block delimiters
class BlockDelimiterDisplay: public QObject, private Base::Counter<BlockDelimiterDisplay>
{

    //* Qt meta object
    Q_OBJECT

    public:

    //* constructor
    explicit BlockDelimiterDisplay(TextEditor*);

    //* block delimiters
    bool setBlockDelimiters( const BlockDelimiter::List& delimiters )
    {
        if( delimiters == delimiters_ ) return false;
        delimiters_ = delimiters;
        return true;
    }

    //* block delimiters
    const BlockDelimiter::List& blockDelimiters() const
    { return delimiters_; }

    //* synchronization
    void synchronize( const BlockDelimiterDisplay* );

    //* number of collapsed block until given block ID
    int collapsedBlockCount( int block ) const
    {
        int out( 0 );
        for( CollapsedBlockMap::const_iterator iter = collapsedBlocks_.begin(); iter != collapsedBlocks_.end() && iter.key() <= block ; ++iter, ++out )
        {}

        return out;
    }


    //* set width
    void setWidth( int );

    //* width
    int width() const
    { return width_; }

    //* paint
    void paint( QPainter& );

    //* mouse press event
    void mousePressEvent( QMouseEvent* );

    //* offset
    void setOffset( int offset )
    { offset_ = offset; }

    //* add block delimiters actions to specified menu
    void addActions( QMenu& );

    //* expand all
    QAction& expandAllAction() const
    { return *expandAllAction_; }

    //* update expand/collapse current block action state
    void updateCurrentBlockActionState();

    //* need update
    void needUpdate();

    //* expand all blocks
    void expandAllBlocks();

    private:

    //* collapse current block
    void _collapseCurrentBlock();

    //* expand current block
    void _expandCurrentBlock();

    //* collapse top level block
    void _collapseTopLevelBlocks();

    //* contents changed
    void _contentsChanged();

    //* block count changed
    void _blockCountChanged();

    //* install actions
    void _installActions();

    //* synchronize BlockFormats and BlockData
    void _synchronizeBlockData() const;

    //* update segments
    void _updateSegments();

    //* update segments
    void _updateSegments( bool );

    //* block marker type
    enum class Type
    {
        BlockBegin,
        BlockEnd
    };

    //* update segment markers
    void _updateSegmentMarkers();

    //* update segment markers
    void _updateMarker( QTextBlock&, int&, BlockMarker&, Type flag ) const;

    //* block pair
    using TextBlockPair = QPair<QTextBlock, QTextBlock>;

    //* find blocks that match a given segment
    TextBlockPair _findBlocks( const BlockDelimiterSegment&, HighlightBlockData*& ) const;

    //* find blocks that match a given segment
    /**
    \param block running block used to parse the document
    \param segment the segment to be found
    \param data the user data associated to the output segment
    */
    TextBlockPair _findBlocks( QTextBlock&, int&, const BlockDelimiterSegment&, HighlightBlockData*& ) const;

    //* select segment from cursor
    void _selectSegmentFromCursor( int );

    //* select segment from mouse position
    void _selectSegmentFromPosition( QPoint  );

    //* selected segment
    void _setSelectedSegment( const BlockDelimiterSegment& segment )
    { selectedSegment_ = segment; }

    //* expand current block
    void _expand( const QTextBlock&, HighlightBlockData*, bool recursive = false ) const;

    //* collapse blocks
    void _collapse( const TextBlockPair&, HighlightBlockData* ) const;

    //* get collapsed data for all blocs between first and second argument
    CollapsedBlockData _collapsedData( const TextBlockPair& ) const;

    //* draw delimiter
    void _drawDelimiter( QPainter& painter, QRect rect, bool collapsed ) const;

    //* associated editor
    TextEditor* editor_ = nullptr;

    //* block delimiters
    BlockDelimiter::List delimiters_;

    //* block segments
    BlockDelimiterSegment::List segments_;

    //* selected block segment
    BlockDelimiterSegment selectedSegment_;

    //* map block id and number of collapsed blocks
    using CollapsedBlockMap = QHash<int, int>;

    //* map block id and number of collapsed blocks
    CollapsedBlockMap collapsedBlocks_;

    //* true when _updateSegments needs to be called in paintEvent
    bool needUpdate_ = true;

    //* foreground color
    QColor foreground_;

    //* background color
    QColor background_;

    //*@name marker dimension
    //@{

    int offset_ = 0;
    int width_ = 0;
    int halfWidth_ = 0;
    int top_ = 0;
    int rectTopLeft_ = 0;
    int rectWidth_ = 0;
    int markerBottomRight_ = 0;
    //@}

    //*@name actions
    //@{

    //* collapse current block
    QAction* collapseCurrentAction_ = nullptr;

    //* expand current block
    QAction* expandCurrentAction_ = nullptr;

    //* collapse top level blocks
    QAction* collapseAction_ = nullptr;

    //* expand all
    QAction* expandAllAction_ = nullptr;

    //@}

};

#endif
