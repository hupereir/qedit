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
*******************************************************************************/

/*!
\file DocumentClass.cpp
\brief Highlight and indentation patterns for a given document class
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <QDomComment>

#include <algorithm>
#include "DocumentClass.h"
#include "HighlightPattern.h"
#include "Str.h"
#include "XmlDef.h"
#include "XmlOption.h"
#include "XmlString.h"



//________________________________________________________
DocumentClass::DocumentClass( void ):
    Counter( "DocumentClass" ),
    default_( false ),
    wrap_( false ),
    emulateTabs_( false ),
    tabSize_( 2 ),
    baseIndentation_( 0 ),
    modified_( false )
{ Debug::Throw( "DocumentClass::DocumentClass.\n" ); }

//________________________________________________________
DocumentClass::DocumentClass( const QDomElement& element ):
    Counter( "DocumentClass" ),
    default_( false ),
    wrap_( false ),
    emulateTabs_( false ),
    tabSize_( 2 ),
    baseIndentation_( 0 ),
    modified_( false )
{
    Debug::Throw( "DocumentClass::DocumentClass.\n" );

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( unsigned int i=0; i<attributes.length(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;

        if( attribute.name() == XML::NAME ) name_ = attribute.value() ;
        else if( attribute.name() == XML::PATTERN ) filePattern_.setPattern( XmlString( attribute.value() ).toText() );
        else if( attribute.name() == XML::FIRSTLINE_PATTERN ) firstlinePattern_.setPattern( XmlString( attribute.value() ).toText() );
        else if( attribute.name() == XML::ICON ) icon_ = XmlString( attribute.value() ).toText();

        // these are kept for backward compatibility
        else if( attribute.name() == XML::OPTIONS )
        {

            if( attribute.value().indexOf( XML::OPTION_WRAP, 0, Qt::CaseInsensitive ) >= 0 ) setWrap( true );
            if( attribute.value().indexOf( XML::OPTION_EMULATE_TABS, 0, Qt::CaseInsensitive ) >= 0 ) emulateTabs_ = true;
            if( attribute.value().indexOf( XML::OPTION_DEFAULT, 0, Qt::CaseInsensitive ) >= 0 ) default_ = true;

        } else if( attribute.name() == XML::BASE_INDENTATION ) _setBaseIndentation( attribute.value().toInt() );

        else Debug::Throw(0) << "DocumentClass::DocumentClass - unrecognized attribute: " << attribute.name() << endl;

    }

    // parse children
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement childElement = child_node.toElement();
        if( childElement.isNull() ) continue;

        if( childElement.tagName() == XML::STYLE )
        {

            const HighlightStyle highlightStyle( childElement );
            highlightStyles_.insert( highlightStyle );

        } else if( childElement.tagName() == XML::KEYWORD_PATTERN || childElement.tagName() == XML::RANGE_PATTERN ) {

            HighlightPattern pattern( childElement );
            if( pattern.isValid() ) highlightPatterns_.push_back( pattern );

        } else if( childElement.tagName() == XML::INDENT_PATTERN ) {

            IndentPattern pattern( childElement );
            if( pattern.isValid() ) indentPatterns_.push_back( pattern );

        } else if( childElement.tagName() == XML::PARENTHESIS ) {

            // parenthesis
            textParenthesis_.push_back( TextParenthesis( childElement ) );

        } else if( childElement.tagName() == XML::BLOCK_DELIMITER ) {

            // block delimiters
            blockDelimiters_.push_back( BlockDelimiter( childElement, blockDelimiters_.size() ) );

        } else if( childElement.tagName() == XML::MACRO ) {

            // text macrox
            TextMacro macro( childElement );
            if( macro.isValid() ) textMacros_.push_back( macro );

        } else if( childElement.tagName() == OPTIONS::OPTION ) {

            XmlOption option( childElement );
            if( option.name() == XML::OPTION_WRAP ) setWrap( option.get<bool>() );
            else if( option.name() == XML::OPTION_EMULATE_TABS ) setEmulateTabs( option.get<bool>() );
            else if( option.name() == XML::OPTION_DEFAULT ) setIsDefault( option.get<bool>() );
            else if( option.name() == XML::BASE_INDENTATION ) _setBaseIndentation( option.get<int>() );
            else if( option.name() == XML::TAB_SIZE ) setTabSize( option.get<int>() );
            else Debug::Throw(0) << "DocumentClass::DocumentClass - unrecognized option " << option.name() << endl;

        } else Debug::Throw(0) << "DocumentClass::DocumentClass - unrecognized child " << childElement.tagName() << ".\n";



    }

    // associate elements
    QStringList warnings = associatePatterns();
    for( QStringList::const_iterator iter = warnings.begin(); iter != warnings.end(); ++iter )
    { Debug::Throw(0) << "DocumentClass::DocumentClass - " << *iter << endl; }

}

//______________________________________________________
bool DocumentClass::operator == ( const DocumentClass& other ) const
{

    return
        name() == other.name() &&
        file() == other.file() &&
        fileMatchingPattern() == other.fileMatchingPattern() &&
        firstLineMatchingPattern() == other.firstLineMatchingPattern() &&
        isDefault() == other.isDefault() &&
        wrap() == other.wrap() &&
        emulateTabs() == other.emulateTabs() &&
        tabSize() == other.tabSize() &&
        icon() == other.icon() &&
        highlightStyles() == other.highlightStyles() &&
        highlightPatterns() == other.highlightPatterns() &&
        indentPatterns() == other.indentPatterns() &&
        parenthesis() == other.parenthesis() &&
        blockDelimiters() == other.blockDelimiters() &&
        textMacros() == other.textMacros() &&
        baseIndentation() == other.baseIndentation();

}

//______________________________________________________
QStringList DocumentClass::associatePatterns( void )
{

    Debug::Throw( "DocumentClass::associatePatterns.\n" );
    QStringList out;

    // assign pattern ids
    // warning: the passed Id is converted internaly into a single bit of a bitset
    // to facilitate patterns bitMask
    unsigned int id(0);
    for( HighlightPattern::List::iterator iter = highlightPatterns_.begin(); iter != highlightPatterns_.end(); ++iter, id++ )
    { iter->setId( id ); }

    // create parent/children hierarchy between highlight patterns
    for( HighlightPattern::List::iterator iter = highlightPatterns_.begin(); iter != highlightPatterns_.end(); ++iter )
        if( iter->parent().size() )
    {
        HighlightPattern::List::iterator parent_iter( std::find_if( highlightPatterns_.begin(), highlightPatterns_.end(), HighlightPattern::SameNameFTor( iter->parent() ) ) );
        if( parent_iter != highlightPatterns_.end() )
        {
            iter->setParentId( (*parent_iter).id() );
            (*parent_iter).addChild( *iter );
        } else {
            QString what;
            QTextStream( &what ) << "Unable to find highlight pattern named " << iter->parent();
            out.push_back( what );
        }
    }

    // assign styles to patterns
    for( HighlightPattern::List::iterator iter = highlightPatterns_.begin(); iter != highlightPatterns_.end(); ++iter )
    {
        HighlightStyle::Set::const_iterator styleIter( std::find_if( highlightStyles_.begin(), highlightStyles_.end(), HighlightStyle::SameNameFTor( iter->style() ) ) );
        if( styleIter != highlightStyles_.end() ) iter->setStyle( *styleIter );
        else {
            QString what;
            QTextStream( &what ) << "Unable to find highlight style named " << iter->style().name();
            out.push_back( what );
        }

    }

    return out;

}


//________________________________________________
QDomElement DocumentClass::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "DocumentClass::domElement.\n" );
    QDomElement out( parent.createElement( XML::DOCUMENT_CLASS ) );

    // dump attributes
    out.setAttribute( XML::NAME, name_ );
    if( !filePattern_.isEmpty() ) out.setAttribute( XML::PATTERN, XmlString( filePattern_.pattern() ).toXml() );
    if( !firstlinePattern_.isEmpty() ) out.setAttribute( XML::FIRSTLINE_PATTERN, XmlString( firstlinePattern_.pattern() ).toXml() );

    // icon
    if( !icon().isEmpty() ) out.setAttribute( XML::ICON, icon() );

    // options
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( "Options" ) );
    out.appendChild( XmlOption( XML::OPTION_WRAP, Option().set<bool>( wrap() ) ).domElement( parent ) );
    out.appendChild( XmlOption( XML::OPTION_EMULATE_TABS, Option().set<bool>( emulateTabs() ) ).domElement( parent ) );
    out.appendChild( XmlOption( XML::OPTION_DEFAULT, Option().set<bool>( isDefault() ) ).domElement( parent ) );
    out.appendChild( XmlOption( XML::BASE_INDENTATION, Option().set<int>( baseIndentation() ) ).domElement( parent ) );

    if( tabSize() > 0 )
    { out.appendChild( XmlOption( XML::TAB_SIZE, Option().set<int>( tabSize() ) ).domElement( parent ) ); }

    // dump highlight styles
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( "Highlight styles" ) );
    for( HighlightStyle::Set::const_iterator iter = highlightStyles_.begin(); iter != highlightStyles_.end(); ++iter )
    { out.appendChild( iter->domElement( parent ) ); }

    // dump highlight patterns
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( "Highlight patterns" ) );
    for( HighlightPattern::List::const_iterator iter = highlightPatterns_.begin(); iter != highlightPatterns_.end(); ++iter )
    { out.appendChild( iter->domElement( parent ) ); }

    // dump indent patterns
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( "Indentation patterns" ) );
    for( IndentPattern::List::const_iterator iter = indentPatterns_.begin(); iter != indentPatterns_.end(); ++iter )
    { out.appendChild( iter->domElement( parent ) ); }

    // dump parenthesis
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( "Parenthesis" ) );
    for( TextParenthesis::List::const_iterator iter = textParenthesis_.begin(); iter != textParenthesis_.end(); ++iter )
    { out.appendChild( iter->domElement( parent ) ); }

    // dump block delimiters
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( "Block delimiters" ) );
    for( BlockDelimiter::List::const_iterator iter = blockDelimiters_.begin(); iter != blockDelimiters_.end(); ++iter )
    { out.appendChild( iter->domElement( parent ) ); }

    // dump text macros
    out.appendChild( parent.createTextNode( "\n\n" ) );
    out.appendChild( parent.createComment( "Text macros" ) );
    for( TextMacro::List::const_iterator iter = textMacros_.begin(); iter != textMacros_.end(); ++iter )
    { out.appendChild( iter->domElement( parent ) ); }

    return out;
}

//________________________________________________
void DocumentClass::clear( void )
{

    Debug::Throw( "DocumentClass::clear.\n" );

    highlightStyles_.clear();
    highlightPatterns_.clear();
    indentPatterns_.clear();
    textParenthesis_.clear();
    textMacros_.clear();

}

//________________________________________________________
bool DocumentClass::match( const File& file ) const
{

    Debug::Throw( "DocumentClass::Match.\n" );

    // check if class is default
    if( default_ ) return true;

    // check if file pattern match
    if( !filePattern_.isEmpty() && filePattern_.isValid() && filePattern_.indexIn( file ) >= 0 ) return true;

    // check if first line of file match firstlinePattern_
    QFile in( file );
    if( !firstlinePattern_.isEmpty() && firstlinePattern_.isValid() && in.open( QIODevice::ReadOnly ) )
    {

        QString line;
        static const QRegExp empty_line_regexp( "(^\\s*$)" );
        while(  in.bytesAvailable() && !(line = in.readLine(1024)).isNull() )
        {

            // skip empty lines
            if( line.isEmpty() || empty_line_regexp.indexIn( line ) >= 0 ) continue;

            // check non empty lines
            return ( firstlinePattern_.indexIn( line ) >= 0 );

        }
    }

    // no match
    return false;
}
