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

#include "DocumentClass.h"
#include "HighlightPattern.h"
#include "XmlDef.h"
#include "XmlOption.h"
#include "XmlString.h"

#include <QRegularExpression>
#include <algorithm>

//________________________________________________________
DocumentClass::DocumentClass():
    Counter( QStringLiteral("DocumentClass") )
{ Debug::Throw( QStringLiteral("DocumentClass::DocumentClass.\n") ); }

//________________________________________________________
DocumentClass::DocumentClass( const QDomElement& element ):
    Counter( QStringLiteral("DocumentClass") )
{
    Debug::Throw( QStringLiteral("DocumentClass::DocumentClass.\n") );

    // parse attributes
    const auto attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        const auto attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;

        if( attribute.name() == Xml::Name ) name_ = attribute.value() ;
        else if( attribute.name() == Xml::Pattern ) filePattern_.setPattern( XmlString( attribute.value() ) );
        else if( attribute.name() == Xml::FirstLinePattern ) firstlinePattern_.setPattern( XmlString( attribute.value() ) );
        else if( attribute.name() == Xml::Icon ) icon_ = XmlString( attribute.value() );

        // these are kept for backward compatibility
        else if( attribute.name() == Xml::Options )
        {

            if( attribute.value().indexOf( Xml::OptionWrap, 0, Qt::CaseInsensitive ) >= 0 ) wrap_ = true;
            if( attribute.value().indexOf( Xml::OptionEmulateTabs, 0, Qt::CaseInsensitive ) >= 0 ) emulateTabs_ = true;
            if( attribute.value().indexOf( Xml::OptionDefault, 0, Qt::CaseInsensitive ) >= 0 ) default_ = true;

        } else if( attribute.name() == Xml::BaseIndentation ) baseIndentation_ = attribute.value().toInt();

    }

    // parse children
    for( auto&& childNode = element.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        const auto childElement = childNode.toElement();
        if( childElement.isNull() ) continue;

        if( childElement.tagName() == Xml::Style )
        {

            const HighlightStyle highlightStyle( childElement );
            highlightStyles_.insert( highlightStyle );

        } else if( childElement.tagName() == Xml::KeywordPattern || childElement.tagName() == Xml::RangePattern ) {

            HighlightPattern pattern( childElement );
            if( pattern.isValid() ) highlightPatterns_.append( pattern );

        } else if( childElement.tagName() == Xml::IndentPattern ) {

            IndentPattern pattern( childElement );
            if( pattern.isValid() ) indentPatterns_.append( pattern );

        } else if( childElement.tagName() == Xml::Parenthesis ) {

            // parenthesis
            textParenthesis_.append( TextParenthesis( childElement ) );

        } else if( childElement.tagName() == Xml::BlockDelimiter ) {

            // block delimiters
            blockDelimiters_.append( BlockDelimiter( childElement, blockDelimiters_.size() ) );

        } else if( childElement.tagName() == Xml::Macro ) {

            // text macrox
            TextMacro macro( childElement );
            if( macro.isValid() ) textMacros_.append( macro );

        } else if( childElement.tagName() == Base::Xml::Option ) {

            XmlOption option( childElement );
            if( option.name() == Xml::OptionWrap ) wrap_ = option.get<bool>();
            else if( option.name() == Xml::OptionEmulateTabs ) emulateTabs_ = option.get<bool>();
            else if( option.name() == Xml::OptionDefault ) default_ = option.get<bool>();
            else if( option.name() == Xml::BaseIndentation ) baseIndentation_ = option.get<int>();
            else if( option.name() == Xml::TabSize ) tabSize_ = option.get<int>();

        }


    }

    // associate elements
    const auto warnings = _associatePatterns();
    for( const auto& warning:warnings )
    { Debug::Throw(0) << "DocumentClass::DocumentClass - " << warning << endl; }

}

//______________________________________________________
bool operator == ( const DocumentClass& first, const DocumentClass& second )
{

    return
        first.name() == second.name() &&
        first.file() == second.file() &&
        first.fileMatchingPattern() == second.fileMatchingPattern() &&
        first.firstLineMatchingPattern() == second.firstLineMatchingPattern() &&
        first.isDefault() == second.isDefault() &&
        first.wrap() == second.wrap() &&
        first.emulateTabs() == second.emulateTabs() &&
        first.tabSize() == second.tabSize() &&
        first.icon() == second.icon() &&
        first.highlightStyles() == second.highlightStyles() &&
        first.highlightPatterns() == second.highlightPatterns() &&
        first.indentPatterns() == second.indentPatterns() &&
        first.parenthesis() == second.parenthesis() &&
        first.blockDelimiters() == second.blockDelimiters() &&
        first.textMacros() == second.textMacros() &&
        first.baseIndentation() == second.baseIndentation();

}

//______________________________________________________
QStringList DocumentClass::_associatePatterns()
{

    Debug::Throw( QStringLiteral("DocumentClass::_associatePatterns.\n") );
    QStringList out;

    // assign pattern ids
    // warning: the passed Id is converted internaly into a single bit of a bitset
    // to facilitate patterns bitMask
    int id(0);
    for( auto& highlightPattern:highlightPatterns_ )
    { highlightPattern.setId( id++ ); }

    // create parent/children hierarchy between highlight patterns
    for( auto& highlightPattern:highlightPatterns_ )
    {
        if( !highlightPattern.parent().isEmpty() )
        {

            auto parentIter( std::find_if( highlightPatterns_.begin(), highlightPatterns_.end(), HighlightPattern::SameNameFTor( highlightPattern.parent() ) ) );
            if( parentIter != highlightPatterns_.end() )
            {
                highlightPattern.setParentId( parentIter->id() );
                parentIter->addChild( highlightPattern );
            } else out << QString( QObject::tr( "Unable to find highlight pattern named %1" ) ).arg( highlightPattern.parent() );

        }

    }

    // assign styles to patterns
    for( auto& pattern:highlightPatterns_ )
    {
        auto styleIter( std::find_if( highlightStyles_.begin(), highlightStyles_.end(), HighlightStyle::SameNameFTor( pattern.style() ) ) );
        if( styleIter != highlightStyles_.end() ) pattern.setStyle( *styleIter );
        else out << QString( QObject::tr( "Unable to find highlight style named %1" ) ).arg( pattern.style().name() );
    }

    return out;

}


//________________________________________________
QDomElement DocumentClass::domElement( QDomDocument& parent ) const
{
    Debug::Throw( QStringLiteral("DocumentClass::domElement.\n") );
    auto out( parent.createElement( Xml::DocumentClass ) );

    // dump attributes
    out.setAttribute( Xml::Name, name_ );
    if( !filePattern_.pattern().isEmpty() ) out.setAttribute( Xml::Pattern, filePattern_.pattern() );
    if( !firstlinePattern_.pattern().isEmpty() ) out.setAttribute( Xml::FirstLinePattern, firstlinePattern_.pattern() );

    // icon
    if( !icon().isEmpty() ) out.setAttribute( Xml::Icon, icon() );

    // options
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( QObject::tr( "Options" ) ) );
    out.appendChild( XmlOption( Xml::OptionWrap, Option().set<bool>( wrap() ) ).domElement( parent ) );
    out.appendChild( XmlOption( Xml::OptionEmulateTabs, Option().set<bool>( emulateTabs() ) ).domElement( parent ) );
    out.appendChild( XmlOption( Xml::OptionDefault, Option().set<bool>( isDefault() ) ).domElement( parent ) );
    out.appendChild( XmlOption( Xml::BaseIndentation, Option().set<int>( baseIndentation() ) ).domElement( parent ) );

    if( tabSize() > 0 )
    { out.appendChild( XmlOption( Xml::TabSize, Option().set<int>( tabSize() ) ).domElement( parent ) ); }

    // dump highlight styles
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( QObject::tr( "Highlight styles" ) ) );
    for( const auto& style:highlightStyles_ )
    { out.appendChild( style.domElement( parent ) ); }

    // dump highlight patterns
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( QObject::tr( "Highlight patterns" ) ) );
    for( const auto& pattern:highlightPatterns_ )
    { out.appendChild( pattern.domElement( parent ) ); }

    // dump indent patterns
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( QObject::tr( "Indentation patterns" ) ) );
    for( const auto& pattern:indentPatterns_ )
    { out.appendChild( pattern.domElement( parent ) ); }

    // dump parenthesis
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( QObject::tr( "Parenthesis" ) ) );
    for( const auto& parenthesis:textParenthesis_ )
    { out.appendChild( parenthesis.domElement( parent ) ); }

    // dump block delimiters
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( QObject::tr( "Block delimiters" ) ) );
    for( const auto& delimiter:blockDelimiters_ )
    { out.appendChild( delimiter.domElement( parent ) ); }

    // dump text macros
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( QObject::tr( "Text macros" ) ) );
    for( const auto& macro:textMacros_ )
    { out.appendChild( macro.domElement( parent ) ); }

    return out;
}

//________________________________________________________
bool DocumentClass::match( const File& file ) const
{

    Debug::Throw( QStringLiteral("DocumentClass::Match.\n") );

    // check if class is default
    if( default_ ) { return true; }

    // check if file pattern match
    if( !filePattern_.pattern().isEmpty() && filePattern_.isValid() && file.contains( filePattern_ ) )
    { return true; }

    // check if first line of file match firstlinePattern_
    QFile in( file );
    if( !firstlinePattern_.pattern().isEmpty() && firstlinePattern_.isValid() && in.open( QIODevice::ReadOnly ) )
    {

        QString line;
        static const QRegularExpression emptyLineRegexp( QStringLiteral("(^\\s*$)") );
        while( in.bytesAvailable() && !(line = in.readLine(1024)).isNull() )
        {

            // skip empty lines
            if( line.isEmpty() || emptyLineRegexp.match( line ).hasMatch() )
            { continue; }

            // check non empty lines
            return ( firstlinePattern_.match( line ).hasMatch() );

        }
    }

    // no match
    return false;
}
