#ifndef IndentPatternType_h
#define IndentPatternType_h

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
#include "IndentPattern.h"

#include <QtGui/QComboBox>
#include <QtGui/QWidget>

//! highlight pattern type selection
class IndentPatternType: public QComboBox, public Counter
{

  //! Qt object
  Q_OBJECT

  public:

  //! constructor
  IndentPatternType( QWidget* parent );

  //! set checkboxes from from
  void setType( const IndentPattern::Type& type );

  //! get format
  IndentPattern::Type type( void ) const;

  signals:

  //! highlight pattern type changed
  void typeChanged( IndentPattern::Type );

  private slots:

  //! checkbox clicked
  void _typeChanged( const QString& button );

};
#endif
