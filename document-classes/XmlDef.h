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


#ifndef XmlDef_h
#define XmlDef_h
/*!
   \file    XmlDef.h
   \brief   Some Xml definitions
   \author  Hugo Pereira
   \version $Revision$
   \date    $Date$
*/

#include <string>

//! Some Xml definitions
namespace XML {
  
  //!@name document class
  //@{
  static const std::string PATTERNS( "patterns" );
  static const std::string DOCUMENT_CLASS( "class" );
  static const std::string PATTERN( "pattern" );
  static const std::string FIRSTLINE_PATTERN( "first_line_pattern" );
  static const std::string ICON( "icon" );
  static const std::string OPTIONS( "options" );
  static const std::string OPTION_WRAP( "wrap" );
  static const std::string OPTION_DEFAULT( "default" );
  //@}
  
  //!@name highlight style
  //@{
  static const std::string STYLE( "style" );
  static const std::string NAME( "name" );  
  static const std::string FORMAT( "format" );
  static const std::string COLOR( "color" );
  //@}
  
  //!@name highlight pattern
  //@{
  static const std::string KEYWORD_PATTERN( "keyword_pattern" );
  static const std::string RANGE_PATTERN( "range_pattern" );
  static const std::string PARENT( "parent" );
  static const std::string KEYWORD( "keyword" );
  static const std::string BEGIN( "begin" );
  static const std::string END( "end" );
  static const std::string COMMENTS( "comments" );
  
  //! used to tell that lines matching pattern are to be ignored when indenting the text
  static const std::string OPTION_NO_INDENT( "no_indent" );
  
  //! used to tell that Range can span across paragraphs
  static const std::string OPTION_SPAN( "span" );
  
  //! used to tell that highlight pattern is not case sensitive
  static const std::string OPTION_NO_CASE( "no_case" );
  
  //@}
  
  //!@name indentation pattern
  //@{
  static const std::string INDENT_PATTERN( "indent_pattern" );
  
  //! indentation pattern rule
  static const std::string RULE( "rule" );
  static const std::string TYPE( "type" );
  static const std::string SCALE( "scale" );
  static const std::string PAR( "par" );
  static const std::string REGEXP( "regexp" );
  
  //! base number of characters to skip before applying the pattern
  static const std::string BASE_INDENTATION( "base_indentation" );
  static const std::string VALUE( "value" );
  
  //@}
  
  //!@name text macro
  //@{
  static const std::string MACRO( "macro" );
  static const std::string ACCELERATOR( "accelerator" );
  static const std::string REPLACEMENT( "replacement" );
  
  //! do not split selection into single lines
  static const std::string OPTION_NO_SPLIT( "no_split" );
  
  //! macro is a separator to be placed in the menu
  static const std::string OPTION_SEPARATOR( "separator" );
  //@}
  
  //!@name text parenthesis
  //@{
  //! text parenthesis
  static const std::string PARENTHESIS( "parenthesis" );
  //@}
  
};

#endif
