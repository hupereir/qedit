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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "Str.h"
#include "TextMacro.h"
#include "XmlDef.h"
#include "XmlString.h"

#include <QtCore/QStringList>
#include <vector>
#include <cassert>

//___________________________________________________________________________
unsigned int TextMacro::idCounter_ = 0;

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
    for( unsigned int i=0; i<attributes.length(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == XML::NAME ) setName( attribute.value() );
        else if( attribute.name() == XML::ACCELERATOR ) setAccelerator( attribute.value() );
        else if( attribute.name() == XML::OPTIONS )
        {
            if( attribute.value().indexOf( XML::OPTION_SEPARATOR, 0, Qt::CaseInsensitive ) >= 0 ) setIsSeparator();
            else if( attribute.value().indexOf( XML::OPTION_AUTOMATIC, 0, Qt::CaseInsensitive ) >= 0 ) setIsAutomatic();
        } else Debug::Throw(0) << "TextMacro::TextMacro - unrecognized attribute: " << attribute.name() << endl;
    }

    Debug::Throw() << "TextMacro::TextMacro - name: " << name() << endl;

    // parse children
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement child_element = child_node.toElement();
        if( child_element.isNull() ) continue;
        if( child_element.tagName() == XML::RULE ) addRule( Rule( child_element ) );
        else Debug::Throw(0) << "TextMacro::TextMacro - unrecognized child: " << child_element.tagName() << endl;
    }

    assert( isSeparator() || !rules_.empty() );

}

//_____________________________________________________
QDomElement TextMacro::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "TextMacro::domElement.\n" );
    QDomElement out( parent.createElement( XML::MACRO ) );

    // dump attributes
    out.setAttribute( XML::NAME, name() );
    if( !accelerator().isEmpty() ) out.setAttribute( XML::ACCELERATOR, accelerator() );

    QString options;
    QTextStream what( &options );
    if( isSeparator() ) what << XML::OPTION_SEPARATOR;
    if( isAutomatic() ) what << XML::OPTION_AUTOMATIC;
    if( !options.isEmpty() ) out.setAttribute( XML::OPTIONS, options );

    // dump children
    for( Rule::List::const_iterator iter = rules_.begin(); iter != rules_.end(); ++iter )
    { out.appendChild( iter->domElement( parent ) ); }
    return out;
}

//_____________________________________________________
QAction* TextMacro::action( void ) const
{

    // create action label
    QString label;
    QTextStream what( &label );
    what << name();
    if( isAutomatic() ) what << " (automatic)";

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
    for( Rule::List::const_iterator iter = rules_.begin(); iter != rules_.end(); ++iter )
    { out += iter->processText( text, position >= 0 ? position+out.second : position ); }
    return out;

}

//_______________________________________________________
TextMacro::Rule::Rule( const QDomElement& element ):
    Counter( "TextMacro::Rule" ),
    no_splitting_( false )
{

    Debug::Throw( "TextMacro::Rule::Rule.\n" );

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( unsigned int i=0; i<attributes.length(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == XML::OPTIONS )
        {
            if( attribute.value().indexOf( XML::OPTION_NO_SPLIT, 0, Qt::CaseInsensitive ) >= 0 ) setNoSplitting();
        } else Debug::Throw(0) << "TextMacro::Rule::Rule - unrecognized attribute: " << attribute.name() << endl;
    }

    // parse children
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement child_element = child_node.toElement();
        if( child_element.isNull() ) continue;
        if( child_element.tagName() == XML::REGEXP ) setPattern( XmlString( child_element.text() ).toText() );
        else if( child_element.tagName() == XML::REPLACEMENT ) setReplaceText( XmlString( child_element.text() ).toText() );
        else Debug::Throw(0) << "TextMacro::Rule::Rule - unrecognized child: " << child_element.tagName() << endl;
    }

}

//_____________________________________________________
QDomElement TextMacro::Rule::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "TextMacro::Rule::domElement.\n" );

    QDomElement out( parent.createElement( XML::RULE ) );

    // options
    if( no_splitting_ ) out.setAttribute( XML::OPTIONS, XML::OPTION_NO_SPLIT );

    // child
    out.
        appendChild( parent.createElement( XML::REGEXP ) ).
        appendChild( parent.createTextNode( XmlString( pattern().pattern() ).toXml() ) );

    out.
        appendChild( parent.createElement( XML::REPLACEMENT ) ).
        appendChild( parent.createTextNode( XmlString( replaceText() ).toXml() ) );
    return out;
}

//_________________________________________________________________________________
TextMacro::Result TextMacro::Rule::processText( QString& text, int position ) const
{
    Debug::Throw( "TextMacro::Rule::ProcessText.\n" );

    // check validity
    if( !isValid() ) return TextMacro::Result();
    if( no_splitting_ ) return _processText( text, position );
    else {

        QStringList lines( text.split( '\n' ) );
        int local_position( position );
        TextMacro::Result out;
        for( QStringList::iterator iter = lines.begin(); iter != lines.end(); ++iter )
        {

            int length = iter->length() + 1;
            out += _processText( *iter, local_position );
            local_position = std::max( -1, local_position - length );

        }

        text = lines.join( "\n" );
        return out;

    }

}

//_________________________________________________________________________________
TextMacro::Result TextMacro::Rule::_processText( QString& text, int position ) const
{
    TextMacro::Result out;
    int current_position( 0 );
    while( ( current_position = pattern_.indexIn( text, current_position ) ) >= 0 )
    {

        Debug::Throw() << "TextMacro::Rule::_processText - position: " << current_position << " text: " << text << endl;

        // replacement occured
        out.first = true;

        // replace in text
        text.replace( current_position, pattern_.matchedLength(), replace_text_ );

        // update output displacements
        if( position >= 0 && current_position <= position + out.second )
        { out.second += replace_text_.length() - std::min( pattern_.matchedLength(), position + out.second - current_position ); }

        // update current position
        current_position += replace_text_.length();

        // end of line pattern must stop after first iteration
        // in order not to enter infinite look
        if( pattern_.pattern() == "$" ) break;

    }

    return out;

}
