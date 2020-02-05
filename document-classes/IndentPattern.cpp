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

#include "IndentPattern.h"
#include "XmlDef.h"
#include "XmlString.h"

//___________________________________________________________________________
int& IndentPattern::_counter()
{
    static int counter( 0 );
    return counter;
}

//_____________________________________________________
IndentPattern::IndentPattern():
    Counter( QStringLiteral("IndentPattern") )
{}

//_____________________________________________________
IndentPattern::IndentPattern( const QDomElement& element ):
    Counter( QStringLiteral("IndentPattern") ),
    id_( _counter()++ )
{
    Debug::Throw( QStringLiteral("IndentPattern::IndentPattern.\n") );

    // parse attributes
    const auto attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {

        const auto attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == Xml::Type ) setType( (Type) attribute.value().toInt() );
        else if( attribute.name() == Xml::Name ) setName( attribute.value() );
        else if( attribute.name() == Xml::Scale ) setScale( attribute.value().toInt() );

    }

    // parse children
    for( auto&& childNode = element.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        const auto childElement = childNode.toElement();
        if( childElement.isNull() ) continue;
        if( childElement.tagName() == Xml::Rule )
        {
            Rule rule( childElement );
            if( rule.isValid() ) addRule( rule );
        }
    }

    Q_ASSERT( !rules().empty() );

}

//_____________________________________________________
QDomElement IndentPattern::domElement( QDomDocument& parent ) const
{
    Debug::Throw( QStringLiteral("IndentPattern::domElement.\n") );
    auto out( parent.createElement( Xml::IndentPattern ) );
    out.setAttribute( Xml::Type, QString::number( Base::toIntegralType(type_) ) );
    if( !name_.isEmpty() ) out.setAttribute( Xml::Name, name_ );
    if( scale_ > 1 ) out.setAttribute( Xml::Scale, QString::number( scale_ ) );

    for( const auto& rule:rules() )
    { out.appendChild( rule.domElement( parent ) ); }

    return out;
}

//____________________________________________________________
bool operator == ( const IndentPattern& first, const IndentPattern& second )
{
    return
        first.name_ == second.name_ &&
        first.type_ == second.type_ &&
        first.scale_ == second.scale_ &&
        first.rules_ == second.rules_;
}

//____________________________________________________________
QString IndentPattern::typeName( Type type )
{
    switch( type )
    {
        default:
        case Type::Nothing: return Xml::IndentNothing;
        case Type::Increment: return Xml::IndentIncrement;
        case Type::Decrement: return Xml::IndentDecrement;
        case Type::DecrementAll: return Xml::IndentDecrementAll;
    }
}

//_____________________________________________________
IndentPattern::Rule::Rule( const QDomElement& element ):
    Counter( QStringLiteral("IndentPattern::Rule") ),
    flags_( None )
{
    Debug::Throw( QStringLiteral("Rule::Rule.\n") );

    // parse attributes
    const auto attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        const auto attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == Xml::Par ) setParagraph( attribute.value().toInt() );
        else if( attribute.name() == Xml::Options )
        {
            if( attribute.value().indexOf( Xml::OptionNoCase, 0, Qt::CaseInsensitive ) >= 0 ) setFlag( CaseInsensitive, true );
        }
    }

    // parse children
    // parse children elements
    for( auto&& childNode = element.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        const auto childElement = childNode.toElement();
        if( childElement.tagName() == Xml::RegExp ) setPattern( XmlString( childElement.text() ) );
    }

    if( flag( CaseInsensitive ) ) regexp_.setPatternOptions( regexp_.patternOptions()|QRegularExpression::CaseInsensitiveOption );
    else regexp_.setPatternOptions( regexp_.patternOptions()&~QRegularExpression::CaseInsensitiveOption );

}

//________________________________________________________
QDomElement IndentPattern::Rule::domElement( QDomDocument& parent ) const
{
    Debug::Throw( QStringLiteral("IndentPattern::Rule::DomElement.\n") );
    auto out( parent.createElement( Xml::Rule ) );
    out.setAttribute( Xml::Par, QString::number( paragraph() ) );
    if( flag( CaseInsensitive ) )  out.setAttribute( Xml::Options, Xml::OptionNoCase );

    out.
        appendChild( parent.createElement( Xml::RegExp ) ).
        appendChild( parent.createTextNode( pattern().pattern() ) );

    return out;
}

//________________________________________________________
bool IndentPattern::Rule::accept( const QString& text ) const
{ return !regexp_.isValid() || text.contains( regexp_ ); }
