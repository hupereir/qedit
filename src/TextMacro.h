#ifndef _TextMacro_h_
#define _TextMacro_h_

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

/*!
  \file TextMacro.h
  \brief Text modification macro
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDomElement>
#include <QDomDocument>
#include <QAction>
#include <QRegExp>
#include <string>

#include "Counter.h"
#include "Debug.h"
#include "Str.h"

//! Text modification macro
/*!
Macros are used to search the selected text and replace every occurence of the
searched regular expression with a text replacement
*/
class TextMacro: public Counter
{

  public:
 
  //! style list
  typedef std::vector<TextMacro> List;

  //! constructor from DomElement
  TextMacro( const QDomElement& element = QDomElement() );

  //! dom element
  QDomElement domElement( QDomDocument& parent ) const;

  //! name
  virtual const std::string& name( void ) const
  { return name_; }

  //! separator flag
  const bool& isSeparator( void ) const
  { return is_separator_; }

  //! modify text passed as argument. Return true if text is modified
  bool processText( QString& text ) const
  {
    if( isSeparator() ) return false;
    bool changed( false );
    for( std::vector<Rule>::const_iterator iter = rules_.begin(); iter != rules_.end(); iter++ )
    { changed |= iter->processText( text ); }
    return changed;
  }

  //! modify text passed as argument. Return true if text is modified
  bool isValid( void ) const
  {
    if( isSeparator() ) return true;
    if( rules_.empty() ) return false;
    for( std::vector<Rule>::const_iterator iter = rules_.begin(); iter != rules_.end(); iter++ )
    { if( !iter->isValid() ) return false; }
    return true;
  }

  //! used to get macro by name
  class SameNameFTor
  {

    public:

    //! constructor
    SameNameFTor( const std::string& name ):
      name_( name )
    {}

    //! predicate
    bool operator() (const TextMacro& pattern ) const
    { return (pattern.name() == name_); }

    private:

    //! predicate
    const std::string name_;

  };

  //! return action
  QAction* action( void ) const
  {
    QAction* out( new QAction( name().c_str(), 0 ) );
    if( !_accelerator().empty() ) out->setShortcut( QKeySequence( _accelerator().c_str() ) );
    return out;
  }
  
  protected:

  //! used to store regular expression and corresponding replacement text
  class Rule: public Counter
  {

    public:

    //! constructor
    Rule( const QDomElement& );

    //! dom element
    QDomElement domElement( QDomDocument& parent ) const;

    //! validity
    bool isValid( void ) const
    { return pattern_.isValid(); }

    //! modify text passed as argument. Return true if text is modified
    bool processText( QString& text ) const;

    private:

    //! pattern
    const QRegExp& _pattern( void ) const
    { return pattern_; }

    //! parent name
    virtual void _setPattern( const std::string& pattern )
    { pattern_.setPattern( pattern.c_str() ); }

    //! replacemenet text
    virtual const QString& _replaceText( void ) const
    { return replace_text_; }

    //! set replacement text
    virtual void _setReplaceText( const QString& text )
    { replace_text_ = text; }

    //! splitting flag
    /*! 
      it is used to decide if the text should be splitted 
      line by line and the rule applied independently on each line
    */
    void _setNoSplitting()
    {  no_splitting_ = true; }

    //!@name flags
    //@{

    //! splitting flag
    /*! 
      it is used to decide if the text should be splitted 
      line by line and the rule applied independently on each line
    */
    bool no_splitting_;

    //@}
    //! match pattern
    QRegExp pattern_;

    //! replacement text
    QString replace_text_;

  };

  //! add a rule
  void _addRule( const Rule& rule )
  { rules_.push_back( rule ); }

  //! name
  virtual void _setName( const std::string& name )
  { name_ = name; }

  //! accelerator
  virtual const std::string& _accelerator( void ) const
  { return accelerator_; }

  //! accelerator
  virtual void _setAccelerator( const std::string& value )
  { accelerator_ = value; }

  //! separator
  virtual void _setIsSeparator()
  { is_separator_ = true; }

  private:

  //! macro name
  std::string name_;

  //! accelerator
  std::string accelerator_;

  //! separator flag
  bool is_separator_;

  //! list of replacement
  std::vector< Rule > rules_;

};
#endif
