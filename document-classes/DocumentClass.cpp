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

#include <algorithm>
#include "DocumentClass.h"
#include "HighlightPattern.h"
#include "Str.h"
#include "XmlDef.h"
#include "XmlUtil.h"

using namespace std;

//________________________________________________________
DocumentClass::DocumentClass( void ):
  Counter( "DocumentClass" ),
  default_( false ),
  wrap_( false ),
  emulate_tabs_( false ),
  base_indentation_( 0 )
{ Debug::Throw( "DocumentClass::DocumentClass.\n" ); }

//________________________________________________________
DocumentClass::DocumentClass( const QDomElement& element ):
  Counter( "DocumentClass" ),
  default_( false ),
  wrap_( false ),
  emulate_tabs_( false ),
  base_indentation_( 0 )
{
  Debug::Throw( "DocumentClass::DocumentClass.\n" );

  // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;

    if( attribute.name() == XML::NAME ) name_ = attribute.value() ;
    else if( attribute.name() == XML::PATTERN ) file_pattern_.setPattern( XmlUtil::xmlToText( attribute.value() ) );
    else if( attribute.name() == XML::FIRSTLINE_PATTERN ) firstline_pattern_.setPattern( XmlUtil::xmlToText( attribute.value() ) ); 
    else if( attribute.name() == XML::ICON ) icon_ = XmlUtil::xmlToText( attribute.value() );
    else if( attribute.name() == XML::OPTIONS )
    {
      
      if( attribute.value().indexOf( XML::OPTION_WRAP, 0, Qt::CaseInsensitive ) >= 0 ) wrap_ = true;    
      if( attribute.value().indexOf( XML::OPTION_EMULATE_TABS, 0, Qt::CaseInsensitive ) >= 0 ) emulate_tabs_ = true;    
      if( attribute.value().indexOf( XML::OPTION_DEFAULT, 0, Qt::CaseInsensitive ) >= 0 ) default_ = true;    
      
    } else if( attribute.name() == XML::BASE_INDENTATION ) _setBaseIndentation( attribute.value().toInt() );
    else Debug::Throw(0) << "DocumentClass::DocumentClass - unrecognized attribute: " << qPrintable( attribute.name() ) << endl;
    
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

    } else cout << "DocumentClass::DocumentClass - unrecognized child " << qPrintable( child_element.tagName() ) << ".\n";

  }

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
    } else Debug::Throw(0) << "DocumentClass::DocumentClass - unable to load parent named " << qPrintable( iter->parent() ) << endl;
  }

  // assign styles to patterns
  for( HighlightPattern::List::iterator iter = highlight_patterns_.begin(); iter != highlight_patterns_.end(); iter++ )
  {
    set<HighlightStyle>::iterator style_iter ( highlight_styles_.find( iter->style() ) );
    if( style_iter != highlight_styles_.end() ) iter->setStyle( *style_iter );
    else Debug::Throw(0) << "HighlightParser::Read - unrecognized style " << qPrintable( iter->style().name() ) << endl;
  }

}

//________________________________________________
QDomElement DocumentClass::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "DocumentClass::domElement.\n" );
  QDomElement out( parent.createElement( XML::DOCUMENT_CLASS ) );
  
  // dump attributes
  out.setAttribute( XML::NAME, name_ );
  if( !file_pattern_.isEmpty() ) out.setAttribute( XML::PATTERN, XmlUtil::textToXml( file_pattern_.pattern() ) );
  if( !firstline_pattern_.isEmpty() ) out.setAttribute( XML::FIRSTLINE_PATTERN, XmlUtil::textToXml( firstline_pattern_.pattern() ) );
  
  // options
  QString options;
  if( wrap() ) options += XML::OPTION_WRAP + " ";
  if( emulateTabs() ) options += XML::OPTION_EMULATE_TABS + " ";
  if( isDefault() ) options += XML::OPTION_DEFAULT + " ";
  if( !options.isEmpty() ) out.setAttribute( XML::OPTIONS, options ); 
  if( baseIndentation() ) out.setAttribute( XML::BASE_INDENTATION, Str().assign<int>( baseIndentation() ).c_str() );

  // icon
  if( !icon().isEmpty() ) out.setAttribute( XML::ICON, icon() );
  
  // dump highlight styles
  for( set<HighlightStyle>::const_iterator iter = highlight_styles_.begin(); iter != highlight_styles_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump highlight patterns
  for( HighlightPattern::List::const_iterator iter = highlight_patterns_.begin(); iter != highlight_patterns_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump indent patterns
  for( IndentPattern::List::const_iterator iter = indent_patterns_.begin(); iter != indent_patterns_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump parenthesis
  for( TextParenthesis::List::const_iterator iter = text_parenthesis_.begin(); iter != text_parenthesis_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump block delimiters
  for( BlockDelimiter::List::const_iterator iter = block_delimiters_.begin(); iter != block_delimiters_.end(); iter++ )
  { out.appendChild( iter->domElement( parent ) ); }

  // dump text macros
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
  if( !file_pattern_.isEmpty() && file_pattern_.isValid() && file_pattern_.indexIn( file.c_str() ) >= 0 ) return true;
  
  // check if first line of file match firstline_pattern_
  ifstream in( file.c_str() );
  if( !firstline_pattern_.isEmpty() && firstline_pattern_.isValid() && in )
  {
    
    static const int linesize( 256 );
    static const QRegExp empty_line_regexp( "(^\\s*$)" ); 
    char line[linesize];
    while( !(in.rdstate() & ios::failbit ) )
    {
      in.getline( line, linesize, '\n' );
      if( !( strlen(line) && empty_line_regexp.indexIn( line ) < 0 ) ) continue;
      if( firstline_pattern_.indexIn( line ) >= 0 ) return true;
      break;
    }
  }
  
  // no match
  return false;
}
