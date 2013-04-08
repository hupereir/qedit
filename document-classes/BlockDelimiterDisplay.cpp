// $Id$

/*****************************************************************************
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

#include "BlockHighlight.h"
#include "TextEditor.h"
#include "TextHighlight.h"
#include "Debug.h"
#include "BlockDelimiterDisplay.h"
#include "HighlightBlockData.h"
#include "XmlOptions.h"

#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>

#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QTreeView>

//____________________________________________________________________________
BlockDelimiterDisplay::BlockDelimiterDisplay(TextEditor* editor ):
    QObject( editor ),
    Counter( "BlockDelimiterDisplay" ),
    editor_( editor ),
    needUpdate_( true ),
    offset_(0)
{

    Debug::Throw( "BlockDelimiterDisplay::BlockDelimiterDisplay.\n" );

    // actions
    _installActions();

    // connections
    connect( &_editor().wrapModeAction(), SIGNAL( toggled( bool ) ), SLOT( needUpdate() ) );
    connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
    connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );

    // initialize width
    // the value is meaningless but needed to avoid early non-initialize variables
    setWidth(0);

}

//__________________________________________
BlockDelimiterDisplay::~BlockDelimiterDisplay()
{ Debug::Throw( "BlockDelimiterDisplay::~BlockDelimiterDisplay.\n" ); }

//__________________________________________
void BlockDelimiterDisplay::synchronize( const BlockDelimiterDisplay* display )
{
    Debug::Throw( "BlockDelimiterDisplay::synchronize.\n" );

    // copy members
    delimiters_ = display->delimiters_;
    segments_ = display->segments_;
    collapsedBlocks_ = display->collapsedBlocks_;
    needUpdate_ = display->needUpdate_;
    offset_ = display->offset_;

    // geometry
    setWidth( display->width() );

    // re-initialize connections
    connect( _editor().document(), SIGNAL( blockCountChanged( int ) ), SLOT( _blockCountChanged() ) );
    connect( _editor().document(), SIGNAL( contentsChanged() ), SLOT( _contentsChanged() ) );

}

//__________________________________________
void BlockDelimiterDisplay::addActions( QMenu& menu )
{

    menu.addAction( &_collapseCurrentAction() );
    menu.addAction( &_collapseAction() );
    menu.addAction( &_expandCurrentAction() );
    menu.addAction( &expandAllAction() );

}

//__________________________________________
void BlockDelimiterDisplay::updateCurrentBlockActionState( void )
{

    // update segments if needed
    _updateSegments();
    _selectSegmentFromCursor(  _editor().textCursor().position() );

    _collapseCurrentAction().setEnabled( _selectedSegment().isValid() && !_selectedSegment().hasFlag( BlockDelimiterSegment::COLLAPSED ) );
    _expandCurrentAction().setEnabled( _selectedSegment().isValid() && _selectedSegment().hasFlag( BlockDelimiterSegment::COLLAPSED ) );

}

//__________________________________________
void BlockDelimiterDisplay::paint( QPainter& painter )
{

    // check delimiters
    if( delimiters_.empty() ) return;

    // store colors
    foreground_ = painter.pen().color();
    background_ = painter.brush().color();

    /* update segments if needed */
    _updateSegments();

    // calculate dimensions
    int yOffset = _editor().verticalScrollBar()->value() - _editor().frameWidth();
    int height( _editor().contentsRect().height() );

    // get begin and end cursor positions
    int firstIndex = _editor().cursorForPosition( QPoint( 0, 0 ) ).position();
    int lastIndex = _editor().cursorForPosition( QPoint( 0,  height ) ).position() + 1;

    // add horizontal offset
    painter.translate( _offset(), 0 );
    height += yOffset;

    // retrieve matching segments
    QTextDocument &document( *_editor().document() );
    QTextBlock block( document.begin() );
    unsigned int id( 0 );

    // optimize drawing by not drawing overlapping segments
    BlockDelimiterSegment previous;
    BlockDelimiterSegment::MutableListIterator iter( segments_ );
    iter.toBack();
    while( iter.hasPrevious() )
    {
        BlockDelimiterSegment& current( iter.previous() );

        // skip segment if outside of visible limits
        if( current.begin().cursor() > lastIndex+1 || current.end().cursor() < firstIndex ) continue;

        // try update segments
        if( current.begin().cursor() >= firstIndex && current.begin().cursor() <= lastIndex )
        { _updateMarker( block, id, current.begin(), BEGIN ); }

        // try update segments
        if( current.end().cursor() >= firstIndex && current.end().cursor() <= lastIndex )
        {  _updateMarker( block, id, current.end(), END ); }

        // skip this segment if included in previous
        if( previous.isValid() && !( current.begin() < previous.begin() || previous.end() < current.end() ) ) continue;
        else previous=current;

        // draw
        int begin( current.begin().isValid() ? current.begin().position()+top_ : 0 );
        int end( ( (!current.empty()) && current.end().isValid() && current.end().position() < height ) ? current.end().position():height );
        painter.drawLine( halfWidth_, begin + halfWidth_, halfWidth_, end );

    }

    // end tick
    foreach( const BlockDelimiterSegment& segment, segments_ )
    {

        if( segment.end().isValid() && segment.end().cursor() < lastIndex && segment.end().cursor() >= firstIndex && !( segment.hasFlag( BlockDelimiterSegment::BEGIN_ONLY ) || segment.empty() ) )
        { painter.drawLine( halfWidth_, segment.end().position(), width_, segment.end().position() ); }

    }

    // begin tick
    for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); ++iter )
    {

        // check validity
        // update active rect
        if( iter->begin().isValid() && iter->begin().cursor() < lastIndex && iter->begin().cursor() >= firstIndex )
        { iter->setActiveRect( QRect( rectTopLeft_, iter->begin().position() + rectTopLeft_, rectWidth_, rectWidth_ ) ); }

    }

    // draw delimiters
    painter.setBrush( foreground_ );
    QPen pen( foreground_ );
    pen.setWidthF( 0.5 );
    painter.setPen( pen );
    painter.setRenderHints( QPainter::Antialiasing );

    for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); ++iter )
    {
        if( iter->begin().isValid() && iter->begin().cursor() < lastIndex && iter->begin().cursor() >= firstIndex )
        { _drawDelimiter( painter, iter->activeRect(), iter->hasFlag( BlockDelimiterSegment::COLLAPSED ) ); }
    }

}

//________________________________________________________
void BlockDelimiterDisplay::mousePressEvent( QMouseEvent* event )
{

    Debug::Throw( "BlockDelimiterDisplay::mousePressEvent.\n" );
    event->accept();

    // check mouse event position
    if( event->pos().x() - _offset() > width() ) return;

    // get position from event
    _updateSegments();
    _updateSegmentMarkers();
    _selectSegmentFromPosition( event->pos()+QPoint( -_offset(), _editor().verticalScrollBar()->value() ) );

    // check button
    switch( event->button() )
    {

        // left mouse button collape/expand current block
        case Qt::LeftButton:
        if( _selectedSegment().isValid() )
        {


            /*
            clear box selection
            because it gets corrupted by the collapsed/expand process
            */
            _editor().clearBoxSelection();

            // retrieve matching segments
            HighlightBlockData* data(0);
            TextBlockPair blocks( _findBlocks( _selectedSegment(), data ) );

            // check if block is collapsed
            QTextBlockFormat blockFormat( blocks.first.blockFormat() );
            if( blockFormat.boolProperty( TextBlock::Collapsed ) )
            {

                bool cursor_visible( _editor().isCursorVisible() );
                _expand( blocks.first, data );
                if( cursor_visible ) _editor().ensureCursorVisible();

            } else {

                _collapse( blocks.first, blocks.second, data );

            }

            // force segment update at next update
            needUpdate_ = true;

        }
        break;

        // right mouse button raise menu
        case Qt::RightButton:
        {

            // update collapse and expand current action state
            _expandCurrentAction().setEnabled( _selectedSegment().isValid() && _selectedSegment().hasFlag( BlockDelimiterSegment::COLLAPSED ) );
            _collapseCurrentAction().setEnabled( _selectedSegment().isValid() && !_selectedSegment().hasFlag( BlockDelimiterSegment::COLLAPSED ) );
            QMenu menu( &_editor() );
            addActions( menu );
            menu.exec( event->globalPos() );

        }
        break;

        default: break;

    }

    return;

}

//________________________________________________________
void BlockDelimiterDisplay::setWidth( const int& width )
{

    Debug::Throw( "BlockDelimiterDisplay::setWidth.\n" );

    // set dimensions needed to redraw marker and lines
    // this is done to minimize the amount of maths in the paintEvent method
    width_ = width;
    if( !( width_ %2 ) ) { width_++; }

    rectWidth_ = width_ - 4;
    halfWidth_ = int(0.5*width_);
    top_ = 2;
    rectTopLeft_ = 2;

}

//________________________________________________________
void BlockDelimiterDisplay::_contentsChanged( void )
{

    // if text is wrapped, line number data needs update at next update
    /* note: this could be further optimized if one retrieve the position at which the contents changed occured */
    if( _editor().lineWrapMode() != QTextEdit::NoWrap )
    {
        needUpdate_ = true;
        _synchronizeBlockData();
    }

}

//________________________________________________________
void BlockDelimiterDisplay::_blockCountChanged( void )
{

    // nothing to be done if wrap mode is not NoWrap, because
    // it is handled in the _contentsChanged slot.
    if( _editor().lineWrapMode() == QTextEdit::NoWrap )
    {
        needUpdate_ = true;
        _synchronizeBlockData();
    }

}

//________________________________________________________
void BlockDelimiterDisplay::_collapseCurrentBlock( void )
{
    Debug::Throw(  "BlockDelimiterDisplay::_collapseCurrentBlock.\n" );

    /* update segments if needed */
    _updateSegments();
    _updateSegmentMarkers();

    if( _selectedSegment().isValid() && !_selectedSegment().hasFlag( BlockDelimiterSegment::COLLAPSED ) )
    {

        // clear box selection
        _editor().clearBoxSelection();

        // find matching blocks
        HighlightBlockData *data(0);
        TextBlockPair blocks( _findBlocks( _selectedSegment(), data ) );

        // collapse
        _collapse( blocks.first, blocks.second, data );

    }

    return;
}

//________________________________________________________
void BlockDelimiterDisplay::_expandCurrentBlock( void )
{

    Debug::Throw( "BlockDelimiterDisplay::_expandCurrentBlock.\n" );

    /* update segments if needed */
    _updateSegments();
    _updateSegmentMarkers();

    if( _selectedSegment().isValid() && _selectedSegment().hasFlag( BlockDelimiterSegment::COLLAPSED ) )
    {

        // find matching blocks
        HighlightBlockData *data(0);
        TextBlockPair blocks( _findBlocks( _selectedSegment(), data ) );

        // collapse
        bool cursor_visible( _editor().isCursorVisible() );
        _editor().clearBoxSelection();
        _expand( blocks.first, data );
        if( cursor_visible ) _editor().ensureCursorVisible();

    }

    return;

}

//________________________________________________________
void BlockDelimiterDisplay::_collapseTopLevelBlocks( void )
{
    Debug::Throw( "BlockDelimiterDisplay::_collapseTopLevelBlocks.\n" );

    /* update segments if needed */
    _updateSegments();
    _updateSegmentMarkers();

    // list of QTextCursor needed to remove blocks
    typedef QList<QTextCursor> CursorList;
    CursorList cursors;

    // get first block
    QTextBlock block( _editor().document()->begin() );
    unsigned int id(0);

    // create Text cursor
    QTextCursor cursor( block );
    cursor.beginEditBlock();

    // loop over segments in reverse order
    BlockDelimiterSegment previous;
    BlockDelimiterSegment::MutableListIterator iter( segments_ );
    iter.toBack();
    while( iter.hasPrevious() )
    {

        BlockDelimiterSegment& current( iter.previous() );
        if( previous.isValid() && !(current.begin() < previous.begin() || previous.end() < current.end() ) )
        { continue; }

        // update "Previous" segment
        previous = current;

        // get matching blocks
        HighlightBlockData *data(0);
        TextBlockPair blocks( _findBlocks( block, id, current, data ) );

        // do nothing if block is already collapsed
        cursor.setPosition( blocks.first.position(), QTextCursor::MoveAnchor );
        QTextBlockFormat blockFormat( cursor.blockFormat() );
        if( blockFormat.boolProperty( TextBlock::Collapsed ) )
        { continue; }

        // update block format
        blockFormat.setProperty( TextBlock::Collapsed, true );
        QVariant variant;
        variant.setValue( _collapsedData( blocks.first, blocks.second ) );
        blockFormat.setProperty( TextBlock::CollapsedData, variant );
        cursor.setBlockFormat( blockFormat );

        // mark contents dirty to force update of current block
        data->setFlag( TextBlock::MODIFIED, true );
        data->setFlag( TextBlock::COLLAPSED, true );
        _editor().document()->markContentsDirty(blocks.first.position(), blocks.first.length()-1);

        cursor.setPosition( blocks.first.position() + blocks.first.length(), QTextCursor::MoveAnchor );
        if( blocks.second.isValid() ) {

            if( blocks.second.next().isValid() ) cursor.setPosition( blocks.second.position() + blocks.second.length(), QTextCursor::KeepAnchor );
            else cursor.setPosition( blocks.second.position() + blocks.second.length() - 1, QTextCursor::KeepAnchor );

        } else {
            cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor );
        }

        // store cursor in list for removal of the edited text at the end of the loop
        cursors << cursor;

    }

    // now remove all text stored in cursor list
    foreach( const QTextCursor& savedCursor, cursors )
    {
        cursor.setPosition( savedCursor.anchor() );
        cursor.setPosition( savedCursor.position(), QTextCursor::KeepAnchor );
        cursor.removeSelectedText();
    }

    cursor.endEditBlock();

    return;
    //}
}

//________________________________________________________
void BlockDelimiterDisplay::expandAllBlocks( void )
{

    Debug::Throw( "BlockDelimiterDisplay::expandAllBlocks.\n" );

    // clear box selection
    _editor().clearBoxSelection();

    /* update segments if needed */
    _updateSegments();
    _updateSegmentMarkers();

    bool cursor_visible( _editor().isCursorVisible() );
    QTextDocument &document( *_editor().document() );
    QTextCursor cursor( document.begin() );
    cursor.beginEditBlock();

    for( QTextBlock block = document.begin(); block.isValid(); block = block.next() )
    {
        // retrieve data and check if collapsed
        if( block.blockFormat().boolProperty( TextBlock::Collapsed ) )
        {
            HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
            _expand( block, data, true );
        }

    }

    cursor.endEditBlock();

    // set cursor position
    if( cursor_visible ) _editor().ensureCursorVisible();

    return;

}

//__________________________________________________________
void BlockDelimiterDisplay::needUpdate( void )
{ needUpdate_ = true; }

//________________________________________________________
void BlockDelimiterDisplay::_installActions( void )
{

    Debug::Throw( "BlockDelimiterDisplay::_installActions.\n" );

    collapse_currentAction_ = new QAction( tr( "Collapse Current Block" ), this );
    collapse_currentAction_->setToolTip( tr( "Collapse current collapsed block" ) );
    collapse_currentAction_->setShortcut( Qt::CTRL + Qt::Key_Minus );
    connect( collapse_currentAction_, SIGNAL( triggered() ), SLOT( _collapseCurrentBlock() ) );
    collapse_currentAction_->setEnabled( false );

    expand_currentAction_ = new QAction( tr( "Expand Current Block" ), this );
    expand_currentAction_->setToolTip( tr( "Expand current collapsed block" ) );
    expand_currentAction_->setShortcut( Qt::CTRL + Qt::Key_Plus );
    connect( expand_currentAction_, SIGNAL( triggered() ), SLOT( _expandCurrentBlock() ) );
    expand_currentAction_->setEnabled( false );

    collapseAction_ = new QAction( tr( "Collapse Top-Level Blocks" ), this );
    collapseAction_->setToolTip( tr( "Collapse all top level blocks" ) );
    collapseAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_Minus );
    connect( collapseAction_, SIGNAL( triggered() ), SLOT( _collapseTopLevelBlocks() ) );
    collapseAction_->setEnabled( true );

    expandAllAction_ = new QAction( tr( "Expand All Blocks" ), this );
    expandAllAction_->setToolTip( tr( "Expand all collapsed blocks" ) );
    expandAllAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_Plus );
    connect( expandAllAction_, SIGNAL( triggered() ), SLOT( expandAllBlocks() ) );
    expandAllAction_->setEnabled( false );

}

//________________________________________________________
void BlockDelimiterDisplay::_synchronizeBlockData( void ) const
{

    QTextDocument &document( *_editor().document() );
    for( QTextBlock block = document.begin(); block.isValid(); block = block.next() )
    {

        // retrieve data and check this block delimiter
        HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
        if( !data ) continue;

        // store collapse state
        QTextBlockFormat blockFormat( block.blockFormat() );
        bool collapsed( blockFormat.boolProperty( TextBlock::Collapsed ) );
        if( data->hasFlag( TextBlock::COLLAPSED ) != collapsed )
        {
            data->setFlag( TextBlock::COLLAPSED, collapsed );
            data->setFlag( TextBlock::MODIFIED, true );
            document.markContentsDirty(block.position(), block.length()-1);
        }

    }

}

//________________________________________________________
void BlockDelimiterDisplay::_updateSegments( void )
{

    if( !needUpdate_ ) return;
    needUpdate_ = false;
    segments_.clear();

    _updateSegments( false );

    // for now only update non commented blocks.
    // _updateSegments( true );

}

//________________________________________________________
void BlockDelimiterDisplay::_updateSegments( bool isCommented )
{

    // keep track of collapsed blocks
    bool hasCollapsedBlocks( false );
    bool hasExpandedBlocks( false );
    unsigned int collapsedBlockCount(0);
    collapsedBlocks_.clear();

    // loop over delimiter types
    bool first( true );
    foreach( const BlockDelimiter& blockDelimiter, delimiters_ )
    {

        // keep track of all starting points
        BlockDelimiterSegment::List startPoints;
        int blockCount(0);
        QTextDocument &document( *_editor().document() );
        for( QTextBlock block = document.begin(); block.isValid(); block = block.next(), blockCount++ )
        {

            // retrieve data and check this block delimiter
            HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
            if( !data ) continue;

            // store collapse state
            QTextBlockFormat blockFormat( block.blockFormat() );
            const bool collapsed( blockFormat.boolProperty( TextBlock::Collapsed ) );

            // get delimiter data
            TextBlock::Delimiter delimiter( data->delimiters().get( blockDelimiter.id() ) );
            if( collapsed )
            {
                Q_ASSERT( blockFormat.hasProperty( TextBlock::CollapsedData ) );
                CollapsedBlockData collapsedBlockData( blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );
                delimiter += collapsedBlockData.delimiters().get( blockDelimiter.id() );
            }

            // check if something is to be done
            if( !( collapsed || delimiter.begin( isCommented ) || delimiter.end( isCommented ) ) ) continue;

            // get block limits
            BlockMarker blockBegin( blockCount, block.position() );
            BlockMarker blockEnd( blockCount, block.position()+block.length() - 1 );

            // store "Ignore" state
            bool ignored = data->ignoreBlock();
            if( delimiter.end( isCommented ) )
            {

                if( !(startPoints.empty() ) && ignored == startPoints.back().hasFlag( BlockDelimiterSegment::IGNORED ) )
                {
                    // if block is both begin and end, only the begin flag is to be drawn.
                    if( delimiter.begin( isCommented ) ) startPoints.back().setFlag( BlockDelimiterSegment::BEGIN_ONLY, true );

                    // store new segment
                    segments_ << startPoints.back().setEnd( blockEnd );
                }

                // pop
                for( int i = 0; i < delimiter.end( isCommented ) && !startPoints.empty() && ignored == startPoints.back().hasFlag( BlockDelimiterSegment::IGNORED ); i++ )
                { startPoints.removeLast(); }

            }

            // add segment
            if( collapsed || delimiter.begin( isCommented ) )
            {

                // prepare segment flags
                unsigned int flags( BlockDelimiterSegment::NONE );
                if( ignored ) flags |= BlockDelimiterSegment::IGNORED;
                if( collapsed ) flags |= BlockDelimiterSegment::COLLAPSED;

                // if block is collapsed, skip one start point (which is self contained)
                for( int i = 0; i < delimiter.begin( isCommented ); i++ )
                { startPoints << BlockDelimiterSegment( blockBegin, blockEnd, flags ); }

                if( collapsed )
                {

                    // store number of collapsed blocks for the current one
                    if( first )
                    {
                        collapsedBlocks_.insert( blockCount, collapsedBlockCount );
                        collapsedBlockCount += blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>().blockCount() - 1;
                    }

                    // add one self contained segment
                    hasCollapsedBlocks = true;
                    segments_ << BlockDelimiterSegment( blockBegin, blockEnd, flags );

                } else hasExpandedBlocks = true;

            }

        }

        // insert the remaining points as empty segments (that will extend to the end of the document)
        /* they are inserted in reverse order to optimize segment drawing in paintEvent */
        BlockDelimiterSegment::ListIterator segmentIter( segments_ );
        segmentIter.toBack();
        while( segmentIter.hasPrevious() ) segments_ << segmentIter.previous();

        // insert total number of collapsed block as last element
        // this is done only for the first block delimiter pair
        if( first )
        {
            collapsedBlocks_.insert( blockCount, collapsedBlockCount );
            first = false;
        }

    }

    // sort segments so that top level comes last
    std::sort( segments_.begin(), segments_.end(), BlockDelimiterSegment::SortFTor() );

    // update expand all action
    expandAllAction().setEnabled( hasCollapsedBlocks );
    _collapseAction().setEnabled( hasExpandedBlocks );

}

//________________________________________________________
void BlockDelimiterDisplay::_updateSegmentMarkers( void )
{

    QTextBlock block( _editor().document()->begin() );
    unsigned int id = 0;
    for( BlockDelimiterSegment::List::iterator iter = segments_.begin(); iter != segments_.end(); ++iter )
    {
        _updateMarker( block, id, iter->begin(), BEGIN );
        _updateMarker( block, id, iter->end(), END );
    }

}

//________________________________________________________
void BlockDelimiterDisplay::_updateMarker( QTextBlock& block, unsigned int& id, BlockMarker& marker, const BlockMarkerType& flag ) const
{

    // find block matching marker id
    if( marker.id() < id ) { for( ; marker.id() < id && block.isValid(); block = block.previous(), id-- ) {} }
    else if( marker.id() > id ) { for( ; marker.id() > id && block.isValid(); block = block.next(), id++ ) {} }
    Q_ASSERT( block.isValid() );

    QRectF rect( _editor().document()->documentLayout()->blockBoundingRect( block ) );
    if( flag == BEGIN ) { marker.setPosition( (int) block.layout()->position().y() ); }
    else { marker.setPosition( (int) (block.layout()->position().y() + rect.height()) ); }

    return;

}

//_____________________________________________________________________________________
BlockDelimiterDisplay::TextBlockPair BlockDelimiterDisplay::_findBlocks(
    const BlockDelimiterSegment& segment,
    HighlightBlockData*& data ) const
{
    QTextBlock block( _editor().document()->begin() );
    unsigned int id( 0 );
    return _findBlocks( block, id, segment, data );
}

//_____________________________________________________________________________________
BlockDelimiterDisplay::TextBlockPair BlockDelimiterDisplay::_findBlocks(
    QTextBlock& block,
    unsigned int& id,
    const BlockDelimiterSegment& segment,
    HighlightBlockData*& data ) const
{

    Debug::Throw( "BlockDelimiterDisplay::_findBlocks.\n" );
    TextBlockPair out;

    // look for first block
    Q_ASSERT( id <= segment.begin().id() );
    for( ; block.isValid() && id < segment.begin().id(); block = block.next(), id++ )
    {}

    Q_ASSERT( block.isValid() );

    // get data and check
    data = dynamic_cast<HighlightBlockData*>( block.userData() );
    Q_CHECK_PTR( data );

    // store
    out.first = block;

    // finish if block is collapsed
    if( block.blockFormat().boolProperty( TextBlock::Collapsed ) )
    { return out; }

    // look for second block
    for( ; block.isValid() && id < segment.end().id(); block = block.next(), id++ )
    {}

    // check if second block is also of "Begin" type
    // NOTE: I am not completely sure about what this is for
    if( block != out.first )
    {
        HighlightBlockData *secondData( dynamic_cast<HighlightBlockData*>( block.userData() ) );
        if( secondData )
        {

            foreach( const TextBlock::Delimiter counter, secondData->delimiters() )
            {
                if( !counter.begin() ) continue;
                block = block.previous();
                id--;
                break;
            }

        }
    }

    // store and return
    out.second = block;
    return out;

}

//________________________________________________________
void BlockDelimiterDisplay::_selectSegmentFromCursor( const int& cursor )
{
    Debug::Throw( "BlockDelimiterDisplay::_selectSegmentFromCursor.\n" );
    BlockDelimiterSegment::List::iterator iter = std::find_if(
        segments_.begin(), segments_.end(),
        BlockDelimiterSegment::ContainsFTor( cursor ) );
    _setSelectedSegment( iter == segments_.end() ? BlockDelimiterSegment():*iter );

}


//________________________________________________________
void BlockDelimiterDisplay::_selectSegmentFromPosition( const QPoint& position )
{
    Debug::Throw( "BlockDelimiterDisplay::_selectSegmentFromPosition.\n" );
    BlockDelimiterSegment::List::const_iterator iter = std::find_if(
        segments_.begin(), segments_.end(),
        BlockDelimiterSegment::ActiveFTor( position ) );
    _setSelectedSegment( iter == segments_.end() ? BlockDelimiterSegment():*iter );
}

//________________________________________________________________________________________
void BlockDelimiterDisplay::_expand( const QTextBlock& block, HighlightBlockData* data, const bool& recursive ) const
{

    // retrieve block format
    QTextBlockFormat blockFormat( block.blockFormat() );

    // retrieve collapsed block data
    Q_ASSERT( blockFormat.hasProperty( TextBlock::CollapsedData ) );
    CollapsedBlockData collapsedData( blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );

    // mark contents dirty to force update of current block
    data->setFlag( TextBlock::MODIFIED, true );
    data->setFlag( TextBlock::COLLAPSED, false );

    _editor().document()->markContentsDirty(block.position(), block.length()-1);

    // create cursor
    QTextCursor cursor( block );
    cursor.beginEditBlock();
    cursor.setPosition( block.position() + block.length() - 1, QTextCursor::MoveAnchor );

    // update collapsed flag associated to data
    blockFormat.setProperty( TextBlock::Collapsed, false );
    cursor.setBlockFormat( blockFormat );

    foreach( const CollapsedBlockData& data, collapsedData.children() )
    {

        cursor.insertBlock();
        cursor.insertText( data.text() );

        // update text block format
        QTextBlockFormat blockFormat( cursor.blockFormat() );
        blockFormat.setProperty( TextBlock::Collapsed, data.collapsed() );

        if( data.collapsed() ) {
            QVariant variant;
            variant.setValue( data );
            blockFormat.setProperty( TextBlock::CollapsedData, variant );
        }

        cursor.setBlockFormat( blockFormat );

        // also expands block if collapsed and recursive is set to true
        if( data.collapsed() && recursive )
        {
            HighlightBlockData *curentData =  new HighlightBlockData();
            cursor.block().setUserData( curentData );
            _expand( cursor.block(), curentData, true );
        }

    }

    cursor.endEditBlock();

}

//________________________________________________________________________________________
void BlockDelimiterDisplay::_collapse( const QTextBlock& first_block, const QTextBlock& secondBlock, HighlightBlockData* data ) const
{

    Debug::Throw( "BlockDelimiterDisplay::_collapse.\n" );

    // create cursor and move at end of block
    QTextCursor cursor( first_block );

    // update block format
    QTextBlockFormat blockFormat( cursor.blockFormat() );
    blockFormat.setProperty( TextBlock::Collapsed, true );

    QVariant variant;
    variant.setValue( _collapsedData( first_block, secondBlock ) );
    blockFormat.setProperty( TextBlock::CollapsedData, variant );

    // start edition
    cursor.beginEditBlock();
    cursor.setBlockFormat( blockFormat );

    cursor.setPosition( first_block.position() + first_block.length(), QTextCursor::MoveAnchor );
    if( secondBlock.isValid() )
    {

        if( secondBlock.next().isValid() ) cursor.setPosition( secondBlock.position() + secondBlock.length(), QTextCursor::KeepAnchor );
        else cursor.setPosition( secondBlock.position() + secondBlock.length() - 1, QTextCursor::KeepAnchor );

    } else {

        cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor );

    }

    cursor.removeSelectedText();
    cursor.endEditBlock();

    // mark contents dirty to force update of current block
    data->setFlag( TextBlock::MODIFIED, true );
    data->setFlag( TextBlock::COLLAPSED, true );

    // mark contents dirty to force update
    _editor().document()->markContentsDirty(first_block.position(), first_block.length()-1);

}

//________________________________________________________________________________________
CollapsedBlockData BlockDelimiterDisplay::_collapsedData( const QTextBlock& first_block, const QTextBlock& secondBlock ) const
{

    CollapsedBlockData collapsedData;
    TextBlock::Delimiter::List collapsedDelimiters;
    if( secondBlock != first_block )
    {
        for( QTextBlock current = first_block.next(); current.isValid(); current = current.next() )
        {

            // create collapse block data
            CollapsedBlockData currentCollapsedData( current );

            // update collapsed delimiters
            // retrieve the TextBlockData associated to this block
            HighlightBlockData* curentData( dynamic_cast<HighlightBlockData*>( current.userData() ) );
            if( curentData ) { collapsedDelimiters += curentData->delimiters(); }

            // also append possible collapsed delimiters
            collapsedDelimiters += currentCollapsedData.delimiters();

            // append collapsed data
            collapsedData.children() << currentCollapsedData;

            if( current == secondBlock ) break;

        }

    }

    // store collapsed delimiters in collapsed data
    collapsedData.setDelimiters( collapsedDelimiters );
    return collapsedData;
}

//_________________________________________________________________________
void BlockDelimiterDisplay::_drawDelimiter( QPainter& painter, const QRect& rect, const bool& collapsed ) const
{

    QRectF local( rect );
    if( collapsed )
    {

        QPointF points[3] =
        {
            QPointF(local.topLeft()) + QPointF( local.width()*1/6, 0 ),
            QPointF(local.bottomLeft()) + QPointF( local.width()*1/6, 0 ),
            local.topLeft() + QPointF( local.width()*5/6, local.height()/2 )
        };

        for( int i=0; i<3; i++ ) { points[i].setX( 0.5 + int(points[i].x() ) ); }

        painter.drawConvexPolygon(points, 3);

    } else {

        QPointF points[3] =
        {
            QPointF(local.topLeft()) + QPointF( 0, local.height()*1/6 ),
            QPointF(local.topRight()) + QPointF( 0, local.height()*1/6 ),
            local.topLeft() + QPointF( local.width()/2, local.height()*5/6 )
        };

        for( int i=0; i<3; i++ ) { points[i].setY( 0.5 + int(points[i].y() ) ); }

        painter.drawConvexPolygon(points, 3);
    }

    return;
}
