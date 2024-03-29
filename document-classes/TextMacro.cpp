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

#include "TextMacro.h"
#include "XmlDef.h"
#include "XmlString.h"

#include <QStringList>

//___________________________________________________________________________
int TextMacro::idCounter_ = 0;

//_____________________________________________________
TextMacro::TextMacro( const QDomElement& element ):
    Counter( QStringLiteral("TextMacro") ),
    id_( idCounter_++ ),
    name_( QStringLiteral("generic") )
{
    Debug::Throw( QStringLiteral("TextMacro::TextMacro.\n") );

    // parse attributes
    const auto attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        const auto attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == Xml::Name ) setName( attribute.value() );
        else if( attribute.name() == Xml::Accelerator ) setAccelerator( attribute.value() );
        else if( attribute.name() == Xml::Options )
        {
            if( attribute.value().indexOf( Xml::OptionSeparator, 0, Qt::CaseInsensitive ) >= 0 ) setIsSeparator();
            else if( attribute.value().indexOf( Xml::OptionAutomatic, 0, Qt::CaseInsensitive ) >= 0 ) setIsAutomatic();
        }
    }

    // parse children
    for( auto&& childNode = element.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        const auto childElement = childNode.toElement();
        if( childElement.isNull() ) continue;
        if( childElement.tagName() == Xml::Rule ) addRule( Rule( childElement ) );
    }

    Q_ASSERT( isSeparator() || !rules_.empty() );

}

//_____________________________________________________
QDomElement TextMacro::domElement( QDomDocument& parent ) const
{
    Debug::Throw( QStringLiteral("TextMacro::domElement.\n") );
    auto out( parent.createElement( Xml::Macro ) );

    // dump attributes
    out.setAttribute( Xml::Name, name() );
    if( !accelerator().isEmpty() ) out.setAttribute( Xml::Accelerator, accelerator() );

    QString options;
    QTextStream what( &options );
    if( isSeparator() ) what << Xml::OptionSeparator;
    if( isAutomatic() ) what << Xml::OptionAutomatic;
    if( !options.isEmpty() ) out.setAttribute( Xml::Options, options );

    // dump children
    for( const auto& rule:rules_ )
    { out.appendChild( rule.domElement( parent ) ); }
    return out;
}

//_____________________________________________________
TextMacro::Result TextMacro::processText( QString& text, int position ) const
{

    if( isSeparator() ) return Result();
    Result out;
    for( const auto& rule:rules_ )
    { addToResult( out, rule.processText( text, position >= 0 ? position+out.second : position ) ); }
    return out;

}

//_______________________________________________________
TextMacro::Rule::Rule( const QDomElement& element ):
    Counter( QStringLiteral("TextMacro::Rule") )
{

    Debug::Throw( QStringLiteral("TextMacro::Rule::Rule.\n") );

    // parse attributes
    const auto attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        const auto attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == Xml::Options )
        {
            if( attribute.value().indexOf( Xml::OptionNoSplit, 0, Qt::CaseInsensitive ) >= 0 ) setNoSplitting();
        }
    }

    // parse children
    for( auto&& childNode = element.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        const auto childElement = childNode.toElement();
        if( childElement.isNull() ) continue;
        if( childElement.tagName() == Xml::RegExp ) setPattern( XmlString( childElement.text() ) );
        else if( childElement.tagName() == Xml::Replacement ) setReplaceText( XmlString( childElement.text() ) );
    }

}

//_____________________________________________________
QDomElement TextMacro::Rule::domElement( QDomDocument& parent ) const
{
    Debug::Throw( QStringLiteral("TextMacro::Rule::domElement.\n") );

    auto out( parent.createElement( Xml::Rule ) );

    // options
    if( noSplitting_ ) out.setAttribute( Xml::Options, Xml::OptionNoSplit );

    // child
    out.
        appendChild( parent.createElement( Xml::RegExp ) ).
        appendChild( parent.createTextNode( pattern().pattern() ) );

    out.
        appendChild( parent.createElement( Xml::Replacement ) ).
        appendChild( parent.createTextNode( replaceText() ) );
    return out;
}

//_________________________________________________________________________________
TextMacro::Result TextMacro::Rule::processText( QString& text, int position ) const
{
    Debug::Throw( QStringLiteral("TextMacro::Rule::ProcessText.\n") );

    // check validity
    if( !isValid() ) return TextMacro::Result();
    if( noSplitting_ ) return _processText( text, position );
    else {

        int localPosition( position );
        TextMacro::Result out;
        auto lines( text.split( QLatin1Char('\n') ) );
        for( auto& line:lines )
        {

            int length = line.length() + 1;
            addToResult( out, _processText( line, localPosition ) );
            localPosition = qMax( -1, localPosition - length );

        }

        text = lines.join( QStringLiteral( "\n" ) );
        return out;

    }

}

//_________________________________________________________________________________
TextMacro::Result TextMacro::Rule::_processText( QString& text, int position ) const
{
    TextMacro::Result out;
    QRegularExpressionMatch match;
    for( int currentPosition = 0; ( currentPosition = text.indexOf( pattern_, currentPosition, &match ) ) >= 0; currentPosition += replaceText_.size() - match.capturedLength() )
    {

        // replacement occured
        out.first = true;

        // replace in text
        text.replace( currentPosition, match.capturedLength(), replaceText_ );

        // update output displacements
        if( position >= 0 && currentPosition <= position + out.second )
        { out.second += replaceText_.length() - qMin( match.capturedLength(), position + out.second - currentPosition ); }

        // end of line pattern must stop after first iteration
        // in order not to enter infinite loop
        if( pattern_.pattern() == QStringLiteral( "$" ) ) break;

    }

    return out;

}
