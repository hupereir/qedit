#ifndef IndentPattern_h
#define IndentPattern_h

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
#include <QRegExp>

//* Base class for syntax highlighting
class IndentPattern: public Counter
{

    public:

    //* typedef for list of patterns
    using List = QList< IndentPattern >;
    using ListIterator = QListIterator< IndentPattern >;

    //* constructor
    IndentPattern( void );

    //* constructor from DomElement
    IndentPattern( const QDomElement& element );

    //* dom element
    QDomElement domElement( QDomDocument& parent ) const;

    //* Id
    int id( void ) const
    { return id_; }

    //* set id
    void setId( int id )
    { id_ = id; }

    //* equal to ftor
    class WeakEqualFTor
    {
        public:

        bool operator()( const IndentPattern& first, const IndentPattern& second ) const
        { return first.id() == second.id(); }

    };

    //* less than ftor
    class WeakLessThanFTor
    {
        public:

        bool operator()( const IndentPattern& first, const IndentPattern& second ) const
        { return first.id() < second.id(); }

    };

    //* equal to operator
    bool operator == (const IndentPattern& ) const;

    //* name
    const QString& name() const
    { return name_; }

    //* name
    void setName( const QString& name )
    { name_ = name; }

    //* reset counter
    static void resetCounter( void )
    { _counter() = 0; }

    //* pattern type enumeration
    enum Type
    {
        Nothing,
        Increment,
        Decrement,
        DecrementAll
    };

    //* pattern type
    Type type( void ) const
    { return type_; }

    //* type
    void setType( Type type )
    { type_ = type; }

    //* type
    QString typeName( void ) const
    { return typeName( type() ); }

    //* type
    static QString typeName( const Type& );

    //* pattern scale
    int scale( void ) const
    { return scale_; }

    //* scale
    void setScale( int scale )
    { scale_ = scale; }

    //* comments
    const QString& comments( void ) const
    { return comments_; }

    //* comments
    void setComments( const QString& value )
    { comments_ = value; }

    //* indentation rule
    /** used to check a regExp against a given paragraph */
    class Rule: public Counter
    {
        public:

        //* shortcut to Rules
        using List = QList< Rule >;
        using ListIterator = QListIterator< Rule >;

        //* pattern flags
        enum Flag
        {

            None = 0,
            CaseInsensitive = 1<<2

        };

        Q_DECLARE_FLAGS( Flags, Flag );

        //* constructor
        Rule( const QDomElement& element = QDomElement() );

        //* dom element
        virtual QDomElement domElement( QDomDocument& parent ) const;

        //* equal to operator
        bool operator == ( const Rule& rule ) const
        {
            return
                pattern().pattern() == rule.pattern().pattern() &&
                paragraph() == rule.paragraph() &&
                flags() == rule.flags();
        }

        //* less than operator
        bool operator < ( const Rule& rule ) const
        {
            if( pattern().pattern() != rule.pattern().pattern() ) return pattern().pattern() < rule.pattern().pattern();
            if( paragraph() != rule.paragraph() ) return paragraph() < rule.paragraph();
            if( flags() != rule.flags() ) return flags() < rule.flags();
            return false;
        }

        //* paragraph
        int paragraph() const
        { return paragraph_; }

        //* set paragraph
        void setParagraph( int par )
        { paragraph_ = par; }

        //* regExp
        void setPattern( const QString& regexp )
        { regexp_.setPattern( regexp ); }

        //* regexp
        const QRegExp& pattern( void ) const
        { return regexp_; }

        //* true if valid
        bool isValid( void ) const
        { return regexp_.isValid(); }

        //* returns true if the text match the rule
        bool accept( const QString& text ) const;

        //*@name flags
        //@{

        //* flags
        Flags flags( void ) const
        { return flags_; }

        //* flags
        void setFlags( Flags flags )
        { flags_ = flags; }

        //* flags
        bool flag( const Flag& flag ) const
        { return flags_ & flag; }

        //* flags
        void setFlag( const Flag& flag, bool value )
        {
            if( value ) flags_ |= flag;
            else flags_ &= (~flag);
        }

        //@}


        private:

        //* paragraph id (vs current)
        int paragraph_;

        //* regexp
        QRegExp regexp_;

        //* flags
        Flags flags_;

        //* streamer
        friend QTextStream& operator << ( QTextStream& out, const Rule& rule )
        {
            out << "Rule - par_id: " << rule.paragraph_ << " RegExp: \"" << rule.regexp_.pattern() << "\"";
            return out;
        }
    };

    //* retrieve rules
    const Rule::List& rules( void ) const
    { return rules_; }

    //* add rule
    void addRule( const Rule& rule )
    { rules_ << rule; }

    //* rules
    void setRules( const Rule::List& rules )
    { rules_ = rules; }

    //* validity
    bool isValid( void ) const
    {
        foreach( const Rule& rule, rules_ )
        { if( !rule.isValid() ) return false; }
        return true;
    }

    private:

    //* unique id counter
    static unsigned int& _counter( void );

    //* unique id
    int id_ = 0;

    //* pattern name
    QString name_;

    //* type
    Type type_ = Nothing;

    //* scale
    /**
    decide how many increment/decrement operations
    should be performed. Default is one
    */
    int scale_ = 1;

    //* comments
    QString comments_;

    //* list of rules to match
    Rule::List rules_;

    //* dumper
    friend QTextStream& operator << ( QTextStream& out, const IndentPattern& pattern )
    {
        out << "IndentPattern - name: " << pattern.name() << " type: " << pattern.type_ << endl;
        foreach( const Rule& rule, pattern.rules_ )
        { out << "  " << rule << endl; }
        return out;
    }

};

Q_DECLARE_OPERATORS_FOR_FLAGS( IndentPattern::Rule::Flags );

#endif
