#ifndef _IndentPatternDialog_h_
#define _IndentPatternDialog_h_

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

#include "CustomDialog.h"
#include "IndentPattern.h"

#include <QtGui/QSpinBox>

class AnimatedLineEditor;
class IndentPatternType;
class IndentPatternRuleList;
class TextEditor;

//! indentation pattern editing dialog
class IndentPatternDialog: public CustomDialog
{

    public:

    //! constructor
    IndentPatternDialog( QWidget* parent );

    //! destructor
    virtual ~IndentPatternDialog( void )
    {}

    //! pattern
    void setPattern( const IndentPattern& pattern );

    //! pattern
    IndentPattern pattern( void );

    private:

    //! initial pattern (to avoid duplication of ids)
    IndentPattern pattern_;

    //! name editor
    AnimatedLineEditor* nameEditor_;

    //! type
    IndentPatternType* patternType_;

    //! scale
    QSpinBox* scaleSpinBox_;

    //! comments
    TextEditor* comments_;

    //! rules
    IndentPatternRuleList* list_;

};

#endif
