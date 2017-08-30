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
    Counter( "TextMacro" ),
    id_( idCounter_++ ),
    name_( "generic" ),
    accelerator_( "" ),
    isSeparator_( false ),
    isAutomatic_( false )
{
    Debug::Throw( "TextMacro::TextMacro.\n" );

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
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
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement child_element = child_node.toElement();
        if( child_element.isNull() ) continue;
        if( child_element.tagName() == Xml::Rule ) addRule( Rule( child_element ) );
    }

    Q_ASSERT( isSeparator() || !rules_.empty() );

}

//_____________________________________________________
QDomElement TextMacro::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "TextMacro::domElement.\n" );
    QDomElement out( parent.createElement( Xml::Macro ) );

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
QAction* TextMacro::action() const
{

    // create action label
    QString label;
    if( isAutomatic() ) label = QString( QObject::tr( "%1 (automatic)" ) ).arg( name() );
    else label = name();

    QAction* out( new QAction( label, 0 ) );
    if( !accelerator().isEmpty() )
    { out->setShortcut( QKeySequence( accelerator() ) ); }

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
    Counter( "TextMacro::Rule" )
{

    Debug::Throw( "TextMacro::Rule::Rule.\n" );

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == Xml::Options )
        {
            if( attribute.value().indexOf( Xml::OptionNoSplit, 0, Qt::CaseInsensitive ) >= 0 ) setNoSplitting();
        }
    }

    // parse children
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement child_element = child_node.toElement();
        if( child_element.isNull() ) continue;
        if( child_element.tagName() == Xml::RegExp ) setPattern( XmlString( child_element.text() ) );
        else if( child_element.tagName() == Xml::Replacement ) setReplaceText( XmlString( child_element.text() ) );
    }

}

//_____________________________________________________
QDomElement TextMacro::Rule::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "TextMacro::Rule::domElement.\n" );

    QDomElement out( parent.createElement( Xml::Rule ) );

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
    Debug::Throw( "TextMacro::Rule::ProcessText.\n" );

    // check validity
    if( !isValid() ) return TextMacro::Result();
    if( noSplitting_ ) return _processText( text, position );
    else {

        int localPosition( position );
        TextMacro::Result out;
        auto lines( text.split( '\n' ) );
        for( auto& line:lines )
        {

            int length = line.length() + 1;
            addToResult( out, _processText( line, localPosition ) );
            localPosition = qMax( -1, localPosition - length );

        }

        text = lines.join( "\n" );
        return out;

    }

}

//_________________________________________________________________________________
TextMacro::Result TextMacro::Rule::_processText( QString& text, int position ) const
{
    TextMacro::Result out;
    for( int currentPosition = 0; ( currentPosition = pattern_.indexIn( text, currentPosition ) ) >= 0; currentPosition += replaceText_.length() )
    {

        Debug::Throw() << "TextMacro::Rule::_processText - position: " << currentPosition << " text: " << text << endl;

        // replacement occured
        out.first = true;

        // replace in text
        text.replace( currentPosition, pattern_.matchedLength(), replaceText_ );

        // update output displacements
        if( position >= 0 && currentPosition <= position + out.second )
        { out.second += replaceText_.length() - qMin( pattern_.matchedLength(), position + out.second - currentPosition ); }

        // end of line pattern must stop after first iteration
        // in order not to enter infinite loop
        if( pattern_.pattern() == "$" ) break;

    }

    return out;

}
