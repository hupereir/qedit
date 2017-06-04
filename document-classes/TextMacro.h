#ifndef TextMacro_h
#define TextMacro_h

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

#include "Counter.h"
#include "Debug.h"

#include <QDomElement>
#include <QDomDocument>
#include <QAction>
#include <QRegExp>
#include <QList>

//* Text modification macro
class TextMacro: private Base::Counter<TextMacro>
{

    public:

    //* list
    using List = QList< TextMacro >;
    using ListIterator = QListIterator< TextMacro >;

    //* constructor
    TextMacro( void ):
        Counter( "TextMacro" ),
        id_( 0 ),
        isSeparator_( true ),
        isAutomatic_( false )
    {}

    //* constructor from DomElement
    TextMacro( const QDomElement& );

    //* dom element
    QDomElement domElement( QDomDocument& parent ) const;

    //* Id
    int id( void ) const
    { return id_; }

    //* set id
    void setId( int id )
    { id_ = id; }

    //* equal to operator
    bool operator == ( const TextMacro& other ) const
    {
        return
            name() == other.name() &&
            accelerator() == other.accelerator() &&
            isSeparator() == other.isSeparator() &&
            rules() == other.rules();

    }

    //* equal to ftor
    class WeakEqualFTor: public std::binary_function< TextMacro, TextMacro, bool>
    {
        public:

        bool operator()( const TextMacro& first, const TextMacro& second ) const
        { return first.id() == second.id(); }

    };

    //* less than ftor
    class WeakLessThanFTor: public std::binary_function< TextMacro, TextMacro, bool>
    {
        public:

        bool operator()( const TextMacro& first, const TextMacro& second ) const
        { return first.id() < second.id(); }

    };

    //* used to check whether some automatic macros are present
    class isAutomaticFTor
    {
        public:

        bool operator()( const TextMacro& macro ) const
        { return macro.isAutomatic(); }

    };

    //* reset counter
    static void resetCounter( void )
    { idCounter_ = 0; }

    //* name
    virtual const QString& name( void ) const
    { return name_; }

    //* name
    virtual void setName( const QString& name )
    { name_ = name; }

    //* accelerator
    virtual const QString& accelerator( void ) const
    { return accelerator_; }

    //* accelerator
    virtual void setAccelerator( const QString& value )
    { accelerator_ = value; }

    //* separator flag
    bool isSeparator( void ) const
    { return isSeparator_; }

    //* separator
    virtual void setIsSeparator( bool value = true )
    { isSeparator_ = value; }

    //* automatic flag
    bool isAutomatic( void ) const
    { return isAutomatic_; }

    //* automatic
    virtual void setIsAutomatic( bool value = true )
    { isAutomatic_ = value; }

    //* result class
    /**
    first is whether any changes where applied or not
    second is the net increment in terms of number of characters between input and output text
    */
    class Result: public QPair<bool, int>
    {

        public:

        //* constructor
        Result( bool changed = false, int increment = 0 ):
            QPair<bool,int>( changed, increment )
        {}

        //* add-to operator
        Result& operator += (const Result& other )
        {
            first |= other.first;
            second += other.second;
            return *this;
        }

    };

    //* modify text passed as argument. Return true if text is modified
    Result processText( QString& text, int position = -1 ) const;

    //* modify text passed as argument. Return true if text is modified
    bool isValid( void ) const
    {
        if( isSeparator() ) return true;
        if( rules_.empty() ) return false;
        for( const auto& rule:rules_ )
        { if( !rule.isValid() ) return false; }
        return true;
    }

    //* used to get macro by name
    class SameNameFTor
    {

        public:

        //* constructor
        SameNameFTor( const QString& name ):
            name_( name )
        {}

        //* predicate
        bool operator() (const TextMacro& pattern ) const
        { return (pattern.name() == name_); }

        private:

        //* predicate
        const QString name_;

    };

    //* return action
    QAction* action( void ) const;

    //* used to store regular expression and corresponding replacement text
    class Rule: private Base::Counter<Rule>
    {

        public:

        using List = QList<Rule>;
        using ListIterator = QListIterator<Rule>;

        //* constructor
        Rule( const QDomElement& = QDomElement() );

        //* dom element
        QDomElement domElement( QDomDocument& parent ) const;

        //* equal to operator
        bool operator == ( const Rule& rule ) const
        {
            return
                pattern().pattern() == rule.pattern().pattern() &&
                replaceText() == rule.replaceText() &&
                split() == rule.split();
        }

        //* less than operator
        bool operator < ( const Rule& rule ) const
        {
            if( pattern().pattern() != rule.pattern().pattern() ) return pattern().pattern() < rule.pattern().pattern();
            if( replaceText() != rule.replaceText() ) return replaceText() < rule.replaceText();
            if( split() != rule.split() ) return split() < rule.split();
            return false;
        }

        //* validity
        bool isValid( void ) const
        { return pattern_.isValid(); }

        //* modify text passed as argument. Return number of changed characters
        Result processText( QString& text, int position ) const;

        //* pattern
        const QRegExp& pattern( void ) const
        { return pattern_; }

        //* parent name
        virtual void setPattern( const QString& pattern )
        { pattern_.setPattern( pattern ); }

        //* replacemenet text
        virtual const QString& replaceText( void ) const
        { return replace_text_; }

        //* set replacement text
        virtual void setReplaceText( const QString& text )
        { replace_text_ = text; }

        bool split() const
        {  return !no_splitting_; }

        //* splitting flag
        /**
        it is used to decide if the text should be splitted
        line by line and the rule applied independently on each line
        */
        void setNoSplitting()
        {  no_splitting_ = true; }

        private:

        //* modify text passed as argument. Return number of changed characters
        Result _processText( QString& text, int position ) const;

        //*@name flags
        //@{

        //* splitting flag
        /**
        it is used to decide if the text should be splitted
        line by line and the rule applied independently on each line
        */
        bool no_splitting_;

        //@}
        //* match pattern
        QRegExp pattern_;

        //* replacement text
        QString replace_text_;

    };

    //* rules
    const Rule::List& rules( void ) const
    { return rules_; }

    //* rules
    void setRules( const Rule::List& rules )
    { rules_ = rules; }

    //* add a rule
    void addRule( const Rule& rule )
    { rules_ << rule; }

    private:

    //* unique id counter
    static int idCounter_;

    //* unique id
    int id_ = 0;

    //* macro name
    QString name_;

    //* accelerator
    QString accelerator_;

    //* separator flag
    bool isSeparator_ = false;

    //* automatic
    /** if true the macro gets automatically executed before saving modifications */
    bool isAutomatic_ = false;

    //* list of replacement
    Rule::List rules_;

};
#endif
