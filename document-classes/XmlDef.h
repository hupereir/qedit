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

#include <QString>

//! Some Xml definitions
namespace XML {
  
  //!@name document class
  //@{
  static const QString PATTERNS( "patterns" );
  static const QString DOCUMENT_CLASS( "class" );
  static const QString PATTERN( "pattern" );
  static const QString FIRSTLINE_PATTERN( "first_line_pattern" );
  static const QString ICON( "icon" );
  static const QString OPTIONS( "options" );
  static const QString OPTION_WRAP( "wrap" );
  static const QString OPTION_DEFAULT( "default" );
  //@}
  
  //!@name highlight style
  //@{
  static const QString STYLE( "style" );
  static const QString NAME( "name" );  
  static const QString FORMAT( "format" );
  static const QString COLOR( "color" );
  //@}
  
  //!@name highlight pattern
  //@{
  static const QString KEYWORD_PATTERN( "keyword_pattern" );
  static const QString RANGE_PATTERN( "range_pattern" );
  static const QString PARENT( "parent" );
  static const QString KEYWORD( "keyword" );
  static const QString BEGIN( "begin" );
  static const QString END( "end" );
  static const QString COMMENTS( "comments" );
  
  //! used to tell that lines matching pattern are to be ignored when indenting the text
  static const QString OPTION_NO_INDENT( "no_indent" );
  
  //! used to tell that Range can span across paragraphs
  static const QString OPTION_SPAN( "span" );
  
  //! used to tell that highlight pattern is not case sensitive
  static const QString OPTION_NO_CASE( "no_case" );
  
  //@}
  
  //!@name indentation pattern
  //@{
  static const QString INDENT_PATTERN( "indent_pattern" );
  
  //! indentation pattern rule
  static const QString RULE( "rule" );
  static const QString TYPE( "type" );
  static const QString SCALE( "scale" );
  static const QString PAR( "par" );
  static const QString REGEXP( "regexp" );
  
  //! base number of characters to skip before applying the pattern
  static const QString BASE_INDENTATION( "base_indentation" );
  static const QString VALUE( "value" );
  
  //@}
  
  //!@name text macro
  //@{
  static const QString MACRO( "macro" );
  static const QString ACCELERATOR( "accelerator" );
  static const QString REPLACEMENT( "replacement" );
  
  //! do not split selection into single lines
  static const QString OPTION_NO_SPLIT( "no_split" );
  
  //! macro is a separator to be placed in the menu
  static const QString OPTION_SEPARATOR( "separator" );
  //@}
  
  //!@name text parenthesis
  //@{
  //! text parenthesis
  static const QString PARENTHESIS( "parenthesis" );
  //@}
  
  //!@name block delimiters
  //@{
  static const QString BLOCK_DELIMITER( "block_delimiter" );
  //@}
  
};

#endif
