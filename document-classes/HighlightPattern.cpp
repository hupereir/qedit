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
QString HighlightPattern::noParentPattern_( QStringLiteral("None") );

//___________________________________________________________________________
HighlightPattern::HighlightPattern():
    Counter( QStringLiteral("HighlightPattern") ),
    name_( QStringLiteral("default") )
{}

//___________________________________________________________________________
HighlightPattern::HighlightPattern( const QDomElement& element ):
    Counter( QStringLiteral("HighlightPattern") ),
    name_( QStringLiteral("default") )
{
    Debug::Throw( QStringLiteral("HighlightPattern::HighlightPattern.\n") );
    if( element.tagName() == Xml::KeywordPattern ) setType( Type::KeywordPattern );
    if( element.tagName() == Xml::RangePattern ) setType( Type::RangePattern );

    const auto attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        const auto attribute( attributes.item( i ).toAttr() );
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
    for( auto&& childNode = element.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        const auto childElement = childNode.toElement();
        if( childElement.isNull() ) continue;
        else if( childElement.tagName() == Xml::Keyword ) setKeyword( XmlString( childElement.text() ).get() );
        else if( childElement.tagName() == Xml::Begin ) setBegin( XmlString( childElement.text() ) );
        else if( childElement.tagName() == Xml::End ) setEnd( XmlString( childElement.text() ) );
    }

}

//______________________________________________________
QDomElement HighlightPattern::domElement( QDomDocument& parent ) const
{
    Debug::Throw( QStringLiteral("HighlightPattern::domElement.\n") );

    auto out( parent.createElement( typeName() ) );
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
bool operator == ( const HighlightPattern& first, const HighlightPattern& second )
{
    return
        first.name_ == second.name_ &&
        first.flags_ == second.flags_ &&
        first.type_ == second.type_ &&
        first.parent_ == second.parent_ &&
        first.style_ == second.style_ &&
        first.keyword_ == second.keyword_ &&
        ( first.type_ != HighlightPattern::Type::RangePattern || first.end_ == second.end_ );
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
bool HighlightPattern::isValid() const
{
    switch( type_ )
    {
        case Type::KeywordPattern: 
        return keyword_.isValid() && !keyword_.pattern().isEmpty();
        
        case Type::RangePattern: 
        return 
            keyword_.isValid() && !keyword_.pattern().isEmpty() &&
            end_.isValid() && !end_.pattern().isEmpty();

        default: return false;
    }
}

//____________________________________________________________
void HighlightPattern::_updatePatternOptions( QRegularExpression& regexp ) const
{
    auto patternOptions( regexp.patternOptions() );
    patternOptions.setFlag( QRegularExpression::CaseInsensitiveOption, hasFlag( CaseInsensitive ) );
    regexp.setPatternOptions( patternOptions );
}

//____________________________________________________________
bool HighlightPattern::_findKeyword( PatternLocationSet& locations, const QString& text, bool& active ) const
{
    // disable activity
    active=false;

    // check RegExp
    if( keyword_.pattern().isEmpty() ) return false;
    
    // process text
    bool found( false );
    auto iter = keyword_.globalMatch( text );
    while( iter.hasNext() )
    {
        const auto match( iter.next() );
        locations.insert( PatternLocation( *this, match.capturedStart(), match.capturedLength() ) );
        found = true;
    }

    return found;
}

//____________________________________________________________
bool HighlightPattern::_findRange( PatternLocationSet& locations, const QString& text, bool& active ) const
{

    // check RegExp
    if( keyword_.pattern().isEmpty() || end_.pattern().isEmpty() ) return false;

    int begin(0);
    int end(0);

    bool found( false );

    // check if pattern spans over paragraphs
    // and was active in previous paragraph
    if( hasFlag( Span ) && active )
    {

        // if active, look for end match
        QRegularExpressionMatch match;
        end = text.indexOf( end_, 0, &match);
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
            end += match.capturedLength();
            locations.insert( PatternLocation( *this, 0, end ) );

        }

    }

    // look for begin and end in current paragraphs
    forever
    {

        // look for begin match
        // start from end index, which is either 0, or the last
        // found end in case of spanning active patterns
        QRegularExpressionMatch beginMatch;
        begin = end;
        begin = text.indexOf( keyword_, begin, &beginMatch );
        if( begin < 0 )
        {
            active = false;
            break;
        }

        // look for end match
        QRegularExpressionMatch endMatch;
        end = text.indexOf( end_, begin, &endMatch );

        // avoid zero length match
        if( begin == end && beginMatch.capturedLength() == endMatch.capturedLength() )
        { end = text.indexOf( end_, begin + beginMatch.capturedLength() ); }

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
        end += endMatch.capturedLength();
        locations.insert( PatternLocation( *this, begin, end-begin ) );

    }

    return found;
}
