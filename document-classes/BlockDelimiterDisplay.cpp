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

#include "BlockDelimiterDisplay.h"
#include "BlockHighlight.h"
#include "Debug.h"
#include "HighlightBlockData.h"
#include "TextBlockRange.h"
#include "TextEditor.h"
#include "TextHighlight.h"
#include "XmlOptions.h"


#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QTextBlock>

#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QTreeView>

//____________________________________________________________________________
BlockDelimiterDisplay::BlockDelimiterDisplay(TextEditor* editor ):
    QObject( editor ),
    Counter( QStringLiteral("BlockDelimiterDisplay") ),
    editor_( editor )
{

    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::BlockDelimiterDisplay.\n") );

    // actions
    _installActions();

    // connections
    connect( &editor_->wrapModeAction(), &QAction::toggled, this, &BlockDelimiterDisplay::needUpdate );
    connect( editor_->document(), &QTextDocument::blockCountChanged, this, &BlockDelimiterDisplay::_blockCountChanged );
    connect( editor_->document(), &QTextDocument::contentsChanged, this, &BlockDelimiterDisplay::_contentsChanged );

    // initialize width
    // the value is meaningless but needed to avoid early non-initialize variables
    setWidth(0);

}

//__________________________________________
void BlockDelimiterDisplay::synchronize( const BlockDelimiterDisplay* other )
{
    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::synchronize.\n") );

    // copy members
    delimiters_ = other->delimiters_;
    segments_ = other->segments_;
    collapsedBlocks_ = other->collapsedBlocks_;
    needUpdate_ = other->needUpdate_;
    offset_ = other->offset_;

    // geometry
    setWidth( other->width() );

    // re-initialize connections
    connect( editor_->document(), &QTextDocument::blockCountChanged, this, &BlockDelimiterDisplay::_blockCountChanged );
    connect( editor_->document(), &QTextDocument::contentsChanged, this, &BlockDelimiterDisplay::_contentsChanged );

}

//__________________________________________
void BlockDelimiterDisplay::addActions( QMenu& menu )
{

    menu.addAction( collapseCurrentAction_ );
    menu.addAction( collapseAction_ );
    menu.addAction( expandCurrentAction_ );
    menu.addAction( expandAllAction_ );

}

//__________________________________________
void BlockDelimiterDisplay::updateCurrentBlockActionState()
{

    // update segments if needed
    _updateSegments();
    _selectSegmentFromCursor(  editor_->textCursor().position() );

    collapseCurrentAction_->setEnabled( selectedSegment_.isValid() && !selectedSegment_.hasFlag( BlockDelimiterSegment::Collapsed ) );
    expandCurrentAction_->setEnabled( selectedSegment_.isValid() && selectedSegment_.hasFlag( BlockDelimiterSegment::Collapsed ) );

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
    int yOffset = editor_->verticalScrollBar()->value() - editor_->frameWidth();
    int height( editor_->contentsRect().height() );

    // get begin and end cursor positions
    int firstIndex = editor_->cursorForPosition( QPoint( 0, 0 ) ).position();
    int lastIndex = editor_->cursorForPosition( QPoint( 0,  height ) ).position() + 1;

    // add horizontal offset
    painter.translate( offset_, 0 );
    height += yOffset;

    // retrieve matching segments
    const auto& document( *editor_->document() );
    auto block( document.begin() );
    int id( 0 );

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
        { _updateMarker( block, id, current.begin(), Type::BlockBegin ); }

        // try update segments
        if( current.end().cursor() >= firstIndex && current.end().cursor() <= lastIndex )
        {  _updateMarker( block, id, current.end(), Type::BlockEnd ); }

        // skip this segment if included in previous
        if( previous.isValid() && !( current.begin() < previous.begin() || previous.end() < current.end() ) ) continue;
        else previous=current;

        // draw
        int begin( current.begin().isValid() ? current.begin().position()+top_ : 0 );
        int end( ( (!current.empty()) && current.end().isValid() && current.end().position() < height ) ? current.end().position():height );
        painter.drawLine( halfWidth_, begin + halfWidth_, halfWidth_, end );

    }

    // end tick
    for( const auto& segment:segments_ )
    {

        if( segment.end().isValid() && segment.end().cursor() < lastIndex && segment.end().cursor() >= firstIndex && !( segment.hasFlag( BlockDelimiterSegment::BeginOnly ) || segment.empty() ) )
        { painter.drawLine( halfWidth_, segment.end().position(), width_, segment.end().position() ); }

    }

    // begin tick
    for( auto&& segment:segments_ )
    {

        // check validity
        // update active rect
        if( segment.begin().isValid() && segment.begin().cursor() < lastIndex && segment.begin().cursor() >= firstIndex )
        { segment.setActiveRect( QRect( rectTopLeft_, segment.begin().position() + rectTopLeft_, rectWidth_, rectWidth_ ) ); }

    }

    // draw delimiters
    painter.setBrush( foreground_ );
    QPen pen( foreground_ );
    pen.setWidthF( 0.5 );
    painter.setPen( pen );
    painter.setRenderHints( QPainter::Antialiasing );

    for( const auto& segment:segments_ )
    {
        if( segment.begin().isValid() && segment.begin().cursor() < lastIndex && segment.begin().cursor() >= firstIndex )
        { _drawDelimiter( painter, segment.activeRect(), segment.hasFlag( BlockDelimiterSegment::Collapsed ) ); }
    }

}

//________________________________________________________
void BlockDelimiterDisplay::mousePressEvent( QMouseEvent* event )
{

    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::mousePressEvent.\n") );
    event->accept();

    // check mouse event position
    if( event->pos().x() - offset_ > width() ) return;

    // get position from event
    _updateSegments();
    _updateSegmentMarkers();
    _selectSegmentFromPosition( event->pos()+QPoint( -offset_ , editor_->verticalScrollBar()->value() ) );

    // check button
    switch( event->button() )
    {

        // left mouse button collape/expand current block
        case Qt::LeftButton:
        if( selectedSegment_.isValid() )
        {


            /*
            clear box selection
            because it gets corrupted by the collapsed/expand process
            */
            editor_->clearBoxSelection();

            // retrieve matching segments
            HighlightBlockData* data( nullptr );
            TextBlockPair blocks( _findBlocks( selectedSegment_, data ) );

            // check if block is collapsed
            const auto blockFormat( blocks.first.blockFormat() );
            if( blockFormat.boolProperty( TextBlock::Collapsed ) )
            {

                bool cursorVisible( editor_->isCursorVisible() );
                _expand( blocks.first, data );
                if( cursorVisible ) editor_->ensureCursorVisible();

            } else {

                _collapse( blocks, data );

            }

            // force segment update at next update
            needUpdate_ = true;

        }
        break;

        // right mouse button raise menu
        case Qt::RightButton:
        {

            // update collapse and expand current action state
            expandCurrentAction_->setEnabled( selectedSegment_.isValid() && selectedSegment_.hasFlag( BlockDelimiterSegment::Collapsed ) );
            collapseCurrentAction_->setEnabled( selectedSegment_.isValid() && !selectedSegment_.hasFlag( BlockDelimiterSegment::Collapsed ) );
            QMenu menu( editor_ );
            addActions( menu );
            menu.exec( event->globalPos() );

        }
        break;

        default: break;

    }

    return;

}

//________________________________________________________
void BlockDelimiterDisplay::setWidth( int width )
{

    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::setWidth.\n") );

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
void BlockDelimiterDisplay::_contentsChanged()
{

    // if text is wrapped, line number data needs update at next update
    /* note: this could be further optimized if one retrieve the position at which the contents changed occured */
    if( editor_->lineWrapMode() != TextEditor::NoWrap )
    {
        needUpdate_ = true;
        _synchronizeBlockData();
    }

}

//________________________________________________________
void BlockDelimiterDisplay::_blockCountChanged()
{

    // nothing to be done if wrap mode is not NoWrap, because
    // it is handled in the _contentsChanged slot.
    if( editor_->lineWrapMode() == TextEditor::NoWrap )
    {
        needUpdate_ = true;
        _synchronizeBlockData();
    }

}

//________________________________________________________
void BlockDelimiterDisplay::_collapseCurrentBlock()
{
    Debug::Throw(  QStringLiteral("BlockDelimiterDisplay::_collapseCurrentBlock.\n") );

    /* update segments if needed */
    _updateSegments();
    _updateSegmentMarkers();

    if( selectedSegment_.isValid() && !selectedSegment_.hasFlag( BlockDelimiterSegment::Collapsed ) )
    {

        // clear box selection
        editor_->clearBoxSelection();

        // find matching blocks
        HighlightBlockData *data = nullptr;
        TextBlockPair blocks( _findBlocks( selectedSegment_, data ) );

        // collapse
        _collapse( blocks, data );

    }

    return;
}

//________________________________________________________
void BlockDelimiterDisplay::_expandCurrentBlock()
{

    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::_expandCurrentBlock.\n") );

    /* update segments if needed */
    _updateSegments();
    _updateSegmentMarkers();

    if( selectedSegment_.isValid() && selectedSegment_.hasFlag( BlockDelimiterSegment::Collapsed ) )
    {

        // find matching blocks
        HighlightBlockData *data = nullptr;
        TextBlockPair blocks( _findBlocks( selectedSegment_, data ) );

        // collapse
        bool cursorVisible( editor_->isCursorVisible() );
        editor_->clearBoxSelection();
        _expand( blocks.first, data );
        if( cursorVisible ) editor_->ensureCursorVisible();

    }

    return;

}

//________________________________________________________
void BlockDelimiterDisplay::_collapseTopLevelBlocks()
{
    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::_collapseTopLevelBlocks.\n") );

    /* update segments if needed */
    _updateSegments();
    _updateSegmentMarkers();

    // list of QTextCursor needed to remove blocks
    using CursorList=QList<QTextCursor>;
    CursorList cursors;

    // get first block
    auto block( editor_->document()->begin() );
    int id(0);

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
        HighlightBlockData *data = nullptr;
        TextBlockPair blocks( _findBlocks( block, id, current, data ) );

        // do nothing if block is already collapsed
        cursor.setPosition( blocks.first.position(), QTextCursor::MoveAnchor );
        auto blockFormat( cursor.blockFormat() );
        if( blockFormat.boolProperty( TextBlock::Collapsed ) ) continue;

        // update block format
        blockFormat.setProperty( TextBlock::Collapsed, true );
        QVariant variant;
        variant.setValue( _collapsedData( blocks ) );
        blockFormat.setProperty( TextBlock::CollapsedData, variant );
        cursor.setBlockFormat( blockFormat );

        // mark contents dirty to force update of current block
        data->setFlag( TextBlock::BlockModified, true );
        data->setFlag( TextBlock::BlockCollapsed, true );
        editor_->document()->markContentsDirty(blocks.first.position(), blocks.first.length()-1);

        cursor.setPosition( blocks.first.position() + blocks.first.length(), QTextCursor::MoveAnchor );
        if( blocks.second.isValid() ) {

            if( blocks.second.next().isValid() ) cursor.setPosition( blocks.second.position() + blocks.second.length(), QTextCursor::KeepAnchor );
            else cursor.setPosition( blocks.second.position() + blocks.second.length() - 1, QTextCursor::KeepAnchor );

        } else {
            cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor );
        }

        // store cursor in list for removal of the edited text at the end of the loop
        cursors.append( cursor );

    }

    // now remove all text stored in cursor list
    for( const auto& savedCursor:cursors )
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
void BlockDelimiterDisplay::expandAllBlocks()
{

    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::expandAllBlocks.\n") );

    // clear box selection
    editor_->clearBoxSelection();

    /* update segments if needed */
    _updateSegments();
    _updateSegmentMarkers();

    bool cursorVisible( editor_->isCursorVisible() );
    auto& document( *editor_->document() );
    QTextCursor cursor( document.begin() );
    cursor.beginEditBlock();

    for( const auto& block:TextBlockRange( document ) )
    {
        // retrieve data and check if collapsed
        if( block.blockFormat().boolProperty( TextBlock::Collapsed ) )
        {
            auto data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
            _expand( block, data, true );
        }

    }

    cursor.endEditBlock();

    // set cursor position
    if( cursorVisible ) editor_->ensureCursorVisible();

    return;

}

//__________________________________________________________
void BlockDelimiterDisplay::needUpdate()
{ needUpdate_ = true; }

//________________________________________________________
void BlockDelimiterDisplay::_installActions()
{

    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::_installActions.\n") );

    collapseCurrentAction_ = new QAction( tr( "Collapse Current Block" ), this );
    collapseCurrentAction_->setToolTip( tr( "Collapse current collapsed block" ) );
    collapseCurrentAction_->setShortcut( Qt::CTRL + Qt::Key_Minus );
    connect( collapseCurrentAction_, &QAction::triggered, this, &BlockDelimiterDisplay::_collapseCurrentBlock );
    collapseCurrentAction_->setEnabled( false );

    expandCurrentAction_ = new QAction( tr( "Expand Current Block" ), this );
    expandCurrentAction_->setToolTip( tr( "Expand current collapsed block" ) );
    expandCurrentAction_->setShortcut( Qt::CTRL + Qt::Key_Plus );
    connect( expandCurrentAction_, &QAction::triggered, this, &BlockDelimiterDisplay::_expandCurrentBlock );
    expandCurrentAction_->setEnabled( false );

    collapseAction_ = new QAction( tr( "Collapse Top-Level Blocks" ), this );
    collapseAction_->setToolTip( tr( "Collapse all top level blocks" ) );
    collapseAction_->setShortcut( Qt::SHIFT|Qt::CTRL|Qt::Key_Minus );
    connect( collapseAction_, &QAction::triggered, this, &BlockDelimiterDisplay::_collapseTopLevelBlocks );
    collapseAction_->setEnabled( true );

    expandAllAction_ = new QAction( tr( "Expand All Blocks" ), this );
    expandAllAction_->setToolTip( tr( "Expand all collapsed blocks" ) );
    expandAllAction_->setShortcut( Qt::SHIFT|Qt::CTRL|Qt::Key_Plus );
    connect( expandAllAction_, &QAction::triggered, this, &BlockDelimiterDisplay::expandAllBlocks );
    expandAllAction_->setEnabled( false );

}

//________________________________________________________
void BlockDelimiterDisplay::_synchronizeBlockData() const
{

    auto& document( *editor_->document() );
    for( const auto& block:TextBlockRange( document ) )
    {

        // retrieve data and check this block delimiter
        auto data = dynamic_cast<HighlightBlockData*>( block.userData() );
        if( !data ) continue;

        // store collapse state
        auto blockFormat( block.blockFormat() );
        bool collapsed( blockFormat.boolProperty( TextBlock::Collapsed ) );
        if( data->hasFlag( TextBlock::BlockCollapsed ) != collapsed )
        {
            data->setFlag( TextBlock::BlockCollapsed, collapsed );
            data->setFlag( TextBlock::BlockModified, true );
            document.markContentsDirty(block.position(), block.length()-1);
        }

    }

}

//________________________________________________________
void BlockDelimiterDisplay::_updateSegments()
{

    if( !needUpdate_ ) return;
    needUpdate_ = false;
    segments_.clear();

    _updateSegments( false );
    _updateSegments( true );

}

//________________________________________________________
void BlockDelimiterDisplay::_updateSegments( bool isCommented )
{

    // keep track of collapsed blocks
    bool hasCollapsedBlocks( false );
    bool hasExpandedBlocks( false );
    int collapsedBlockCount(0);
    collapsedBlocks_.clear();

    // loop over delimiter types
    bool first( true );
    for( const auto& blockDelimiter:delimiters_ )
    {

        // keep track of all starting points
        BlockDelimiterSegment::List startPoints;
        int blockCount(0);
        QTextDocument &document( *editor_->document() );
        const TextBlockRange range( document );
        for( auto&& iter = range.begin(); iter != range.end(); ++iter, ++blockCount )
        {

            const auto& block( *iter );

            // retrieve data and check this block delimiter
            HighlightBlockData* data = (dynamic_cast<HighlightBlockData*>( block.userData() ) );
            if( !data ) continue;

            // store collapse state
            auto blockFormat( block.blockFormat() );
            const bool collapsed( blockFormat.boolProperty( TextBlock::Collapsed ) );

            // get delimiter data
            auto delimiter( data->delimiters().get( blockDelimiter.id() ) );
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

                if( !(startPoints.empty() ) && ignored == startPoints.back().hasFlag( BlockDelimiterSegment::Ignored ) )
                {
                    // if block is both begin and end, only the begin flag is to be drawn.
                    if( delimiter.begin( isCommented ) ) startPoints.back().setFlag( BlockDelimiterSegment::BeginOnly, true );

                    // store new segment
                    segments_.append( startPoints.back().setEnd( blockEnd ) );
                }

                // pop
                for( int i = 0; i < delimiter.end( isCommented ) && !startPoints.empty() && ignored == startPoints.back().hasFlag( BlockDelimiterSegment::Ignored ); i++ )
                { startPoints.removeLast(); }

            }

            // add segment
            if( collapsed || delimiter.begin( isCommented ) )
            {

                // prepare segment flags
                BlockDelimiterSegment::Flags flags( BlockDelimiterSegment::None );
                if( ignored ) flags |= BlockDelimiterSegment::Ignored;
                if( collapsed ) flags |= BlockDelimiterSegment::Collapsed;

                // if block is collapsed, skip one start point (which is self contained)
                for( int i = 0; i < delimiter.begin( isCommented ); i++ )
                { startPoints.append( BlockDelimiterSegment( blockBegin, blockEnd, flags ) ); }

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
                    segments_.append( BlockDelimiterSegment( blockBegin, blockEnd, flags ) );

                } else hasExpandedBlocks = true;

            }

        }

        // insert the remaining points as empty segments (that will extend to the end of the document)
        /* they are inserted in reverse order to optimize segment drawing in paintEvent */
        BlockDelimiterSegment::ListIterator segmentIter( segments_ );
        segmentIter.toBack();
        while( segmentIter.hasPrevious() ) segments_.append( segmentIter.previous() );

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
    expandAllAction_->setEnabled( hasCollapsedBlocks );
    collapseAction_->setEnabled( hasExpandedBlocks );

}

//________________________________________________________
void BlockDelimiterDisplay::_updateSegmentMarkers()
{

    QTextBlock block( editor_->document()->begin() );
    int id = 0;
    for( auto& segment:segments_ )
    {
        _updateMarker( block, id, segment.begin(), Type::BlockBegin );
        _updateMarker( block, id, segment.end(), Type::BlockEnd );
    }

}

//________________________________________________________
void BlockDelimiterDisplay::_updateMarker( QTextBlock& block, int& id, BlockMarker& marker, Type flag ) const
{

    // find block matching marker id
    if( marker.id() < id ) { for( ; marker.id() < id && block.isValid(); block = block.previous(), id-- ) {} }
    else if( marker.id() > id ) { for( ; marker.id() > id && block.isValid(); block = block.next(), id++ ) {} }

    QRectF rect( editor_->document()->documentLayout()->blockBoundingRect( block ) );
    if( flag == Type::BlockBegin ) { marker.setPosition( (int) block.layout()->position().y() ); }
    else { marker.setPosition( (int) (block.layout()->position().y() + rect.height()) ); }

    return;

}

//_____________________________________________________________________________________
BlockDelimiterDisplay::TextBlockPair BlockDelimiterDisplay::_findBlocks(
    const BlockDelimiterSegment& segment,
    HighlightBlockData*& data ) const
{
    QTextBlock block( editor_->document()->begin() );
    int id( 0 );
    return _findBlocks( block, id, segment, data );
}

//_____________________________________________________________________________________
BlockDelimiterDisplay::TextBlockPair BlockDelimiterDisplay::_findBlocks(
    QTextBlock& block,
    int& id,
    const BlockDelimiterSegment& segment,
    HighlightBlockData*& data ) const
{

    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::_findBlocks.\n") );
    TextBlockPair out;

    // look for first block
    for( ; block.isValid() && id < segment.begin().id(); block = block.next(), id++ )
    {}

    // get data and check
    data = dynamic_cast<HighlightBlockData*>( block.userData() );

    // store
    out.first = block;

    // finish if block is collapsed
    if( block.blockFormat().boolProperty( TextBlock::Collapsed ) )
    { return out; }

    // look for second block
    for( ; block.isValid() && id < segment.end().id(); block = block.next(), id++ )
    {}

    // check if second block is also of "Begin" type
    if( block != out.first )
    {
        auto secondData( dynamic_cast<HighlightBlockData*>( block.userData() ) );
        if( secondData )
        {

            const auto delimiters( secondData->delimiters().get() );
            if( std::any_of( delimiters.begin(), delimiters.end(),
                []( const TextBlock::Delimiter& delimiter ) { return delimiter.begin(); } ) )
            {
                block = block.previous();
                --id;
            }

        }
    }

    // store and return
    out.second = block;
    return out;

}

//________________________________________________________
void BlockDelimiterDisplay::_selectSegmentFromCursor( int cursor )
{
    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::_selectSegmentFromCursor.\n") );
    auto iter = std::find_if( segments_.begin(), segments_.end(), BlockDelimiterSegment::ContainsFTor( cursor ) );
    _setSelectedSegment( iter == segments_.end() ? BlockDelimiterSegment():*iter );
}


//________________________________________________________
void BlockDelimiterDisplay::_selectSegmentFromPosition( QPoint position )
{
    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::_selectSegmentFromPosition.\n") );
    auto iter = std::find_if(
        segments_.begin(), segments_.end(),
        BlockDelimiterSegment::ActiveFTor( position ) );
    _setSelectedSegment( iter == segments_.end() ? BlockDelimiterSegment():*iter );
}

//________________________________________________________________________________________
void BlockDelimiterDisplay::_expand( const QTextBlock& block, HighlightBlockData* data, bool recursive ) const
{

    // retrieve block format
    auto blockFormat( block.blockFormat() );

    // retrieve collapsed block data
    const auto collapsedData( blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );

    // mark contents dirty to force update of current block
    data->setFlag( TextBlock::BlockModified, true );
    data->setFlag( TextBlock::BlockCollapsed, false );

    editor_->document()->markContentsDirty(block.position(), block.length()-1);

    // create cursor
    QTextCursor cursor( block );
    cursor.beginEditBlock();
    cursor.setPosition( block.position() + block.length() - 1, QTextCursor::MoveAnchor );

    // update collapsed flag associated to data
    blockFormat.setProperty( TextBlock::Collapsed, false );
    cursor.setBlockFormat( blockFormat );

    for( const auto& data:collapsedData.children() )
    {

        cursor.insertBlock();
        cursor.insertText( data.text() );

        // update text block format
        auto blockFormat( cursor.blockFormat() );
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
            auto curentData =  new HighlightBlockData;
            cursor.block().setUserData( curentData );
            _expand( cursor.block(), curentData, true );
        }

    }

    cursor.endEditBlock();

}

//________________________________________________________________________________________
void BlockDelimiterDisplay::_collapse( const BlockDelimiterDisplay::TextBlockPair& blocks, HighlightBlockData* data ) const
{

    Debug::Throw( QStringLiteral("BlockDelimiterDisplay::_collapse.\n") );

    // create cursor and move at end of block
    QTextCursor cursor( blocks.first );

    // update block format
    auto blockFormat( cursor.blockFormat() );
    blockFormat.setProperty( TextBlock::Collapsed, true );

    QVariant variant;
    variant.setValue( _collapsedData( blocks ) );
    blockFormat.setProperty( TextBlock::CollapsedData, variant );

    // mark contents dirty to force update of current block
    data->setFlag( TextBlock::BlockModified, true );
    data->setFlag( TextBlock::BlockCollapsed, true );

    // start edition
    cursor.beginEditBlock();
    cursor.setBlockFormat( blockFormat );

    cursor.setPosition( blocks.first.position() + blocks.first.length(), QTextCursor::MoveAnchor );
    if( blocks.second.isValid() )
    {

        if( blocks.second.next().isValid() ) cursor.setPosition( blocks.second.position() + blocks.second.length(), QTextCursor::KeepAnchor );
        else cursor.setPosition( blocks.second.position() + blocks.second.length() - 1, QTextCursor::KeepAnchor );

    } else {

        cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor );

    }

    cursor.removeSelectedText();
    cursor.endEditBlock();

    // mark contents dirty to force update
    editor_->document()->markContentsDirty(blocks.first.position(), blocks.first.length()-1);

}

//________________________________________________________________________________________
CollapsedBlockData BlockDelimiterDisplay::_collapsedData( const BlockDelimiterDisplay::TextBlockPair& blocks ) const
{

    CollapsedBlockData collapsedData;
    TextBlock::Delimiter::List collapsedDelimiters;
    if( blocks.second != blocks.first )
    {
        for( const auto& block:TextBlockRange( blocks.first.next(), blocks.second.next() ) )
        {

            // break on invalid block
            if( !block.isValid() ) break;

            // create collapse block data
            CollapsedBlockData currentCollapsedData( block );

            // update collapsed delimiters
            // retrieve the TextBlockData associated to this block
            HighlightBlockData* curentData( dynamic_cast<HighlightBlockData*>( block.userData() ) );
            if( curentData ) { collapsedDelimiters += curentData->delimiters(); }

            // also append possible collapsed delimiters
            collapsedDelimiters += currentCollapsedData.delimiters();

            // append collapsed data
            collapsedData.children().append( currentCollapsedData );

        }

    }

    // store collapsed delimiters in collapsed data
    collapsedData.setDelimiters( collapsedDelimiters );
    return collapsedData;
}

//_________________________________________________________________________
void BlockDelimiterDisplay::_drawDelimiter( QPainter& painter, QRect rect, bool collapsed ) const
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
