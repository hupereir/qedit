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
    const BlockDelimiter::List& blockDelimiters( void ) const
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
    int width( void ) const
    { return width_; }

    //* paint
    virtual void paint( QPainter& );

    //* mouse press event
    virtual void mousePressEvent( QMouseEvent* );

    //* offset
    void setOffset( int offset )
    { offset_ = offset; }

    //* add block delimiters actions to specified menu
    void addActions( QMenu& );

    //* expand all
    QAction& expandAllAction( void ) const
    { return *expandAllAction_; }

    //* update expand/collapse current block action state
    void updateCurrentBlockActionState( void );

    public Q_SLOTS:

    //* need update
    void needUpdate( void );

    //* expand all blocks
    void expandAllBlocks( void );

    private Q_SLOTS:

    //* collapse current block
    void _collapseCurrentBlock( void );

    //* expand current block
    void _expandCurrentBlock( void );

    //* collapse top level block
    void _collapseTopLevelBlocks( void );

    //* contents changed
    void _contentsChanged( void );

    //* block count changed
    void _blockCountChanged( void );

    private:

    //* install actions
    void _installActions( void );

    //* synchronize BlockFormats and BlockData
    void _synchronizeBlockData( void ) const;

    //* update segments
    void _updateSegments( void );

    //* update segments
    void _updateSegments( bool );

    //* offest
    int _offset( void ) const
    { return offset_; }

    //*@name actions
    //@{

    //* expand current block
    QAction& _collapseCurrentAction( void ) const
    { return *collapseCurrentAction_; }

    //* expand current block
    QAction& _expandCurrentAction( void ) const
    { return *expandCurrentAction_; }

    //* collapse top level block
    QAction& _collapseAction( void ) const
    { return *collapseAction_; }

    //@}

    //* block marker type
    enum class Type
    {
        BlockBegin,
        BlockEnd
    };

    //* update segment markers
    void _updateSegmentMarkers( void );

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
    void _selectSegmentFromPosition( const QPoint& );

    //* selected segment
    void _setSelectedSegment( const BlockDelimiterSegment& segment )
    { selectedSegment_ = segment; }

    //* expand current block
    void _expand( const QTextBlock&, HighlightBlockData*, bool recursive = false ) const;

    //* collapse blocks
    void _collapse( const QTextBlock&, const QTextBlock&, HighlightBlockData* ) const;

    //* get collapsed data for all blocs between first and second argument
    CollapsedBlockData _collapsedData( const QTextBlock&, const QTextBlock& ) const;

    //* draw delimiter
    void _drawDelimiter( QPainter& painter, const QRect& rect, bool collapsed ) const;

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
