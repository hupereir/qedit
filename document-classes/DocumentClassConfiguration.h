#ifndef DocumentClassConfiguration_h
#define DocumentClassConfiguration_h

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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "Counter.h"
#include "DocumentClass.h"

#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

class AnimatedLineEditor;
class BrowsedLineEditor;

//! List box for HighlightPatterns
class DocumentClassConfiguration: public QWidget, public Counter
{

    public:

    //! constructor
    DocumentClassConfiguration( QWidget* parent = 0 );

    //! set document class
    void setDocumentClass( const DocumentClass& document_class );

    //! update document class
    DocumentClass documentClass( void );

    private:

    //! document class
    DocumentClass documentClass_;

    //! name editor
    AnimatedLineEditor* nameEditor_;

    //! icon
    AnimatedLineEditor* iconEditor_;

    //! name editor
    BrowsedLineEditor* fileEditor_;

    //! file pattern editor
    AnimatedLineEditor* filePatternEditor_;

    //! first line pattern editor
    AnimatedLineEditor* firstLinePatternEditor_;

    //! base indentation
    QSpinBox* baseIndentationSpinBox_;

    //! default
    QCheckBox* defaultCheckBox_;

    //! wrap
    QCheckBox* wrapCheckBox_;

    //! wrap
    QCheckBox* tabEmulationCheckbox_;

    //! tab size
    QSpinBox* tabSizeSpinBox_;

};

#endif
