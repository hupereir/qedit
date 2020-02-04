#ifndef DocumentClass_h
#define DocumentClass_h

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

#include "BlockDelimiter.h"
#include "Counter.h"
#include "Debug.h"
#include "IndentPattern.h"
#include "TextParenthesis.h"
#include "TextMacro.h"
#include "File.h"
#include "Functors.h"
#include "HighlightPattern.h"
#include "HighlightStyle.h"

#include <QDomDocument>
#include <QDomElement>
#include <QRegularExpression>

#include <functional>

//* Highlight and indentation patterns for a given document class
class DocumentClass final: private Base::Counter<DocumentClass>
{

    public:

    //* constructor
    explicit DocumentClass();

    //* constructor
    explicit DocumentClass( const QDomElement& );

    //* write to DomElement
    QDomElement domElement( QDomDocument& parent ) const;

    //*@name accessors
    //@{

    //* name
    const QString& name() const
    { return name_; }

    //* file
    const File& file() const
    { return file_; }

    //* icon name
    const QString& icon() const
    { return icon_; }

    //* default
    bool isDefault() const
    { return default_; }

    //* build in
    bool isBuildIn() const
    { return buildIn_; }

    //* filename matching pattern
    const QRegularExpression& fileMatchingPattern() const
    { return filePattern_; }

    //* first line matching pattern
    const QRegularExpression& firstLineMatchingPattern() const
    { return firstlinePattern_; }

    //* return true if document class match filename
    bool match( const File& file ) const;

    //* returns true if document class enables wrapping by default
    bool wrap() const
    { return wrap_; }

    //* returns true if document class enables tab emulation by default
    bool emulateTabs() const
    { return emulateTabs_; }

    //* tab size
    int tabSize() const
    { return tabSize_; }

    //* hightlight styles
    const HighlightStyle::Set& highlightStyles() const
    { return highlightStyles_; }

    //* highligh patterns
    const HighlightPattern::List& highlightPatterns() const
    { return highlightPatterns_; }

    //* list of indentation patterns
    const IndentPattern::List& indentPatterns() const
    { return indentPatterns_; }

    //* base indentation
    /**
    it is used which classes for which
    a certain amount of leading space character are
    not to be considered when indenting. This is the case
    for the first 6 space characters in fortran files
    */
    int baseIndentation() const
    { return baseIndentation_; }

    //* text parenthesis
    const TextParenthesis::List& parenthesis() const
    { return textParenthesis_; }

    //* block delimiters
    const BlockDelimiter::List& blockDelimiters() const
    { return blockDelimiters_; }

    //* text macros
    const TextMacro::List& textMacros() const
    { return textMacros_; }

    //@}

    //*@name modifiers
    //@{

    //* name
    void setName( const QString& name )
    { name_ = name; }

    //* set file
    void setFile( const File& file )
    { file_ = file; }

    //* default
    void setIsDefault( bool value )
    { default_ = value; }

    //* buid-in
    void setIsBuildIn( bool value )
    { buildIn_ = value; }

    //@}

    //* equal to ftor
    class WeakEqualFTor:
        public Base::Functor::BinaryEqual<DocumentClass, const QString&, &DocumentClass::name>,
        public std::binary_function< DocumentClass, DocumentClass, bool>
    {};

    //* less than ftor
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

    //* used to match pointers to DocumentClass with same name
    using SameNameFTor = Base::Functor::Unary<DocumentClass, const QString&, &DocumentClass::name>;

    //* used to get non default document class matching a file
    class MatchFileFTor
    {
        public:

        //* constructor
        explicit MatchFileFTor( const File& file ):
            file_( file )
            {}

        //* predicate
        bool operator () (const DocumentClass& documentClass ) const
        { return documentClass.match( file_ ) && !documentClass.isDefault(); }

        private:

        //* predicted file
        const File file_;

    };

    //* used to counts number of default patterns registered
    using IsDefaultFTor = Base::Functor::UnaryTrue<DocumentClass, &DocumentClass::isDefault>;

    protected:

    //* perform associations between highlight patterns and highlight styles
    /** returns list of warnings if any */
    QStringList _associatePatterns();

    private:

    //* name
    QString name_;

    //* parent file
    File file_;

    //* file pattern
    QRegularExpression filePattern_;

    //* first line pattern
    /** it is used to see if a file match based on its first line rather than its extension */
    QRegularExpression firstlinePattern_;

    //* is class default
    bool default_ = false;

    //* is class build-in
    bool buildIn_ = false;

    //* wrap flag
    bool wrap_ = false;

    //* tab emulation
    bool emulateTabs_ = false;

    //* tab size
    int tabSize_ = 2;

    //* icon name
    QString icon_;

    //* set of highlight styles
    HighlightStyle::Set highlightStyles_;

    //* list of highlight patterns
    HighlightPattern::List highlightPatterns_;

    //* list of indentation patterns
    IndentPattern::List indentPatterns_;

    //* list of text parenthesis
    TextParenthesis::List textParenthesis_;

    //* block delimiter
    BlockDelimiter::List blockDelimiters_;

    //* list of text parenthesis
    TextMacro::List textMacros_;

    //* base indentation
    /**
    this is the number of space characters to add prior to any text indentation
    */
    int baseIndentation_ = 0;

};

//* strict equal to operator
bool operator == (const DocumentClass&, const DocumentClass& );

#endif
