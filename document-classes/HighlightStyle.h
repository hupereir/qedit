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
#include "Functors.h"
#include "TextFormat.h"

#include <QColor>
#include <QFont>
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QSet>
#include <QList>

//* Base class for syntax highlighting
class HighlightStyle final: private Base::Counter<HighlightStyle>
{

    public:

    //* constructor
    explicit HighlightStyle(
        const QString& name = QStringLiteral("default"),
        TextFormat::Flags format = TextFormat::Default,
        const QColor& color = QColor() ):
        Counter( QStringLiteral("HighlightStyle") ),
        name_( name ),
        format_( format ),
        color_( color )
    {}

    //* constructor from DomElement
    explicit HighlightStyle( const QDomElement& );

    //!@name accessors
    //@{

    //* write to DomElement
    QDomElement domElement( QDomDocument& parent ) const;

    //* name
    const QString& name() const
    { return name_; }

    //* format
    TextFormat::Flags fontFormat() const
    { return format_; }

    //* background color
    const QColor& backgroundColor() const
    { return backgroundColor_; }

    //* color
    const QColor& color() const
    { return color_; }

    //@}

    //!@name modifiers
    //@{

    //* name
    void setName( const QString& name )
    { name_ = name; }

    //* format
    void setFontFormat( TextFormat::Flags format )
    { format_ = format; }

    //* color
    void setBackgroundColor( const QColor& color )
    { backgroundColor_ = color; }

    //* color
    void setColor( const QColor& color )
    { color_ = color; }

    //@}

    //* same name ftor
    using SameNameFTor = Base::Functor::Unary<HighlightStyle, const QString&, &HighlightStyle::name>;

    //* equal to ftor
    using WeakEqualFTor = Base::Functor::BinaryEqual<HighlightStyle, const QString&, &HighlightStyle::name>;

    //* less than ftor
    using WeakLessThanFTor = Base::Functor::BinaryLess<HighlightStyle, const QString&, &HighlightStyle::name>;

    //* typedef for list of patterns
    using Set = QSet<HighlightStyle>;

    //* typedef for list of patterns
    using List = QList<HighlightStyle>;

    private:

    //* pattern name
    QString name_;

    //* format (bitwise or of TextFormatInfo)
    TextFormat::Flags format_ = TextFormat::Default;

    //* background color
    QColor backgroundColor_;

    //* color
    QColor color_;

    //* true if any attributes is different from argument
    friend bool operator == ( const HighlightStyle&, const HighlightStyle& );

};

inline uint qHash( const HighlightStyle& style )
{ return qHash( style.name() ); }

#endif
