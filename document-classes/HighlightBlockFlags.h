#ifndef HighlightBlockFlags_h
#define HighlightBlockFlags_h

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

#include "TextBlockFlags.h"

#include <QTextFormat>

namespace TextBlock
{

    enum Flag
    {

        //! block is modified
        MODIFIED = 1<<2,

        //! block is collapsed
        COLLAPSED = 1<<3,

        //! block is a diff-wise added block
        DIFF_ADDED = 1<<4,

        //! block is a diff-wise conflict block
        DIFF_CONFLICT = 1<<5,

        //! user tagged blocks
        USER_TAG = 1<<6,

        //! all tags
        ALL_TAGS = DIFF_ADDED | DIFF_CONFLICT | USER_TAG

    };

    Q_DECLARE_FLAGS( Flags, Flag );

    //! block format properties
    enum BlockFormatProperties
    {

        Collapsed = QTextFormat::UserProperty | (1<<0),
        CollapsedData = QTextFormat::UserProperty | (1<<1)

    };

};

Q_DECLARE_OPERATORS_FOR_FLAGS( TextBlock::Flags );

#endif
