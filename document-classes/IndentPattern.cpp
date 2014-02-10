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

#include "IndentPattern.h"
#include "XmlDef.h"
#include "XmlString.h"

//___________________________________________________________________________
unsigned int& IndentPattern::_counter( void )
{
    static unsigned int counter( 0 );
    return counter;
}

//_____________________________________________________
IndentPattern::IndentPattern( void ):
    Counter( "IndentPattern" ),
    id_(0),
    name_( "" ),
    type_( Nothing ),
    scale_( 1 )
{}

//_____________________________________________________
IndentPattern::IndentPattern( const QDomElement& element ):
    Counter( "IndentPattern" ),
    id_( _counter()++ ),
    name_( "" ),
    type_( Nothing ),
    scale_( 1 )
{
    Debug::Throw( "IndentPattern::IndentPattern.\n" );

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( unsigned int i=0; i<attributes.length(); i++ )
    {

        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == Xml::Type ) setType( (Type) attribute.value().toInt() );
        else if( attribute.name() == Xml::Name ) setName( attribute.value() );
        else if( attribute.name() == Xml::Scale ) setScale( attribute.value().toInt() );
        else Debug::Throw(0) << "IndentPattern::IndentPattern - unrecognized attribute: " << attribute.name() << endl;

    }

    // parse children
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement child_element = child_node.toElement();
        if( child_element.isNull() ) continue;
        if( child_element.tagName() == Xml::Rule )
        {
            Rule rule( child_element );
            if( rule.isValid() ) addRule( rule );
        } else if( child_element.tagName() == Xml::Comments ) {

            setComments( XmlString( child_element.text() ).toText() );

        } else Debug::Throw(0) << "IndentPattern::IndentPattern - unrecognized child: " << child_element.tagName() << endl;
    }

    Q_ASSERT( !rules().empty() );

}

//_____________________________________________________
QDomElement IndentPattern::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "IndentPattern::domElement.\n" );
    QDomElement out( parent.createElement( Xml::IndentPattern ) );
    out.setAttribute( Xml::Type, QString::number( type() ) );
    if( !name().isEmpty() ) out.setAttribute( Xml::Name, name() );
    if( scale() > 1 ) out.setAttribute( Xml::Scale, QString::number( scale() ) );

    for( Rule::List::const_iterator iter = rules().begin(); iter != rules().end(); ++iter )
    { out.appendChild( iter->domElement( parent ) ); }

    if( !comments().isEmpty() )
    {
        out.
            appendChild( parent.createElement( Xml::Comments ) ).
            appendChild( parent.createTextNode( XmlString( comments() ).toXml() ) );
    }

    return out;
}

//____________________________________________________________
bool IndentPattern::operator == ( const IndentPattern& other ) const
{
    return
        name() == other.name() &&
        type() == other.type() &&
        scale() == other.scale() &&
        rules() == other.rules() &&
        comments() == other.comments();
}

//____________________________________________________________
QString IndentPattern::typeName( const Type& type )
{
    switch( type )
    {
        default:
        case Nothing: return Xml::IndentNothing;
        case Increment: return Xml::IndentIncrement;
        case Decrement: return Xml::IndentDecrement;
        case DecrementAll: return Xml::IndentDecrementAll;
    }
}

//_____________________________________________________
IndentPattern::Rule::Rule( const QDomElement& element ):
    Counter( "IndentPattern::Rule" ),
    flags_( None )
{
    Debug::Throw( "Rule::Rule.\n" );

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( unsigned int i=0; i<attributes.length(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == Xml::Par ) setParagraph( attribute.value().toInt() );
        else if( attribute.name() == Xml::Options )
        {
            if( attribute.value().indexOf( Xml::OptionNoCase, 0, Qt::CaseInsensitive ) >= 0 ) setFlag( CaseInsensitive, true );
        }
        else Debug::Throw(0) << "Rule::Rule - unrecognized attribute: " << attribute.name() << endl;
    }

    // parse children
    // parse children elements
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement child_element = child_node.toElement();
        if( child_element.tagName() == Xml::RegExp ) setPattern( XmlString( child_element.text() ).toText() );
        else Debug::Throw(0) << "Rule::Rule - unrecognized child: " << child_element.tagName() << endl;
    }

    regexp_.setCaseSensitivity( flag( CaseInsensitive ) ? Qt::CaseInsensitive : Qt::CaseSensitive );

}

//________________________________________________________
QDomElement IndentPattern::Rule::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "IndentPattern::Rule::DomElement.\n" );
    QDomElement out( parent.createElement( Xml::Rule ) );
    out.setAttribute( Xml::Par, QString::number( paragraph() ) );
    if( flag( CaseInsensitive ) )  out.setAttribute( Xml::Options, Xml::OptionNoCase );

    out.
        appendChild( parent.createElement( Xml::RegExp ) ).
        appendChild( parent.createTextNode( XmlString( pattern().pattern() ).toXml() ) );

    return out;
}

//________________________________________________________
bool IndentPattern::Rule::accept( const QString& text ) const
{

    if( !pattern().isValid() ) return true;
    return pattern().indexIn( text ) >= 0;

}
