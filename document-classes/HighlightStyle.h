#ifndef HighlightStyle_h
#define HighlightStyle_h

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
#include "TextFormat.h"

#include <QColor>
#include <QFont>
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QSet>
#include <QList>

//* Base class for syntax highlighting
class HighlightStyle: private Base::Counter<HighlightStyle>
{

    public:

    //* constructor
    explicit HighlightStyle(
        const QString& name = "default",
        Format::TextFormatFlags format = Format::Default,
        const QColor& color = QColor() ):
        Counter( "HighlightStyle" ),
        name_( name ),
        format_( format ),
        color_( color )
    {}

    //* constructor from DomElement
    explicit HighlightStyle( const QDomElement& );

    //* write to DomElement
    QDomElement domElement( QDomDocument& parent ) const;

    //* name
    virtual const QString& name( void ) const
    { return name_; }

    //* same name ftor
    class SameNameFTor
    {
        public:

        //* constructor
        explicit SameNameFTor( const HighlightStyle& style ):
            name_( style.name_ )
            {}

        //* predicate
        bool operator() (const HighlightStyle& other ) const
        { return other.name_ == name_; }

        private:

        QString name_;

    };

    //* equal to ftor
    class WeakEqualFTor
    {
        public:

        bool operator()( const HighlightStyle& first, const HighlightStyle& second ) const
        { return first.name_ == second.name_; }

    };

    //* less than ftor
    class WeakLessThanFTor
    {
        public:

        bool operator()( const HighlightStyle& first, const HighlightStyle& second ) const
        { return first.name_ < second.name_; }

    };

    //* typedef for list of patterns
    using Set = QSet<HighlightStyle>;

    //* typedef for list of patterns
    using List = QList<HighlightStyle>;

    //* true if any attributes is different from argument
    /** this is a stricter comparison than the != operator */
    bool operator == ( const HighlightStyle& style ) const;

    //* name
    virtual void setName( const QString& name )
    { name_ = name; }

    //* format
    virtual Format::TextFormatFlags fontFormat( void ) const
    { return format_; }

    //* format
    virtual void setFontFormat( Format::TextFormatFlags format )
    { format_ = format; }

    //* color
    virtual const QColor& color( void ) const
    { return color_; }

    //* color
    virtual void setColor( const QColor& color )
    { color_ = color; }

    private:

    //* pattern name
    QString name_;

    //* format (bitwise or of TextFormatInfo)
    Format::TextFormatFlags format_ = Format::Default;

    //* color
    QColor color_;

};

inline uint qHash( const HighlightStyle& style )
{ return qHash( style.name() ); }

#endif
