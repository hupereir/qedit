#ifndef _HighlightStyle_h_
#define _HighlightStyle_h_

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

#include "Counter.h"
#include "TextFormat.h"

#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QList>

//! Base class for syntax highlighting
class HighlightStyle: public Counter
{

    public:

    //! constructor
    HighlightStyle(
        const QString& name = "default",
        FORMAT::TextFormatFlags format = FORMAT::Default,
        const QColor& color = Qt::black ):
        Counter( "HighlightStyle" ),
        name_( name ),
        format_( format ),
        color_( color )
    {}

    //! constructor from DomElement
    HighlightStyle( const QDomElement& element );

    //! write to DomElement
    QDomElement domElement( QDomDocument& parent ) const;

    //! name
    virtual const QString& name( void ) const
    { return name_; }

    //! same name ftor
    class SameNameFTor
    {
        public:

        //! constructor
        SameNameFTor( const HighlightStyle& style ):
            name_( style.name() )
            {}

        //! predicate
        bool operator() (const HighlightStyle& other ) const
        { return other.name() == name_; }

        private:

        QString name_;

    };

    //! equal to ftor
    class WeakEqualFTor
    {
        public:

        bool operator()( const HighlightStyle& first, const HighlightStyle& second ) const
        { return first.name() == second.name(); }

    };

    //! less than ftor
    class WeakLessThanFTor
    {
        public:

        bool operator()( const HighlightStyle& first, const HighlightStyle& second ) const
        { return first.name() < second.name(); }

    };

    //! typedef for list of patterns
    typedef QSet<HighlightStyle> Set;

    //! typedef for list of patterns
    typedef QList<HighlightStyle> List;

    //! true if any attributes is different from argument
    /*! this is a stricter comparison than the != operator */
    bool operator == ( const HighlightStyle& style ) const;

    //! name
    virtual void setName( const QString& name )
    { name_ = name; }

    //! format
    virtual FORMAT::TextFormatFlags fontFormat( void ) const
    { return format_; }

    //! format
    virtual void setFontFormat( FORMAT::TextFormatFlags format )
    { format_ = format; }

    //! color
    virtual const QColor& color( void ) const
    { return color_; }

    //! color
    virtual void setColor( const QColor& color )
    { color_ = color; }

    private:

    //! pattern name
    QString name_;

    //! format (bitwise or of TextFormatInfo)
    FORMAT::TextFormatFlags format_;

    //! color
    QColor color_;

};

inline unsigned int qHash( const HighlightStyle& style )
{ return qHash( style.name() ); }

#endif
