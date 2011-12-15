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
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
****************************************************************************/

#include "PrintHelper.h"

#include "HighlightBlockData.h"
#include "TextDisplay.h"

#include <QtGui/QTextLayout>
#include <QtGui/QTextLine>

//__________________________________________________________________________
void PrintHelper::print( QPrinter* printer )
{
    Debug::Throw( "PrintHelper::print.\n" );

    // create painter on printer
    QPainter painter;
    painter.begin(printer);

    const QFont font( editor_->font() );
    const QFontMetrics metrics( font, printer );
    const int leading( metrics.leading() );
    const int headerHeight( ( metrics.height() + leading ) * 1.5 );
    const int footerHeight( ( metrics.height() + leading ) * 1.5 );
    const QRect headerRect( 0, 0, printer->pageRect().width(), headerHeight );
    const QRect footerRect( 0, printer->pageRect().height() - footerHeight, printer->pageRect().width(), footerHeight );

    QRect pageRect( headerRect.bottomLeft(), footerRect.topRight() );
    pageRect.adjust( 0, headerRect.height()/2, 0, -footerRect.height()/2 );

    int pageNumber(1);

    // draw first header
    _printHeader( &painter, headerRect, pageNumber );

    // get list of blocks from document
    QPointF position( pageRect.topLeft() );
    for( QTextBlock block( editor_->document()->begin() ); block.isValid(); block = block.next() )
    {

        // construct text layout
        QTextLayout textLayout( block.text(), font, printer );

        // layout text
        textLayout.beginLayout();
        qreal height = 0;
        while (1)
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

        // get highlight block data associated to this block
        HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
        if( data )
        {
            PatternLocationSet patterns( data->locations() );
            for( PatternLocationSet::const_iterator iter = patterns.begin(); iter != patterns.end(); iter++ )
            {
                QTextLayout::FormatRange formatRange;
                formatRange.start = iter->position();
                formatRange.length = iter->length();
                formatRange.format = iter->format();
                formatRanges.push_back( formatRange );
            }

            // save formats
            textLayout.setAdditionalFormats( formatRanges );

        }

        textLayout.endLayout();

        // increase page
        int textLayoutHeight( textLayout.boundingRect().height() );
        if( (position.y() + textLayoutHeight ) > pageRect.bottom() )
        {

            _printFooter( &painter, footerRect, pageNumber );

            position = pageRect.topLeft();
            printer->newPage();

            // increment page number
            pageNumber++;
            _printHeader( &painter, headerRect, pageNumber );

        }

        // render
        textLayout.draw( &painter, position );

        // update position
        position.setY( position.y() + textLayoutHeight );

    }

    // print last page number
    _printFooter( &painter, footerRect, pageNumber );

    painter.end();

}

//___________________________________________________________________________
void PrintHelper::_printHeader( QPainter* painter, const QRect& rect, int pageNumber ) const
{

    Debug::Throw( "TextDisplay::_printHeader.\n" );

    // render page number
    painter->save();
    painter->setPen( QColor( "#888888" ) );
    painter->drawLine( rect.bottomLeft()+QPoint(0,1), rect.bottomRight()+QPoint(0,1) );

    // date
    painter->drawText( rect, Qt::AlignVCenter|Qt::AlignLeft, TimeStamp::now().toString( TimeStamp::DATE ) );

    // draw file name on the center
    if( !( editor_->file().isEmpty() || editor_->isNewDocument() ) )
    { painter->drawText( rect, Qt::AlignCenter, editor_->file().localName() ); }

    // draw page number on the right
    painter->drawText( rect, Qt::AlignVCenter|Qt::AlignRight, QString().setNum( pageNumber ) );
    painter->restore();

}

//___________________________________________________________________________
void PrintHelper::_printFooter( QPainter* painter, const QRect& rect, int pageNumber ) const
{

    Debug::Throw( "TextDisplay::_printFooter.\n" );

    // render page number
    painter->save();
    painter->setPen( QColor( "#888888" ) );
    painter->drawLine( rect.topLeft()-QPoint(0,1), rect.topRight()-QPoint(0,1) );

    // draw file name on the left
    if( !( editor_->file().isEmpty() || editor_->isNewDocument() ) )
    { painter->drawText( rect, Qt::AlignVCenter|Qt::AlignLeft, editor_->file() ); }

    // draw page number on the right
    painter->drawText( rect, Qt::AlignVCenter|Qt::AlignRight, QString().setNum( pageNumber ) );
    painter->restore();

}
