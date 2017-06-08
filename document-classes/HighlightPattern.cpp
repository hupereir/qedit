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

#include "HighlightPattern.h"

#include "PatternLocationSet.h"
#include "XmlDef.h"
#include "XmlString.h"

//___________________________________________________________________________
QString HighlightPattern::noParentPattern_( "None" );

//___________________________________________________________________________
HighlightPattern::HighlightPattern( void ):
    Counter( "HighlightPattern" ),
    name_( "default" )
{}

//___________________________________________________________________________
HighlightPattern::HighlightPattern( const QDomElement& element ):
    Counter( "HighlightPattern" ),
    name_( "default" )
{
    Debug::Throw( "HighlightPattern::HighlightPattern.\n" );
    if( element.tagName() == Xml::KeywordPattern ) setType( Type::KeywordPattern );
    if( element.tagName() == Xml::RangePattern ) setType( Type::RangePattern );

    QDomNamedNodeMap attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;

        if( attribute.name() == Xml::Name ) setName( attribute.value() );
        else if( attribute.name() == Xml::Parent ) setParent( attribute.value() );
        else if( attribute.name() == Xml::Style ) setStyle( HighlightStyle( attribute.value() ) );

        else if( attribute.name() == Xml::Options )
        {
            if( attribute.value().indexOf( Xml::OptionSpan, 0, Qt::CaseInsensitive ) >= 0 ) setFlag( Span, true );
            if( attribute.value().indexOf( Xml::OptionNoIndent, 0, Qt::CaseInsensitive ) >= 0 ) setFlag( NoIndent, true );
            if( attribute.value().indexOf( Xml::OptionNoCase, 0, Qt::CaseInsensitive ) >= 0 ) setFlag( CaseInsensitive, true );
            if( attribute.value().indexOf( Xml::OptionComment, 0, Qt::CaseInsensitive ) >= 0 ) setFlag( Comment, true );

        }

    }

    // parse children
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement child_element = child_node.toElement();
        if( child_element.isNull() ) continue;
        else if( child_element.tagName() == Xml::Keyword ) setKeyword( XmlString( child_element.text() ) );
        else if( child_element.tagName() == Xml::Begin ) setBegin( XmlString( child_element.text() ) );
        else if( child_element.tagName() == Xml::End ) setEnd( XmlString( child_element.text() ) );
    }

}

//______________________________________________________
QDomElement HighlightPattern::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "HighlightPattern::domElement.\n" );

    QDomElement out( parent.createElement( typeName() ) );
    out.setAttribute( Xml::Name, name_ );
    out.setAttribute( Xml::Parent, HighlightPattern::parent_ );
    out.setAttribute( Xml::Style, style_.name() );

    // options:
    QString options;
    if( hasFlag( Span ) ) options += Xml::OptionSpan + " ";
    if( hasFlag( NoIndent ) ) options += Xml::OptionNoIndent + " ";
    if( hasFlag( CaseInsensitive ) ) options += Xml::OptionNoCase + " ";
    if( hasFlag( Comment ) ) options += Xml::OptionComment + " ";
    if( !options.isEmpty() ) out.setAttribute( Xml::Options, options );

    // regexps
    if( type_ == Type::KeywordPattern )
    {

        out.
            appendChild( parent.createElement( Xml::Keyword ) ).
            appendChild( parent.createTextNode( keyword_.pattern() ) );

    } else if( type_ == Type::RangePattern ) {

        out.
            appendChild( parent.createElement( Xml::Begin ) ).
            appendChild( parent.createTextNode( keyword_.pattern() ) );
        out.
            appendChild( parent.createElement( Xml::End ) ).
            appendChild( parent.createTextNode( end_.pattern() ) );
    }

    return out;
}

//____________________________________________________________
bool HighlightPattern::operator ==( const HighlightPattern& other ) const
{
    return
        name_ == other.name_ &&
        flags_ == other.flags_ &&
        type_ == other.type_ &&
        parent_ == other.parent_ &&
        style_ == other.style_ &&
        keyword_ == other.keyword_ &&
        ( type_ != Type::RangePattern || end_ == other.end_ );
}

//____________________________________________________________
QString HighlightPattern::typeName( Type type )
{
    switch( type )
    {
        default:
        case Type::KeywordPattern: return Xml::KeywordPattern;
        case Type::RangePattern: return Xml::RangePattern;
    }
}

//____________________________________________________________
bool HighlightPattern::_findKeyword( PatternLocationSet& locations, const QString& text, bool& active ) const
{

    // disable activity
    active=false;

    // check RegExp
    if( keyword_.isEmpty() ) return false;

    // process text
    bool found( false );
    int position( 0 );
    while( position >= 0 )
    {
        position = keyword_.indexIn( text, position );
        if( position >= 0 )
        {
            found = true;
            locations.insert( PatternLocation( *this, position, keyword_.matchedLength() ) );
            position += keyword_.matchedLength();
        }
    }

    return found;

}

//____________________________________________________________
bool HighlightPattern::_findRange( PatternLocationSet& locations, const QString& text, bool& active ) const
{

    // check RegExp
    if( begin().isEmpty() || end().isEmpty() ) return false;

    int begin(0);
    int end(0);

    bool found( false );

    // check if pattern spans over paragraphs
    // and was active in previous paragraph
    if( hasFlag( Span ) && active )
    {

        // if active, look for end match
        end = HighlightPattern::end().indexIn( text, 0 );
        if( end < 0 )
        {

            // no match found.
            // pattern is still active for next paragraph
            // the whole paragraph match the pattern
            locations.insert( PatternLocation( *this, 0, text.size() ) );
            return true;

        } else {

            // found matching end.
            // pattern is not active any more but one needs to check if it does not start again
            active = false;
            found = true;
            end+=HighlightPattern::end().matchedLength();
            locations.insert( PatternLocation( *this, 0, end ) );

        }

    }

    // look for begin and end in current paragraphs
    while( 1 )
    {

        // look for begin match
        // start from end index, which is either 0, or the last
        // found end in case of spanning active patterns
        begin = end;
        begin = HighlightPattern::begin().indexIn( text, begin );
        if( begin < 0 )
        {
            active = false;
            break;
        }

        // look for end match
        end = HighlightPattern::end().indexIn( text, begin );

        // avoid zero length match
        if( begin == end && HighlightPattern::begin().matchedLength() == HighlightPattern::end().matchedLength() )
        { end = HighlightPattern::end().indexIn( text, begin+HighlightPattern::begin().matchedLength() ); }

        if( end < 0 )
        {
            if( hasFlag( Span ) )
            {
                // no end found.
                // Pattern will still be active in next paragraph
                found = true;
                active = true;
                locations.insert( PatternLocation( *this, begin, text.size()-begin ) );
            }

            break;
        }

        // found end matching begin
        // append new text location
        found = true;
        end += HighlightPattern::end().matchedLength();
        locations.insert( PatternLocation( *this, begin, end-begin ) );

    }

    return found;
}
