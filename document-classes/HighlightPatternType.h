#ifndef HighlightPatternType_h
#define HighlightPatternType_h

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


/*!
   \file HighlightPatternType.h
   \brief highlight pattern type selection
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QCheckBox>
#include <QWidget>
#include <map>

#include "Counter.h"
#include "HighlightPattern.h"

//! highlight pattern type selection
class HighlightPatternType: public QWidget, public Counter
{

  //! Qt object 
  Q_OBJECT
  
  public:

  //! constructor
  HighlightPatternType( QWidget* parent );

  //! set checkboxes from from
  void setType( const HighlightPattern::Type& type );

  //! get format
  HighlightPattern::Type type( void ) const;

  signals:
  
  //! highlight pattern type changed
  void typeChanged( HighlightPattern::Type );
  
  private slots:
  
  //! checkbox clicked
  void _typeChanged( QAbstractButton* button );
  
  private:

  //! map checkboxes and highlight pattern type
  typedef std::map<QAbstractButton*, HighlightPattern::Type> CheckBoxMap;
  
  //! map checkboxes and highlight pattern type
  CheckBoxMap checkboxes_;
    
};
#endif
