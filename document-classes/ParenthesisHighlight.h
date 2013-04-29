#ifndef ParenthesisHighlight_h
#define ParenthesisHighlight_h

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

/*!
\file ParenthesisHighlight.h
\brief handles parenthesis matching highlighting
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <QApplication>
#include <QBasicTimer>
#include <QTextBlock>

#include "Counter.h"
#include "Debug.h"

class TextEditor;

//! handles parenthesis matching highlighting
class ParenthesisHighlight: public QObject, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    ParenthesisHighlight( TextEditor* );

    //! destructor
    virtual ~ParenthesisHighlight( void )
    { Debug::Throw( "ParenthesisHighlight::~ParenthesisHighlight.\n" ); }

    //! enable/disable
    void setEnabled( const bool& value )
    { enabled_ = value; }

    //! enable/disable
    const bool& isEnabled( void ) const
    { return enabled_; }

    //! synchronize
    void synchronize( const ParenthesisHighlight& );

    //! clear highlighted block
    //void clear( void );
    QList<QTextBlock> clear( void );

    //! highlight current (absolute) location
    void highlight( const int&, const int& );

    protected:

    //! highlight
    void _highlight( void );

    private:

    //! parent editor
    TextEditor* parent_;

    //! associated timer
    QBasicTimer timer_;

    //! true if enabled
    bool enabled_;

    //! parenthesis location
    int location_;

    //! length
    int length_;

    //! true when cleared
    bool cleared_;

};

#endif
