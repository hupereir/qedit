#ifndef DocumentClass_h
#define DocumentClass_h

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

#include "BlockDelimiter.h"
#include "Counter.h"
#include "Debug.h"
#include "IndentPattern.h"
#include "TextParenthesis.h"
#include "TextMacro.h"
#include "File.h"
#include "HighlightPattern.h"
#include "HighlightStyle.h"

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtCore/QRegExp>

//! Highlight and indentation patterns for a given document class
class DocumentClass: public Counter
{

    public:

    //! constructor
    DocumentClass( void );

    //! constructor
    DocumentClass( const QDomElement& );

    //! destructor
    ~DocumentClass( void )
    {}

    //! write to DomElement
    QDomElement domElement( QDomDocument& parent ) const;

    //! strict equal to operator
    bool operator == (const DocumentClass& documentClass ) const;

    //!@name accessors
    //@{

    //! name
    const QString& name( void ) const
    { return name_; }

    //! file
    const File& file( void ) const
    { return file_; }

    //! icon name
    const QString& icon( void ) const
    { return icon_; }

    //! default
    bool isDefault( void ) const
    { return default_; }

    //! build in
    bool isBuildIn( void ) const
    { return buildIn_; }

    //! filename matching pattern
    const QRegExp& fileMatchingPattern( void ) const
    { return filePattern_; }

    //! first line matching pattern
    const QRegExp& firstLineMatchingPattern( void ) const
    { return firstlinePattern_; }

    //! return true if document class match filename
    bool match( const File& file ) const;

    //! returns true if document class enables wrapping by default
    const bool& wrap( void ) const
    { return wrap_; }

    //! returns true if document class enables tab emulation by default
    bool emulateTabs( void ) const
    { return emulateTabs_; }

    //! tab size
    int tabSize( void ) const
    { return tabSize_; }

    //! hightlight styles
    const HighlightStyle::Set& highlightStyles() const
    { return highlightStyles_; }

    //! highligh patterns
    const HighlightPattern::List& highlightPatterns() const
    { return highlightPatterns_; }

    //! list of indentation patterns
    const IndentPattern::List& indentPatterns() const
    { return indentPatterns_; }

    //! base indentation
    /*!
    it is used which classes for which
    a certain amount of leading space character are
    not to be considered when indenting. This is the case
    for the first 6 space characters in fortran files
    */
    const int& baseIndentation( void ) const
    { return baseIndentation_; }

    //! text parenthesis
    const TextParenthesis::List& parenthesis() const
    { return textParenthesis_; }

    //! block delimiters
    const BlockDelimiter::List& blockDelimiters() const
    { return blockDelimiters_; }

    //! text macros
    const TextMacro::List& textMacros() const
    { return textMacros_; }

    //@}

    //!@name modifiers
    //@{

    //! name
    void setName( const QString& name )
    { name_ = name; }

    //! set file
    void setFile( const File& file )
    { file_ = file; }

    //! default
    void setIsDefault( bool value )
    { default_ = value; }

    //! buid-in
    void setIsBuildIn( bool value )
    { buildIn_ = value; }

    //@}

    //! equal to ftor
    class WeakEqualFTor: public std::binary_function< DocumentClass, DocumentClass, bool>
    {
        public:

        bool operator()( const DocumentClass& first, const DocumentClass& second ) const
        { return first.name() == second.name(); }

    };

    //! less than ftor
    class WeakLessThanFTor: public std::binary_function< DocumentClass, DocumentClass, bool>
    {
        public:

        bool operator()( const DocumentClass& first, const DocumentClass& second ) const
        {
            if( first.isDefault() )  return false;
            else if( second.isDefault() ) return true;
            else if( first.isBuildIn() && !second.isBuildIn() ) return false;
            else if( !first.isBuildIn() && second.isBuildIn() ) return true;
            return first.name() < second.name();
        }

    };

    //! used to match pointers to DocumentClass with same name
    class SameNameFTor
    {
        public:

        //! constructor
        SameNameFTor( const QString& name ):
            name_( name )
            {}

        //! predicate
        bool operator() (const DocumentClass& documentClass ) const
        { return documentClass.name() == name_; }

        private:

        //! predicted name
        QString name_;
    };

    //! used to get non default document class matching a file
    class MatchFileFTor
    {
        public:

        //! constructor
        MatchFileFTor( const File& file ):
            file_( file )
            {}

        //! predicate
        bool operator () (const DocumentClass& documentClass ) const
        { return documentClass.match( file_ ) && !documentClass.isDefault(); }

        private:

        //! predicted file
        const File file_;

    };

    //! used to counts number of default patterns registered
    class IsDefaultFTor
    {
        public:

        //! predicate
        bool operator() (const DocumentClass& documentClass ) const
        { return documentClass.isDefault(); }

    };

    protected:

    //! perform associations between highlight patterns and highlight styles
    /*! returns list of warnings if any */
    QStringList _associatePatterns( void );

    private:

    //! name
    QString name_;

    //! parent file
    File file_;

    //! file pattern
    QRegExp filePattern_;

    //! first line pattern
    /*! it is used to see if a file match based on its first line rather than its extension */
    QRegExp firstlinePattern_;

    //! is class default
    bool default_;

    //! is class build-in
    bool buildIn_;

    //! wrap flag
    bool wrap_;

    //! tab emulation
    bool emulateTabs_;

    //! tab size
    int tabSize_;

    //! icon name
    QString icon_;

    //! set of highlight styles
    HighlightStyle::Set highlightStyles_;

    //! list of highlight patterns
    HighlightPattern::List highlightPatterns_;

    //! list of indentation patterns
    IndentPattern::List indentPatterns_;

    //! list of text parenthesis
    TextParenthesis::List textParenthesis_;

    //! block delimiter
    BlockDelimiter::List blockDelimiters_;

    //! list of text parenthesis
    TextMacro::List textMacros_;

    //! base indentation
    /*!
    this is the number of space characters to add prior to any text indentation
    */
    int baseIndentation_;

};


#endif
