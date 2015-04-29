#ifndef HighlightPattern_h
#define HighlightPattern_h

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

#include "HighlightStyle.h"
#include "Counter.h"
#include "Debug.h"

#include <QDomElement>
#include <QDomDocument>
#include <QRegExp>
#include <QString>
#include <QList>

class PatternLocationSet;

//* Base class for syntax highlighting
class HighlightPattern: public Counter
{

    public:

    //* pattern flags
    enum Flag
    {
        None = 0,
        Span = 1<<0,
        NoIndent = 1<<1,
        CaseInsensitive = 1<<2,
        Comment = 1<<3
    };

    Q_DECLARE_FLAGS( Flags, Flag );

    //* typedef for list of patterns
    using List = QList< HighlightPattern >;
    using ListIterator = QListIterator< HighlightPattern >;

    //* no parent pattern
    static QString noParentPattern_;

    //* constructor from DomElement
    HighlightPattern( const QDomElement& element = QDomElement() );

    //* dom element
    QDomElement domElement( QDomDocument& parent ) const;

    //* set id
    /*!
    The unique ID has a single bit set to 1, to use
    faster active pattern masks, with no shift operators
    */
    void setId( const int& id )
    { id_ = (1<<id); }

    //* unique id
    const int& id( void ) const
    { return id_; }

    //* equal to operator
    bool operator == (const HighlightPattern& other ) const;

    //* equal to ftor
    class WeakEqualFTor: public std::binary_function< HighlightPattern, HighlightPattern, bool>
    {
        public:

        bool operator()( const HighlightPattern& first, const HighlightPattern& second ) const
        { return first.id() == second.id(); }

    };

    //* less than ftor
    class WeakLessThanFTor: public std::binary_function< HighlightPattern, HighlightPattern, bool>
    {
        public:

        bool operator()( const HighlightPattern& first, const HighlightPattern& second ) const
        { return first.id() < second.id(); }

    };

    //* name
    const QString& name( void ) const
    { return name_; }

    //* name
    void setName( const QString& name )
    { name_ = name; }

    //* pattern type
    enum Type
    {
        Undefined,
        KeywordPattern,
        RangePattern
    };

    //*type
    const Type& type( void ) const
    { return type_; }

    //*type
    void setType( const Type& type )
    { type_ = type; }

    //* type
    QString typeName( void ) const
    { return typeName( type() ); }

    //* type
    static QString typeName( const Type& type );

    //* parent name
    const QString& parent( void ) const
    { return parent_; }

    //* parent name
    void setParent( const QString& parent )
    { parent_ = parent; }

    //* parent id
    const int& parentId( void ) const
    { return parentId_; }

    //* parent id
    void setParentId( const int& id )
    { parentId_ = id; }

    //* text style
    const HighlightStyle& style( void ) const
    { return style_; }

    //* text style
    void setStyle( const HighlightStyle& style )
    { style_ = style; }

    //* child patterns
    const List& children( void ) const
    { return children_; }

    //* add child
    void addChild( HighlightPattern child )
    { children_ << child; }

    //* clear children
    void clearChildren( void )
    { children_.clear(); }

    //* keyword regexp
    const QRegExp& keyword( void ) const
    { return keyword_; }

    //* keyword
    virtual void setKeyword( const QString& keyword )
    { keyword_.setPattern( keyword ); }

    //* begin regexp
    const QRegExp& begin( void ) const
    { return keyword(); }

    //* keyword
    virtual void setBegin( const QString& keyword )
    { setKeyword( keyword ); }

    //* end regexp
    const QRegExp& end( void ) const
    { return end_; }

    //* range end pattern
    virtual void setEnd( const QString& keyword )
    { end_.setPattern( keyword ); }

    //* comments
    const QString& comments( void ) const
    { return comments_; }

    //* comments
    void setComments( const QString& comments )
    { comments_ = comments; }

    //*@name flags
    //@{

    //* flags
    const Flags& flags( void ) const
    { return flags_; }

    //* flags
    void setFlags( const Flags& flags )
    { flags_ = flags; }

    //* flags
    bool hasFlag( const Flag& flag ) const
    { return flags_ & flag; }

    //* flags
    void setFlag( const Flag& flag, bool value )
    {
        if( value ) flags_ |= flag;
        else flags_ &= (~flag);
    }

    //@}

    //* validity
    bool isValid( void ) const
    {
        switch( type() )
        {
            case KeywordPattern: return keyword_.isValid();
            case RangePattern: return keyword_.isValid() && end_.isValid();
            default: return false;
        }
    }

    //* process text and update the matching locations.
    /*!
    Returns true if at least one match is found.
    Locations and active parameters are changed
    */
    bool processText( PatternLocationSet& locations, const QString& text, bool& active ) const
    {
        switch( type() )
        {
            case KeywordPattern: return _findKeyword( locations, text, active );
            case RangePattern: return _findRange( locations, text, active );
            default: return false;
        }
    }

    //* used to get patterns by name
    class SameNameFTor
    {

        public:

        //* constructor
        SameNameFTor( const QString& name ):
            name_( name )
            {}

        //* predicate
        bool operator() (const HighlightPattern& pattern ) const
        { return pattern.name() == name_; }

        private:

        //* predicate
        const QString name_;

    };

    //* used to pattern by id
    class SameIdFTor
    {

        public:

        //* constructor
        SameIdFTor( const int& id ):
            id_( id )
            {}

        //* predicate
        bool operator() ( const HighlightPattern& pattern ) const
        { return pattern.id() == id_; }

        private:

        //* predicted id
        int id_;

    };

    protected:

    //* find keyword pattern
    bool _findKeyword( PatternLocationSet&, const QString&, bool& ) const;

    //* find range pattern
    bool _findRange( PatternLocationSet&, const QString&, bool& ) const;

    private:

    //* unique id
    /*!
    The unique ID has a single bit set to 1, to use
    faster active pattern masks, with no shift operators
    */
    int id_;

    //* type
    Type type_;

    //* pattern name
    QString name_;

    //* parent pattern name
    QString parent_;

    //* parent pattern id
    int parentId_;

    //* style
    HighlightStyle style_;

    //* child patterns
    List children_;

    //* comments
    QString comments_;

    //* flags
    Flags flags_;

    //*@name patterns
    //@{

    //* keyword regexp (or begin in case of range pattern)
    QRegExp keyword_;

    //* range end regexp
    QRegExp end_;

    //@}

    //*@name dumpers
    //@{
    //* dump
    friend QTextStream& operator << ( QTextStream& out, const HighlightPattern& pattern )
    {
        out << "id: " << pattern.id() << " name: " << pattern.name() << " parent name:" << pattern.parent();
        return out;
    }

    //@}

};

Q_DECLARE_OPERATORS_FOR_FLAGS( HighlightPattern::Flags )

#endif
