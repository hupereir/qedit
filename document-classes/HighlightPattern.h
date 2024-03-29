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

#include "Counter.h"
#include "Debug.h"
#include "Functors.h"
#include "HighlightStyle.h"

#include <QDomElement>
#include <QDomDocument>
#include <QRegularExpression>
#include <QString>
#include <QList>

class PatternLocationSet;

//* Base class for syntax highlighting
class HighlightPattern final: private Base::Counter<HighlightPattern>
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

    //* default constructor
    explicit HighlightPattern();

    //* constructor from DomElement
    explicit HighlightPattern( const QDomElement& );

    //*@name accessors
    //@{

    //* dom element
    QDomElement domElement( QDomDocument& parent ) const;

    //* unique id
    int id() const
    { return id_; }

    //* name
    const QString& name() const
    { return name_; }

    //* pattern type
    enum class Type
    {
        Undefined,
        KeywordPattern,
        RangePattern
    };

    //* type
    Type type() const
    { return type_; }

    //* type
    QString typeName() const
    { return typeName( type_ ); }

    //* type
    static QString typeName( Type type );

    //* parent name
    const QString& parent() const
    { return parent_; }

    //* parent id
    int parentId() const
    { return parentId_; }

    //* text style
    const HighlightStyle& style() const
    { return style_; }

    //* child patterns
    const List& children() const
    { return children_; }

    //* keyword regexp
    const QRegularExpression& keyword() const
    { return keyword_; }

    //* begin regexp
    const QRegularExpression& begin() const
    { return keyword_; }

    //* end regexp
    const QRegularExpression& end() const
    { return end_; }

    //* flags
    const Flags& flags() const
    { return flags_; }

    //* flags
    bool hasFlag( HighlightPattern::Flag flag ) const
    { return flags_ & flag; }

    //* validity
    bool isValid() const;

    //@}

    //*@name modifiers
    //@{

    //* set id
    /**
    The unique ID has a single bit set to 1, to use
    faster active pattern masks, with no shift operators
    */
    void setId( int id )
    { id_ = (1<<id); }

    //* name
    void setName( const QString& name )
    { name_ = name; }

    //*type
    void setType( Type type )
    { type_ = type; }

    //* parent style name
    void setParent( const QString &parent )
    { parent_ = parent; }

    //* parent id
    void setParentId( int id )
    { parentId_ = id; }

    //* text style
    void setStyle( const HighlightStyle& style )
    { style_ = style; }

    //* add child
    void addChild( const HighlightPattern &child )
    { children_ << child; }

    //* clear children
    void clearChildren()
    { children_.clear(); }

    //* keyword
    void setKeyword( const QString& value )
    { 
        keyword_.setPattern( value ); 
        _updatePatternOptions( keyword_ );
    }

    //* keyword
    void setBegin( const QString& value )
    { setKeyword( value ); }

    //* range end pattern
    void setEnd( const QString& value )
    { 
        end_.setPattern( value ); 
        _updatePatternOptions( end_ );
    }
    
    //* keyword
    void setKeyword( const QRegularExpression& value )
    {
        keyword_ = value; 
        _updatePatternOptions( keyword_ );
    }
    
    //* begin
    void setBegin( const QRegularExpression& value )
    { setKeyword( value ); }
   
    //* end
    void setEnd( const QRegularExpression& value )
    {
        end_ = value;
        _updatePatternOptions( end_ );
    }

    //* flags
    void setFlags( HighlightPattern::Flags flags )
    { 
        flags_ = flags; 
        _updatePatternOptions( keyword_ );
        _updatePatternOptions( end_ );
    }

    //* flags
    void setFlag( HighlightPattern::Flag flag, bool value )
    {
        if( value ) flags_ |= flag;
        else flags_ &= (~flag);
        _updatePatternOptions( keyword_ );
        _updatePatternOptions( end_ );
    }

    //* process text and update the matching locations.
    /**
    Returns true if at least one match is found.
    Locations and active parameters are changed
    */
    bool processText( PatternLocationSet& locations, const QString& text, bool& active ) const
    {
        switch( type_ )
        {
            case Type::KeywordPattern: return _findKeyword( locations, text, active );
            case Type::RangePattern: return _findRange( locations, text, active );
            default: return false;
        }
    }

    //@}

    //* used to get patterns by name
    using SameNameFTor = Base::Functor::Unary<HighlightPattern, const QString&, &HighlightPattern::name>;

    //* used to pattern by id
    using SameIdFTor = Base::Functor::Unary<HighlightPattern, int, &HighlightPattern::id>;

    private:

    //* update pattern options for provided regular expression
    /** explicitly, implements caseinsensitivity */
    void _updatePatternOptions( QRegularExpression& ) const;
    
    //* find keyword pattern
    bool _findKeyword( PatternLocationSet&, const QString&, bool& ) const;

    //* find range pattern
    bool _findRange( PatternLocationSet&, const QString&, bool& ) const;

    //* unique id
    /**
    The unique ID has a single bit set to 1, to use
    faster active pattern masks, with no shift operators
    */
    int id_ = 0;

    //* type
    Type type_ = Type::Undefined;

    //* pattern name
    QString name_;

    //* parent pattern name
    QString parent_;

    //* parent pattern id
    int parentId_ = 0;

    //* style
    HighlightStyle style_;

    //* child patterns
    List children_;

    //* flags
    Flags flags_ = None;

    //*@name patterns
    //@{

    //* keyword regexp (or begin in case of range pattern)
    QRegularExpression keyword_;

    //* range end regexp
    QRegularExpression end_;

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

    //* equal to operator
    friend bool operator == (const HighlightPattern&, const HighlightPattern& );

};

Q_DECLARE_OPERATORS_FOR_FLAGS( HighlightPattern::Flags )

#endif
