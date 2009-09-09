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

using namespace std;

//________________________________________________________
DocumentClass::DocumentClass( void ):
  Counter( "DocumentClass" ),
  default_( false ),
  wrap_( false ),
  emulate_tabs_( false ),
  tab_size_( 0 ),
  base_indentation_( 0 ),
  modified_( false )
{ Debug::Throw( "DocumentClass::DocumentClass.\n" ); }

//________________________________________________________
DocumentClass::DocumentClass( const QDomElement& element ):
  Counter( "DocumentClass" ),
  default_( false ),
  wrap_( false ),
  emulate_tabs_( false ),
  base_indentation_( 0 ),
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
    else if( attribute.name() == XML::PATTERN ) file_pattern_.setPattern( XmlString( attribute.value() ).toText() );
    else if( attribute.name() == XML::FIRSTLINE_PATTERN ) firstline_pattern_.setPattern( XmlString( attribute.value() ).toText() );
    else if( attribute.name() == XML::ICON ) icon_ = XmlString( attribute.value() ).toText();

    // these are kept for backward compatibility
    else if( attribute.name() == XML::OPTIONS )
    {

      if( attribute.value().indexOf( XML::OPTION_WRAP, 0, Qt::CaseInsensitive ) >= 0 ) setWrap( true );
      if( attribute.value().indexOf( XML::OPTION_EMULATE_TABS, 0, Qt::CaseInsensitive ) >= 0 ) emulate_tabs_ = true;
      if( attribute.value().indexOf( XML::OPTION_DEFAULT, 0, Qt::CaseInsensitive ) >= 0 ) default_ = true;

    } else if( attribute.name() == XML::BASE_INDENTATION ) _setBaseIndentation( attribute.value().toInt() );

    else Debug::Throw(0) << "DocumentClass::DocumentClass - unrecognized attribute: " << attribute.name() << endl;

  }

  // parse children
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
  {
    QDomElement child_element = child_node.toElement();
    if( child_element.isNull() ) continue;

    if( child_element.tagName() == XML::STYLE ) highlight_styles_.insert( HighlightStyle( child_element ) );
    else if( child_element.tagName() == XML::KEYWORD_PATTERN || child_element.tagName() == XML::RANGE_PATTERN )
    {

      HighlightPattern pattern( child_element );
      if( pattern.isValid() ) highlight_patterns_.push_back( pattern );

    } else if( child_element.tagName() == XML::INDENT_PATTERN ) {

      IndentPattern pattern( child_element );
      if( pattern.isValid() ) indent_patterns_.push_back( pattern );

    } else if( child_element.tagName() == XML::PARENTHESIS ) {

      // parenthesis
      text_parenthesis_.push_back( TextParenthesis( child_element ) );

    } else if( child_element.tagName() == XML::BLOCK_DELIMITER ) {

      // block delimiters
      block_delimiters_.push_back( BlockDelimiter( child_element, block_delimiters_.size() ) );

    } else if( child_element.tagName() == XML::MACRO ) {

      // text macrox
      TextMacro macro( child_element );
      if( macro.isValid() ) text_macros_.push_back( macro );

    } else if( child_element.tagName() == OPTIONS::OPTION ) {

      XmlOption option( child_element );
      if( option.name() == XML::OPTION_WRAP ) setWrap( option.get<bool>() );
      else if( option.name() == XML::OPTION_EMULATE_TABS ) setEmulateTabs( option.get<bool>() );
      else if( option.name() == XML::OPTION_DEFAULT ) setIsDefault( option.get<bool>() );
      else if( option.name() == XML::BASE_INDENTATION ) _setBaseIndentation( option.get<int>() );
      else if( option.name() == XML::TAB_SIZE ) setTabSize( option.get<int>() );
      else Debug::Throw(0) << "DocumentClass::DocumentClass - unrecognized option " << option.name() << endl;

    } else Debug::Throw(0) << "DocumentClass::DocumentClass - unrecognized child " << child_element.tagName() << ".\n";



  }

  // associate elements
  QStringList warnings = associatePatterns();
  for( QStringList::const_iterator iter = warnings.begin(); iter != warnings.end(); iter++ )
  { Debug::Throw(0) << "DocumentClass::read - " << *iter << endl; }

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
  for( HighlightPattern::List::iterator iter = highlight_patterns_.begin(); iter != highlight_patterns_.end(); iter++, id++ )
  { iter->setId( id ); }

  // create parent/children hierarchy between highlight patterns
  for( HighlightPattern::List::iterator iter = highlight_patterns_.begin(); iter != highlight_patterns_.end(); iter++ )
  if( iter->parent().size() )
  {
    HighlightPattern::List::iterator parent_iter( find_if( highlight_patterns_.begin(), highlight_patterns_.end(), HighlightPattern::SameNameFTor( iter->parent() ) ) );
    if( parent_iter != highlight_patterns_.end() )
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
  for( HighlightPattern::List::iterator iter = highlight_patterns_.begin(); iter != highlight_patterns_.end(); iter++ )
  {
    set<HighlightStyle>::iterator style_iter ( highlight_styles_.find( iter->style() ) );
    if( style_iter != highlight_styles_.end() ) iter->setStyle( *style_iter );
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
  if( !file_pattern_.isEmpty() ) out.setAttribute( XML::PATTERN, XmlString( file_pattern_.pattern() ).toXml() );
  if( !firstline_pattern_.isEmpty() ) out.setAttribute( XML::FIRSTLINE_PATTERN, XmlString( firstline_pattern_.pattern() ).toXml() );

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
  for( set<HighlightStyle>::const_iterator iter = highlight_styles_.begin(); iter != highlight_styles_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump highlight patterns
  out.appendChild( parent.createTextNode( "\n\n" ) );
  out.appendChild( parent.createComment( "Highlight patterns" ) );
  for( HighlightPattern::List::const_iterator iter = highlight_patterns_.begin(); iter != highlight_patterns_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump indent patterns
  out.appendChild( parent.createTextNode( "\n\n" ) );
  out.appendChild( parent.createComment( "Indentation patterns" ) );
  for( IndentPattern::List::const_iterator iter = indent_patterns_.begin(); iter != indent_patterns_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump parenthesis
  out.appendChild( parent.createTextNode( "\n\n" ) );
  out.appendChild( parent.createComment( "Parenthesis" ) );
  for( TextParenthesis::List::const_iterator iter = text_parenthesis_.begin(); iter != text_parenthesis_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump block delimiters
  out.appendChild( parent.createTextNode( "\n\n" ) );
  out.appendChild( parent.createComment( "Block delimiters" ) );
  for( BlockDelimiter::List::const_iterator iter = block_delimiters_.begin(); iter != block_delimiters_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump text macros
  out.appendChild( parent.createTextNode( "\n\n" ) );
  out.appendChild( parent.createComment( "Text macros" ) );
  for( TextMacro::List::const_iterator iter = text_macros_.begin(); iter != text_macros_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  return out;
}

//________________________________________________
void DocumentClass::clear( void )
{

  Debug::Throw( "DocumentClass::clear.\n" );

  highlight_styles_.clear();
  highlight_patterns_.clear();
  indent_patterns_.clear();
  text_parenthesis_.clear();
  text_macros_.clear();

}

//________________________________________________________
bool DocumentClass::match( const File& file ) const
{

  Debug::Throw( "DocumentClass::Match.\n" );

  // check if class is default
  if( default_ ) return true;

  // check if file pattern match
  if( !file_pattern_.isEmpty() && file_pattern_.isValid() && file_pattern_.indexIn( file ) >= 0 ) return true;

  // check if first line of file match firstline_pattern_
  QFile in( file );
  if( !firstline_pattern_.isEmpty() && firstline_pattern_.isValid() && in.open( QIODevice::ReadOnly ) )
  {

    QString line;
    static const QRegExp empty_line_regexp( "(^\\s*$)" );
    while(  in.bytesAvailable() && !(line = in.readLine(1024)).isNull() )
    {

      // skip empty lines
      if( line.isEmpty() || empty_line_regexp.indexIn( line ) >= 0 ) continue;

      // check non empty lines
      return ( firstline_pattern_.indexIn( line ) >= 0 );

    }
  }

  // no match
  return false;
}
