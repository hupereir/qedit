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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software , Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file CollapsedBlockData.h
  \brief stores collapsed block informations
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QString>
#include <QTextBlock>
#include <vector>

#include "Counter.h"

class CollapsedBlockData: public Counter
{
  
  public:
  
  //! list
  typedef std::vector<CollapsedBlockData> List;
  
  //! constructor
  CollapsedBlockData( const QTextBlock& block );
    
  //! text
  const QString& text( void ) const
  { return text_; }
    
  //! children
  const List& children( void ) const
  { return children_; }
    
  //! returns all text contained in collapsed data
  /*!
  this is equivalent to expanding the entire block.
  The method is recursive
  */
  QString toPlainText( void ) const;
  
  private:
    
  //! text
  QString text_;
  
  //! children
  List children_;
    
};

#endif
