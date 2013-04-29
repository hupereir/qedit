#ifndef CollapsedBlockData_h
#define CollapsedBlockData_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "TextBlockDelimiter.h"

#include <QTextBlock>
#include <QString>
#include <QList>

//! store collapsed block text and state
class CollapsedBlockData
{

    public:

    //! list
    typedef QList<CollapsedBlockData> List;

    //! constructor
    CollapsedBlockData( void ):
        collapsed_( false )
    {}

    //! constructor
    CollapsedBlockData( const QTextBlock& block );

    //! text
    const QString& text( void ) const
    { return text_; }

    //! collapsed
    const bool& collapsed( void ) const
    { return collapsed_; }

    //! number of blocks stored by this data object
    unsigned int blockCount( void ) const;

    //!@name block limits
    //@{

    //! delimiters
    const TextBlock::Delimiter::List& delimiters( void ) const
    { return delimiters_; }

    //! delimiters
    void setDelimiters( const TextBlock::Delimiter::List& delimiters )
    { delimiters_ = delimiters; }

    //@}

    //! children
    const List& children( void ) const
    { return children_; }

    //! children
    List& children( void )
    { return children_; }

    //! children
    void setChildren( const List& children )
    { children_ = children; }

    //! returns all text contained in collapsed data
    /*!
    this is equivalent to expanding the entire block.
    The method is recursive
    */
    QString toPlainText( void ) const;

    private:

    //! text
    QString text_;

    //! collapsed flag
    bool collapsed_;

    //! collapsed delimiters
    TextBlock::Delimiter::List delimiters_;

    //! children
    List children_;

};

Q_DECLARE_METATYPE( CollapsedBlockData )

#endif
