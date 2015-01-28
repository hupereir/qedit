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

#include "HtmlHelper.h"

#include "HtmlTextNode.h"
#include "PatternLocationSet.h"
#include "TextDisplay.h"
#include "TextHighlight.h"

//__________________________________________________________________________
void HtmlHelper::print( QIODevice* device )
{
    Debug::Throw( "HtmlHelper::print.\n" );

    // check editor
    Q_CHECK_PTR( editor_ );

    // retrieve HTML string from current display
    QString htmlString( _htmlString() );
    device->write( _htmlString().toLatin1() );

}

//_____________________________________________________________________
QString HtmlHelper::_htmlString( void )
{

    QDomDocument document( "Html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\"" );

    // html
    QDomElement html = document.appendChild( document.createElement( "Html" ) ).toElement();
    html.setAttribute( "xmlns", "Http://www.w3.org/1999/xhtml" );

    // head
    QDomElement head = html.appendChild( document.createElement( "Head" ) ).toElement();
    QDomElement meta;

    // meta information
    meta = head.appendChild( document.createElement( "meta" ) ).toElement();
    meta.setAttribute( "content", "Text/html; charset=iso-8859-1" );
    meta.setAttribute( "Http-equiv", "Content-Type" );
    meta = head.appendChild( document.createElement( "meta" ) ).toElement();
    meta.setAttribute( "content", "QEdit" );
    meta.setAttribute( "name", "Generator" );

    // title
    QDomElement title = head.appendChild( document.createElement( "Title" ) ).toElement();
    title.appendChild( document.createTextNode( editor_->file() ) );

    // body
    html.
        appendChild( document.createElement( "Body" ) ).
        appendChild( _htmlNode( document ) );

    /*
    the following replacements are needed
    to have correct implementation of leading space characters, tabs
    and end of line
    */
    QString htmlString( document.toString(0) );
    htmlString = htmlString.replace( "</span>\n", "</span>", Qt::CaseInsensitive );
    htmlString = htmlString.replace( "<br/>", "", Qt::CaseInsensitive );
    return htmlString;
}


//_____________________________________________________________________
QDomElement HtmlHelper::_htmlNode( QDomDocument& document )
{

    // clear highlight locations and rehighlight
    QDomElement out = document.createElement( "Pre" );

    int activeId( 0 );

    // loop over text blocks
    for( QTextBlock block = editor_->document()->begin(); block.isValid(); block = block.next() )
    {

        // need to redo highlighting rather that us HighlightBlockData
        // because the latter do not store autospell patterns.
        PatternLocationSet locations;
        if( editor_->textHighlight().isHighlightEnabled() )
        {
            locations = editor_->textHighlight().locationSet( block.text(), activeId );
            activeId = locations.activeId().second;
        }

        // retrieve text
        QString text( block.text() );

        // current pattern
        QDomElement span;
        int currentPatternId = -1;
        bool lineBreak( false );
        int line_index( 0 );

        // parse text
        QString buffer("");
        for( int index = 0; index < text.size(); index++, line_index++ )
        {

            // parse locations
            PatternLocation location;
            PatternLocation::SetIterator iter( locations );
            iter.toBack();
            const PatternLocation::ContainsFTor ftor( index );
            while( iter.hasPrevious() )
            {
                const PatternLocation& current( iter.previous() );
                if( ftor( current ) )
                {
                    location = current;
                    break;
                }
            }

            int patternId( location.id() );
            if( patternId != currentPatternId || index == 0 || lineBreak )
            {

                // append text to current element and reset stream
                if( !buffer.isEmpty() )
                {
                    if( span.isNull() ) span  = out.appendChild( document.createElement( "Span" ) ).toElement();
                    HtmlTextNode( buffer, span, document );
                    if( lineBreak )
                    {
                        out.appendChild( document.createElement( "Br" ) );
                        lineBreak = false;
                        line_index = 0;
                    }
                    buffer = "";
                }

                // update pattern
                currentPatternId = patternId;

                // update current element
                span = out.appendChild( document.createElement( "Span" ) ).toElement();
                if( location.isValid() )
                {

                    // retrieve font format
                    Format::TextFormatFlags format( location.fontFormat() );
                    QString buffer;
                    QTextStream formatStream( &buffer );
                    if( format & Format::Underline ) formatStream << "Text-decoration: underline; ";
                    if( format & Format::Italic ) formatStream << "font-style: italic; ";
                    if( format & Format::Bold ) formatStream << "font-weight: bold; ";
                    if( format & Format::Strike ) formatStream << "Text-decoration: line-through; ";

                    // retrieve color
                    const QColor& color = location.color();
                    if( color.isValid() ) formatStream << "color: " << color.name() << "; ";

                    span.setAttribute( "Style", buffer );

                }
            }

            buffer += text[index];

        }

        if( !buffer.isEmpty() )
        {
            if( span.isNull() ) span  = out.appendChild( document.createElement( "Span" ) ).toElement();
            span.appendChild( document.createTextNode( buffer ) );
        }

        out.appendChild( document.createElement( "Br" ) );

    }

    return out;
}
