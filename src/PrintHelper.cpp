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

#include "PrintHelper.h"

#include "PatternLocationSet.h"
#include "TextDisplay.h"
#include "TextHighlight.h"

#include <QTextLayout>
#include <QTextLine>

//__________________________________________________________________________
void PrintHelper::print( QPrinter* printer )
{
    Debug::Throw( "PrintHelper::print.\n" );

    // check editor
    Q_CHECK_PTR( editor_ );

    // setup
    setupPage( printer );
    setFile( editor_->file() );

    // create painter on printer
    QPainter painter;
    painter.begin(printer);

    // first page
    _newPage( printer, &painter );

    const QFont font( editor_->font() );
    const QFontMetrics metrics( font, printer );
    const int leading( metrics.leading() );
    const QRect pageRect( _pageRect() );

    int activeId( 0 );

    // get list of blocks from document
    QPointF position( pageRect.topLeft() );
    for( QTextBlock block( editor_->document()->begin() ); block.isValid(); block = block.next() )
    {

        // construct text layout
        QTextLayout textLayout( block.text(), font, printer );

        // layout text
        textLayout.beginLayout();
        qreal height(0);
        forever
        {
            QTextLine line = textLayout.createLine();
            if (!line.isValid()) break;

            line.setLineWidth( pageRect.width() );
            height += leading;
            line.setPosition(QPointF(0, height));
            height += line.height();
        }

        // create ranges
        QList<QTextLayout::FormatRange> formatRanges;

        // need to redo highlighting rather that use HighlightBlockData
        // because the latter do not store autospell patterns.
        PatternLocationSet patterns;
        if( editor_->textHighlight().isHighlightEnabled() )
        {
            patterns = editor_->textHighlight().locationSet( block.text(), activeId );
            activeId = patterns.activeId().second;
        }

        // get highlight block data associated to this block
        foreach( const PatternLocation& pattern, patterns )
        {
            QTextLayout::FormatRange formatRange;
            formatRange.start = pattern.position();
            formatRange.length = pattern.length();
            formatRange.format = pattern.format();
            formatRanges << formatRange;
        }

        // save formats
        textLayout.setAdditionalFormats( formatRanges );
        textLayout.endLayout();

        // increase page
        int textLayoutHeight( textLayout.boundingRect().height() );
        if( (position.y() + textLayoutHeight ) > pageRect.bottom() )
        {
            _newPage( printer, &painter );
            position = pageRect.topLeft();
        }

        // render
        textLayout.draw( &painter, position );

        // update position
        position.setY( position.y() + textLayoutHeight );

    }

    painter.end();

}
